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
/** NetX Component                                                        */ 
/**                                                                       */
/**   Multicast Domain Name System (mDNS)                                 */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define NX_MDNS_SOURCE_CODE


/* Force error checking to be disabled in this module */
#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif /* NX_DISABLE_ERROR_CHECKING  */


/* Include necessary system files.  */
#include    "nxd_mdns.h"
#include    "tx_thread.h"

/* Define the application protocol.  */
static CHAR         *nx_mdns_app_protocol[]= {"_tcp", "_udp", NX_NULL};

/* Define the service type.  */
#ifndef NX_MDNS_DISABLE_CLIENT
static CHAR         *nx_mdns_service_types[] =
{
    "_device-info",
    "_http",
    "_https",
    "_guid",
    "_h323",
    "_ntp",
    "_objective",
    "_rdp",
    "_remote",
    "_rtsp",
    "_sip",
    "_smb",
    "_soap",
    "_ssh",
    "_telnet",
    "_tftp",
    "_xmpp-client",
    NX_NULL
};
#endif /* NX_MDNS_DISABLE_CLIENT  */


/* Define the mDNS internal Function.  */
static VOID         _nx_mdns_udp_receive_notify(NX_UDP_SOCKET *socket_ptr);
static VOID         _nx_mdns_timer_entry(ULONG mdns_value);
static VOID         _nx_mdns_timer_set(NX_MDNS *mdns_ptr, NX_MDNS_RR  *record_rr, ULONG timer_count);
static VOID         _nx_mdns_timer_event_process(NX_MDNS *mdns_ptr);
static VOID         _nx_mdns_thread_entry(ULONG mdns_value);
static UINT         _nx_mdns_packet_process(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UINT interface_index);
static UINT         _nx_mdns_packet_create(NX_MDNS *mdns_ptr, NX_PACKET **packet_ptr, UCHAR type);
static VOID         _nx_mdns_packet_send(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UINT interface_index);
static UINT         _nx_mdns_packet_rr_add(NX_PACKET *packet_ptr, NX_MDNS_RR *rr, UINT op, UINT packet_type); 
static UINT         _nx_mdns_packet_rr_set(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, NX_MDNS_RR *rr_ptr, UINT op, UINT interface_index);
static UINT         _nx_mdns_packet_rr_data_set(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, NX_MDNS_RR *rr_ptr, UINT op);
#ifdef NX_MDNS_ENABLE_ADDRESS_CHECK
static UINT         _nx_mdns_packet_address_check(NX_PACKET *packet_ptr);
#endif /* NX_MDNS_ENABLE_ADDRESS_CHECK  */
static UINT         _nx_mdns_service_name_assemble(UCHAR *name, UCHAR *type, UCHAR *sub_type, UCHAR *domain, UCHAR *record_buffer, UINT buffer_size, UINT *type_index);
static UINT         _nx_mdns_service_name_resolve(UCHAR *srv_name, UCHAR **name, UCHAR **type, UCHAR **domain);
static UINT         _nx_mdns_rr_delete(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_rr); 
static UINT         _nx_mdns_rr_size_get(UCHAR *resource, NX_PACKET *packet_ptr);
static UINT         _nx_mdns_name_match(UCHAR *src, UCHAR *dst, UINT length);  
static UINT         _nx_mdns_name_size_calculate(UCHAR *name, NX_PACKET *packet_ptr);
static UINT         _nx_mdns_name_string_encode(UCHAR *ptr, UCHAR *name);
static UINT         _nx_mdns_name_string_decode(UCHAR *data, UINT start, UINT data_length, UCHAR *buffer, UINT size); 
static UINT         _nx_mdns_txt_string_encode(UCHAR *ptr, UCHAR *name);
static UINT         _nx_mdns_txt_string_decode(UCHAR *data, UINT data_length, UCHAR *buffer, UINT size);
static VOID         _nx_mdns_short_to_network_convert(UCHAR *ptr, USHORT value);
static VOID         _nx_mdns_long_to_network_convert(UCHAR *ptr, ULONG value);

#ifndef NX_MDNS_DISABLE_SERVER
static VOID         _nx_mdns_address_change_process(NX_MDNS *mdns_ptr);
static UINT         _nx_mdns_host_name_register(NX_MDNS *mdns_ptr, UCHAR type, UINT interface_index);
static UINT         _nx_mdns_service_interface_delete(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UINT interface_index);
#if !defined NX_DISABLE_IPV4 || defined NX_MDNS_ENABLE_IPV6
static UINT         _nx_mdns_rr_a_aaaa_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG *address, UINT addr_length, UCHAR type, UINT interface_index);
#endif /* !NX_DISABLE_IPV4 || NX_MDNS_ENABLE_IPV6  */
static UINT         _nx_mdns_rr_srv_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG ttl, UCHAR set, USHORT priority, USHORT weights, USHORT port, UCHAR *target, NX_MDNS_RR **insert_rr, UINT interface_index);
static UINT         _nx_mdns_rr_txt_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG ttl, UCHAR set, UCHAR *txt, NX_MDNS_RR **insert_rr, UINT interface_index); 
static UINT         _nx_mdns_rr_ptr_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG ttl, UCHAR set, UCHAR *ptr_name, UCHAR is_valid, NX_MDNS_RR **insert_rr, UINT interface_index);

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
static UINT         _nx_mdns_rr_nsec_add(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR add_a, UCHAR add_aaaa, UCHAR type, UINT interface_index);
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES */

static UINT         _nx_mdns_rr_parameter_set(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, ULONG ttl, UINT rdata_length, UCHAR set, UCHAR is_register, UCHAR is_valid, NX_MDNS_RR *rr_record, UINT interface_index);
static UINT         _nx_mdns_conflict_process(NX_MDNS *mdns_ptr, NX_MDNS_RR  *record_rr);
static UINT         _nx_mdns_additional_resource_record_find(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_ptr);
static VOID         _nx_mdns_additional_a_aaaa_find(NX_MDNS *mdns_ptr, UCHAR *name, UINT interface_index);
static VOID         _nx_mdns_probing_send(NX_MDNS *mdns_ptr, UINT interface_index);
static VOID         _nx_mdns_announcing_send(NX_MDNS *mdns_ptr, UINT interface_index);
static VOID         _nx_mdns_response_send(NX_MDNS *mdns_ptr, UINT interface_index);
#ifndef NX_DISABLE_IPV4
static VOID         _nx_mdns_ip_address_change_notify(NX_IP *ip_ptr, VOID *additional_info);
#endif /* NX_DISABLE_IPV4 */
#ifdef NX_MDNS_ENABLE_IPV6
static VOID         _nx_mdns_ipv6_address_change_notify(NX_IP *ip_ptr, UINT method, UINT interface_index, UINT index, ULONG *ipv6_address);
#endif /* NX_MDNS_ENABLE_IPV6  */
#endif /* NX_MDNS_DISABLE_SERVER  */

#ifndef NX_MDNS_DISABLE_CLIENT
static VOID         _nx_mdns_service_change_notify_process(NX_MDNS *mdns_ptr, NX_MDNS_RR *new_rr, UCHAR is_present);
static UINT         _nx_mdns_service_addition_info_get(NX_MDNS *mdns_ptr, UCHAR *srv_name, NX_MDNS_SERVICE *service, UINT interface_index);
static UINT         _nx_mdns_service_mask_match(NX_MDNS *mdns_ptr, UCHAR *service_type, ULONG service_mask);
static UINT         _nx_mdns_one_shot_query(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, NX_MDNS_RR **out_rr, ULONG wait_option, UINT interface_index);
static UINT         _nx_mdns_continuous_query(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, UINT interface_index);
static VOID         _nx_mdns_query_send(NX_MDNS *mdns_ptr, UINT interface_index);
static UINT         _nx_mdns_query_check(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, UINT one_shot, NX_MDNS_RR **search_rr, UINT interface_index);
static VOID         _nx_mdns_query_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
static VOID         _nx_mdns_query_thread_suspend(TX_THREAD **suspension_list_head, VOID (*suspend_cleanup)(TX_THREAD * NX_CLEANUP_PARAMETER),
                                                  NX_MDNS *mdns_ptr, NX_MDNS_RR **rr, ULONG wait_option);
static VOID         _nx_mdns_query_thread_resume(TX_THREAD **suspension_list_head, NX_MDNS *mdns_ptr, NX_MDNS_RR *rr);
static UINT         _nx_mdns_known_answer_find(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_ptr); 
static UINT         _nx_mdns_packet_rr_process(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UINT interface_index);
#endif /* NX_MDNS_DISABLE_CLIENT  */

UINT         _nx_mdns_cache_initialize(NX_MDNS *mdns_ptr, VOID *local_cache_ptr, UINT local_cache_size, VOID *peer_cache_ptr, UINT peer_cache_size);
UINT         _nx_mdns_cache_add_resource_record(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_ptr, NX_MDNS_RR **insert_ptr, UCHAR *is_present);
UINT         _nx_mdns_cache_delete_resource_record(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_ptr);
UINT         _nx_mdns_cache_find_resource_record(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_ptr, UINT match_type, NX_MDNS_RR **search_result);
UINT         _nx_mdns_cache_add_string(NX_MDNS *mdns_ptr, UINT cache_type, VOID *string_ptr, UINT string_len, VOID **insert_ptr, UCHAR find_string, UCHAR add_name);
UINT         _nx_mdns_cache_delete_string(NX_MDNS *mdns_ptr, UINT cache_type, VOID *string_ptr, UINT string_len);
VOID         _nx_mdns_cache_delete_rr_string(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_rr);


/* Define the mDNS STRUCT.  */
static struct       NX_MDNS_STRUCT  *_nx_mdns_created_ptr;
static CHAR         _nx_mdns_dns_sd[NX_MDNS_DNS_SD_MAX + 1] = "_services._dns-sd._udp.local";
static UCHAR        temp_string_buffer[NX_MDNS_NAME_MAX + 1];
static UCHAR        target_string_buffer[NX_MDNS_NAME_MAX + 1];

#ifdef NX_MDNS_ENABLE_IPV6
static NXD_ADDRESS  NX_MDNS_IPV6_MULTICAST_ADDRESS;
#endif /* NX_MDNS_ENABLE_IPV6 */

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_create                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS create function call    */ 
/*                                                                        */ 
/*    Note: the name string is generated by internal logic and it is      */
/*    always NULL-terminated.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    priority                              The priority of mDNS Thread   */
/*    stack_ptr                             Stack pointer for mDNS Thread */ 
/*    stack_size                            Stack size for mDNS Thread    */
/*    host_name                             Pointer to host name          */
/*    local_cache_ptr                       Pointer to local cache        */ 
/*    local_cache_size                      The size of local cache       */ 
/*    peer_cache_ptr                        Pointer to peer cache         */ 
/*    peer_cache_size                       The size of peer cache        */ 
/*    probing_notify                        mDNS probing notify           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_create                       Actual mDNS create function   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_mdns_create(NX_MDNS *mdns_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool,
                       UINT priority, VOID *stack_ptr, ULONG stack_size, UCHAR *host_name,
                       VOID *local_cache_ptr, UINT local_cache_size,
                       VOID *peer_cache_ptr, UINT peer_cache_size,
                       VOID (*probing_notify)(NX_MDNS *mdns_ptr, UCHAR *name, UINT probing_state))
{

UINT    status;
UCHAR   *ptr;


    /* Check for invalid input pointers.  */
    if ((!ip_ptr) || (!mdns_ptr) || (!stack_ptr) || (!packet_pool) || (!host_name))
    {    
        return(NX_PTR_ERROR);
    }    

    /* Check for invalid non pointer input. */
    if ((ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (stack_size < TX_MINIMUM_STACK) ||
        (mdns_ptr -> nx_mdns_id == NX_MDNS_ID))
    {
        return(NX_MDNS_PARAM_ERROR);
    }    

    /* Check the host name format, RFC 1033,1034,1035 recomend that host names contain only letters, digits, and hyphens RFC6763, Appendix E, Page44. */
    ptr = host_name;
    while (*ptr != '\0')
    {
        if (((*ptr >= 'a') && (*ptr <= 'z')) ||
            ((*ptr >= 'A') && (*ptr <= 'Z')) ||
            ((*ptr >= '0') && (*ptr <= '9')) ||
            ((*ptr == '-')))
            ptr++;
        else
            return(NX_MDNS_HOST_NAME_ERROR);
    }

    /* Check the default domain name size.  */
    if ((sizeof("local") - 1) > NX_MDNS_DOMAIN_NAME_MAX)
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }    
    
#ifndef NX_MDNS_DISABLE_SERVER
    
    /* Check for invalid input pointers, cache header and tail (8 bytes).  */
    if ((!local_cache_ptr) || (local_cache_size < 8))
    {    
        return(NX_PTR_ERROR);
    } 
    
    /* Make sure record_buffer is 4-byte aligned. */
    if ((((UINT)local_cache_ptr & 0x3) != 0) ||
        ((local_cache_size & 0x3) != 0))
    {
        return(NX_MDNS_CACHE_ERROR);
    }
#endif /* NX_MDNS_DISABLE_SERVER */

#ifndef NX_MDNS_DISABLE_CLIENT

    /* Check for invalid input pointers, cache header and tail (8 bytes).  */
    if ((!peer_cache_ptr) || (peer_cache_size < 8))
    {    
        return(NX_PTR_ERROR);
    } 
    
    /* Make sure peer cache is 4-byte aligned. */
    if ((((UINT)peer_cache_ptr & 0x3) != 0) ||
        ((peer_cache_size & 0x3) != 0))
    {
        return(NX_MDNS_CACHE_ERROR);
    }
#endif /* NX_MDNS_DISABLE_CLIENT */
    
    /* Call actual mDNS create service.  */
    status =  _nx_mdns_create(mdns_ptr, ip_ptr, packet_pool, priority,
                              stack_ptr, stack_size, host_name,
                              local_cache_ptr, local_cache_size,
                              peer_cache_ptr, peer_cache_size,
                              probing_notify);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_create                                     PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the mDNS structure and associated IP      */ 
/*    instance for mDNS operation. In doing so, it creates a UDP socket   */ 
/*    for communication with the server and a mDNS processing thread      */
/*                                                                        */ 
/*    Note: the name string is generated by internal logic and it is      */
/*    always NULL-terminated.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    priority                              The priority of mDNS Thread   */
/*    stack_ptr                             Stack pointer for mDNS Thread */ 
/*    stack_size                            Stack size for mDNS Thread    */
/*    host_name                             Pointer to host name          */
/*    local_cache_ptr                       Pointer to local cache        */ 
/*    local_cache_size                      The size of local cache       */ 
/*    peer_cache_ptr                        Pointer to peer cache         */ 
/*    peer_cache_size                       The size of peer cache        */ 
/*    probing_notify                        mDNS probing notify           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_create                  Create the mDNS UDP socket    */ 
/*    nx_udp_socket_delete                  Delete the mDNS UDP socket    */ 
/*    nx_udp_socket_bind                    Bind the mDNS UDP socket      */
/*    nx_udp_socket_unbind                  Unbind the mDNS UDP socket    */
/*    nx_udp_socket_receive_notify          Register the mDNS function    */
/*    nx_ip_address_change_notify           Register IPv4 address function*/
/*    nxd_ipv6_address_change_notify        Register IPv6 address function*/
/*    tx_mutex_create                       Create the mDNS mutex         */ 
/*    tx_mutex_delete                       Delete the mDNS mutex         */ 
/*    tx_thread_create                      Create the mDNS thread        */ 
/*    tx_thread_delete                      Delete the mDNS thread        */  
/*    tx_event_flags_create                 Create the ThreadX flag event */
/*    _nx_mdns_cache_initialize             Initialize the mDNS cache     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            used internal ip address    */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_mdns_create(NX_MDNS *mdns_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool,
                      UINT priority, VOID *stack_ptr, ULONG stack_size, UCHAR *host_name,
                      VOID *local_cache_ptr, UINT local_cache_size,
                      VOID *peer_cache_ptr, UINT peer_cache_size,
                      VOID (*probing_notify)(NX_MDNS *mdns_ptr, UCHAR *name, UINT probing_state))
{

UINT    status;
UINT    host_name_size;


    /* Check the size of the host name string. The last four characters in the 
       host_name are reserved so DNS-SD is able to append " (2)" during the conflict 
       resolution process. */
#if (NX_MDNS_HOST_NAME_MAX >= NX_MDNS_LABEL_MAX)
    if (_nx_utility_string_length_check((CHAR *)host_name, &host_name_size, (NX_MDNS_LABEL_MAX - 4)))
#else
    if (_nx_utility_string_length_check((CHAR *)host_name, &host_name_size, (NX_MDNS_HOST_NAME_MAX - 4)))
#endif
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Initialize the mDNS control block to zero.  */
    memset((void *) mdns_ptr, 0, sizeof(NX_MDNS));

#ifdef NX_MDNS_ENABLE_IPV6

    /* Initialize the IPv6 Multicast address.  */
    memset(&NX_MDNS_IPV6_MULTICAST_ADDRESS, 0, sizeof(NXD_ADDRESS));
    NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_version = NX_IP_VERSION_V6;
    NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[0] = 0xFF020000;  
    NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[1] = 0x00000000;
    NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[2] = 0x00000000;
    NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[3] = 0x000000FB;    
#endif /* NX_MDNS_ENABLE_IPV6 */

    /* Save the IP pointer.  */
    mdns_ptr -> nx_mdns_ip_ptr = ip_ptr;

    /* Save the Packet pool.  */
    mdns_ptr -> nx_mdns_packet_pool_ptr = packet_pool;
        
    /* Save the host name.  */  
    memcpy((char*)mdns_ptr -> nx_mdns_host_name, (const char*)host_name, (host_name_size + 1)); /* Use case of memcpy is verified. */
    
    /* Set the domain name as "local" by default.  */
    memcpy((char*)mdns_ptr -> nx_mdns_domain_name, "local", sizeof("local")); /* Use case of memcpy is verified. */

    /* Assign the resource record change notify. */
    mdns_ptr -> nx_mdns_probing_notify = probing_notify;
        
    /* Set the mDNS announcing period.  */
    mdns_ptr -> nx_mdns_announcing_period = (USHORT)NX_MDNS_ANNOUNCING_PERIOD;

    /* Set the mDNS announcing retransmission interval.  */
    mdns_ptr -> nx_mdns_announcing_retrans_interval = (USHORT)NX_MDNS_ANNOUNCING_RETRANS_INTERVAL;

    /* Set the mDNS announcing period interval.  */
    mdns_ptr -> nx_mdns_announcing_period_interval = (ULONG)NX_MDNS_ANNOUNCING_PERIOD_INTERVAL;
    
    /* Set the mDNS announcing count between one announcing period.  */
    mdns_ptr -> nx_mdns_announcing_count = (UCHAR)NX_MDNS_ANNOUNCING_COUNT;

    /* Set the mDNS announcing factor.  */
    mdns_ptr -> nx_mdns_announcing_factor = (UCHAR)NX_MDNS_ANNOUNCING_FACTOR;
      
    /* Set the mDNS announcing max time.  */
    mdns_ptr -> nx_mdns_announcing_max_time = (UCHAR)NX_MDNS_ANNOUNCING_MAX_TIME;

    /* Set the pointer of global variable mDNS.  */
    _nx_mdns_created_ptr = mdns_ptr;

#ifndef NX_MDNS_DISABLE_SERVER

#ifndef NX_DISABLE_IPV4
    /* Setup the IP address change callback function. */
    ip_ptr -> nx_ip_address_change_notify_internal = _nx_mdns_ip_address_change_notify;
#endif /* NX_DISABLE_IPV4 */

#ifdef NX_MDNS_ENABLE_IPV6

    /* Setup the IPv6 address change callback function. */
    ip_ptr -> nx_ipv6_address_change_notify_internal =  _nx_mdns_ipv6_address_change_notify;
#endif /* NX_MDNS_ENABLE_IPV6  */
#endif /* NX_MDNS_DISABLE_SERVER */

    /* Create the Socket and check the status */
    status = nx_udp_socket_create(mdns_ptr -> nx_mdns_ip_ptr, &(mdns_ptr -> nx_mdns_socket), "Multicast DNS",
                                  NX_MDNS_UDP_TYPE_OF_SERVICE, NX_MDNS_UDP_FRAGMENT_OPTION, 
                                  NX_MDNS_UDP_TIME_TO_LIVE, NX_MDNS_UDP_QUEUE_DEPTH);

    /* Determine if it was successful.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }
    
    /* Bind the UDP socket to the mDNS port.  */
    status =  nx_udp_socket_bind(&(mdns_ptr -> nx_mdns_socket), NX_MDNS_UDP_PORT, TX_NO_WAIT);
    
    /* Check for error */
    if (status)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));
        return(status);
    }
    
    /* Register UDP receive callback. */
    status = nx_udp_socket_receive_notify(&(mdns_ptr -> nx_mdns_socket), _nx_mdns_udp_receive_notify);

    /* Check for error */
    if (status)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));
        return(status);
    }

    /* Create the mDNS mutex.  */
    status =  tx_mutex_create(&(mdns_ptr -> nx_mdns_mutex), 
                              "mDNS Mutex", TX_NO_INHERIT);

    /* Determine if the semaphore creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));

        /* No, return error status.  */
        return(status);
    }    

    /* Create the internal mDNS event flag object.  */
    status = tx_event_flags_create(&mdns_ptr -> nx_mdns_events, "mDNS Thread Events");
    
    /* Determine if the event flags creation was successful.  */
    if (status)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));

        /* Delete the mDNS mutex.  */
        tx_mutex_delete(&(mdns_ptr -> nx_mdns_mutex));     

        /* No, return error status.  */
        return(status);
    }

    /* Create the mDNS processing thread.  */
    status =  tx_thread_create(&(mdns_ptr -> nx_mdns_thread), "mDNS Thread",
                               _nx_mdns_thread_entry, (ULONG) mdns_ptr,
                               stack_ptr, stack_size, priority, priority, 
                               1, TX_AUTO_START);

    /* Determine if the thread creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));

        /* Delete the mDNS mutex.  */
        tx_mutex_delete(&(mdns_ptr -> nx_mdns_mutex));
        
        /* Delete the mDNS events.  */
        tx_event_flags_delete(&(mdns_ptr -> nx_mdns_events)); 

        /* No, return error status.  */
        return(status);
    }
    
    /* Create the timer of Resource record lifetime.  */
    status =  tx_timer_create(&(mdns_ptr -> nx_mdns_timer), "mDNS Timer", 
                              _nx_mdns_timer_entry, (ULONG) mdns_ptr, 
                              0xFFFFFFFF, 0, TX_NO_ACTIVATE);

    /* Determine if the thread creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));

        /* Delete the mDNS mutex.  */
        tx_mutex_delete(&(mdns_ptr -> nx_mdns_mutex));
        
        /* Delete the mDNS events.  */
        tx_event_flags_delete(&(mdns_ptr -> nx_mdns_events));

        /* Delete the mDNS thread.  */
        tx_thread_delete(&(mdns_ptr -> nx_mdns_thread)); 

        /* No, return error status.  */
        return(status);
    }
    
    /* Initialize the buffer.  */    
    status =  _nx_mdns_cache_initialize(mdns_ptr, local_cache_ptr, local_cache_size, 
                                        peer_cache_ptr, peer_cache_size);
    
    /* Determine if the buffer initialize was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));
        nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));

        /* Delete the mDNS timer.  */
        tx_timer_delete(&(mdns_ptr -> nx_mdns_timer));
        
        /* Delete the mDNS mutex.  */
        tx_mutex_delete(&(mdns_ptr -> nx_mdns_mutex));
        
        /* Delete the mDNS events.  */
        tx_event_flags_delete(&(mdns_ptr -> nx_mdns_events));

        /* Delete the mDNS thread.  */
        tx_thread_delete(&(mdns_ptr -> nx_mdns_thread)); 

        /* No, return error status.  */
        return(status);
    }
    
    /* Update the mDNS structure ID.  */
    mdns_ptr -> nx_mdns_id = NX_MDNS_ID;

    /* The random delay of first probing for RR. */
    mdns_ptr -> nx_mdns_first_probing_delay = (ULONG)(1 + (((ULONG)NX_RAND()) % NX_MDNS_PROBING_TIMER_COUNT));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_delete                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS delete function call.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_delete                       Actual mDNS delete function   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_mdns_delete(NX_MDNS *mdns_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)        
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS delete service.  */
    status =  _nx_mdns_delete(mdns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_delete                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the mDNS instance.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    nx_udp_socket_unbind                  Unbind the mDNS UDP socket    */
/*    nx_udp_socket_delete                  Delete the mDNS UDP socket    */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    tx_mutex_delete                       Delete the mDNS mutex         */ 
/*    tx_thread_terminate                   Terminate the mDNS thread     */ 
/*    tx_thread_delete                      Delete the mDNS thread        */  
/*    tx_event_flags_delete                 Delete the mDNS events flag   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_mdns_delete(NX_MDNS *mdns_ptr)
{
    
        
    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
 
    /* Clear the mDNS structure ID. */
    mdns_ptr -> nx_mdns_id =  0;
       
    /* Set the pointer of global variable mDNS.  */
    _nx_mdns_created_ptr = NX_NULL;
         
    /* Unbind the port.  */
    nx_udp_socket_unbind(&(mdns_ptr -> nx_mdns_socket));

    /* Delete the UDP socket.  */
    nx_udp_socket_delete(&(mdns_ptr -> nx_mdns_socket));
    
    /* Terminate the mDNS processing thread.  */
    tx_thread_terminate(&(mdns_ptr -> nx_mdns_thread));

    /* Delete the mDNS processing thread.  */
    tx_thread_delete(&(mdns_ptr -> nx_mdns_thread));
    
    /* Delete the event flags.  */
    tx_event_flags_delete(&(mdns_ptr -> nx_mdns_events));

    /* Delete the timer.  */
    tx_timer_delete(&(mdns_ptr -> nx_mdns_timer));
       
    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Delete the mDNS mutex.  */
    tx_mutex_delete(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_cache_notify_set                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS cache full notify       */ 
/*    function call.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    cache_full_notify                     Cache full notify function    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_cache_notify_set             Actual cache notify           */  
/*                                            set function                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_cache_notify_set(NX_MDNS *mdns_ptr, VOID (*cache_full_notify_cb)(NX_MDNS *mdns_ptr, UINT, UINT))
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS cache notify set function.  */
    status =  _nx_mdns_cache_notify_set(mdns_ptr, cache_full_notify_cb);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_notify_set                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function set the cache full notify function.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_notify_set(NX_MDNS *mdns_ptr, VOID (*cache_full_notify_cb)(NX_MDNS *mdns_ptr, UINT, UINT))
{


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Set the cache notify.  */
    mdns_ptr -> nx_mdns_cache_full_notify = cache_full_notify_cb;
    
    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_cache_notify_clear                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS cache full notify       */ 
/*    function call.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    cache_full_notify                     Cache full notify function    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_cache_notify_clear           Actual cache notify           */ 
/*                                            clear function              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_cache_notify_clear(NX_MDNS *mdns_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS cache notify clear function.  */
    status =  _nx_mdns_cache_notify_clear(mdns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_notify_clear                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function set the cache full notify function.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_notify_clear(NX_MDNS *mdns_ptr)
{


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Clear the cache notify.  */
    mdns_ptr -> nx_mdns_cache_full_notify = NX_NULL;
    
    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_ignore_set                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS service ignore mask     */ 
/*    set function call.                                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    service_mask                          The service mask              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_service_ignore_set           Actual ignore set function    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_ignore_set(NX_MDNS *mdns_ptr, ULONG service_mask)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS service ignore mask set function.  */
    status =  _nx_mdns_service_ignore_set(mdns_ptr, service_mask);

    /* Return status.  */
    return(status);
}   


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_ignore_set                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the service mask to ignore the service.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    service_mask                          The service mask              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the MDNS mutex            */ 
/*    tx_mutex_put                          Put the MDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_ignore_set(NX_MDNS *mdns_ptr, ULONG service_mask)
{


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Set the service ignore mask.  */
    mdns_ptr -> nx_mdns_service_ignore_mask = service_mask;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_notify_set                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS service notify function */ 
/*    call.                                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    service_mask                          The service mask              */ 
/*    service_change_notify                 Service change notify function*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_service_notify_set           Actual mDNS service notify    */ 
/*                                            set function                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_notify_set(NX_MDNS *mdns_ptr, ULONG service_mask,
                                  VOID (*service_change_notify)(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state))
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS service notify set function.  */
    status =  _nx_mdns_service_notify_set(mdns_ptr, service_mask, service_change_notify);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_notify_set                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function set the service mask and notify callback function.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    service_mask                          The service mask              */ 
/*    service_change_notify                 Service change notify function*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_notify_set(NX_MDNS *mdns_ptr, ULONG service_mask,
                                 VOID (*service_change_notify)(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state))
{


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Set the service mask.  */
    mdns_ptr -> nx_mdns_service_notify_mask = service_mask;

    /* Assign the service change notify. */
    mdns_ptr -> nx_mdns_service_change_notify = service_change_notify;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_notify_clear                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS service notify function */ 
/*    call.                                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_service_notify_clear         Actual service notify         */
/*                                            clear function              */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_notify_clear(NX_MDNS *mdns_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {
    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {

        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS service notify clear function.  */
    status =  _nx_mdns_service_notify_clear(mdns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_notify_clear                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clears the service mask and notify callback function. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_notify_clear(NX_MDNS *mdns_ptr)
{


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Clear the service mask.  */
    mdns_ptr -> nx_mdns_service_notify_mask = 0;

    /* Clear the service change notify. */
    mdns_ptr -> nx_mdns_service_change_notify = NX_NULL;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_CLIENT  */


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_announcement_timing_set           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS announcement timing     */ 
/*    set function call.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    t                                     Announcing period             */ 
/*    p                                     Announcing count              */ 
/*    k                                     Announcing factor             */ 
/*    retrans_interval                      Announcing retransmission     */ 
/*                                            interval                    */ 
/*    interval                              Announcing period interval    */ 
/*    max_time                              Announcing max time           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_service_announcement_timing_set                            */ 
/*                                          Actual mDNS announcement      */ 
/*                                            timing set function         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_announcement_timing_set(NX_MDNS *mdns_ptr, UINT t, UINT p, UINT k, UINT retrans_interval, ULONG period_interval, UINT max_time)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((!mdns_ptr) || (!t) || (!p) || (!k) || (!period_interval) || (!max_time))
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS delete service.  */
    status =  _nx_mdns_service_announcement_timing_set(mdns_ptr, t, p, k, retrans_interval, period_interval, max_time);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_announcement_timing_set            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function set the service mask and notify callback function.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    t                                     Announcing period             */ 
/*    p                                     Announcing count              */ 
/*    k                                     Announcing factor             */ 
/*    retrans_interval                      Announcing retransmission     */ 
/*                                            interval                    */ 
/*    interval                              Announcing period interval    */ 
/*    max_time                              Announcing max time           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_announcement_timing_set(NX_MDNS *mdns_ptr, UINT t, UINT p, UINT k, UINT retrans_interval, ULONG period_interval, UINT max_time)
{


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Set the mDNS announcing period.  */
    mdns_ptr -> nx_mdns_announcing_period = (USHORT)t;
    
    /* Set the mDNS announcing count between one announcing period.  */
    mdns_ptr -> nx_mdns_announcing_count = (UCHAR)p;
    
    /* Set the mDNS announcing factor.  */
    mdns_ptr -> nx_mdns_announcing_factor = (UCHAR)k;

    /* Set the mDNS announcing retransmission interval.  */
    mdns_ptr -> nx_mdns_announcing_retrans_interval = (USHORT)retrans_interval;

    /* Set the mDNS announcing period interval.  */
    mdns_ptr -> nx_mdns_announcing_period_interval = (ULONG)period_interval;

    /* Set the mDNS announcing max time.  */
    mdns_ptr -> nx_mdns_announcing_max_time = (UCHAR)max_time;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_SERVER  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_enable                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS enable function call.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_enable                       Actual mDNS enable function   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_enable(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input or invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS delete service.  */
    status =  _nx_mdns_enable(mdns_ptr, interface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_enable                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables the mDNS of the physical host interface by    */ 
/*    interface index.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    nx_ipv4_multicast_interface_join      Join the IPv4 Multicast group */ 
/*    nxd_ipv6_multicast_interface_join     Join the IPv6 Multicast group */ 
/*    _nx_mdns_host_name_register           Register the host name        */
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_enable(NX_MDNS *mdns_ptr, UINT interface_index)
{
   
UINT        status;

#ifndef NX_MDNS_DISABLE_SERVER
ULONG       *head;
NX_MDNS_RR  *p;
#endif /* NX_MDNS_DISABLE_SERVER */

#ifdef NX_MDNS_ENABLE_IPV6
NX_INTERFACE        *interface_ptr;
NXD_IPV6_ADDRESS    *ipv6_address;
#endif /* NX_MDNS_ENABLE_IPV6  */

    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Check to see if mDNS is already enabled on this interface.  */
    if (mdns_ptr -> nx_mdns_interface_enabled[interface_index] == NX_TRUE)
    {
        return(NX_MDNS_ALREADY_ENABLED);
    }

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Enable the mdns function.  */
    mdns_ptr -> nx_mdns_interface_enabled[interface_index] = NX_TRUE;

#ifdef NX_MDNS_ENABLE_IPV6
    /* Set the interface pointer.  */
    interface_ptr = &(mdns_ptr -> nx_mdns_ip_ptr -> nx_ip_interface[interface_index]);

    /* Find the link-local address as source adress. Get first address from interface.  */
    ipv6_address = interface_ptr -> nxd_interface_ipv6_address_list_head;

    /* Loop to check the address.  */
    while (ipv6_address)
    {

        /* Check for link-local address. */
        if (IPv6_Address_Type(ipv6_address -> nxd_ipv6_address) & IPV6_ADDRESS_LINKLOCAL)
        {
            break;
        }
        ipv6_address = ipv6_address -> nxd_ipv6_address_next;
    }

    /* Check if found the link-local address.  */
    if(ipv6_address)
    {

        /* Set the IPv6 link-local address index.  */
        mdns_ptr -> nx_mdns_ipv6_address_index[interface_index] = ipv6_address -> nxd_ipv6_address_index;
    }
    else
    {

        /* No available address, set the address index as 0xFFFFFFFF.  */
        mdns_ptr -> nx_mdns_ipv6_address_index[interface_index] = 0xFFFFFFFF;
    }
#endif /* NX_MDNS_ENABLE_IPV6  */

    /* Join the group.  */  
    status = nx_ipv4_multicast_interface_join(mdns_ptr -> nx_mdns_ip_ptr, NX_MDNS_IPV4_MULTICAST_ADDRESS, interface_index);

    /* Check status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return(status);
    }
    
#ifdef NX_MDNS_ENABLE_IPV6
    status = nxd_ipv6_multicast_interface_join(mdns_ptr -> nx_mdns_ip_ptr, &NX_MDNS_IPV6_MULTICAST_ADDRESS, interface_index);

    /* Check status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return(status);
    }
#endif /* NX_MDNS_ENABLE_IPV6  */
    
#ifndef NX_MDNS_DISABLE_SERVER

    /* Register the host name.  */
    status = _nx_mdns_host_name_register(mdns_ptr, NX_TRUE, interface_index);

    /* Check status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return(status);
    }

    /* Probing the all resource record wheneven a Multicast DNS responder starts up, waks up from sleep, receives an indication of a network interface "Link CHange" event.RFC6762, Section8, Page25.   */
    
    /* Get the header to the local buffer. */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;

    /* Set the pointer.  */
    head = (ULONG*)(*head);   

    /* Check the resource record.  */
    for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Check whether the resource record is valid. */
        if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
            continue;

        /* Check the state and delete flag, GOODBYE state and DELETE FLAG means this resource record should be deleted.  */
        if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_GOODBYE) &&
            (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_DELETE))
        {

            /* Delete the resource records.  */
            _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, p);
            continue;
        }

        /* Check the resource reocrd type.  */
        if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) ||
            (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC))
        {            
            p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_VALID;
            continue;
        }

        /* Check the unique flag.  */
        if (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE)
        {
            /* For a unique type, we need to probe it on the network
            to guarantee its uniqueness. */
            p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_PROBING;
            p -> nx_mdns_rr_timer_count = mdns_ptr -> nx_mdns_first_probing_delay;
            p -> nx_mdns_rr_retransmit_count = NX_MDNS_PROBING_RETRANSMIT_COUNT;
        }
        else
        {
            /* If the record is not marked as unique, start the announcement process. */
            p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_ANNOUNCING;
            p -> nx_mdns_rr_timer_count = NX_MDNS_ANNOUNCING_TIMER_COUNT;
            p -> nx_mdns_rr_retransmit_lifetime = mdns_ptr -> nx_mdns_announcing_period;

            /* Check the announcing max time.  */
            if (mdns_ptr -> nx_mdns_announcing_max_time != NX_MDNS_ANNOUNCING_FOREVER)
                p -> nx_mdns_rr_announcing_max_time = (UCHAR)(mdns_ptr -> nx_mdns_announcing_max_time - 1);
            else
                p -> nx_mdns_rr_announcing_max_time = NX_MDNS_ANNOUNCING_FOREVER;

            /* Set the retransmit count.  */
            if (mdns_ptr -> nx_mdns_announcing_retrans_interval)
                p -> nx_mdns_rr_retransmit_count = mdns_ptr -> nx_mdns_announcing_count;
            else
                p -> nx_mdns_rr_retransmit_count = 1;
        }

        /* Set the mDNS timer.  */
        _nx_mdns_timer_set(mdns_ptr, p, p -> nx_mdns_rr_timer_count);
    }
#endif /* NX_MDNS_DISABLE_SERVER */

    /* Set the mdns started flag.  */  
    mdns_ptr -> nx_mdns_started = NX_TRUE;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return.  */
    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_disable                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS disable function call.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_disable                      Actual mDNS disable function  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_disable(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input or invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS delete service.  */
    status =  _nx_mdns_disable(mdns_ptr, interface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_disable                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function disables the mDNS of the physical host interface by   */ 
/*    interface index.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    nx_ipv4_multicast_leave               Leave the IPv4 Multicast group*/ 
/*    nxd_ipv6_multicast_interface_leave    Leave the IPv6 Multicast group*/  
/*    _nx_mdns_cache_initialize             Initialize the mDNS peer cache*/ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_disable(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT        dns_sd_size;
UINT        i;
ULONG       *head;
NX_MDNS_RR  *p;


    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Check to see if mDNS is not enabled on this interface. 
       Note: Only support one interface yet.  */
    if (mdns_ptr -> nx_mdns_interface_enabled[interface_index] == NX_FALSE)
    {
        return(NX_MDNS_NOT_ENABLED);
    }

    /* Check the DNS-SD string.  */
    if (_nx_utility_string_length_check((CHAR *)_nx_mdns_dns_sd, &dns_sd_size, NX_MDNS_DNS_SD_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Disable the mdns function.  */
    mdns_ptr -> nx_mdns_interface_enabled[interface_index] = NX_FALSE;

    /* Leave the group.  */
    nx_ipv4_multicast_interface_leave(mdns_ptr -> nx_mdns_ip_ptr, NX_MDNS_IPV4_MULTICAST_ADDRESS, interface_index);
   
#ifdef NX_MDNS_ENABLE_IPV6
    nxd_ipv6_multicast_interface_leave(mdns_ptr -> nx_mdns_ip_ptr, &NX_MDNS_IPV6_MULTICAST_ADDRESS, interface_index);
#endif /* NX_MDNS_ENABLE_IPV6  */

#ifndef NX_MDNS_DISABLE_CLIENT

    /* Get the local buffer head. */
    head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;

    /* Set the pointer.  */
    head = (ULONG*)(*head);   

    /* Delete all services on this interface.  */
    for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_peer_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {

        /* Check whether the resource record is valid. */
        if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
            continue;

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Delete the resource record.  */
        _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, p);
    }
#endif /* NX_MDNS_DISABLE_CLIENT */
    
#ifndef NX_MDNS_DISABLE_SERVER

    /* Get the local buffer head. */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;

    /* Set the pointer.  */
    head = (ULONG*)(*head);   

    /* Send the Goodbye message, RFC6762, Section10.1, Page33.  */
    for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {

        /* Check whether the resource record is valid. */
        if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
            continue;

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Skip the NSEC and DNS-SD PTR resource record.  */
        if ((p ->nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC) ||
            ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) &&
             (!_nx_mdns_name_match(p -> nx_mdns_rr_name, (UCHAR *)_nx_mdns_dns_sd, dns_sd_size))))
        {

            /* Suspend the resource record.  */
            p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_SUSPEND;
        }
        else
        {

            /* Set the state to send Goodbye packet.  */
            p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_GOODBYE;

            /* Clear the retransmit count.  */
            p -> nx_mdns_rr_retransmit_count = NX_MDNS_GOODBYE_RETRANSMIT_COUNT;

            /* Set the timer count. 250ms.  */
            p -> nx_mdns_rr_timer_count = NX_MDNS_GOODBYE_TIMER_COUNT;

            /* Set the mDNS timer.  */
            _nx_mdns_timer_set(mdns_ptr, p, p -> nx_mdns_rr_timer_count);
        }
    } 
#endif /* NX_MDNS_DISABLE_SERVER */

    /* Check if all interfaces are disabled.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {
        if (mdns_ptr -> nx_mdns_interface_enabled[i])
            break;
    }

    /* Set the mdns started flag.  */
    if (i == NX_MAX_PHYSICAL_INTERFACES)
        mdns_ptr -> nx_mdns_started = NX_FALSE;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_local_domain_set                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the mDNS domain sets             */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    domain_name                           Domain name                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_mdns_domain_name_set              Actual domain set function    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_domain_name_set(NX_MDNS *mdns_ptr, UCHAR *domain_name)
{
    
UINT    status;


    /* Check for invalid input pointers.  */
    if ((!mdns_ptr) || (!domain_name))
    {    
        return(NX_PTR_ERROR);
    }    
      
    /* Check for invalid non pointer input or invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS domain name set service.  */
    status =  _nx_mdns_domain_name_set(mdns_ptr, domain_name);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_domain_name_set                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function sets the mDNS domain name. By default is "local".     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    domain_name                           Domain name                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_domain_name_set(NX_MDNS *mdns_ptr, UCHAR *domain_name)
{
  
UINT        index;
UINT        domain_name_size;


    /* Check the size.  */
    if (_nx_utility_string_length_check((CHAR *)domain_name, &domain_name_size, NX_MDNS_DOMAIN_NAME_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Save the new domain name.  */
    memcpy(&mdns_ptr -> nx_mdns_domain_name[0], domain_name, domain_name_size); /* Use case of memcpy is verified. */
    mdns_ptr -> nx_mdns_domain_name[domain_name_size] = NX_NULL;

    /* Initialize the struct.  */
    memset(_nx_mdns_dns_sd, 0, NX_MDNS_DNS_SD_MAX);

    /* Update the services dns-sd.  */
    memcpy(_nx_mdns_dns_sd, "_services._dns-sd._udp.", (sizeof("_services._dns-sd._udp.") - 1)); /* Use case of memcpy is verified. */
    index = (sizeof("_services._dns-sd._udp.") - 1);
    memcpy(&_nx_mdns_dns_sd[index], domain_name, domain_name_size); /* Use case of memcpy is verified. */

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_add                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the mDNS service add             */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  Service instance name         */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    txt                                   The txt string of the service */ 
/*    ttl                                   The ttl of the service        */ 
/*    priority                              The priority of target host   */
/*    weights                               Service weight                */
/*    port                                  The port on this target host  */
/*    is_unique                             The RR set of the service     */
/*    interface_index                       The interface index           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_mdns_service_add                  Actual service add function   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_add(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UCHAR *txt, ULONG ttl,
                           USHORT priority, USHORT weights, USHORT port, UCHAR is_unique, UINT interface_index)
{
                    
UINT    status;  


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }

    /* Check for invalid server attributes. */
    if ((mdns_ptr -> nx_mdns_id != NX_MDNS_ID) || (!port))
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Check the size of the service name string. The last four characters in the 
       service name are reserved so DNS-SD is able to append " (2)" during the conflict 
       resolution process. */
#if (NX_MDNS_SERVICE_NAME_MAX >= NX_MDNS_LABEL_MAX)
    if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, (NX_MDNS_LABEL_MAX - 4)))
#else
    if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, (NX_MDNS_SERVICE_NAME_MAX - 4)))
#endif
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Check the type.  */
    if (_nx_utility_string_length_check((CHAR *)type, NX_NULL, NX_MDNS_TYPE_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Check the txt string size.  */
    if (txt)
    {
        if (_nx_utility_string_length_check((CHAR *)txt, NX_NULL, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }
    
    /* Check the txt string size.  */
    if (sub_type)
    {
        if (_nx_utility_string_length_check((CHAR *)sub_type, NX_NULL, NX_MDNS_LABEL_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Check the interface index.  */
    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Call actual mDNS service add service.  */
    status =  _nx_mdns_service_add(mdns_ptr, name, type, sub_type, txt, ttl, priority, 
                                   weights, port, is_unique, interface_index);

    /* Return status.  */
    return(status);    
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_add                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function adds the mDNS services, and the PTR, SRV and TXT      */ 
/*    records into the local buffer.                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  Service nstance name          */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    txt                                   The txt string of the service */ 
/*    ttl                                   The ttl of the service        */ 
/*    priority                              The priority of target host   */
/*    weights                               Service weight                */
/*    port                                  The port on this target host  */
/*    is_unique                             The RR set of the service     */
/*    interface_index                       The interface index           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */  
/*    _nx_mdns_service_name_assemble        Assemble the service name     */
/*    _nx_mdns_rr_srv_add                   Add the SRV resource record   */ 
/*    _nx_mdns_rr_txt_add                   Add the TXT resource record   */ 
/*    _nx_mdns_rr_ptr_add                   Add the PTR resource record   */ 
/*    _nx_mdns_rr_delete                    Delete the resource record    */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_add(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UCHAR *txt, ULONG ttl,
                          USHORT priority, USHORT weights, USHORT port, UCHAR is_unique, UINT interface_index)
{
  
UINT        status;
UINT        type_index;
NX_MDNS_RR *srv_rr;
NX_MDNS_RR *txt_rr;
NX_MDNS_RR *ptr_rr;
NX_MDNS_RR *dns_sd_rr;
ULONG       *head;
NX_MDNS_RR  *p;
ULONG        srv_ttl;
ULONG        txt_ttl;
ULONG        ptr_ttl;
UINT         string_length;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Check the ttl value.  */
    if (ttl)
    {

        /* Use user supplied TTL value if it is not zero. */
        srv_ttl = ttl;
        txt_ttl = ttl;
        ptr_ttl = ttl;
    }
    else
    {

        /* Use the RFC recommended TTL values if user doesn't supply one.  */
        srv_ttl = NX_MDNS_RR_TTL_HOST;
        txt_ttl = NX_MDNS_RR_TTL_OTHER;
        ptr_ttl = NX_MDNS_RR_TTL_OTHER;
    }

    /* Construct the SRV name.  */
    status = _nx_mdns_service_name_assemble(name, type, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);

    /* Check the status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return (status);
    }

    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)(&temp_string_buffer[0]), &string_length, NX_MDNS_NAME_MAX))
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Check whether the same service name exist.  */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache; 
    head = (ULONG*)(*head);   

    /* Check the resource record.  */
    for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {         
        if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
            continue;

        /* Check the interface.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV)
        {  

            /* Compare the service name.  */
            if (!_nx_mdns_name_match(&temp_string_buffer[0], p -> nx_mdns_rr_name, string_length))
            {

                /* Release the mDNS mutex.  */
                tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

                return(NX_MDNS_EXIST_SAME_SERVICE);
            }
        } 
    }

    /* Construct the SRV target name.  */
    status = _nx_mdns_service_name_assemble(mdns_ptr -> nx_mdns_host_name, NX_NULL, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &target_string_buffer[0], NX_MDNS_NAME_MAX, NX_NULL);
    
    /* Check the status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return (status);
    }

    /* Add the SRV resource records message.  */
    status = _nx_mdns_rr_srv_add(mdns_ptr, &temp_string_buffer[0], srv_ttl, is_unique, priority, weights, port, &target_string_buffer[0], &srv_rr, interface_index); 
    
    /* Check the status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return(status);
    }

    /* Add the TXT resource records message.  */
    status = _nx_mdns_rr_txt_add(mdns_ptr, &temp_string_buffer[0], txt_ttl, is_unique, txt, &txt_rr, interface_index);
    
    /* Check the status.  */
    if (status)
    {

        /* Delete the SRV records.  */
        _nx_mdns_rr_delete(mdns_ptr, srv_rr);

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }
    
    /* Add the dns-sd PTR resource records message.  */
    status = _nx_mdns_rr_ptr_add(mdns_ptr, (UCHAR *)_nx_mdns_dns_sd, ptr_ttl, NX_MDNS_RR_SET_SHARED, &temp_string_buffer[type_index], NX_TRUE, &dns_sd_rr, interface_index);
    
    /* Check the status.  */
    if (status)
    {

        /* Delete the TXT records.  */
        _nx_mdns_rr_delete(mdns_ptr, txt_rr);

        /* Delete the SRV records.  */
        _nx_mdns_rr_delete(mdns_ptr, srv_rr);        

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }

    /* Add the PTR resource records message.  */
    status = _nx_mdns_rr_ptr_add(mdns_ptr, &temp_string_buffer[type_index], ptr_ttl, NX_MDNS_RR_SET_SHARED, &temp_string_buffer[0], NX_TRUE, &ptr_rr, interface_index);
    
    /* Check the status.  */
    if (status)
    {

        /* Delete the TXT records.  */
        _nx_mdns_rr_delete(mdns_ptr, txt_rr);

        /* Delete the SRV records.  */
        _nx_mdns_rr_delete(mdns_ptr, srv_rr);
        
        /* Delete the DNS_SD PTR records.  */
        _nx_mdns_rr_delete(mdns_ptr, dns_sd_rr);

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }

    /* Add another PTR type for the subtype. */
    if (sub_type)
    {

        /* Construct the PTR target name.  */
        status = _nx_mdns_service_name_assemble(NX_NULL, type, sub_type, mdns_ptr -> nx_mdns_domain_name, &target_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);

        /* Check the status.  */
        if (status)
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

            return (status);
        }

        /* Add the Sub PTR resource records message.  */
        status = _nx_mdns_rr_ptr_add(mdns_ptr, &target_string_buffer[type_index], ptr_ttl, NX_MDNS_RR_SET_SHARED, &temp_string_buffer[0], NX_TRUE, NX_NULL, interface_index);

        /* Check the status.  */
        if (status)
        {

            /* Delete the PTR records.  */
            _nx_mdns_rr_delete(mdns_ptr, ptr_rr);

            /* Delete the DNS_SD PTR records.  */
            _nx_mdns_rr_delete(mdns_ptr, dns_sd_rr);

            /* Delete the SRV records.  */
            _nx_mdns_rr_delete(mdns_ptr, srv_rr);

            /* Delete the TXT records.  */
            _nx_mdns_rr_delete(mdns_ptr, txt_rr);

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

            return(status);
        }
    } 
   
#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES

    /* Add the NSEC resource record for service.  */
    _nx_mdns_rr_nsec_add(mdns_ptr, &temp_string_buffer[0], NX_FALSE, NX_FALSE, NX_MDNS_ADD_NSEC_FOR_SERVICE, interface_index);
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the mDNS service delete          */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_mdns_service_delete               Actual service delete function*/ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_delete(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type)
{
    
UINT    status;

    /* Check for invalid input pointers.  */
    if ((!mdns_ptr) || (!name) || (!type))
    {    
        return(NX_PTR_ERROR);
    }    
    
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Check the size.  */
    if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, NX_MDNS_LABEL_MAX) ||
        _nx_utility_string_length_check((CHAR *)type, NX_NULL, NX_MDNS_TYPE_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Call actual mDNS service delete service.  */
    status =  _nx_mdns_service_delete(mdns_ptr, name, type, sub_type);

    /* Return status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function deletes the mDNS services.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_mdns_service_delete_internal      Actual service delete function*/ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_delete(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type)
{

UINT    i;
UINT    status = NX_MDNS_NOT_ENABLED;
UINT    service_delete_success = NX_FALSE;

    /* Delete service from all enabled interfaces.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i ++)
    {

        /* Call actual mDNS service delete service.  */
        status =  _nx_mdns_service_interface_delete(mdns_ptr, name, type, sub_type, i);
                    
        /* Check status.  */
        if (status == NX_MDNS_SUCCESS)
        {
            service_delete_success = NX_TRUE;
        }
    }
    
    /* Check if delete service on all enabled interface.  */
    if (service_delete_success)
        return(NX_MDNS_SUCCESS);
    else
        return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_interface_delete                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function deletes the mDNS services, and remove SRV and TXT     */ 
/*    records from local buffer.                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */  
/*    _nx_mdns_service_name_assemble        Assemble the service name     */
/*    _nx_mdns_name_match                   Match the name string         */ 
/*    _nx_mdns_rr_delete                    Delete the resource record    */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_interface_delete(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UINT interface_index)
{
  
UINT        status;
UINT        found;
UINT        delete_flag;
UINT        type_index;
ULONG      *head;
NX_MDNS_RR *p;    
UINT        rr_name_length;
UINT        rr_ptr_name_length;

    NX_PARAMETER_NOT_USED(sub_type);

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    /* Initialize the parameters.  */
    type_index = 0;
    delete_flag = NX_FALSE;

    /* Construct the SRV name.  */
    status = _nx_mdns_service_name_assemble(name, type, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);
    
    /* Check the status.  */
    if (status)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return (status);
    }

    /* Get head. */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
    head = (ULONG*)(*head);

    for (p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {   

        /* Clear the found value.  */
        found = NX_FALSE;

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Check whether the resource record is valid. */
        if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
            continue;

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)(p -> nx_mdns_rr_name), &rr_name_length, NX_MDNS_NAME_MAX))
        {
            continue;
        }

        /* Compare the RDATA. */
        switch (p -> nx_mdns_rr_type)
        {   

            case NX_MDNS_RR_TYPE_SRV:
            case NX_MDNS_RR_TYPE_TXT:
            {
                if (!_nx_mdns_name_match(p -> nx_mdns_rr_name, &temp_string_buffer[0], rr_name_length))
                {
                    found = NX_TRUE;
                    delete_flag = NX_TRUE;
                }
                break;
            }
            case NX_MDNS_RR_TYPE_PTR:
            {

                /* Check string length.  */
                if (_nx_utility_string_length_check((CHAR *)(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name), &rr_ptr_name_length, NX_MDNS_NAME_MAX))
                {
                    break;
                }

                /* Find the PTR and Sub PTR resource record which pointer to the SRV resource record.*/
                if (!_nx_mdns_name_match(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, &temp_string_buffer[0], rr_ptr_name_length))
                {
                    found = NX_TRUE;
                }

                /* Find one DNS-SD PRT which pointer to the PTR resource record.  */
                if ((!_nx_mdns_name_match(p -> nx_mdns_rr_name, (UCHAR *)_nx_mdns_dns_sd, rr_name_length)) &&
                    (!_nx_mdns_name_match(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, &temp_string_buffer[type_index], rr_ptr_name_length)))
                {

                    /* Check the count.  */
                    if (p -> nx_mdns_rr_count)
                    {
                        p -> nx_mdns_rr_count --;
                    }
                    else
                    {

                        /* Delete this resource record directly.  */
                        _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, p);
                    }
                }
                break;
            }

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
            case NX_MDNS_RR_TYPE_NSEC:
            {
                if (!_nx_mdns_name_match(p -> nx_mdns_rr_name, &temp_string_buffer[0], rr_name_length))
                {            

                    /* Delete this resource record directly.  */
                    _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, p);
                }
                break;
            }
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
        }

        if (found)
        {

            /* Delete the resource records.  */
            status = _nx_mdns_rr_delete(mdns_ptr, p);

            /* Check the status.  */
            if (status)
            {
                /* Release the mDNS mutex.  */
                tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

                return(status);
            }
        }
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
               
    if (delete_flag == NX_TRUE)
        return(NX_MDNS_SUCCESS);
    else
        return(NX_MDNS_ERROR);
}
#endif /* NX_MDNS_DISABLE_SERVER */


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_one_shot_query                      PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS resource record query   */ 
/*    add function call.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    service                               Pointer to response service   */  
/*    timeout                               The timeour for service query */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */   
/*    _nx_mdns_service_one_shot_query       Actual query service function */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_one_shot_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, NX_MDNS_SERVICE *service, UINT timeout)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if ((!mdns_ptr) || (!type) || (!service))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for mDNS started flag.  */
    if (!mdns_ptr -> nx_mdns_started)
    {
        return(NX_MDNS_NOT_STARTED);
    }
    
    /* Check for name.  */
    if (name)
    {

        /* Check the name size.  */
        if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, NX_MDNS_LABEL_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Check the type size.  */
    if (_nx_utility_string_length_check((CHAR *)type, NX_NULL, NX_MDNS_TYPE_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    } 

    /* Check the sub type.  */
    if (sub_type)
    {

        /* Check the name size.  */
        if (_nx_utility_string_length_check((CHAR *)sub_type, NX_NULL, NX_MDNS_LABEL_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Call actual mDNS create service.  */
    status = _nx_mdns_service_one_shot_query(mdns_ptr, name, type, sub_type, service, timeout);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_one_shot_query                     PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts service one-shot query on all enabled          */ 
/*    interfaces.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    service                               Pointer to response service   */  
/*    timeout                               The timeour for service query */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_service_name_assemble        Assemble the service name     */
/*    _nx_mdns_service_name_resolve         Resolve the service name      */
/*    _nx_mdns_query                        Send the One Shot query       */
/*    _nx_mdns_service_addition_info_get    Get additional info of service*/   
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_one_shot_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, NX_MDNS_SERVICE *service, UINT timeout)
{

UINT        type_index;
UINT        status;
NX_MDNS_RR *answer_rr;
UCHAR      *query_name;
USHORT      query_type;
UINT        i;
UINT        name_length;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), NX_WAIT_FOREVER);

    /* Initialize the struct.  */
    memset(service, 0, sizeof(NX_MDNS_SERVICE));

    /* Loop to start one-shot query on all enabled interfaces until get the answer or query timeout.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Check if this interface is enabled.  */
        if (!mdns_ptr -> nx_mdns_interface_enabled[i])
            continue;

        /* Step1. Construct the query name and set the query type.  */
        if (name)
        {

            /* Construct the SRV name.  */
            status = _nx_mdns_service_name_assemble(name, type, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, NX_NULL);

            /* Check the status.  */
            if (status)
            {

                /* Release the mDNS mutex.  */
                tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
                return(status);
            }

            /* Set the query name and query type (NX_MDNS_RR_TYPE_ALL for SRV and TXT).  */
            query_name = temp_string_buffer;
            query_type = NX_MDNS_RR_TYPE_ALL;
        }
        else
        {

            /* Construct the PTR name.  */
            status = _nx_mdns_service_name_assemble(NX_NULL, type, sub_type, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);

            /* Check the status.  */
            if (status)
            {

                /* Release the mDNS mutex.  */
                tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
                return (status);
            }

            /* Set the query name and query type.  */
            query_name = &temp_string_buffer[type_index];
            query_type = NX_MDNS_RR_TYPE_PTR;
        }

        /* Step2. Start one shot query.  */
        status = _nx_mdns_one_shot_query(mdns_ptr, query_name, query_type, &answer_rr, timeout, i);

        /* Check the status.  */
        if ((status == NX_SUCCESS) ||
            (status == NX_MDNS_EXIST_UNIQUE_RR) ||
            (status == NX_MDNS_EXIST_SHARED_RR))
        {

            /* Check the query type.  */
            if (name)
            {

                /* Store the service name from SRV record.  */
                if (_nx_utility_string_length_check((CHAR *)answer_rr -> nx_mdns_rr_name, &name_length, NX_MDNS_NAME_MAX))
                {

                    /* Release the mDNS mutex.  */
                    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
                    return (NX_MDNS_DATA_SIZE_ERROR);
                }
                memcpy((char *)(service -> buffer), (char*)(answer_rr -> nx_mdns_rr_name), name_length); /* Use case of memcpy is verified. */
            }
            else
            {

                /* Store the service name from PTR record.  */
                if (_nx_utility_string_length_check((CHAR *)answer_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, &name_length, NX_MDNS_NAME_MAX))
                {

                    /* Release the mDNS mutex.  */
                    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
                    return (NX_MDNS_DATA_SIZE_ERROR);
                }
                memcpy((CHAR *)(service -> buffer), (CHAR *)(answer_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name), name_length); /* Use case of memcpy is verified. */
            }

            /* Get the additional information of service.  */
            _nx_mdns_service_addition_info_get(mdns_ptr, service -> buffer, service, i);

            /* Reslove the service name.  */
            status = _nx_mdns_service_name_resolve(service -> buffer, &(service -> service_name), &(service -> service_type), &(service -> service_domain));

            /* Check status.  */
            if (status)
                continue;

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

            /* Return success.  */
            return (NX_MDNS_SUCCESS);
        }
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return error.  */
    return (NX_MDNS_ERROR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_one_shot_query                             PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS resource record into remote buffer,     */ 
/*    mDNS thread send the query message.                                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The resource record name      */ 
/*    type                                  The resource record type      */ 
/*    out_rr                                Pointer to response RR        */ 
/*    one_shot                              One shot or continuous        */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _tx_thread_system_suspend             Suspend the thread            */ 
/*    _nx_mdns_query_check                  Check the query RR            */ 
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into cache                  */ 
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                            from cache                  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_one_shot_query(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, NX_MDNS_RR **out_rr, ULONG wait_option, UINT interface_index)
{

UINT        status;
NX_MDNS_RR  *rr;
NX_MDNS_RR  *insert_rr;
NX_MDNS_RR  temp_resource_record;
UINT        name_length;


    /* Clear the return record.  */
    *out_rr =   NX_NULL;

    /* Check the query RR.  */
    status = _nx_mdns_query_check(mdns_ptr, name, type, NX_TRUE, &rr, interface_index);

    /* Check the state.  */
    if (status)
    {

        /* Whether exists unique resource record in cache.  */
        if ((status == NX_MDNS_EXIST_UNIQUE_RR) ||
            (status == NX_MDNS_EXIST_SHARED_RR))
        {
            *out_rr = rr;
            return (NX_MDNS_SUCCESS);
        }
        else
        {
            return(status);
        }
    }

    if (wait_option != 0)
    {

        /* Initialize the struct.  */
        memset(&temp_resource_record, 0, sizeof(NX_MDNS_RR));

        if (_nx_utility_string_length_check((CHAR *)name, &name_length, NX_MDNS_NAME_MAX))
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Add the name.  */
        status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, name, name_length,
                                           (VOID **)(&temp_resource_record.nx_mdns_rr_name), NX_FALSE, NX_TRUE);

        /* Check for error.  */
        if (status)
        {
            return(status);
        }

        /* Add the parameters.  */
        temp_resource_record.nx_mdns_rr_type = type;
        temp_resource_record.nx_mdns_rr_class = NX_MDNS_RR_CLASS_IN;

        /* Set the resource record status.  */
        temp_resource_record.nx_mdns_rr_state = NX_MDNS_RR_STATE_QUERY;

        /* Remote resource record, set the owner flag.  */
        temp_resource_record.nx_mdns_rr_word = (temp_resource_record.nx_mdns_rr_word | NX_MDNS_RR_FLAG_PEER);

        /* Set the interface index.  */
        temp_resource_record.nx_mdns_rr_interface_index = (UCHAR)interface_index;

        /* Add the resource record.  */
        status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, &temp_resource_record, &insert_rr, NX_NULL);

        /* Check for error.  */
        if (status)
        {   

            /* Delete the same strings. */
            _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, temp_resource_record.nx_mdns_rr_name, 0);
            return(status);
        }

        /* A multicast DNS querier should also delay the first query of the series by 
           a randomly chosen amount in the range 20-120ms.  */
        insert_rr -> nx_mdns_rr_timer_count = (ULONG)(NX_MDNS_QUERY_DELAY_MIN + (((ULONG)NX_RAND()) % NX_MDNS_QUERY_DELAY_RANGE));
        insert_rr -> nx_mdns_rr_retransmit_lifetime = NX_MDNS_TIMER_COUNT_RANGE;

        /* Set the mDNS timer.  */
        _nx_mdns_timer_set(mdns_ptr, insert_rr, insert_rr -> nx_mdns_rr_timer_count);

        /* Release the mDNS mutex to process response.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        /* Suspend the thread on this mDNS query attempt.  */
        _nx_mdns_query_thread_suspend(&(mdns_ptr -> nx_mdns_rr_receive_suspension_list), _nx_mdns_query_cleanup, mdns_ptr, out_rr, wait_option);

        /* Get the mDNS mutex.  */
        tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), NX_WAIT_FOREVER);

        /* Determine if a packet was received successfully.  */
        if (_tx_thread_current_ptr -> tx_thread_suspend_status != NX_MDNS_SUCCESS)
        {

            /* Delete the query resource record.  */
            _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, insert_rr);
        }

        /* Return the status.  */
        return(_tx_thread_current_ptr -> tx_thread_suspend_status);
    }
    else
    {

        /* Immediate return, return error completion.  */
        return(NX_MDNS_NO_RR);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_continuous_query                    PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS resource record query   */ 
/*    add function call.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_service_continuous_query     Actual mDNS query RR function */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_continuous_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }  
    
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)        
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Check for mDNS stareted flag.  */
    if (!mdns_ptr -> nx_mdns_started)
    {
        return(NX_MDNS_NOT_STARTED);
    }

    /* Check the type. If the type is null,indicate lookup the all service type, the name and sub type must be null.  */
    if (!type)
    {
        if ((name) || (sub_type))
        {
            return(NX_MDNS_PARAM_ERROR);
        }
    }
    else
    {

        /* Check the name, If the name is non-null, the sub type must be null.  */
        if ((name) && (sub_type))
        {   
            return(NX_MDNS_PARAM_ERROR);
        }
    }

    /* Check the name.  */
    if (name)
    {

        /* Check the name size.  */
        if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, NX_MDNS_LABEL_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Check the type.  */
    if (type)
    {
          
        /* Check the type size.  */
        if (_nx_utility_string_length_check((CHAR *)type, NX_NULL, NX_MDNS_TYPE_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }      
    } 

    /* Check the sub type.  */
    if (sub_type)
    {

        /* Check the sub type size.  */
        if (_nx_utility_string_length_check((CHAR *)sub_type, NX_NULL, NX_MDNS_LABEL_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }      
    }

    /* Call actual mDNS create service.  */
    status =  _nx_mdns_service_continuous_query(mdns_ptr, name, type, sub_type);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_continuous_query                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts service continuous query on all enabled        */ 
/*    interfaces.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_service_name_assemble        Assemble the service name     */
/*    _nx_mdns_service_name_resolve         Resolve the service name      */
/*    _nx_mdns_query                        Send the One Shot query       */
/*    _nx_mdns_service_addition_info_get    Get additional info of service*/   
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_continuous_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type)
{

UINT        status = NX_MDNS_ERROR;
UINT        type_index;
UCHAR      *query_name;
USHORT      query_type;
UINT        query_success = NX_FALSE;
UINT        i;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Step1. Construct the query name and set the query type.  */
    /* Check the type. if the type is null, indicate search all service.  */
    if (!type)
    {

        /* Set the query name and query type.  */
        query_name = (UCHAR *)_nx_mdns_dns_sd;
        query_type = NX_MDNS_RR_TYPE_PTR;
    }
    else if (name)
    {

        /* Construct the SRV name.  */
        status = _nx_mdns_service_name_assemble(name, type, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, NX_NULL);

        /* Check the status.  */
        if (status)
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
            return (status);
        }

        /* Set the query name and query type (NX_MDNS_RR_TYPE_ALL for SRV and TXT).  */
        query_name = temp_string_buffer;
        query_type = NX_MDNS_RR_TYPE_ALL;
    }
    else
    {

        /* Construct the PTR name.  */
        status = _nx_mdns_service_name_assemble(NX_NULL, type, sub_type, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);

        /* Check the status.  */
        if (status)
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
            return (status);
        }

        /* Set the query name and query type.  */
        query_name = &temp_string_buffer[type_index];
        query_type = NX_MDNS_RR_TYPE_PTR;
    }

    /* Step2. Loop to start continuous query on all enabled interfaces.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Check if this interface is enabled.  */
        if (!mdns_ptr -> nx_mdns_interface_enabled[i])
            continue;

        /* Start continuous query.  */
        status = _nx_mdns_continuous_query(mdns_ptr, query_name, query_type, i);
        
        /* Check the status.  */
        if ((status == NX_SUCCESS) ||
            (status == NX_MDNS_EXIST_SAME_QUERY) ||
            (status == NX_MDNS_EXIST_UNIQUE_RR) ||
            (status == NX_MDNS_EXIST_SHARED_RR))
            query_success = NX_TRUE;
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Check if start continuous query success.  */
    if (query_success)
        return (NX_MDNS_SUCCESS);
    else
        return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_continuous_query                           PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts continuous query on specified interfaces.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    interface_index                       The interface index           */ 
/*    name                                  The resource record name      */ 
/*    type                                  The resource record type      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_query_check                  Check the query RR            */ 
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into cache                  */ 
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                            from cache                  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_continuous_query(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, UINT interface_index)
{

UINT        status;
NX_MDNS_RR  *insert_rr;
NX_MDNS_RR  temp_resource_record;
UINT        name_length;


    /* Check the query RR.  */
    status = _nx_mdns_query_check(mdns_ptr, name, type, NX_FALSE, NX_NULL, interface_index);

    /* Check the state.  */
    if (status)
    {
        return(status);
    }

    /* Initialize the struct.  */
    memset(&temp_resource_record, 0, sizeof(NX_MDNS_RR));

    if (_nx_utility_string_length_check((CHAR *)name, &name_length, NX_MDNS_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Add the name.  */
    status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, name, name_length,
                                       (VOID **)(&temp_resource_record.nx_mdns_rr_name), NX_FALSE, NX_TRUE);

    /* Check for error.  */
    if (status)
    {
        return(status);
    }

    /* Add the parameters.  */
    temp_resource_record.nx_mdns_rr_type = type;
    temp_resource_record.nx_mdns_rr_class = NX_MDNS_RR_CLASS_IN;

    /* Set the resource record status.  */
    temp_resource_record.nx_mdns_rr_state = NX_MDNS_RR_STATE_QUERY;

    /* Remote resource record, set the owner flag.  */
    temp_resource_record.nx_mdns_rr_word = (temp_resource_record.nx_mdns_rr_word | NX_MDNS_RR_FLAG_PEER);
        
    /* Continuous query, set the query type flag.  */
    if ((type != NX_MDNS_RR_TYPE_A) &&
        (type != NX_MDNS_RR_TYPE_AAAA))
    {
        temp_resource_record.nx_mdns_rr_word = (temp_resource_record.nx_mdns_rr_word | NX_MDNS_RR_FLAG_CONTINUOUS_QUERY);
    }

    /* Set the interface index.  */
    temp_resource_record.nx_mdns_rr_interface_index = (UCHAR)interface_index;

    /* Add the resource record.  */
    status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, &temp_resource_record, &insert_rr, NX_NULL);

    /* Check for error.  */
    if (status)
    {

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, temp_resource_record.nx_mdns_rr_name, 0);
        return(status);
    }

    /* A multicast DNS querier should also delay the first query of the series by 
        a randomly chosen amount in the range 20-120ms.  */
    insert_rr -> nx_mdns_rr_timer_count = (ULONG)(NX_MDNS_QUERY_DELAY_MIN + (((ULONG)NX_RAND()) % NX_MDNS_QUERY_DELAY_RANGE));
    insert_rr -> nx_mdns_rr_retransmit_lifetime = NX_MDNS_TIMER_COUNT_RANGE;

    /* Set the mDNS timer.  */
    _nx_mdns_timer_set(mdns_ptr, insert_rr, insert_rr -> nx_mdns_rr_timer_count);

    /* Return success.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_query_stop                          PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS resource record query   */ 
/*    add function call.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */   
/*    _nx_mdns_service_query_stop           Actual query RR function      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_query_stop(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {
        return(NX_PTR_ERROR);
    }  
    
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)        
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    if (name)
    {

        /* Check the size.  */
        if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Call actual mDNS create service.  */
    status =  _nx_mdns_service_query_stop(mdns_ptr, name, type, sub_type);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_query_stop                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS resource record into peer service cache,*/ 
/*    mDNS thread send the query message using continuous type.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_service_name_assemble        Assemble the service name     */
/*    _nx_mdns_name_match                   Match the name string         */  
/*    _nx_mdns_rr_delete                    Delete the resource record    */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_query_stop(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type)
{
    
UINT        status;
UINT        type_index = 0;  
UINT        query_stop = NX_FALSE;
ULONG       *head;
NX_MDNS_RR  *p; 
UINT         rr_name_length;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
            
    if (type)
    {        
        if (name)
        {

            /* Construct the SRV name.  */
            status = _nx_mdns_service_name_assemble(name, type, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, NX_NULL);

            /* Check the status.  */
            if (status)
            {

                /* Release the mDNS mutex.  */
                tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
                return (status);
            }
        }
        else
        {

            /* Construct the PTR name.  */
            status = _nx_mdns_service_name_assemble(NX_NULL, type, sub_type, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);

            /* Check the status.  */
            if (status)
            {

                /* Release the mDNS mutex.  */
                tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
                return (status);
            }
        }    
    }
    
    /* Get the remote buffer head. */
    head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;

    if (!head)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        /* Return a successful status.  */
        return(NX_MDNS_CACHE_ERROR);
    }
    
    /* Set the pointer.  */
    head = (ULONG*)(*head);

    /* Loop to delete query resource record.  */
    for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_peer_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {

        /* Check the resource record state. */
        if (p -> nx_mdns_rr_state != NX_MDNS_RR_STATE_QUERY)
            continue;

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)(p -> nx_mdns_rr_name), &rr_name_length, NX_MDNS_NAME_MAX))
        {
            continue;
        }

        if (!type)
        {
            if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) &&
                (!_nx_mdns_name_match(p -> nx_mdns_rr_name, (UCHAR *)_nx_mdns_dns_sd, rr_name_length)))
            {

                /* Delete the resource records.  */
                status = _nx_mdns_rr_delete(mdns_ptr, p);

                /* Check status.  */
                if (status == NX_MDNS_SUCCESS)
                    query_stop = NX_TRUE;
            }
        }
        else
        {      
            if (name)
            {
                if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_ALL) &&
                    (!_nx_mdns_name_match(p -> nx_mdns_rr_name, &temp_string_buffer[0], rr_name_length)))
                {

                    /* Delete the resource records.  */
                    status = _nx_mdns_rr_delete(mdns_ptr, p);

                    /* Check status.  */
                    if (status == NX_MDNS_SUCCESS)
                        query_stop = NX_TRUE;
                }
            }
            else
            {
                if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR)&&
                    (!_nx_mdns_name_match(p -> nx_mdns_rr_name, &temp_string_buffer[type_index], rr_name_length)))
                {

                    /* Delete the resource records.  */
                    status = _nx_mdns_rr_delete(mdns_ptr, p);

                    /* Check status.  */
                    if (status == NX_MDNS_SUCCESS)
                        query_stop = NX_TRUE;
                }
            }
        }
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Check if stop service continuous query.  */
    if (query_stop == NX_TRUE)
        return(NX_MDNS_SUCCESS);
    else
        return(NX_MDNS_ERROR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_service_lookup                              PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS resource record query   */ 
/*    add function call.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    service_index                         The index of the service      */ 
/*    service                               Pointer to Service instance   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_service_lookup               Actual mDNS query RR function */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_service_lookup(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UINT service_index, NX_MDNS_SERVICE *service)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }  
    
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)        
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    if (name)
    {

        /* Check the size.  */
        if (_nx_utility_string_length_check((CHAR *)name, NX_NULL, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Call actual mDNS create service.  */
    status =  _nx_mdns_service_lookup(mdns_ptr, name, type, sub_type, service_index, service);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_lookup                             PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS resource record into peer service cache,*/ 
/*    mDNS thread send the query message using continuous type.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    service_index                         The index of the service      */ 
/*    service                               Pointer to Service instance   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    tx_time_get                           Get the time                  */ 
/*    _nx_mdns_service_name_assemble        Assemble the service name     */
/*    _nx_mdns_service_name_resolve         Resolve the service name      */
/*    _nx_mdns_service_addition_info_get    Get additional info of service*/
/*    _nx_mdns_name_match                   Match the name string         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_service_lookup(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UINT service_index, NX_MDNS_SERVICE *service)
{
    
UINT        status;
UINT        type_index;
UINT        dns_sd_flag;
UINT        sub_type_flag;
UINT        index;
UINT        count;
UINT        found;
UCHAR       *srv_name = NX_NULL;
UCHAR       *ptr;
UCHAR       *tmp_sub_type;
UCHAR       *tmp_type;
UCHAR       *tmp_domain;
NX_MDNS_RR  *p = NX_NULL;
NX_MDNS_RR  *p1;
ULONG       *head, *tail;
UCHAR       i;
UINT        interface_index = 0;
UINT        temp_string_length;
UINT        dns_sd_length;
UINT        srv_name_length;
UINT        target_string_length;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
      
    /* Zero out the content of service */
    memset(service, 0, sizeof(NX_MDNS_SERVICE));

    /* Initialize the struct.  */
    count = 0;
    found = NX_FALSE;

    /* Check the type.  */
    if (type)
    {
        if (name)
        {

            /* Construct the Service name.  */
            status = _nx_mdns_service_name_assemble(name, type, NX_NULL, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, NX_NULL);

        }
        else
        {

            /* Construct the Service name.  */
            status = _nx_mdns_service_name_assemble(NX_NULL, type, sub_type, mdns_ptr -> nx_mdns_domain_name, &temp_string_buffer[0], NX_MDNS_NAME_MAX, &type_index);
        }
        /* Check the status.  */
        if (status)
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
            return (status);
        }

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)temp_string_buffer, &temp_string_length, NX_MDNS_NAME_MAX))
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
            return (NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Check the DNS-SD string.  */
    if (_nx_utility_string_length_check((CHAR *)_nx_mdns_dns_sd, &dns_sd_length, NX_MDNS_DNS_SD_MAX))
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Loop to search local and peer cache.  */
    for(i = 0; i < 2; i++)
    {

        /* Set the pointer. */
        if(i == NX_MDNS_CACHE_TYPE_LOCAL)
        {
#ifndef NX_MDNS_DISABLE_SERVER
            head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
#else
            continue;
#endif /* NX_MDNS_DISABLE_SERVER */
        }
        else
        {
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
        }

        if(head == NX_NULL)
            continue;

        /* Set the pointer.  */
        tail = (ULONG*)(*head);

        /* Check the resource record.  */
        for(p = (NX_MDNS_RR*)((UCHAR*)(head + 1)); (ULONG*)p < tail; p++)
        {

            /* Check whether the resource record is valid. */
            if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID) || 
                (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY))
                continue;

            if((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) || (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV))
            {

                if(type)
                {
                    if (_nx_mdns_name_match(p -> nx_mdns_rr_name, &temp_string_buffer[0], temp_string_length))
                        continue;
                }
                else if(p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV)
                    continue;

                if(name && (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV))
                {

                    /* Store the service name.  */
                    srv_name = p -> nx_mdns_rr_name;
                    interface_index = p -> nx_mdns_rr_interface_index;
                }
                else
                {

                    /* Set the service name pointer.  */
                    srv_name = p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name;
                    interface_index = p -> nx_mdns_rr_interface_index;
                }

                /* Check string length.  */
                if (_nx_utility_string_length_check((CHAR *)srv_name, &srv_name_length, NX_MDNS_NAME_MAX))
                    continue;

                /* Store the service name.  */
                memcpy((CHAR *)(service -> buffer), (CHAR *)srv_name, srv_name_length + 1); /* Use case of memcpy is verified. */

                /* Reslove the service name and check the PTR rdata name. Ignore the PTR when the PTR rdata does not pointer the service.  */
                status = _nx_mdns_service_name_resolve(service -> buffer, &(service -> service_name), &(service -> service_type), &(service -> service_domain));

                if (status)
                    continue;

                if(type == NX_NULL)
                {
                
                    /* Set the parameters.  */
                    sub_type_flag = NX_FALSE;
                    dns_sd_flag = NX_FALSE;
                    index = 0;

                    /* Check the PTR resource record with sub type.  */
                    ptr = p -> nx_mdns_rr_name;
                    while (*ptr != '\0')
                    {
                        if (*ptr == '.')
                        {
                            if (!_nx_mdns_name_match(ptr - index, (UCHAR *)"_sub", 4))
                            {
                                sub_type_flag = NX_TRUE;
                                break;
                            }
                            index = 0;
                        }
                        else              
                            index ++;
                        ptr ++;      
                    }
                    if (!_nx_mdns_name_match(p -> nx_mdns_rr_name, (UCHAR *)_nx_mdns_dns_sd, dns_sd_length))
                    {
                        dns_sd_flag = NX_TRUE;
                    }

                    /* Check the DNS-SD PTR reousrce record.  */
                    if ((dns_sd_flag == NX_TRUE) ||
                        (sub_type_flag == NX_TRUE))
                    {

                        /* Find the PTR resource record which pointer to the service.  */
                        for(p1 = (NX_MDNS_RR*)((UCHAR*)(head + 1)); (ULONG*)p1 < tail; p1++)
                        {

                            /* Check whether the resource record is valid. */
                            if ((p1 == p) ||
                                (p1 -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID) || 
                                (p1 -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY))
                                continue;

                            /* Check the interface index.  */
                            if (p1 -> nx_mdns_rr_interface_index != p -> nx_mdns_rr_interface_index)
                                continue;

                            if (p1 -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR)
                            {
                                if ((dns_sd_flag == NX_TRUE))
                                {

                                    /* Check string length.  */
                                    if (_nx_utility_string_length_check((CHAR *)(p1 -> nx_mdns_rr_name), &target_string_length, NX_MDNS_NAME_MAX))
                                    {
                                        continue;
                                    }

                                    /* Set the pointer.  */
                                    memcpy((CHAR *)&target_string_buffer[0], (CHAR *)(p1 -> nx_mdns_rr_name), target_string_length + 1); /* Use case of memcpy is verified. */

                                    /* Reslove the type.  */
                                    status = _nx_mdns_service_name_resolve(&target_string_buffer[0], &tmp_sub_type, &tmp_type, &tmp_domain);

                                    if (status)
                                        continue;

                                    /* Check string length.  */
                                    if (_nx_utility_string_length_check((CHAR *)tmp_type, &target_string_length, NX_MDNS_TYPE_MAX))
                                    {
                                        continue;
                                    }

                                    /* Construct the type and doamin, _http._tcp.local.  */
                                    *(tmp_type + target_string_length) = '.';

                                    /* Check string length.  */
                                    if (_nx_utility_string_length_check((CHAR *)tmp_type, &target_string_length, NX_MDNS_TYPE_MAX))
                                    {
                                        continue;
                                    }

                                    /* Compare the DNS_SD rdata.  */
                                    if (!_nx_mdns_name_match(tmp_type, p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, target_string_length))
                                    {

                                        /* Exist resource record including more info.  */
                                        count --;
                                        break;
                                    }
                                }
                                else
                                {
                                
                                    /* Exist the PTR RR which pointer to the same service.  */
                                    if (p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name == p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name)
                                    {
                                        count --;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            
                /* Check the count state.  */
                if (count == service_index)
                {
                    /* Yes, find. */
                    found = NX_TRUE;   
                    break;
                }
                count ++;
            }
        }

        /* Check if find the service.  */
        if (found == NX_TRUE)
        {
            break;
        }
    }

    if (found)
    {

        /* Update the elasped time.  */
        p -> nx_mdns_rr_elapsed_time = tx_time_get();

        /* Get the additional information.  */
        _nx_mdns_service_addition_info_get(mdns_ptr, srv_name, service, interface_index);

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return(NX_MDNS_SUCCESS);
    }
    else
    {        
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
        return(NX_MDNS_NO_MORE_ENTRIES);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_peer_cache_clear                            PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS peer service cache      */ 
/*    clear function call.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_mdns_peer_cache_clear             Actual peer cache clear       */
/*                                            function                    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_peer_cache_clear(NX_MDNS *mdns_ptr)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr)
    {    
        return(NX_PTR_ERROR);
    }  
    
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)        
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS create service.  */
    status =  _nx_mdns_peer_cache_clear(mdns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_peer_cache_clear                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clears the mDNS peer service cache.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_cache_initialize             Initialize the cache          */   
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_peer_cache_clear(NX_MDNS *mdns_ptr)
{
   
UINT    status;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
 
    status = _nx_mdns_cache_initialize(mdns_ptr, NX_NULL, NX_NULL, mdns_ptr -> nx_mdns_peer_service_cache, mdns_ptr -> nx_mdns_peer_service_cache_size);

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a error status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_mdns_host_address_get                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the mDNS host address get        */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    service                               Pointer to response service   */  
/*    timeout                               The timeour for service query */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_host_address_get             Actual mDNS host address      */ 
/*                                            get function                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_host_address_get(NX_MDNS *mdns_ptr, UCHAR *host_name, ULONG *ipv4_address, ULONG *ipv6_address,  UINT timeout)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if ((!mdns_ptr) || (!host_name))
    {    
        return(NX_PTR_ERROR);
    }  

    /* Check for mDNS started flag.  */
    if (!mdns_ptr -> nx_mdns_started)
    {
        return(NX_MDNS_NOT_STARTED);
    }

    /* Call actual mDNS create service.  */
    status = _nx_mdns_host_address_get(mdns_ptr, host_name, ipv4_address, ipv6_address, timeout);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_host_address_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS query record into peer buffer,          */ 
/*    mDNS thread send the query message using one-shot type.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of the service       */ 
/*    subtype                               The subtype of the service    */ 
/*    service                               Pointer to response service   */  
/*    timeout                               The timeour for service query */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */  
/*    tx_time_get                           Get the mDNS time             */ 
/*    _nx_mdns_host_check                   Check the host info           */  
/*    _nx_mdns_answer_wait                  Wait the answer               */
/*    _nx_mdns_cache_add_query              Add the query into cache      */  
/*    _nx_mdns_cache_delete_query           Delete the query from cache   */   
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_host_address_get(NX_MDNS *mdns_ptr, UCHAR *host_name, ULONG *ipv4_address, ULONG *ipv6_address, UINT timeout)
{

UINT                status;
ULONG               start_time; 
ULONG               current_time;
ULONG               elapsed_time;
ULONG               wait_time = timeout;
NX_MDNS_RR         *a_rr;
NX_MDNS_RR         *aaaa_rr;
UCHAR               host_name_query[NX_MDNS_NAME_MAX + 1];
UINT                i = 0;
UCHAR               domain_flag = NX_FALSE;
UINT                answer = NX_FALSE;
UINT                domain_name_length;


    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)host_name, NX_NULL, NX_MDNS_HOST_NAME_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), NX_WAIT_FOREVER);

    /* Clear the buffer.  */
    memset(host_name_query, 0, NX_MDNS_NAME_MAX + 1);

    /* Copy the host name into host name query buffer.  */
    while(host_name[i] != NX_NULL)
    {
        host_name_query[i] = host_name[i];
        if (host_name[i] == '.')
        {

            /* Update the flag.  */
            domain_flag = NX_TRUE;
        }
        i++;
    }

    /* Check if include the domain.  */
    if (domain_flag == NX_FALSE)
    {
        host_name_query[i++] = '.';

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)mdns_ptr -> nx_mdns_domain_name, &domain_name_length, NX_MDNS_DOMAIN_NAME_MAX))
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr->nx_mdns_mutex));
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        memcpy((char*)(&host_name_query[i]), (const char*)mdns_ptr -> nx_mdns_domain_name, domain_name_length + 1); /* Use case of memcpy is verified. The NX_MDNS_HOST_NAME_MAX and NX_MDNS_DOMAIN_NAME_MAX are limited in nxd_mdns.h. */
    }

    /* Initialize the value.  */
    if (ipv4_address)
        *ipv4_address = NX_NULL;
    if (ipv6_address)
        memset(ipv6_address, 0, 16);

    /* Start host address query on all enabled interfaces until get the answer or query timeout.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Check if this interface is enabled.  */
        if (!mdns_ptr -> nx_mdns_interface_enabled[i])
            continue;

        /* Get the query start time.  */
        wait_time = timeout;
        start_time = tx_time_get();
         
        /* Get the host IPv4 address.  */
        if (ipv4_address)
        {

            /* One shot query for host name.  */
            status = _nx_mdns_one_shot_query(mdns_ptr, host_name_query, NX_MDNS_RR_TYPE_A, &a_rr, wait_time, i);

            /* Check the status.  */
            if (status == NX_MDNS_SUCCESS)
            {

                /* Set the IPv4 address.  */
                *ipv4_address = a_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address;
                answer = NX_TRUE;
            }
        }

        /* Get the host IPv6 address.  */
        if (ipv6_address)
        {

            /* How much time has elapsed? */
            current_time = tx_time_get();

            /* Has the time wrapped? */
            if (current_time >= start_time)
            {
                /* No, simply subtract to get the elapsed time.   */
                elapsed_time =  current_time - start_time;
            }
            else
            {

                /* Yes it has. Time has rolled over the 32-bit boundary.  */
                elapsed_time =  (((ULONG) 0xFFFFFFFF) - start_time) + current_time;
            }

            /* Update the timeout.  */
            if (wait_time > elapsed_time)
                wait_time -= elapsed_time;
            else
                wait_time = 0;

            /* Lookup the service.  */
            status = _nx_mdns_one_shot_query(mdns_ptr, host_name_query, NX_MDNS_RR_TYPE_AAAA, &aaaa_rr, wait_time, i);

            /* Check the status.  */
            if (status == NX_MDNS_SUCCESS)
            {

                /* Set the IPv6 address.  */
                *ipv6_address = aaaa_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa .nx_mdns_rr_aaaa_address[0];
                *(ipv6_address + 1) = aaaa_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa .nx_mdns_rr_aaaa_address[1];
                *(ipv6_address + 2) = aaaa_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa .nx_mdns_rr_aaaa_address[2];
                *(ipv6_address + 3) = aaaa_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa .nx_mdns_rr_aaaa_address[3];
                answer = NX_TRUE;
            }
        }

        /* Check if get IPv4 address or IPv6 address.  */
        if (answer == NX_TRUE)
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
            return (NX_MDNS_SUCCESS); 
        }
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
    return (NX_MDNS_ERROR); 
}
#endif /* NX_MDNS_DISABLE_CLIENT */


#ifndef NX_MDNS_DISABLE_SERVER                              
#if !defined NX_DISABLE_IPV4 || defined NX_MDNS_ENABLE_IPV6
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_a_aaaa_add                                PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS A/AAAA resource record into local cache.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    set                                   The resource record set       */ 
/*    address                               The resource record address   */ 
/*    insert_rr                             Pointer to insert resource    */ 
/*                                            record                      */
/*    name                                  Host name                     */ 
/*    ttl                                   The resource record ttl       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */   
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into cache                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_host_name_register           Register the host name        */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_a_aaaa_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG *address, UINT addr_length, UCHAR type, UINT interface_index)
{
UINT        status;
NX_MDNS_RR *insert_rr;
USHORT      rr_type;
NX_MDNS_RR  temp_resource_record;


    /* Set the resource record type.  */
    if(addr_length == 4)
        rr_type = NX_MDNS_RR_TYPE_A;
    else
        rr_type = NX_MDNS_RR_TYPE_AAAA;

    /* Set the resource record parameters.  */
    status = _nx_mdns_rr_parameter_set(mdns_ptr, name, rr_type, NX_MDNS_RR_TTL_HOST, addr_length, NX_MDNS_RR_SET_UNIQUE, type, NX_FALSE, &temp_resource_record, interface_index);
      
    /* Check for error.  */
    if (status)
    {
        return(status);
    }
    
    /* Add the parameters.  */
    if(rr_type == NX_MDNS_RR_TYPE_A)
    {
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address = *address;
    }
    else
    {
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[0] = *address;
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[1] = *(address +1);
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[2] = *(address + 2);
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[3] = *(address + 3);
    }

    /* Add the resource record.  */
    status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, &insert_rr, NX_NULL);
     
    /* Check for error.  */
    if (status)
    {

        /* Delete the name strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);
        return(status);
    }
    
    /* Set the mDNS timer.  */
    _nx_mdns_timer_set(mdns_ptr, insert_rr, insert_rr -> nx_mdns_rr_timer_count);
    
    /* Return a successful status.  */
    return(NX_SUCCESS);
}
#endif /* !NX_DISABLE_IPV4 || NX_MDNS_ENABLE_IPV6  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_ptr_add                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS PTR resource record into local buffer.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The resource record name      */ 
/*    ttl                                   The resource record ttl       */ 
/*    set                                   The resource record set       */ 
/*    ptr_name                              The domain name               */ 
/*    insert_rr                             Pointer to insert resource    */
/*                                            record                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */
/*    _nx_mdns_timer_set                    Set the mDNS timer            */ 
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into cache                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_ptr_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG ttl, UCHAR set, UCHAR *ptr_name, UCHAR is_valid, NX_MDNS_RR **insert_rr, UINT interface_index)
{
  
UINT        status;
NX_MDNS_RR  *ptr_rr;
NX_MDNS_RR  temp_resource_record;
UINT        ptr_name_length;


    if (_nx_utility_string_length_check((CHAR *)ptr_name, &ptr_name_length, NX_MDNS_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Set the resource record parameters. Rdata length: PTR name, string should include the first'.' and last '\0'.  */
    status = _nx_mdns_rr_parameter_set(mdns_ptr, name, NX_MDNS_RR_TYPE_PTR, ttl, (ptr_name_length + 2), set, NX_TRUE, is_valid, &temp_resource_record, interface_index);
  
    /* Check for error.  */
    if (status)
    {   
        
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }    
    
    /* Add the ptr name.  */
    status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, ptr_name, ptr_name_length,
                                       (VOID **)(&temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name),
                                       NX_FALSE, NX_TRUE);
  
    /* Check for error.  */
    if (status)
    {   
        
        /* Delete the strings. */
        _nx_mdns_cache_delete_string(mdns_ptr,  NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }

    /* Add the resource record.  */
    status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, &ptr_rr, NX_NULL);
     
    /* Check for error.  */
    if (status)
    {   

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, 0);
        
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }    

    /* Set the mDNS timer.  */
    _nx_mdns_timer_set(mdns_ptr, ptr_rr, ptr_rr -> nx_mdns_rr_timer_count);

    if (insert_rr)
        *insert_rr = ptr_rr;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_srv_add                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS SRV resource record into local buffer.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  Service nstance name          */ 
/*    ttl                                   The ttl of the service        */ 
/*    set                                   The resource record set       */ 
/*    priority                              The priority of target host   */ 
/*    weights                               Service weight                */
/*    port                                  The port on this target host  */
/*    target                                The target host               */
/*    insert_rr                             Pointer to insert record      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */
/*    _nx_mdns_timer_set                    Set the mDNS timer            */ 
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into cache                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_srv_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG ttl, UCHAR set, USHORT priority, 
                                USHORT weights, USHORT port, UCHAR *target, NX_MDNS_RR **insert_rr, UINT interface_index)
{
  
UINT        status;
NX_MDNS_RR  *srv_rr;
NX_MDNS_RR  temp_resource_record;
UINT        target_length;

    if (_nx_utility_string_length_check((CHAR *)target, &target_length, NX_MDNS_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
        
    /* Set the resource record parameters. Rdata length: PRIORITY, WEIGHTS, PORT, TARGET, name string should include the first'.' and last '\0'.  */
    status = _nx_mdns_rr_parameter_set(mdns_ptr, name, NX_MDNS_RR_TYPE_SRV, ttl, target_length + 8, set, NX_TRUE, NX_FALSE, &temp_resource_record, interface_index);
  
    /* Check for error.  */
    if (status)
    {   
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }    
    
    /* Add the name.  */
    status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, target, target_length,
                                       (VOID **)(&temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target),
                                       NX_FALSE, NX_TRUE);
  
    /* Check for error.  */
    if (status)
    {   

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);
        
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }

    /* Set the SRV parameters.  */
    temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_priority = priority;
    temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_weights = weights;
    temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_port = port;
    
    /* Add the resource record.  */
    status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, &srv_rr, NX_NULL);
     
    /* Check for error.  */
    if (status)
    {   
        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target, 0);
        
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }    
    
    /* Set the mDNS timer.  */
    _nx_mdns_timer_set(mdns_ptr, srv_rr, srv_rr -> nx_mdns_rr_timer_count);

    if (insert_rr)
        *insert_rr = srv_rr;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_txt_add                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS TXT resource record into local buffer.  */ 
/*    the TXT records are formatted in a "key=value" notation with ";"    */
/*    acting as separator when more then one key is available.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The resource record name      */ 
/*    ttl                                   The resource record ttl       */ 
/*    set                                   The resource record set       */ 
/*    txt                                   The txt string                */ 
/*    insert_rr                             Pointer to insert record      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into the cache              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_txt_add(NX_MDNS *mdns_ptr, UCHAR *name, ULONG ttl, UCHAR set, UCHAR *txt, NX_MDNS_RR **insert_rr, UINT interface_index)
{
  
UINT        status;
NX_MDNS_RR  *txt_rr;
NX_MDNS_RR  temp_resource_record;
UINT        txt_length;


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
        
    /* Set the resource record parameters.  */
    status = _nx_mdns_rr_parameter_set(mdns_ptr, name, NX_MDNS_RR_TYPE_TXT, ttl, 0, set, NX_TRUE, NX_FALSE, &temp_resource_record, interface_index);
  
    /* Check for error.  */
    if (status)
    {   
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }    
    
    if (txt)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)txt, &txt_length, NX_MDNS_NAME_MAX))
        {

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Calculate the text string.inlcude the one byte label.  */
        temp_resource_record.nx_mdns_rr_rdata_length = (USHORT)(txt_length + 1);

        /* Add the name.  */
        status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, txt, txt_length,
                                           (VOID **)(&temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data),
                                           NX_FALSE, NX_TRUE);

        /* Check for error.  */
        if (status)
        {   

            /* Delete the same strings. */
            _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);

            /* Release the mDNS mutex.  */
            tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

            return(status);
        }
    }
    else
    {
        temp_resource_record.nx_mdns_rr_rdata_length = 1;
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data = NX_NULL;
    }
       
    /* Add the resource record.  */
    status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, &txt_rr, NX_NULL);
     
    /* Check for error.  */
    if (status)
    {     

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr,NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);

        if (txt)
        {

            /* Delete the same strings. */
            _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data, 0);
        }
        
        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        return(status);
    }    
    
    /* Set the mDNS timer.  */
    _nx_mdns_timer_set(mdns_ptr, txt_rr, txt_rr -> nx_mdns_rr_timer_count);

    if (insert_rr)
        *insert_rr = txt_rr;

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_nsec_add                                  PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS A resource record into local cache.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The resource record name      */ 
/*    add_a                                 Flag for adding A record      */ 
/*    add_aaaa                              Flag for adding AAAA record   */ 
/*    type                                  Type for host or service      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */   
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*    _nx_mdns_cache_add_string             Add the string into cache     */ 
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into cache                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_host_name_register           Register the host name        */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_nsec_add(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR add_a, UCHAR add_aaaa, UCHAR type, UINT interface_index)
{
UINT        status;
NX_MDNS_RR *insert_rr;
UCHAR       bitmap_length = 0;
ULONG       rdata_length = 0;
NX_MDNS_RR  temp_resource_record;
UINT        name_length;


    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)name, &name_length, NX_MDNS_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Add the next domain name, include the first'.' and last '\0'.  */
    rdata_length = name_length + 2;

    /* Add the window block and bitmap length.  */
    rdata_length += 2;

    /* Add the NSEC with A/AAAA Bitmap.  */
    if (type == NX_MDNS_ADD_NSEC_FOR_HOST)
    {
        /* Set the Bitmaps length.  */
        if (add_aaaa == NX_TRUE)
            bitmap_length = 4;
        else    
            bitmap_length = 1;
    }
    else
    {

        /* Add the NSEC with SRV/TXT Bitmap.   */
        bitmap_length = 5;
    }

    /* Add the Bitmaps.  */
    rdata_length += bitmap_length;

    /* Set the resource record parameters.  */
    status = _nx_mdns_rr_parameter_set(mdns_ptr, name, NX_MDNS_RR_TYPE_NSEC, NX_MDNS_RR_TTL_HOST, rdata_length,
                                       NX_MDNS_RR_SET_UNIQUE, NX_FALSE, NX_TRUE, &temp_resource_record, interface_index);
      
    /* Check for error.  */
    if (status)
    {   
        return(status);
    }
              
    /* Add the next domain name.  */
    status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, name, name_length,
                                       (VOID **)(&temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_next_domain),
                                       NX_FALSE, NX_TRUE);
                            
    /* Check for error.  */
    if (status)
    {   
        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);

        return(status);
    }

    /* Set the window block and bitmap length.  */
    temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_window_block = 0;
    temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length = bitmap_length;

    /* Add the Bitmap for host.  */
    if (type == NX_MDNS_ADD_NSEC_FOR_HOST)
    {

        /* Encode the Bitmap.  */
        if ((add_a == NX_TRUE) && (add_aaaa == NX_TRUE))
        {

            /* Add the A Bitmap.  */
            temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[0] = 64;

            /* Add the AAAA Bitmap.  */
            temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[3] = 8;
        }
        else if ((add_a == NX_TRUE))
        {         

            /* Add the A Bit map.  */
            temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[0] = 64;
        }
        else
        {       

            /* Add the AAAA Bitmap.  */
            temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[3] = 8;
        }
    }
    else
    {
        /* Add the TXT Bitmap for service.  */
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[2] =  128; 

        /* Add the SRV Bitmap for service.  */
        temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[4] =  64;

    }

    /* Add the resource record.  */
    status = _nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, &insert_rr, NX_NULL);
     
    /* Check for error.  */
    if (status)
    {   
        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_name, 0);   

        /* Delete the same strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, temp_resource_record.nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_next_domain, 0);

        return(status);
    }
    
    /* Return a successful status.  */
    return(NX_SUCCESS);
}
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_parameter_set                           PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the parameters of resource record.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The resource record name      */ 
/*    type                                  The resource record type      */  
/*    ttl                                   The resource record ttl       */
/*    rdata_lenth                           The resource record length    */  
/*    set                                   The resource record set       */  
/*    is_register                           Register flag                 */  
/*    is_valid                              Valid flag                    */ 
/*    rr_record                             Pointer to resource record    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    None                                                                */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_parameter_set(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, ULONG ttl, UINT rdata_length,
                                      UCHAR set, UCHAR is_register, UCHAR is_valid, NX_MDNS_RR *rr_record, UINT interface_index)
{

UINT        status;
UINT        name_length;


    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)name, &name_length, NX_MDNS_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Initialize the struct.  */
    memset(rr_record, 0, sizeof(NX_MDNS_RR));

    /* Add the name.  */
    status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, name, name_length,
                                       (VOID **)(&rr_record -> nx_mdns_rr_name), NX_FALSE, NX_TRUE);

    /* Check for error.  */
    if (status)
        return(status);

    /* Set the parameters.  */
    rr_record -> nx_mdns_rr_interface_index = (UCHAR)interface_index;
    rr_record -> nx_mdns_rr_type = type;
    rr_record -> nx_mdns_rr_class = NX_MDNS_RR_CLASS_IN;
    rr_record -> nx_mdns_rr_ttl = ttl;
    rr_record -> nx_mdns_rr_rdata_length = (USHORT)rdata_length;  
    
    /* Set the resource record as uniqure.  */
    if (set == NX_MDNS_RR_SET_UNIQUE)
        rr_record -> nx_mdns_rr_word = (rr_record -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_UNIQUE);

    /* Check for mDNS started flag.  */
    if (mdns_ptr -> nx_mdns_started)
    {
        
        /* Check the valid flag for PTR resource record.  */
        if (is_valid == NX_TRUE)
        {
            /* Set the resource record state.  */
            rr_record -> nx_mdns_rr_state = NX_MDNS_RR_STATE_VALID;
        }
        else
        {
            
            /* The is_register value be used for A/AAAA type, the value being NX_TRUE indicates the operation is to register the host name(probing and announcing the A/AAAA). The value being NX_FALSE indicates the operation is an address change, (announcing the A/AAAA).  */
            if ((set == NX_MDNS_RR_SET_UNIQUE) && (is_register == NX_TRUE))
            {

                /* Set the resource record status.  */
                rr_record -> nx_mdns_rr_state = NX_MDNS_RR_STATE_PROBING;
                rr_record -> nx_mdns_rr_timer_count = mdns_ptr -> nx_mdns_first_probing_delay;
                rr_record -> nx_mdns_rr_retransmit_count = NX_MDNS_PROBING_RETRANSMIT_COUNT;
            }    
            else
            {

                /* Set the resource record state.  */
                rr_record -> nx_mdns_rr_state = NX_MDNS_RR_STATE_ANNOUNCING;
                rr_record -> nx_mdns_rr_timer_count = NX_MDNS_ANNOUNCING_TIMER_COUNT;
                rr_record -> nx_mdns_rr_retransmit_lifetime = mdns_ptr -> nx_mdns_announcing_period;                

                /* Check the announcing max time.  */
                if (mdns_ptr -> nx_mdns_announcing_max_time != NX_MDNS_ANNOUNCING_FOREVER)
                    rr_record -> nx_mdns_rr_announcing_max_time = (UCHAR)(mdns_ptr -> nx_mdns_announcing_max_time - 1);
                else
                    rr_record -> nx_mdns_rr_announcing_max_time = NX_MDNS_ANNOUNCING_FOREVER;

                /* Set the retransmit count.  */
                if (mdns_ptr -> nx_mdns_announcing_retrans_interval)
                    rr_record -> nx_mdns_rr_retransmit_count = mdns_ptr -> nx_mdns_announcing_count;
                else
                    rr_record -> nx_mdns_rr_retransmit_count = 1;
            }
        }
    }
    else
    {
        
        /* Set the resource record status.  */
        rr_record -> nx_mdns_rr_state = NX_MDNS_RR_STATE_SUSPEND;
    }

    return(NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_SERVER */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_delete                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the mDNS resource record from the             */ 
/*    local bufferor remote buffer according to the resource record set.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    record_rr                             The resource record           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                            from cache                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_rr_delete(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_rr)
{
  
UINT        status = NX_MDNS_SUCCESS; 

#ifndef NX_MDNS_DISABLE_CLIENT
ULONG       *head;
NX_MDNS_RR  *p;        
#endif /* NX_MDNS_DISABLE_CLIENT */

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    

    /* Check for mDNS started flag.  */
    if (mdns_ptr -> nx_mdns_started)
    {

        /* Check the RR owner.  */
        if (!(record_rr -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_PEER))
        {

#ifndef NX_MDNS_DISABLE_SERVER
            if ((record_rr -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                (record_rr -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING))
            {

                /* Set the state to send Goodbye packet.  */
                record_rr -> nx_mdns_rr_state = NX_MDNS_RR_STATE_GOODBYE;

                /* Set the retransmit count.  */
                record_rr -> nx_mdns_rr_retransmit_count = NX_MDNS_GOODBYE_RETRANSMIT_COUNT;

                /* Set the timer count.  */
                record_rr -> nx_mdns_rr_timer_count = NX_MDNS_GOODBYE_TIMER_COUNT;

                /* Set the delete flag.  */
                record_rr -> nx_mdns_rr_word = (record_rr -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_DELETE);

                /* Set the mDNS timer.  */
                _nx_mdns_timer_set(mdns_ptr, record_rr, record_rr -> nx_mdns_rr_timer_count);
            }
            else            
            {

                /* Delete the resource records.  */
                status = _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, record_rr);
            }

#endif /* NX_MDNS_DISABLE_SERVER */
        }
        else
        {

#ifndef NX_MDNS_DISABLE_CLIENT
            if (record_rr -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY)
            {

                /* Set the pointer.  */            
                head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;

                if(head)
                {
                    head = (ULONG*)(*head);

                    /* Check the remote resource record, stop the updating resource record.  */
                    for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_peer_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
                    {

                        if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
                            continue;

                        if (p == record_rr)
                            continue;

                        if ((p -> nx_mdns_rr_name  == record_rr -> nx_mdns_rr_name) &&
                            (p -> nx_mdns_rr_type == record_rr -> nx_mdns_rr_type) &&
                            (p -> nx_mdns_rr_class == record_rr -> nx_mdns_rr_class))
                        {

                            if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID)
                            {

                                /* Clear the Updating flag.  */
                                p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_UPDATING));
                            }

                            if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_UPDATING) 
                            {
                                if (p -> nx_mdns_rr_retransmit_count)
                                {

                                    /* mDNS querier need not update the RR, The original updating ttl is at 80%, 85%, 90%, 95%. Update the timer.*/
                                    /* timer_count: P, timer_count: C, rr_ttl: T.
                                    P = P + C * T * 5%.  */
                                    p -> nx_mdns_rr_timer_count = (p -> nx_mdns_rr_timer_count + p -> nx_mdns_rr_retransmit_count * p -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * 5 /100);

                                    /* Clear the retransmit count.  */
                                    p -> nx_mdns_rr_retransmit_count = 0;

                                    /* Set the mDNS timer.  */
                                    _nx_mdns_timer_set(mdns_ptr, p, p -> nx_mdns_rr_timer_count);
                                }
                            }
                        }
                    }
                }
            }

            /* Delete the resource record.  */
            status = _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, record_rr);

#endif /* NX_MDNS_DISABLE_CLIENT */
        }
    }
    else
    {
        /* Check the RR owner.  */
        if (!(record_rr -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_PEER))
        {

#ifndef NX_MDNS_DISABLE_SERVER
            status = _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, record_rr);
#endif /* NX_MDNS_DISABLE_SERVER  */
        }
        else
        {

#ifndef NX_MDNS_DISABLE_CLIENT
            status = _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, record_rr);
#endif /* NX_MDNS_DISABLE_CLIENT  */
        }
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return status.  */
    return (status);
}


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_local_cache_clear                            PORTABLE C    */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the mDNS instance.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_mdns_local_cache_clear            Actual local cache clear      */  
/*                                            function                    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_mdns_local_cache_clear(NX_MDNS *mdns_ptr)
{
        
UINT    status;


    /* Check for invalid input pointers.  */
    if (!mdns_ptr) 
    {   
        return(NX_PTR_ERROR);
    }  
    
    /* Check for invalid server attributes. */
    if (mdns_ptr -> nx_mdns_id != NX_MDNS_ID)        
    {
        return(NX_MDNS_PARAM_ERROR);
    }

    /* Call actual mDNS local cache clear service.  */
    status =  _nx_mdns_local_cache_clear(mdns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_local_cache_clear                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clears the mDNS local cache.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */ 
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                            from cache                  */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_local_cache_clear(NX_MDNS *mdns_ptr)
{

UINT            status = NX_MDNS_SUCCESS;
ULONG           *head;
NX_MDNS_RR      *p;      


    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);
    
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
    head = (ULONG*)(*head);

    for (p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {   

        /* Delete the resource records.  */
        status = _nx_mdns_rr_delete(mdns_ptr, p);

        /* Check the status.  */
        if (status)
            break;
    }

    /* Release the mDNS mutex.  */
    tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

    /* Return a successful status.  */
    return(status);
}
#endif /* NX_MDNS_DISABLE_SERVER  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_name_resolve                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resloves the service name, type, domain.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    srv_name                              Pointer to SRV name           */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of to the service    */ 
/*    domain                                The domain                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    mDNS component                                                      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_service_name_resolve(UCHAR *srv_name, UCHAR **name, UCHAR **type, UCHAR **domain)
{
 
UINT    i;
UINT    count;
UINT    prepend_index;
UINT    append_index;
UINT    type_index;
UINT    domain_index;
UCHAR   *pointer;
UINT     protocol_length;


    /* Initialize the parameters.  */
    count = 0;
    prepend_index = 0;
    append_index = 0;
    type_index = 0;
    domain_index = 0;
    
    /* Set the service name pointer.  */
    pointer = srv_name;

    /* Service Instance name,  */
    /* <Instance>.<sn>._tcp.<domain> : 
       name: <Instance>
       type: <sn>._tcp 
       domain: <domain>.  */
    while (*srv_name != '\0')
    {

        /* Update the index.  */
        append_index ++;

        if (*srv_name == '.')
        {

            /* Set the value.  */
            i = 0;            

            while (nx_mdns_app_protocol[i])
            {

                /* Check string length.  */
                if (_nx_utility_string_length_check(nx_mdns_app_protocol[i], &protocol_length, NX_MDNS_TYPE_MAX))
                {
                    continue;
                }

                /* Find the key string. "_tcp" , "_udp" etc.  */
                if (!_nx_mdns_name_match(pointer + prepend_index, (UCHAR *)(nx_mdns_app_protocol[i]), protocol_length))
                {

                    /* Get the <domain> pointer.  */
                    /* EL-PC.Test._http._tcp.local
                       type index pointer to the _http._tcp, 
                       domain_index pointer to the local.  */
                    domain_index = append_index;

                    /* Set the name pointer.  */
                    if (type_index)
                    {
                        *name = pointer;

                        /* Set the trailing null of name.  */
                        *(pointer + type_index -1) = '\0';
                    }
                    else
                    {
                        *name = NX_NULL;
                    }
                    
                    /* Set the type pointer.  */
                    *type = pointer + type_index;

                    /* Set the trailing null of type.  */
                    *(pointer + domain_index -1) = '\0';
                    
                    /* Set the type pointer.  */
                    *domain = pointer + domain_index;

                    return(NX_MDNS_SUCCESS);
                }
                i ++;
            } 

            /* Update the '.' count.  */
            count ++;
            
            /* The type index is pointer to the <sn>.  */
            if (count != 1)
            {

                /* Update the type index.  */
                type_index = prepend_index;
            }
            
            prepend_index = append_index;
        }

        srv_name ++;
    }

    return(NX_MDNS_ERROR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_name_assemble                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resloves the service name, type, domain.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    srv_name                              Pointer to SRV name           */ 
/*    name                                  The name of the service       */ 
/*    type                                  The type of to the service    */ 
/*    domain                                The domain                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    mDNS component                                                      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_service_name_assemble(UCHAR *name, UCHAR *type, UCHAR *sub_type, UCHAR *domain, UCHAR *record_buffer, UINT buffer_size, UINT *type_index)
{
 
UINT        index;
UINT        length;

    /* Initialize the struct.  */
    index = 0;
    memset(record_buffer, 0, buffer_size);
    if (type_index)
    {
        *type_index = 0;
    }

    /* Construct the service name.  */

    /* Add the name.  */
    if (name)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)name, &length, NX_MDNS_NAME_MAX))
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Verify buffer length. */
        if (index + length + 1 > buffer_size)
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        memcpy((CHAR *)(record_buffer + index), (const char*)name, length); /* Use case of memcpy is verified. */
        index += length;

        *(record_buffer + index) = '.';
        index ++;   
    }

    /* Add the sub type.  */
    if (sub_type)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)sub_type, &length, NX_MDNS_LABEL_MAX))
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Verify buffer length. */
        if (index + length + 1 + 4 + 1 > buffer_size)
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        if (type_index)
        {
            *type_index = index;
        }
        memcpy((CHAR *)(record_buffer + index), (const char*)sub_type, length); /* Use case of memcpy is verified. */
        index += length;
        *(record_buffer + index) = '.';
        index ++;     
        
        /* Add the key word "_sub".  */
        memcpy((CHAR *)(record_buffer + index), (const char*)"_sub", 4); /* Use case of memcpy is verified. */
        index += 4;
        *(record_buffer + index) = '.';
        index ++; 
    }

    /* Add the type.  */
    if (type)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)type, &length, NX_MDNS_TYPE_MAX))
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Verify buffer length. */
        if (index + length + 1 > buffer_size)
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Set the type index.  */
        if ((type_index) &&
            (!sub_type))
        {
            *type_index = index;
        }

        memcpy((CHAR *)record_buffer + index, (const char*)type, length); /* Use case of memcpy is verified. */
        index += length;
        *(record_buffer + index) = '.';
        index ++;    
    }

    /* Add the domain. */
    if (domain)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)domain, &length, NX_MDNS_DOMAIN_NAME_MAX))
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Verify buffer length. */
        if (index + length > buffer_size)
        {
            return (NX_MDNS_DATA_SIZE_ERROR);
        }

        memcpy((CHAR *)(record_buffer + index), (const char*)domain, length); /* Use case of memcpy is verified. */
        index += length;
    }

    return(NX_MDNS_SUCCESS);
}


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    nx_mdns_host_name_register                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resloves the service name, type, domain.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    type                                  The type of operation         */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_rr_a_aaaa_add                Add the A/AAAA resource record*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_enable                       Enable mDNS                   */
/*    _nx_mdns_address_change_process       Process address change        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_host_name_register(NX_MDNS *mdns_ptr, UCHAR type, UINT interface_index)
{

#if !defined NX_DISABLE_IPV4 || defined NX_MDNS_ENABLE_IPV6
UINT    status;
#endif /* !NX_DISABLE_IPV4 || NX_MDNS_ENABLE_IPV6 */
NX_IP   *ip_ptr;
UINT    index = 0;
UCHAR   add_a = NX_FALSE;
UCHAR   add_aaaa = NX_FALSE;
UINT    host_name_length;

#ifdef NX_MDNS_ENABLE_IPV6
NXD_IPV6_ADDRESS *ipv6_address_ptr;
#endif /* NX_MDNS_ENABLE_IPV6  */


#if defined NX_DISABLE_IPV4 && !defined NX_MDNS_ENABLE_IPV6
    NX_PARAMETER_NOT_USED(type);
#endif /* NX_DISABLE_IPV4 && ! NX_MDNS_ENABLE_IPV6  */

    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)mdns_ptr -> nx_mdns_host_name, &host_name_length, NX_MDNS_HOST_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Construct the A/AAAA name.  */
    memcpy((CHAR *)(&temp_string_buffer[index]), (const char*)mdns_ptr -> nx_mdns_host_name, host_name_length); /* Use case of memcpy is verified. */
    index += host_name_length;

    temp_string_buffer[index] = '.';
    index ++;    
    memcpy((CHAR *)(&temp_string_buffer[index]), (CHAR *)"local", sizeof("local")); /* Use case of memcpy is verified. The NX_MDNS_HOST_NAME_MAX is limited in nxd_mdns.h.*/

    /* Set the ip pointer.  */
    ip_ptr = mdns_ptr -> nx_mdns_ip_ptr;

    /* Get the IP mutex.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifndef NX_DISABLE_IPV4
    /* Add the A resource records message.  */
    if (ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_address)
    {

        /* Add the A resource records message.  */
        status = _nx_mdns_rr_a_aaaa_add(mdns_ptr, &temp_string_buffer[0], &(ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_address),
                                        NX_MDNS_IPV4_ADDRESS_LENGTH, type, interface_index);

        /* Check the status.  */
        if (status)
        {

            /* Release the IP mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            return(status);
        }

        /* Set the flag.  */
        add_a = NX_TRUE;
    }
#endif /* NX_DISABLE_IPV4  */

#ifdef NX_MDNS_ENABLE_IPV6
    /* Get the IPv6 address pointer.*/
    ipv6_address_ptr = ip_ptr -> nx_ip_interface[interface_index].nxd_interface_ipv6_address_list_head;

    while(ipv6_address_ptr)
    {

        if (ipv6_address_ptr -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID)
        {

            /* Add the AAAA resource records message.  */
            status = _nx_mdns_rr_a_aaaa_add(mdns_ptr, &temp_string_buffer[0], (ULONG *)(&ipv6_address_ptr -> nxd_ipv6_address[0]),
                                            NX_MDNS_IPV6_ADDRESS_LENGTH, type, interface_index);

            /* Check the status.  */
            if (status)
            {

                /* Release the IP mutex.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                return(status);
            }                   

            /* Set the flag.  */
            add_aaaa = NX_TRUE;
        }

        /* Update the pointer.  */
        ipv6_address_ptr = ipv6_address_ptr -> nxd_ipv6_address_next;
    }
#endif /* NX_MDNS_ENABLE_IPV6  */

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES

    /* Add the NSEC resource record.  */
    if ((add_a == NX_TRUE) || (add_aaaa == NX_TRUE))
        _nx_mdns_rr_nsec_add(mdns_ptr, &temp_string_buffer[0], add_a, add_aaaa, NX_MDNS_ADD_NSEC_FOR_HOST, interface_index);
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

    /* Release the IP mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_SERVER */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_timer_entry                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles waking up the mDNS helper thread to process   */
/*    the timer event on a periodic basis.                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_value                            mDNS instance for a ulong     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX system timer thread                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_timer_entry(ULONG mdns_value)
{
    
NX_MDNS     *mdns_ptr;


    /* Setup mDNS pointer from the input value.  */
    mdns_ptr = (NX_MDNS *) mdns_value;
    
    /* Set the timer event.  */
    tx_event_flags_set(&(mdns_ptr -> nx_mdns_events), NX_MDNS_TIMER_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_timer_set                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process all resource records according to the state,  */
/*    Set the flags to send the probing, announcing, query, response and  */ 
/*    goodbye mDNS message. Delete the resource record when the resource  */ 
/*    record is invalid.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    tx_timer_info_get                     Get the timer info            */
/*    tx_timer_deactivate                   Deactivate the timer          */
/*    tx_timer_change                       Change the timer              */
/*    tx_timer_activate                     Activate the timer            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_enable                       Enable the mDNS               */
/*    _nx_mdns_rr_a_aaaa_add                Add the A/AAAA resource record*/
/*    _nx_mdns_rr_ptr_add                   Add the ptr resource record   */
/*    _nx_mdns_rr_srv_add                   Add the srv resource record   */
/*    _nx_mdns_rr_txt_add                   Add the txt resource record   */
/*    _nx_mdns_rr_delete                    Delete the resource record    */
/*    _nx_mdns_local_cache_clear            Clear the local cache         */
/*    _nx_mdns_query                        Send the continuous query     */
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*    _nx_mdns_conflict_process             Process the conflict          */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_timer_set(NX_MDNS *mdns_ptr, NX_MDNS_RR  *record_rr, ULONG timer_count)
{
      

TX_INTERRUPT_SAVE_AREA
ULONG       remaining_ticks;
ULONG       schedule_count;
UINT        active;

    if (timer_count)
    {
        
        /* Disable interrupt */
        TX_DISABLE

        /* Get the remaining_ticks of mDNS timer;*/
        tx_timer_info_get(&mdns_ptr -> nx_mdns_timer, TX_NULL, &active, &remaining_ticks, TX_NULL, TX_NULL);

        /* If the timer is not active, the remaining ticks and schedule count are set to 0. */
        if(active == NX_FALSE)
        {
            remaining_ticks = 0;
            schedule_count = 0;
        }
        else
        {

            /* Get the reschedule timer count of last timer.  */
            schedule_count = mdns_ptr -> nx_mdns_timer_min_count;
        }

        /* Check the timer.  if exceed the timer count range, update the timer. */
        if((timer_count < remaining_ticks) || active == NX_FALSE)
        {

            /* Set the minimum timer count.
            The minimum timer count indicate the intervals between last timer event and next timer event. */
            mdns_ptr -> nx_mdns_timer_min_count = (schedule_count - remaining_ticks) + timer_count;

            /* Change the timer ticks with minimum timer count.*/
            tx_timer_deactivate(&(mdns_ptr -> nx_mdns_timer));
            tx_timer_change(&(mdns_ptr -> nx_mdns_timer), timer_count, timer_count );
            tx_timer_activate(&(mdns_ptr -> nx_mdns_timer));
        }

        /* Update the Resource record life timer count*/
        record_rr -> nx_mdns_rr_timer_count = (schedule_count - remaining_ticks) + timer_count;

        /* Restore interrupts.  */
        TX_RESTORE
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_timer_event_process                        PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process all resource records according to the state,  */
/*    Set the flags to send the probing, announcing, query, response and  */ 
/*    goodbye mDNS message. Delete the resource record when the resource  */ 
/*    record is invalid.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags to wake mDNS  */
/*    tx_timer_deactivate                   Deactivate the timer          */
/*    tx_timer_change                       Change the timer              */
/*    tx_timer_activate                     Activate the timer            */ 
/*    _nx_mdns_service_name_resolve         Resolve the service name      */
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                            from the cache              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_thread_entry                 Processing thread for mDNS    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_timer_event_process(NX_MDNS *mdns_ptr)
{
 
ULONG       event_flags = 0;
ULONG       timer_min_count = 0xFFFFFFFF;
ULONG       *head, *tail;
NX_MDNS_RR  *p;
UCHAR       i;

#ifndef NX_MDNS_DISABLE_CLIENT
ULONG       remaining_ticks;
#endif /* NX_MDNS_DISABLE_CLIENT  */

#ifndef NX_MDNS_DISABLE_SERVER
UINT        status;
NX_MDNS_RR  *p1;
UCHAR       *service_name;
UCHAR       *service_type;
UCHAR       *service_domain;
UINT        rr_name_length;
#endif /* NX_MDNS_DISABLE_SERVER */


    for(i = 0; i < 2; i++)
    {

        /* Set the pointer. */
        if(i == NX_MDNS_CACHE_TYPE_LOCAL)
        {
#ifndef NX_MDNS_DISABLE_SERVER
            head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
#else
            continue;
#endif /* NX_MDNS_DISABLE_SERVER */
        }
        else
        {
#ifndef NX_MDNS_DISABLE_CLIENT
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
#else
            continue;
#endif /* NX_MDNS_DISABLE_CLIENT  */
        }

        if(head == NX_NULL)
            continue;

        tail = (ULONG*)(*head);

        /* Check the remote resource record lifetime.  */
        for(p = (NX_MDNS_RR*)(head + 1); (ULONG*)p < tail; p++)
        {

            if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
                continue;

            /* Update the remaining ticks of peer resource record.  */
            if (((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                 (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_UPDATING)))
            {
                if (p -> nx_mdns_rr_remaining_ticks > mdns_ptr -> nx_mdns_timer_min_count)
                {
                    p -> nx_mdns_rr_remaining_ticks -= mdns_ptr -> nx_mdns_timer_min_count;                    
                }
                else
                {
                    p -> nx_mdns_rr_remaining_ticks = 0;
                }
            }

            /* Calculate the time interval for two responses.*/
            if (p -> nx_mdns_rr_response_interval > mdns_ptr -> nx_mdns_timer_min_count)
            {
                p -> nx_mdns_rr_response_interval = (ULONG)(p -> nx_mdns_rr_response_interval - mdns_ptr -> nx_mdns_timer_min_count);

                /* Compare the timer count.and set the minimum timer count. */
                if ((p -> nx_mdns_rr_response_interval != 0) &&
                    (p -> nx_mdns_rr_response_interval < timer_min_count))
                {
                    timer_min_count = p -> nx_mdns_rr_response_interval;
                }
            }
            else
            {
                p -> nx_mdns_rr_response_interval = 0;
            }

            /* If the timer count is zero, means do not do anything */
            if (p -> nx_mdns_rr_timer_count == 0)
                continue;

            if (p -> nx_mdns_rr_timer_count > mdns_ptr -> nx_mdns_timer_min_count)
            {
                p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_timer_count - mdns_ptr -> nx_mdns_timer_min_count;

                /* Check the timer count range.  */
                if (p -> nx_mdns_rr_timer_count <= NX_MDNS_TIMER_COUNT_RANGE)
                {
                    p -> nx_mdns_rr_timer_count = 0;
                }
                else
                {
                    /* Compare the timer count.and set the minimum timer count. */
                    if (p -> nx_mdns_rr_timer_count < timer_min_count)
                    {
                        timer_min_count = p -> nx_mdns_rr_timer_count;
                    }
                }
            }
            else
            {
                p -> nx_mdns_rr_timer_count = 0;
            }
            
            /* If the timer count is zero, means the timeout. */
            if (p -> nx_mdns_rr_timer_count != 0)
                continue;

            /* Check the state. */
            switch (p -> nx_mdns_rr_state)
            {
                
#ifndef NX_MDNS_DISABLE_SERVER
                case NX_MDNS_RR_STATE_PROBING:
                {

                    /* mDNS Server probing the RRs at 0ms, 250ms, 500ms. RFC6762, Section8.1, Page26.  */
                    if (p -> nx_mdns_rr_retransmit_count)
                    {

                        /* Set the send flag.*/
                        p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST; 

                        /* Set the probing flag.  */
                        event_flags = event_flags | NX_MDNS_PROBING_SEND_EVENT; 

                        /* Set the timer count. 250ms.  */
                        p -> nx_mdns_rr_timer_count = NX_MDNS_PROBING_TIMER_COUNT;

                        /* Compare the timer count.and set the minimum timer count. */
                        if (p -> nx_mdns_rr_timer_count < timer_min_count)
                        {
                            timer_min_count = p -> nx_mdns_rr_timer_count;
                        }
                    }
                    else            
                    {

                        /* Probing complete, and move to the next step, Announcing. */
                        p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_ANNOUNCING;

                        /* Set the first announcing timer interval.  */
                        p -> nx_mdns_rr_timer_count = NX_MDNS_ANNOUNCING_TIMER_COUNT;

                        /* Set the retransmit count.  */
                        if (mdns_ptr -> nx_mdns_announcing_retrans_interval)
                            p -> nx_mdns_rr_retransmit_count = mdns_ptr -> nx_mdns_announcing_count;
                        else
                            p -> nx_mdns_rr_retransmit_count = 1;

                        /* Set the next timer count.  */
                        p -> nx_mdns_rr_retransmit_lifetime = mdns_ptr -> nx_mdns_announcing_period;

                        /* Check the announcing max time.  */
                        if (mdns_ptr -> nx_mdns_announcing_max_time != NX_MDNS_ANNOUNCING_FOREVER)
                            p -> nx_mdns_rr_announcing_max_time = (UCHAR)(mdns_ptr -> nx_mdns_announcing_max_time - 1);
                        else
                            p -> nx_mdns_rr_announcing_max_time = NX_MDNS_ANNOUNCING_FOREVER;
                        
                        /* Yes, probing complete, add the related PTR resource records.  */
                        if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV)
                        {

                            /* Check string length.  */
                            if (_nx_utility_string_length_check((CHAR *)p -> nx_mdns_rr_name, &rr_name_length, NX_MDNS_NAME_MAX))
                            {
                                break;
                            }

                            /* Store the service name.  */
                            memcpy((CHAR *)(&temp_string_buffer[0]), (const char*)p -> nx_mdns_rr_name, rr_name_length + 1); /* Use case of memcpy is verified. */

                            /* Reslove the service name.  */
                            status = _nx_mdns_service_name_resolve(&temp_string_buffer[0], &service_name, &service_type, &service_domain);

                            /* Check the status.  */
                            if (status)
                            {
                                break;
                            }

                            /* Service name registered success, invoke the notify function.  */
                            if (mdns_ptr -> nx_mdns_probing_notify)
                            {
                                (mdns_ptr -> nx_mdns_probing_notify)(mdns_ptr, service_name, NX_MDNS_LOCAL_SERVICE_REGISTERED_SUCCESS);
                            }  
                        }

                        if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A)
                        {

                            /* Service name registered success, invoke the notify function.  */
                            if (mdns_ptr -> nx_mdns_probing_notify)
                            {
                                (mdns_ptr -> nx_mdns_probing_notify)(mdns_ptr, mdns_ptr -> nx_mdns_host_name, NX_MDNS_LOCAL_HOST_REGISTERED_SUCCESS);
                            }
                        }

                        /* Probing complete, Has been wating for 250ms, Directly announcing the resource record.  */
                        p --;
                    }
                    break;
                }

                case NX_MDNS_RR_STATE_ANNOUNCING:
                {

                    /* mDNS Server Announcing the Resource records.  */
                    if (p -> nx_mdns_rr_retransmit_count)
                    {

                        /* Set the send flag to send the probing. */
                        p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;

                        /* Set the announcing flag.  */
                        event_flags = event_flags | NX_MDNS_ANNOUNCING_SEND_EVENT; 

                        /* Yes, probing complete, add the related PTR resource records and NSEC resource records. */
                        if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) || 
                            (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A) || 
                            (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_AAAA))
                        {

                            for (p1 = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p1 < tail; p1++)
                            {

                                /* Check the state.  */
                                if (p1 -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
                                    continue;

                                /* Check the interface.  */
                                if (p1 -> nx_mdns_rr_interface_index != p -> nx_mdns_rr_interface_index)
                                    continue;

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                                if ((p1 ->nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC) &&
                                    (p1 -> nx_mdns_rr_name == p -> nx_mdns_rr_name)) 
                                {
                                     
                                    /* Set the send flag to send the probing. */
                                    p1 -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;
                                }
#endif /*NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

                                if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) &&
                                    (p1 -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) &&
                                    (p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name == p -> nx_mdns_rr_name))
                                {                                            

                                    /* Set the send flag to send the probing. */
                                    p1 -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;
                                }
                            }
                        }

                        /* Check last the retransmit in this period announcing.  */
                        if (p -> nx_mdns_rr_retransmit_count - 1)
                        {

                            if (mdns_ptr -> nx_mdns_announcing_retrans_interval)
                            {

                                /* Set the timer count.  */
                                p -> nx_mdns_rr_timer_count = mdns_ptr -> nx_mdns_announcing_retrans_interval;

                                /* Compare the timer count and set the minimum timer count. */
                                if (p -> nx_mdns_rr_timer_count < timer_min_count)
                                {
                                    timer_min_count = p -> nx_mdns_rr_timer_count;
                                }
                            }
                        }
                        else
                        {
                            if (p -> nx_mdns_rr_announcing_max_time)
                            {

                                /* Update the timer count.  */
                                p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_retransmit_lifetime;

                                /* Calculate the next timer count.   */
                                p -> nx_mdns_rr_retransmit_lifetime = (ULONG)(p -> nx_mdns_rr_retransmit_lifetime * (ULONG)(2 << (mdns_ptr -> nx_mdns_announcing_factor - 1)));

                                /* Check the announcing period time interval.  */
                                if (p -> nx_mdns_rr_retransmit_lifetime > mdns_ptr -> nx_mdns_announcing_period_interval)
                                    p -> nx_mdns_rr_retransmit_lifetime = mdns_ptr -> nx_mdns_announcing_period_interval;

                                /* Set the retransmit count.  */
                                if (mdns_ptr -> nx_mdns_announcing_retrans_interval)
                                    p -> nx_mdns_rr_retransmit_count = mdns_ptr -> nx_mdns_announcing_count;
                                else
                                    p -> nx_mdns_rr_retransmit_count = 1;

                                /* Set the retransmit count.  */
                                p -> nx_mdns_rr_retransmit_count++;

                                /* Check the announcing time.  */
                                if (p -> nx_mdns_rr_announcing_max_time != NX_MDNS_ANNOUNCING_FOREVER)
                                    p -> nx_mdns_rr_announcing_max_time--;

                                /* Compare the timer count and set the minimum timer count. */
                                if (p -> nx_mdns_rr_timer_count < timer_min_count)
                                {
                                    timer_min_count = p -> nx_mdns_rr_timer_count;
                                }
                            }
                            else
                            {

                                /* Announcing complete, Update the resource record state valid,  . */
                                p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_VALID;

                                /* Clear the timer count.  */
                                p -> nx_mdns_rr_timer_count = 0;
                            }
                        }
                    }
                    break;
                }

                case NX_MDNS_RR_STATE_GOODBYE:
                {

                    /* mDNS Server send Goodbye packet.  */
                    if (p -> nx_mdns_rr_retransmit_count)
                    {

                        /* Set the send flag to send the probing. */
                        p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;

                        /* Set the announcing flag.  */
                        event_flags = event_flags | NX_MDNS_ANNOUNCING_SEND_EVENT;

                        /* Set the timer count.  */
                        p -> nx_mdns_rr_timer_count = NX_MDNS_GOODBYE_TIMER_COUNT;

                        /* Compare the timer count.and set the minimum timer count. */
                        if (p -> nx_mdns_rr_timer_count < timer_min_count)
                        {
                            timer_min_count = p -> nx_mdns_rr_timer_count;
                        }
                    }
                    else            
                    {
                        
                        /* Check for RR delete flag.  */
                        if (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_DELETE)
                        {

                            /* Delete the resource records.  */
                            _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, p);
                        }
                        else
                        {

                            /* Suspend the resource record.  */
                            p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_SUSPEND;
                            p -> nx_mdns_rr_timer_count = 0;
                        }
                    }
                    break;
                }
#endif /* NX_MDNS_DISABLE_SERVER */


                case NX_MDNS_RR_STATE_VALID:
                {

                    if(i == NX_MDNS_CACHE_TYPE_PEER)
                    {
                        /* Set the resource record status.  */
                        p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_UPDATING;

                        /* Check the updating flag.  */
                        if (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UPDATING)
                        {

                            /* Set the retransmit count.  */
                            p -> nx_mdns_rr_retransmit_count = NX_MDNS_RR_UPDATE_COUNT;

                            /* 50% of the record lifetime has elapsed,the querier should plan to issure a query at 80%-82% of the record lifetime */
                            p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * (ULONG)(30 + (((ULONG)NX_RAND()) % 3)) / 100;
                        }
                        else
                        {
                            /* Set the retransmit count.  */
                            p -> nx_mdns_rr_retransmit_count = 0;

                            /* 50% of the record lifetime has elapsed, Delete the resource record at 100% of the record lifetime */
                            p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * 50 / 100;
                        }

                        /* Compare the timer count.and set the minimum timer count. */
                        if (p -> nx_mdns_rr_timer_count < timer_min_count)
                        {
                            timer_min_count = p -> nx_mdns_rr_timer_count;
                        }
                    }

                    /* Check the send flag.  */
                    else 
                    {
                        if (p -> nx_mdns_rr_send_flag)
                        {

                            /* Set the send flag to send the response. */
                            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;     

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                            /* Clear the NSEC additional send.  */
                            if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC)
                                p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_additional_send = NX_FALSE;   
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

                            /* mDNS Responder MUST NOT multicast a record until at least one second has elapsed since the last time that record was multicast. RFC6762, Section6, Page16.  */
                            /* Set the next response time interval.  */
                            p -> nx_mdns_rr_response_interval = (ULONG)(NX_MDNS_RESPONSE_INTERVAL + NX_MDNS_TIMER_COUNT_RANGE);

                            /* Compare the timer count.and set the minimum timer count. */
                            if (p -> nx_mdns_rr_response_interval < timer_min_count)
                            {
                                timer_min_count = p -> nx_mdns_rr_response_interval;
                            }

                            /* Set the announcing flag.  */
                            event_flags = event_flags | NX_MDNS_RESPONSE_SEND_EVENT;
                        }

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                        else 
                        {
                            if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC)
                            {     
                                if (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_additional_send)
                                {
                                    p -> nx_mdns_rr_word |= NX_MDNS_RR_FLAG_ADDITIONAL;
                                    p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_additional_send = NX_FALSE;   

                                    /* Set the announcing flag.  */
                                    event_flags = event_flags | NX_MDNS_RESPONSE_SEND_EVENT;   
                                }
                            }
                        } 
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
                    }
                    break;
                }
            

#ifndef NX_MDNS_DISABLE_CLIENT
                case NX_MDNS_RR_STATE_QUERY:
                {

                    /* mDNS Client did not receive the response, send the query again.  */
                    if (!p -> nx_mdns_rr_timer_count)
                    {

                        /* Check the Duplicate Question. If the flag set, Host should not send the query until the next timeout.  */
                        if (!(p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_DUPLICATE_QUERY))
                        {

                            /* Set the send flag to send the query. */
                            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;

                            /* Set the query flag.  */
                            event_flags = event_flags | NX_MDNS_QUERY_SEND_EVENT; 
                        }

                        /* Clear the duplicate flag.  */
                        p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & ~NX_MDNS_RR_FLAG_DUPLICATE_QUERY);

                        /* Double the next timer count.   */
                        p -> nx_mdns_rr_retransmit_lifetime = p -> nx_mdns_rr_retransmit_lifetime << 1;
                                                        
                        /* The interval between the first two queries MUST be at least one second. RFC6762, Section5.2, Page9.  */
                        if (p -> nx_mdns_rr_retransmit_lifetime < NX_MDNS_QUERY_MIN_TIMER_COUNT)
                        {
                            p -> nx_mdns_rr_retransmit_lifetime = NX_MDNS_QUERY_MIN_TIMER_COUNT;
                        }
                        
                        /* When the interval between queries reaches or exceeds 60 minutes, a querier MAY cap the interval to a maximum of 60 minutes. RFC6762, Section5.2, Page10.  */
                        if (p -> nx_mdns_rr_retransmit_lifetime > NX_MDNS_QUERY_MAX_TIMER_COUNT)
                        {
                            p -> nx_mdns_rr_retransmit_lifetime = NX_MDNS_QUERY_MAX_TIMER_COUNT;
                        }

                        /* Update the timer count.  */
                        p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_retransmit_lifetime;

                        /* Compare the timer count.and set the minimum timer count. */
                        if (p -> nx_mdns_rr_timer_count < timer_min_count)
                        {
                            timer_min_count = p -> nx_mdns_rr_timer_count;
                        }
                    }
                    break;
                }

                case NX_MDNS_RR_STATE_UPDATING:
                {

                    /* The querier should plan to issure a query at 80%-82% of the record lifetime, and then if no answer is received,
                       at 85%-87%, 90%-92% and 95%-97%. the record is deleted when it reaches 100% of its lifetime. RFC6762, Section5.2, Page10.  */
                    if (p -> nx_mdns_rr_retransmit_count)
                    {

                        /* Set the send flag to retransmit its query. */
                        p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;

                        /* Set the query flag.  */
                        event_flags = event_flags | NX_MDNS_QUERY_SEND_EVENT; 

                        /* Update the timer count. Client should send the updating message every 5% of ttl.   */
                        if (p -> nx_mdns_rr_retransmit_count == 1)
                            p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_remaining_ticks;
                        else
                        {
                            remaining_ticks = p -> nx_mdns_rr_remaining_ticks;
                            p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * 5 / 100;

                            /* Get the remaining ticks in 5% of TTL. */
                            while(remaining_ticks > p -> nx_mdns_rr_timer_count)
                                remaining_ticks -= p -> nx_mdns_rr_timer_count;

                            p -> nx_mdns_rr_timer_count = remaining_ticks + (p -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * (ULONG)(((ULONG)NX_RAND()) % 3) / 100);
                        }

                        /* Compare the timer count.and set the minimum timer count. */
                        if (p -> nx_mdns_rr_timer_count < timer_min_count)
                        {
                            timer_min_count = p -> nx_mdns_rr_timer_count;
                        }
                    }
                    else
                    {

                        /* Delete the resource record.  */
                        _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, p);
                    }
                    break;
                }                        

                case NX_MDNS_RR_STATE_DELETE:
                case NX_MDNS_RR_STATE_POOF_DELETE:
                {

                    /* Delete the resource record.  */
                    _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, p);
                    break;
                }
#endif /* NX_MDNS_DISABLE_CLIENT  */

                default:
                {
                    break;
                }
            }
        }
    }
    
    /* If the specified timer rate is different with the new timer min count,then update it.*/
    if (mdns_ptr -> nx_mdns_timer_min_count != timer_min_count)
    {

        /* Set the minimum timer count.*/
        mdns_ptr -> nx_mdns_timer_min_count = timer_min_count;

        /* Deactivate the timer. */
        tx_timer_deactivate(&(mdns_ptr -> nx_mdns_timer));

        if (timer_min_count != 0xFFFFFFFF)
        {

            /* Change the timer ticks with minimum timer count.*/
            tx_timer_change(&(mdns_ptr -> nx_mdns_timer), 
                            mdns_ptr -> nx_mdns_timer_min_count, 
                            mdns_ptr -> nx_mdns_timer_min_count);
            tx_timer_activate(&(mdns_ptr -> nx_mdns_timer));
        }
    }

    /* Set the event to send mDNS query.  */
    tx_event_flags_set(&(mdns_ptr -> nx_mdns_events), event_flags, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_udp_receive_notify                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the mDNS UDP receive notify callback.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Pointer to udp socket         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags               */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    UDP receive callback                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_udp_receive_notify(NX_UDP_SOCKET *socket_ptr)
{


    NX_PARAMETER_NOT_USED(socket_ptr);

    /* Check the mDNS.  */
    if(_nx_mdns_created_ptr)
    {
        
        /* Set the receive UDP packet notify. */
        tx_event_flags_set(&(_nx_mdns_created_ptr -> nx_mdns_events), NX_MDNS_PKT_RX_EVENT, TX_OR);
    }

    return;
}

#ifndef NX_MDNS_DISABLE_SERVER
#ifndef NX_DISABLE_IPV4
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_ip_address_change_notify                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the mDNS UDP receive notify callback.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Pointer to udp socket         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags               */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    UDP receive callback                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_ip_address_change_notify(NX_IP *ip_ptr, VOID *additional_info)
{


    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(additional_info);

    /* Check the mDNS.  */
    if(_nx_mdns_created_ptr)
    {
        
        /* Set the address change event event. */
        tx_event_flags_set(&(_nx_mdns_created_ptr -> nx_mdns_events), NX_MDNS_ADDRESS_CHANGE_EVENT, TX_OR);
    }

    return;
}
#endif /* NX_DISABLE_IPV4 */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_ipv6_address_change_notify                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the mDNS UDP receive notify callback.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Pointer to udp socket         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags               */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    UDP receive callback                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifdef NX_MDNS_ENABLE_IPV6
static VOID _nx_mdns_ipv6_address_change_notify(NX_IP *ip_ptr, UINT method, UINT interface_index, UINT index, ULONG *ipv6_address)
{

    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(method);
    NX_PARAMETER_NOT_USED(interface_index);
    NX_PARAMETER_NOT_USED(index);
    NX_PARAMETER_NOT_USED(ipv6_address);

    /* Check the mDNS.  */
    if(_nx_mdns_created_ptr)
    {
        
        /* Set the address change event. */
        tx_event_flags_set(&(_nx_mdns_created_ptr -> nx_mdns_events), NX_MDNS_ADDRESS_CHANGE_EVENT, TX_OR);
    }
    return;
}
#endif /* NX_MDNS_ENABLE_IPV6  */
#endif /* NX_MDNS_DISABLE_SERVER  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_thread_entry                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the entry point for the mDNS helper thread.        */ 
/*    mDNS helper thread is responsible for processing mDNS events.       */ 
/*    Receive and send the mDNS packet.                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_value                            Pointer to mDNS instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */
/*    tx_mutex_put                          Put the mDNS mutex            */
/*    tx_event_flags_get                    Get the mDNS events           */
/*    nx_udp_socket_receive                 Receive the mDNS packet       */
/*    nx_packet_release                     Release the mDNS packet       */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */ 
/*    _nx_mdns_announcing_send              Send announcing message       */ 
/*    _nx_mdns_probing_send                 Send probing message          */ 
/*    _nx_mdns_query_send                   Send query message            */ 
/*    _nx_mdns_response_send                Send response message         */ 
/*    _nx_mdns_timer_event_process          Process the mDNS timer event  */
/*    _nx_mdns_address_change_process       Process the address change    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX Scheduler                                                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID  _nx_mdns_thread_entry(ULONG mdns_value)
{

NX_MDNS         *mdns_ptr;
NX_PACKET       *packet_ptr;
ULONG            mdns_events;
UINT             status;
UINT             interface_index;


    /* Setup mDNS pointer from the input value.  */
    mdns_ptr =  (NX_MDNS *) mdns_value;

    /* Get the mDNS mutex.  */
    tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

    /* Loop to process events for this mDNS instance.  */
    while(1)
    {

        /* Release the mDNS mutex.  */
        tx_mutex_put(&(mdns_ptr -> nx_mdns_mutex));

        /* Pickup mDNS event flags.  */
        tx_event_flags_get(&mdns_ptr -> nx_mdns_events, 
                           NX_MDNS_ALL_EVENTS, TX_OR_CLEAR, 
                           &mdns_events, TX_WAIT_FOREVER);

        /* Get the mDNS mutex.  */
        tx_mutex_get(&(mdns_ptr -> nx_mdns_mutex), TX_WAIT_FOREVER);

        /* Check for an UDP packet receive event. */
        if (mdns_events & NX_MDNS_PKT_RX_EVENT)
        {
            while (1)
            {

                /* Receive a UDP packet.  */
                status =  _nx_udp_socket_receive(&mdns_ptr -> nx_mdns_socket, 
                                                 &packet_ptr, TX_NO_WAIT);

                /* Check status.  */
                if (status != NX_SUCCESS)
                    break;

#ifndef NX_DISABLE_PACKET_CHAIN

                /* Discard the chained packets.  */
                if (packet_ptr -> nx_packet_next)
                {                    
                    nx_packet_release(packet_ptr);
                    continue;
                }
#endif
                
                /* Get the interface.  */
                if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                {
                    interface_index = packet_ptr -> nx_packet_ip_interface -> nx_interface_index;
                }
                else
                {
                    interface_index = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached -> nx_interface_index;
                }

                /* Check the interface.  */
                if (!mdns_ptr -> nx_mdns_interface_enabled[interface_index])
                {
                    /* Release the packet since mDNS is not enabled on this interface. */
                    nx_packet_release(packet_ptr);
                    continue;
                }

                /* Yes, Get the mDNS packet, process it.  */
                _nx_mdns_packet_process(mdns_ptr, packet_ptr, interface_index);

                /* Release the packet. */
                nx_packet_release(packet_ptr);
            }
        }

        /* Loop to send mDNS message for each interface.  */
        for (interface_index = 0; interface_index < NX_MAX_PHYSICAL_INTERFACES; interface_index++)
        {

            /* Check if this interface is enabled. 
               Exception: Goodbye message can be sent out after disable.  */
            if ((!mdns_ptr -> nx_mdns_interface_enabled[interface_index]) &&
                (!(mdns_events & NX_MDNS_ANNOUNCING_SEND_EVENT)))
                continue;

#ifndef NX_MDNS_DISABLE_SERVER

            /* Check for an mDNS probing send event. */
            if (mdns_events & NX_MDNS_PROBING_SEND_EVENT)
            {

                /* Send probing.  */
                _nx_mdns_probing_send(mdns_ptr, interface_index);
            }

            /* Check for an mDNS announcing send event. */
            if (mdns_events & NX_MDNS_ANNOUNCING_SEND_EVENT)
            {

                /* Send announcing.  */
                _nx_mdns_announcing_send(mdns_ptr, interface_index);
            }

            /* Check for an mDNS response send event. */
            if (mdns_events & NX_MDNS_RESPONSE_SEND_EVENT)
            {

                /* Send response.  */
                _nx_mdns_response_send(mdns_ptr, interface_index);
            }
#endif /* NX_MDNS_DISABLE_SERVER */

#ifndef NX_MDNS_DISABLE_CLIENT
            /* Check for an mDNS query send event. */
            if (mdns_events & NX_MDNS_QUERY_SEND_EVENT)
            {

                /* Send query.  */
                _nx_mdns_query_send(mdns_ptr, interface_index);
            }
#endif /* NX_MDNS_DISABLE_CLIENT */
        }

#ifndef NX_MDNS_DISABLE_SERVER

        /* Check for an mDNS address change event. */
        if (mdns_events & NX_MDNS_ADDRESS_CHANGE_EVENT)
        {

            /* Process the address change event.  */
            _nx_mdns_address_change_process(mdns_ptr);
        }
#endif /* NX_MDNS_DISABLE_SERVER */

        /* Check for an mDNS timer process event. */
        if (mdns_events & NX_MDNS_TIMER_EVENT)
        {

            /* Process the timer event.  */
            _nx_mdns_timer_event_process(mdns_ptr);
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process                             PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks the interface and the IP address of received   */
/*    packet, then preocesses the packet.                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    packet_ptr                            Pointer to mDNS packet        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_release                     Release the mDNS packet       */ 
/*    _tx_thread_system_resume              Resume thread service         */ 
/*    _nx_mdns_timer_set                    Set the mDNS timer            */
/*    _nx_mdns_name_string_decode           Decode the name size          */ 
/*    _nx_mdns_name_size_calculate          Calculate the name size       */ 
/*    _nx_mdns_rr_size_get                  Get the resource record size  */ 
/*    _nx_mdns_conflict_process             Process the mDNS conflict     */
/*    _nx_mdns_cache_find_resource_record   Find the mDNS resource record */ 
/*    _nx_mdns_packet_address_check         Check the address and port    */ 
/*    _nx_mdns_packet_rr_process            Process resource record       */ 
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                            from cache                  */ 
/*    nxd_udp_packet_info_extract           Extract the packet info       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_thread_entry                 Processing thread for mDNS    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_packet_process(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UINT interface_index)
{
    
USHORT              mdns_flags;
UCHAR              *data_ptr;
USHORT              question_count; 
USHORT              authority_count;
USHORT              answer_count;
UINT                index;
NX_MDNS_RR         *rr_search;
NX_MDNS_RR          temp_resource_record;

#ifndef NX_MDNS_DISABLE_SERVER
ULONG              *head;
NX_MDNS_RR         *p;
#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
ULONG               match_count;
NX_MDNS_RR         *nsec_rr;
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
#endif /* NX_MDNS_DISABLE_SERVER  */


#ifdef NX_MDNS_ENABLE_ADDRESS_CHECK
    /* Check the address and port.  */
    if (_nx_mdns_packet_address_check(packet_ptr))
    {
        return(NX_MDNS_ERROR);
    }
#endif /* NX_MDNS_ENABLE_ADDRESS_CHECK  */

    /* Check the packet length.  */
    if (packet_ptr -> nx_packet_length < NX_MDNS_QDSECT_OFFSET)
    {
        return(NX_MDNS_ERROR);
    }

    /* Extract the message type which should be the first byte.  */
    mdns_flags = NX_MDNS_GET_USHORT_DATA(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_FLAGS_OFFSET);

    /* Get the question count.  */
    question_count = NX_MDNS_GET_USHORT_DATA(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_QDCOUNT_OFFSET);
   
    /* Determine if we have any 'answers' to our DNS query. */
    answer_count = NX_MDNS_GET_USHORT_DATA(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ANCOUNT_OFFSET);

    /* Also check if there are any 'hints' from the Authoritative nameserver. */
    authority_count = NX_MDNS_GET_USHORT_DATA(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_NSCOUNT_OFFSET); 

    /* Update the answer count. Ignore the authority count and additional count for query message.  */
    if ((mdns_flags & NX_MDNS_RESPONSE_FLAG) == NX_MDNS_RESPONSE_FLAG)
    {
        answer_count = (USHORT)(answer_count + authority_count);

        /* Include Additional section as well */
        answer_count = (USHORT)(answer_count + NX_MDNS_GET_USHORT_DATA(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ARCOUNT_OFFSET));
    }

    /* Skip the Header. Point at the start of the question.  */
    data_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_QDSECT_OFFSET; 

    /* Process all the Question Section.  */
    for (index = 0; index < question_count; index++)
    {

        /* Check for data_ptr.  */
        if (data_ptr >= packet_ptr -> nx_packet_append_ptr)
            break;

        if ((mdns_flags & NX_MDNS_RESPONSE_FLAG) == NX_MDNS_RESPONSE_FLAG)
        {

            /* Multicast DNS responses MUST NOT contain any question in the Question Section, 
               Any questions in the Question Section of a received Multicast DNS response MUST be silently ignored. RFC6762, Section6, Page14.  */
            data_ptr += (_nx_mdns_name_size_calculate(data_ptr, packet_ptr) + 4);
        }
        else
        {

#ifndef NX_MDNS_DISABLE_CLIENT

            /* Step1, mDNS Client process the Duplicate Question Suppression, RFC6762, Section7.3, Page24.  */
            if (_nx_mdns_packet_rr_set(mdns_ptr, packet_ptr, data_ptr, &temp_resource_record, NX_MDNS_RR_OP_PEER_SET_QUESTION, interface_index) == NX_MDNS_SUCCESS)
            {

                /* Check the class top bit "QM".   */
                if ((temp_resource_record.nx_mdns_rr_class & NX_MDNS_RR_CLASS_TOP_BIT) != NX_MDNS_RR_CLASS_TOP_BIT)
                {

                    /* Find the same resource record in remote buffer.  */
                    if(_nx_mdns_cache_find_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, &temp_resource_record, NX_MDNS_RR_MATCH_EXCEPT_RDATA, &rr_search) == NX_MDNS_SUCCESS)
                    {

                        /* Duplicate Question Suppression.  */
                        if (rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY)
                        {

                            if (_nx_mdns_known_answer_find(mdns_ptr, rr_search) == NX_MDNS_NO_KNOWN_ANSWER)
                            {

                                /* Yes, set the duplicate flag. process this flag in timer event process function.  */
                                rr_search -> nx_mdns_rr_word = rr_search -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_DUPLICATE_QUERY;
                            }
                        }

#ifdef NX_MDNS_ENABLE_CLIENT_POOF
                        /* Passive Observation Of Failures, RFC6762, Section10.5, Page38. */
                        if ((rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                            (rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_POOF_DELETE))
                        {
                            rr_search -> nx_mdns_rr_poof_count ++;

                            /* After seeing two or more of these queries, and seeing no multicast response containing the expected anser within ten seconds, the record should be flushed from the cache.  */
                            if ((rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) &&
                                (rr_search -> nx_mdns_rr_poof_count >= NX_MDNS_POOF_MIN_COUNT))
                            {
                                rr_search -> nx_mdns_rr_state = NX_MDNS_RR_STATE_POOF_DELETE;
                                rr_search -> nx_mdns_rr_timer_count = NX_MDNS_POOF_TIMER_COUNT;

                                /* Set the mDNS timer.  */
                                _nx_mdns_timer_set(mdns_ptr, rr_search, rr_search -> nx_mdns_rr_timer_count);
                            }
                        }
#endif /* NX_MDNS_ENABLE_CLIENT_POOF */
                    }
                }
            }
#endif /* NX_MDNS_DISABLE_CLIENT */
            
#ifndef NX_MDNS_DISABLE_SERVER

            /* Step2, mDNS Server process the normal query.  */
            if (_nx_mdns_packet_rr_set(mdns_ptr, packet_ptr, data_ptr, &temp_resource_record, NX_MDNS_RR_OP_LOCAL_SET_QUESTION, interface_index) == NX_MDNS_SUCCESS)
            {

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                /* Set the match count.  */
                match_count = 0;
                nsec_rr = NX_NULL;
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

                /* Get head. */
                head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
                head = (ULONG*)(*head);

                /* Find the same record.  */
                for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
                {

                    /* Check the interface index.  */
                    if (p -> nx_mdns_rr_interface_index != interface_index)
                        continue;

                    /* Check whether the resource record is valid. */
                    if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
                        continue;

                    /* Check whether the same record it is. RFC6762, Section6, Page13. */
                    /* The rules: rrname must match the question name.
                    rrtype must match the question qtype unless the qtype is "ANY" or the rrtype is "CNAME". 
                    rrclass must match the question qclass unless the qclass is "ANY". */
                    if (p -> nx_mdns_rr_name != temp_resource_record.nx_mdns_rr_name)
                        continue;

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES

                    /* Check the NSEC type.  */
                    if (p ->nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC)
                        nsec_rr = p; 
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

                    if ((p -> nx_mdns_rr_type != temp_resource_record.nx_mdns_rr_type) &&
                        (temp_resource_record.nx_mdns_rr_type != NX_MDNS_RR_TYPE_ALL) &&
                        (p -> nx_mdns_rr_type != NX_MDNS_RR_TYPE_CNAME))
                        continue;

                    /* Check the RR class, Ignore the top bit.  */
                    if ((p -> nx_mdns_rr_class != (temp_resource_record.nx_mdns_rr_class & NX_MDNS_TOP_BIT_MASK)) &&
                        ((temp_resource_record.nx_mdns_rr_class & NX_MDNS_TOP_BIT_MASK)!= NX_MDNS_RR_CLASS_ALL))
                        continue;

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES

                    /* Update the match count.  */
                    match_count ++;
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

                    /* Check the state.  */
                    if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                        (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING))
                    {

                        /* Check the send flag, Flag is set indicate this resource record shoud be sent.  */
                        if (p -> nx_mdns_rr_send_flag)
                        {

                            /* Set the flag to send this Resource records via multicast. 
                            Store the old flag into the top bits. 
                            Store the new flag into the low bits.  
                            Format: 0001 0001 */
                            p -> nx_mdns_rr_send_flag = ((NX_MDNS_RR_SEND_MULTICAST << 4) | NX_MDNS_RR_SEND_MULTICAST);
                        }
                        else
                        {

                            /* Set the flag to send this Resource records via multicast.*/
                            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;
                        }   

                        /* In the case where the query has the TC (truncated) bit set, indicating that subsequent Known-Answer packets will follow,
                           Responders SHOULD delay their responses by a random amount of time selected with uniform random distribution in the range 400-500ms. RFC6762, Section6, Page 15.  */
                        if (mdns_flags & NX_MDNS_TC_FLAG)
                        {            
                            p -> nx_mdns_rr_timer_count = (ULONG)(NX_MDNS_RESPONSE_TC_DELAY_MIN + (((ULONG)NX_RAND()) % NX_MDNS_RESPONSE_TC_DELAY_RANGE));
                        }
                        else
                        {

                            /* A mDNS responder is only required to delay its transmission as necessary to ensure an interval of at least 250ms 
                            since the last time the record was multicast on that interface.  RFC6762, Section6, Page16 */    
                            if (authority_count)
                            {
                                if (p -> nx_mdns_rr_response_interval > (NX_MDNS_RESPONSE_INTERVAL - NX_MDNS_RESPONSE_PROBING_TIMER_COUNT))
                                    p -> nx_mdns_rr_response_interval = (ULONG)(p -> nx_mdns_rr_response_interval - (NX_MDNS_RESPONSE_INTERVAL - NX_MDNS_RESPONSE_PROBING_TIMER_COUNT));
                                else
                                    p -> nx_mdns_rr_response_interval = 0;
                            }                                 

                            /* Check the response interval.  */
                            if (p -> nx_mdns_rr_response_interval <= NX_MDNS_TIMER_COUNT_RANGE)
                            {

                                /* Set the timer count according to the resource record set.RFC6762, Section6, Page14.  */
                                if ((p ->nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE) == NX_MDNS_RR_FLAG_UNIQUE)
                                {
                                    p -> nx_mdns_rr_timer_count = NX_MDNS_RESPONSE_UNIQUE_DELAY;
                                }
                                else
                                {
                                    /* Set the timer count, delay 20-120ms.  */
                                    p -> nx_mdns_rr_timer_count = (ULONG)(NX_MDNS_RESPONSE_SHARED_DELAY_MIN + (((ULONG)NX_RAND()) % NX_MDNS_RESPONSE_SHARED_DELAY_RANGE));
                                }
                            }
                            else
                            {
                                p -> nx_mdns_rr_timer_count = p -> nx_mdns_rr_response_interval;
                            }
                        }

                        /* Set the mDNS timer.  */
                        _nx_mdns_timer_set(mdns_ptr, p, p -> nx_mdns_rr_timer_count);
                    }
                }

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                /* Send this NSEC response as additional answer.  */
                if ((match_count == 0) && (nsec_rr)) 
                {                     
                    nsec_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_additional_send = NX_TRUE;
                    nsec_rr -> nx_mdns_rr_timer_count = NX_MDNS_RESPONSE_UNIQUE_DELAY;
                    _nx_mdns_timer_set(mdns_ptr, nsec_rr, nsec_rr -> nx_mdns_rr_timer_count);
                }
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
            }
#endif /* NX_MDNS_DISABLE_SERVER */

            /* Update the data_ptr.  */
            data_ptr += (_nx_mdns_name_size_calculate(data_ptr, packet_ptr) + 4);
        }
    }
    
    /* Process all the Known-Answer records.  */
    for (index = 0; index < answer_count; index++)
    {

        /* Check for data_ptr.  */
        if (data_ptr >= packet_ptr -> nx_packet_append_ptr)
            break;

        if ((mdns_flags & NX_MDNS_RESPONSE_FLAG) == NX_MDNS_RESPONSE_FLAG)
        {

#ifndef NX_MDNS_DISABLE_SERVER
            /* Step1, Cooperating Multicast DNS Responders, RFC6762, Section6.6, Page21. */
            /* Does the same rname and rdata name of resource record exist in the local buffer.  */
            if (_nx_mdns_packet_rr_set(mdns_ptr, packet_ptr, data_ptr, &temp_resource_record, NX_MDNS_RR_OP_LOCAL_SET_ANSWER, interface_index) == NX_MDNS_SUCCESS)
            {
                
                /* Find the same rname, rrtype, rrclass of resource record in local buffer.  */
                if(_nx_mdns_cache_find_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, NX_MDNS_RR_MATCH_EXCEPT_RDATA, &rr_search) == NX_MDNS_SUCCESS)
                {
                                    
                    /* Probing state, conflict Resolution.  */
                    if ((rr_search -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE) &&
                        (rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_PROBING))
                    {
                        _nx_mdns_conflict_process(mdns_ptr, rr_search);

                        /* Update the data_ptr.  */
                        data_ptr += _nx_mdns_rr_size_get(data_ptr, packet_ptr);

                        continue;
                    }
                }
                
                /* Find the same rname, rrtype, rrclass and identical rdata of resource record in local buffer.  */
                if(_nx_mdns_cache_find_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, NX_MDNS_RR_MATCH_ALL, &rr_search) == NX_MDNS_SUCCESS)
                {

                    /* If the TTL in this record(B) is less than half the true TTL in local record(A), A MUST announce local record via multicast. RFC6762, Section6.6,Page21.  */
                    if ((temp_resource_record.nx_mdns_rr_ttl << 1) < rr_search -> nx_mdns_rr_ttl)
                    {
                                            
                        /* Set the flag.  */
                        rr_search -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_MULTICAST;

                        /* Set the timer count, delay 20-120ms.  */
                        rr_search -> nx_mdns_rr_timer_count = (ULONG)(NX_MDNS_RESPONSE_SHARED_DELAY_MIN + (((ULONG)NX_RAND()) % NX_MDNS_RESPONSE_SHARED_DELAY_RANGE));

                        /* Set the mDNS timer.  */
                        _nx_mdns_timer_set(mdns_ptr, rr_search, rr_search -> nx_mdns_rr_timer_count);
                    }

                    /* Duplicate Answer Suppression, RFC6762, Section7.4,Page24. 
                       The TTL in that record is not less than the TTL this host would have given. Host should not send this response again.  */
                    if (temp_resource_record.nx_mdns_rr_ttl >= rr_search -> nx_mdns_rr_ttl)
                    {

                        if (rr_search -> nx_mdns_rr_send_flag)
                        {

                            /* Clear the flag.  */
                            rr_search -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR;
                        }
                    }

                    /* Check the resource records set.  */
                    if (rr_search -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE)
                    {

                        /* Update the data_ptr.  */
                        data_ptr += _nx_mdns_rr_size_get(data_ptr, packet_ptr);

                        continue;
                    }
                }
            }
            
            /* Did the same name of resource record have existed in local buffer.  */
            if (_nx_mdns_packet_rr_set(mdns_ptr, packet_ptr, data_ptr, &temp_resource_record, NX_MDNS_RR_OP_LOCAL_SET_QUESTION, interface_index) == NX_MDNS_SUCCESS)
            {
                
                /* Find the same name, rrtype, rrclass and different rdata of resource record in local buffer.  */
                if(_nx_mdns_cache_find_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, NX_MDNS_RR_MATCH_EXCEPT_RDATA, &rr_search) == NX_MDNS_SUCCESS)
                {

                    /* Check the resource records set. RFC6762, Section6.6, Page21.  */
                    if (rr_search -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE)
                    {
                        _nx_mdns_conflict_process(mdns_ptr, rr_search);

                        /* Update the data_ptr.  */
                        data_ptr += _nx_mdns_rr_size_get(data_ptr, packet_ptr);

                        continue;
                    }
                }
            }
#endif /* NX_MDNS_DISABLE_SERVER */

#ifndef NX_MDNS_DISABLE_CLIENT
            /* Step2. Add the response resource records in remote buffer.  */  
            _nx_mdns_packet_rr_process(mdns_ptr, packet_ptr, data_ptr, interface_index);
#endif /* NX_MDNS_DISABLE_CLIENT */
        }
        else
        {
#ifndef NX_MDNS_DISABLE_CLIENT
            /* Clear the record.  */

            /* Step1, mDNS Client process the Known-Answer about Duplicate Question Suppression.  */
            if (_nx_mdns_packet_rr_set(mdns_ptr, packet_ptr, data_ptr, &temp_resource_record, NX_MDNS_RR_OP_PEER_SET_QUESTION, interface_index) == NX_MDNS_SUCCESS)
            {

                /* Find the same resource record in remote buffer.  */
                if(_nx_mdns_cache_find_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, &temp_resource_record, NX_MDNS_RR_MATCH_EXCEPT_RDATA, &rr_search) == NX_MDNS_SUCCESS)
                {

                    /* Duplicate Question Suppression.  */
                    if (rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY)
                    {

                        /* Yes, the query includes the Known-Answer Section, clear the duplicate flag.  */
                        rr_search -> nx_mdns_rr_word = (USHORT)(rr_search -> nx_mdns_rr_word & ~NX_MDNS_RR_FLAG_DUPLICATE_QUERY);
                    }

#ifdef NX_MDNS_ENABLE_CLIENT_POOF
                    /* Passive Observation Of Failures, RFC6762, Section10.5, Page38. */
                    if ((rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                        (rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_POOF_DELETE))
                    {
                        rr_search -> nx_mdns_rr_poof_count --;

                        /* Check the count.  */
                        if ((rr_search -> nx_mdns_rr_state == NX_MDNS_RR_STATE_POOF_DELETE) &&
                            (rr_search -> nx_mdns_rr_poof_count < NX_MDNS_POOF_MIN_COUNT))
                        {
                            rr_search -> nx_mdns_rr_state = NX_MDNS_RR_STATE_VALID;
                            rr_search -> nx_mdns_rr_timer_count = 0;
                        }
                    }
#endif /* NX_MDNS_ENABLE_CLIENT_POOF */
                }
            }
#endif /* NX_MDNS_DISABLE_CLIENT */
            
#ifndef NX_MDNS_DISABLE_SERVER

            /* Step2. mDNS Server process the Known-Answer of normal query.  */
            if (_nx_mdns_packet_rr_set(mdns_ptr, packet_ptr, data_ptr, &temp_resource_record, NX_MDNS_RR_OP_LOCAL_SET_ANSWER, interface_index) == NX_MDNS_SUCCESS)
            {

                /* Find the same resource record.  */
                if(_nx_mdns_cache_find_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_resource_record, NX_MDNS_RR_MATCH_ALL, &rr_search) == NX_MDNS_SUCCESS)
                {

                    /* mDNS responder Must Not answer a mDNS query if the answer included the Answer Section with an RR TTL at least half the correct value.  RFC6762, Section7.1, Page23.  */
                    if ((!(rr_search -> nx_mdns_rr_send_flag & NX_MDNS_RR_SEND_FLAG_MASK)) &&
                        ((temp_resource_record.nx_mdns_rr_ttl << 1) >= rr_search -> nx_mdns_rr_ttl))
                    {

                        /* Top four bit is zero and this resource reocrd is Known answer. 
                        Clear the send flag. */
                        rr_search -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR;
                    }
                }
            }
#endif /* NX_MDNS_DISABLE_SERVER */
        }

        /* Update the data_ptr.  */
        data_ptr += _nx_mdns_rr_size_get(data_ptr, packet_ptr);
    }

    return(NX_MDNS_SUCCESS);
}


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_probing_send                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends mDNS probing message.                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    none                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID    _nx_mdns_probing_send(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT                status;
NX_PACKET           *packet_ptr;
ULONG               *head;
NX_MDNS_RR          *p;
USHORT              question_count = 0;
USHORT              answer_count = 0;
USHORT              authority_count = 0;
USHORT              additional_count = 0;
UCHAR               resend_flag = NX_FALSE;
USHORT              total_size; 
USHORT              rr_size;
UINT                name_size;
NX_MDNS_RR          *p1;
UINT                rr_name_length;


    /* Create the mdns packet and add the mDNS header.  */
    status = _nx_mdns_packet_create(mdns_ptr, &packet_ptr, NX_TRUE);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Set the total size.  */
    total_size = (USHORT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr);

    /* Set the head pointer. */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;

    /* Add the answer resource record.  */
    for (p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Check the send flag.  */
        if (p -> nx_mdns_rr_send_flag == 0)
            continue;

        /* Check the state. */
        if (p -> nx_mdns_rr_state != NX_MDNS_RR_STATE_PROBING)
            continue;

        /* Add the resource record into packet.  */

        /* Calculate the question and answer size before sending the probing message, make sure the question and authority in one packet.  */

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)p -> nx_mdns_rr_name, &rr_name_length, NX_MDNS_NAME_MAX))
        {
            continue;
        }

        /* Calculate the name size. The name should plus the '.' and '\0'. */
        name_size = rr_name_length + 2;

        /* Calculate the resource record size for question. QNAME, QTYPE, QCLASS.  */
        rr_size = (USHORT)(name_size + 4); 

        /* Calcuate the resource record size for authority. NAME, TYPE, CLASS, TTL, RDLENGTH, RDATA.  */
        rr_size = (USHORT)(rr_size + (name_size + 10 + p -> nx_mdns_rr_rdata_length));

        /* Check if need add other authority answers for this same question.  */
        for(p1 = (NX_MDNS_RR*)(head + 1); (ULONG)p1 < *head; p1++)
        {

            /* Check the interface index.  */
            if (p1 -> nx_mdns_rr_interface_index != interface_index)
                continue;

            /* Check the state.  */
            if (p1 -> nx_mdns_rr_state != NX_MDNS_RR_STATE_PROBING)
                continue;

            /* Check the name and flag.  */
            if ((p1 -> nx_mdns_rr_name == p -> nx_mdns_rr_name) &&
                (p1 -> nx_mdns_rr_send_flag) &&
                (p1 != p))
            {

                /* Calculate the resource record size for other authority. NAME, TYPE, CLASS, TTL, RDLENGTH, RDATA.  */
                rr_size = (USHORT)(rr_size + (name_size + 10 + p1 -> nx_mdns_rr_rdata_length));

                /* Check the packet size.  */
                if (rr_size <= total_size)
                {

                    /* Set the authority answer flag.  */
                    p1 -> nx_mdns_rr_word = p1 -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_AUTHORIY_ANSWER; 

                    /* Clear the send flag.  */
                    p1 -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR; 
                }
                else
                {
                    resend_flag = NX_TRUE;
                    break;
                }
            }
        }

        /* Check the packet size.  */
        if (rr_size > total_size)
        {
            resend_flag = NX_TRUE;
            continue;
        }
        else
            total_size = (USHORT)(total_size - rr_size);

        status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_QUESTION, NX_MDNS_PACKET_PROBING);

        if (status)
        {
            resend_flag = NX_TRUE;
        }
        else
        {

            /* Set the authority answer flag.  */
            p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_AUTHORIY_ANSWER; 

            /* Update the question count.  */
            question_count ++;

            /* Clear the flag.  */
            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR;
        }
    }

    /* Add the authority answer resource record.  */
    for (p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Check whether set the resource record send flag. */
        if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_PROBING) &&
            (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_AUTHORIY_ANSWER))
        {

            /* Add the resource record into packet.  */
            status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_ANSWER, NX_MDNS_PACKET_PROBING); 
            if (status)
                resend_flag = NX_TRUE;
            else
            {

                /* Update the retransmit count.  */
                p -> nx_mdns_rr_retransmit_count --;

                /* Update the authority count.  */
                authority_count ++;

                /* Clear the additional flag.  */
                p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_AUTHORIY_ANSWER));
            }
        }

        /* Clear the answer flag.  */
        p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_ANSWER));
    }

    if (!question_count &&
        !answer_count &&
        !authority_count &&
        !additional_count)
    {
        
        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Update the question count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_QDCOUNT_OFFSET, question_count);

    /* Update the answer count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ANCOUNT_OFFSET, answer_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_NSCOUNT_OFFSET, authority_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ARCOUNT_OFFSET, additional_count);

    /* Send the mDNS packet.  */
    _nx_mdns_packet_send(mdns_ptr, packet_ptr, interface_index);

    /* Resend the packet.  */
    if (resend_flag)  
        tx_event_flags_set(&(mdns_ptr -> nx_mdns_events), NX_MDNS_PROBING_SEND_EVENT, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_announcing_send                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends mDNS announcing message.                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID    _nx_mdns_announcing_send(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT                status;
NX_PACKET           *packet_ptr;  
NX_PACKET           *new_packet_ptr;  
ULONG               *head;
NX_MDNS_RR          *p;
USHORT              question_count = 0;
USHORT              answer_count = 0;
USHORT              authority_count = 0;
USHORT              additional_count = 0;
UINT                i;
UCHAR               resend_flag = NX_FALSE;


    /* Create the mdns packet and add the mDNS header.  */
    status = _nx_mdns_packet_create(mdns_ptr, &packet_ptr, NX_FALSE);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Set the head pointer. */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;

    /* Add the answer resource record.  */
    for(p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        if(p -> nx_mdns_rr_send_flag == 0) 
            continue;

        /* Check valid state. */
        if(!(((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_GOODBYE) || 
              (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING) || 
              (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID))))
            continue;
             
        /* Add the resource record into packet.  */
        status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_ANSWER, NX_MDNS_PACKET_RESPONSE);

        if (status)
        {
            resend_flag = NX_TRUE;
        }
        else
        {

            /* Update the retransmit count.  */
            p -> nx_mdns_rr_retransmit_count --;

            /* Update the count.  */
            answer_count++;

            /* Clear the flag.  */
            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR;
        }
    }

    if (!question_count &&
        !answer_count &&
        !authority_count &&
        !additional_count)
    {
        
        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Update the question count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_QDCOUNT_OFFSET, question_count);

    /* Update the answer count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ANCOUNT_OFFSET, answer_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_NSCOUNT_OFFSET, authority_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ARCOUNT_OFFSET, additional_count);

    /* If the interval time is 0, repeat advertise.  */
    if (!mdns_ptr -> nx_mdns_announcing_retrans_interval)
    {
        for (i = 1; i < mdns_ptr -> nx_mdns_announcing_count; i ++)
        {

            /* Allocate new packet.  */
            status = nx_packet_copy(packet_ptr, &new_packet_ptr, mdns_ptr -> nx_mdns_packet_pool_ptr, NX_NO_WAIT);

            /* Check for errors. */
            if (status)
            {
                break;
            }

            /* Send the mDNS packet.  */
            _nx_mdns_packet_send(mdns_ptr, new_packet_ptr, interface_index);
        }
    }

    /* Send the mDNS packet.  */
    _nx_mdns_packet_send(mdns_ptr, packet_ptr, interface_index);

    /* Resend the packet.  */
    if (resend_flag)  
        tx_event_flags_set(&(mdns_ptr -> nx_mdns_events), NX_MDNS_ANNOUNCING_SEND_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_response_send                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends mDNS response.                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    none                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID    _nx_mdns_response_send(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT                status;
NX_PACKET           *packet_ptr;
ULONG               *head;
NX_MDNS_RR          *p;
USHORT              question_count = 0;
USHORT              answer_count = 0;
USHORT              authority_count = 0;
USHORT              additional_count = 0;
UCHAR               resend_flag = NX_FALSE;


    /* Create the mdns packet and add the mDNS header.  */
    status = _nx_mdns_packet_create(mdns_ptr, &packet_ptr, NX_FALSE);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Set the head pointer. */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;

    /* Add the answer resource record.  */
    for(p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        if(p -> nx_mdns_rr_send_flag == 0) 
            continue;

        /* Check valid state. */
        if ((p -> nx_mdns_rr_timer_count != 0) ||
             !((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING) || 
               (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID)))
            continue;

        /* Add the resource record into packet.  */
        status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_ANSWER, NX_MDNS_PACKET_RESPONSE);

        if (status)
        {
            resend_flag = NX_TRUE;
        }
        else
        {

            /* Set the answer flag to skip the resource record which has been added in answer section when find the additional records.  */
            p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_ANSWER;

            /* When including a DNS-SD Service Instance Enumeration or Selective Instance Enumeration PTR record in a response packet,
               the server/responder SHOULD include the folowing additional records, SRV,TXT, A and AAAA.RFC3763, Section12.1, Page30.  */
            _nx_mdns_additional_resource_record_find(mdns_ptr, p);

            /* Update the count.  */
            answer_count++;

            /* Clear the flag.  */
            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR;
        }
    }

    /* Add the additional answer resource record.  */
    for(p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Check whether set the resource record send flag. */
        if ((p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_ADDITIONAL) &&
            ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
             (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING)))
        {

            /* Add the resource record into packet.  */
            status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_ANSWER, NX_MDNS_PACKET_RESPONSE);    
            if (status)
            {
                resend_flag = NX_TRUE;
            }
            else
            {

                /* Update the additional count.  */
                additional_count++;  

                /* Clear the additional flag.  */
                p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_ADDITIONAL)); 
            }
        }

        /* Clear the answer flag.  */
        p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_ANSWER));
    }

    if (!question_count &&
        !answer_count &&
        !authority_count &&
        !additional_count)
    {
        
        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Update the question count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_QDCOUNT_OFFSET, question_count);

    /* Update the answer count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ANCOUNT_OFFSET, answer_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_NSCOUNT_OFFSET, authority_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ARCOUNT_OFFSET, additional_count);

    /* Send the mDNS packet.  */
    _nx_mdns_packet_send(mdns_ptr, packet_ptr, interface_index);

    /* Resend the packet.  */
    if (resend_flag)  
        tx_event_flags_set(&(mdns_ptr -> nx_mdns_events), NX_MDNS_RESPONSE_SEND_EVENT, TX_OR);   
}
#endif /* NX_MDNS_DISABLE_SERVER */


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_query_send                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends mDNS query message.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    none                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID    _nx_mdns_query_send(NX_MDNS *mdns_ptr, UINT interface_index)
{

UINT                status;
NX_PACKET           *packet_ptr;
ULONG               *head;
NX_MDNS_RR          *p;
USHORT              question_count = 0;
USHORT              answer_count = 0;
USHORT              authority_count = 0;
USHORT              additional_count = 0;
UCHAR               resend_flag = NX_FALSE;
USHORT              tc_bit;
UCHAR               more_known_answer = NX_FALSE;
UINT                i;


    /* Create the mdns packet and add the mDNS header.  */
    status = _nx_mdns_packet_create(mdns_ptr, &packet_ptr, NX_TRUE);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Set the head pointer. */
    head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;

    /* Add the query resource record.  */
    for(p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        if(p -> nx_mdns_rr_send_flag == 0) 
            continue;

        /* Check valid state. */
        if(!(((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY) || 
              (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_UPDATING))))
            continue;

        /* Add the resource record into packet.  */
        status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_QUESTION, NX_MDNS_PACKET_QUERY);

        if (status)
        {
            resend_flag = NX_TRUE;
        }
        else
        {

            /* Update the retransmit count for UPDATING state.  */
            if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_UPDATING)
                p -> nx_mdns_rr_retransmit_count --;

            /* Update the count.  */
            question_count++;

            /* Find the known answer resource records.  */
            _nx_mdns_known_answer_find(mdns_ptr, p);

            /* Clear the flag.  */
            p -> nx_mdns_rr_send_flag = NX_MDNS_RR_SEND_FLAG_CLEAR;
        }
    }

    /* Add the known answer resource record.  */
    for (i = 0; i < 2; i++)
    {
        if (i == 0)
        {
#ifndef NX_MDNS_DISABLE_SERVER
            head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
#else
            continue;
#endif /* NX_MDNS_DISABLE_SERVER  */
        }
        else
        {
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
        }

        /* Add the peer resource code as known answer for query.  */
        for(p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
        {

            /* Check the interface index.  */
            if (p -> nx_mdns_rr_interface_index != interface_index)
                continue;

            /* Check whether set the resource record send flag. */
            if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) &&
                (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_KNOWN_ANSWER))
            {

                /* Add the resource record into packet.  */
                status = _nx_mdns_packet_rr_add(packet_ptr, p, NX_MDNS_PACKET_ADD_RR_ANSWER, NX_MDNS_PACKET_QUERY); 
                if (status)
                {                
                    more_known_answer = NX_TRUE; 
                    resend_flag = NX_TRUE;
                }
                else
                {
                    /* Update the count.  */
                    answer_count ++;    

                    /* Clear the additional flag.  */
                    p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_KNOWN_ANSWER));
                }
            }

            /* Clear the answer flag.  */
            p -> nx_mdns_rr_word = (USHORT)(p -> nx_mdns_rr_word & (~NX_MDNS_RR_FLAG_ANSWER));
        }
    }

    if (!question_count &&
        !answer_count &&
        !authority_count &&
        !additional_count)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Set the TC bit.  */
    if (more_known_answer)
    {                  
        tc_bit = NX_MDNS_TC_FLAG;
         *(USHORT *)(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_FLAGS_OFFSET) |= NX_CHANGE_USHORT_ENDIAN(tc_bit);
    }

    /* Update the question count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_QDCOUNT_OFFSET, question_count);

    /* Update the answer count in header.  */
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ANCOUNT_OFFSET, answer_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_NSCOUNT_OFFSET, authority_count);
    _nx_mdns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_ARCOUNT_OFFSET, additional_count);

    /* Send the mDNS packet.  */
    _nx_mdns_packet_send(mdns_ptr, packet_ptr, interface_index);

    /* Resend the packet.  */
    if (resend_flag)  
        tx_event_flags_set(&(mdns_ptr -> nx_mdns_events), NX_MDNS_QUERY_SEND_EVENT, TX_OR);   
}
#endif /* NX_MDNS_DISABLE_CLIENT  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_create                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates the mDNS packet and add the mDNS header data. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    packet_ptr                            Pointer to mDNS packet        */ 
/*    is_query                              Query flag                    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_allocate                    Allocate the mDNS packet      */ 
/*    nx_packet_release                     Release the mDNS packet       */ 
/*    _nx_mdns_short_to_network_convert     Add the data into the packet  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_announcing_send              Send announcing message       */ 
/*    _nx_mdns_probing_send                 Send probing message          */ 
/*    _nx_mdns_query_send                   Send query message            */ 
/*    _nx_mdns_response_send                Send response message         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT    _nx_mdns_packet_create(NX_MDNS *mdns_ptr, NX_PACKET **packet_ptr, UCHAR is_query)
{
    
UINT        status;
USHORT      flags;


    /* Allocate the mDNS packet.  */
#ifdef NX_MDNS_ENABLE_IPV6
    status =  nx_packet_allocate(mdns_ptr -> nx_mdns_packet_pool_ptr, packet_ptr, NX_IPv6_UDP_PACKET, NX_NO_WAIT);    
#else
    status =  nx_packet_allocate(mdns_ptr -> nx_mdns_packet_pool_ptr, packet_ptr, NX_IPv4_UDP_PACKET, NX_NO_WAIT);
#endif /* NX_MDNS_ENABLE_IPV6  */

    /* Check for errors. */
    if (status)
    {
        
        /* Return error.  */    
        return(status);
    }

    /* Check if there is enough room to fill with mDNS header.  */
    if ((UINT)((*packet_ptr) -> nx_packet_data_end - (*packet_ptr) -> nx_packet_append_ptr) < NX_MDNS_QDSECT_OFFSET)
    {

        /* Release the packet.  */
        nx_packet_release(*packet_ptr);
        return(NX_MDNS_PACKET_ERROR);
    }

    /* Add the mDNS header.  */

    /* Set the transaction ID.  */
    *(USHORT *)((*packet_ptr) -> nx_packet_prepend_ptr + NX_MDNS_ID_OFFSET) = 0;

    /* Set the flags and Command.  */   
    if (is_query == NX_TRUE)
    {
        flags = NX_MDNS_QUERY_FLAG;
    }
    else
    {
        flags = (NX_MDNS_RESPONSE_FLAG | NX_MDNS_AA_FLAG);
    }
    
    /* Adjust for endianness. */
    NX_CHANGE_USHORT_ENDIAN(flags);

    /* Set the flags and Command.  */  
    *(USHORT *)((*packet_ptr) -> nx_packet_prepend_ptr + NX_MDNS_FLAGS_OFFSET) = flags;

    /* Initialize counts to 0.  */  
    *(ULONG *)((*packet_ptr) -> nx_packet_prepend_ptr + NX_MDNS_QDCOUNT_OFFSET) = 0;
    *(ULONG *)((*packet_ptr) -> nx_packet_prepend_ptr + NX_MDNS_NSCOUNT_OFFSET) = 0;

    /* Set the pointer.  */
    (*packet_ptr) -> nx_packet_append_ptr = (*packet_ptr) -> nx_packet_prepend_ptr + NX_MDNS_QDSECT_OFFSET;
    (*packet_ptr) -> nx_packet_length = NX_MDNS_QDSECT_OFFSET;

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_send                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends the mDNS packet.                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    packet_ptr                            Pointer to mDNS packet        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_copy                        Copy the mDNS packet          */ 
/*    nx_packet_release                     Release the mDNS packet       */ 
/*    nx_udp_socket_send                    Send the udp packet           */
/*    nxd_udp_socket_send                   Send the udp packet           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_announcing_send              Send announcing message       */ 
/*    _nx_mdns_probing_send                 Send probing message          */ 
/*    _nx_mdns_query_send                   Send query message            */ 
/*    _nx_mdns_response_send                Send response message         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID    _nx_mdns_packet_send(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UINT interface_index)
{

#if !defined NX_DISABLE_IPV4 || defined NX_MDNS_ENABLE_IPV6
UINT                status;
#endif /* !NX_DISABLE_IPV4 || NX_MDNS_ENABLE_IPV6  */
#ifdef NX_MDNS_ENABLE_IPV6
UINT                ipv6_packet = NX_FALSE;
NX_PACKET           *new_packet_ptr;
UINT                address_index = mdns_ptr -> nx_mdns_ipv6_address_index[interface_index];
#endif /*  NX_MDNS_ENABLE_IPV6  */


#if defined NX_DISABLE_IPV4 && !defined NX_MDNS_ENABLE_IPV6
    NX_PARAMETER_NOT_USED(mdns_ptr);
    NX_PARAMETER_NOT_USED(interface_index);
#endif /* NX_DISABLE_IPV4 && ! NX_MDNS_ENABLE_IPV6  */

#ifdef NX_MDNS_ENABLE_IPV6
    if (address_index != 0xFFFFFFFF)
    {

        /* Allocate new IPv6 packet.  */
        status = nx_packet_copy(packet_ptr, &new_packet_ptr, mdns_ptr -> nx_mdns_packet_pool_ptr, NX_NO_WAIT);

        /* Check for errors. */
        if (!status)
        {
            ipv6_packet = NX_TRUE;
        }
    }
#endif /* NX_MDNS_ENABLE_IPV6  */

#ifndef NX_DISABLE_IPV4
    /* Send the IPv4 mDNS message.  */
    status =  nx_udp_socket_source_send(&mdns_ptr -> nx_mdns_socket, packet_ptr, NX_MDNS_IPV4_MULTICAST_ADDRESS, NX_MDNS_UDP_PORT, interface_index);
    
    /* If an error is detected, the packet was not sent and we have to release the packet. */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }
#else

    /* Release the packet.  */
    nx_packet_release(packet_ptr);
#endif /* NX_DISABLE_IPV4  */

#ifdef NX_MDNS_ENABLE_IPV6

    /* Check if send IPv6 packet.  */
    if (ipv6_packet == NX_TRUE)
    {

        /* Send the IPv6 mDNS message.  */
        status =  nxd_udp_socket_source_send(&mdns_ptr -> nx_mdns_socket, new_packet_ptr, &NX_MDNS_IPV6_MULTICAST_ADDRESS, NX_MDNS_UDP_PORT, address_index);
    
        /* If an error is detected, the packet was not sent and we have to release the packet. */
        if (status != NX_SUCCESS)
        {

            /* Release the packet.  */
            nx_packet_release(new_packet_ptr);
        }
    }
#endif /* NX_MDNS_ENABLE_IPV6  */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_rr_add                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS resource record into the packet.        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    packet_ptr                            Pointer to mDNS packet        */
/*    rr                                    Pointer to mDNS record        */
/*    op                                    RR adding Operation           */
/*    packet_type                           Packet type                   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_mdns_name_string_encode           Encode the name string        */ 
/*    _nx_mdns_short_to_network_convert     Convert and add the data      */
/*    _nx_mdns_txt_string_encode            Encode the txt string         */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_announcing_send              Send announcing message       */ 
/*    _nx_mdns_probing_send                 Send probing message          */ 
/*    _nx_mdns_query_send                   Send query message            */ 
/*    _nx_mdns_response_send                Send response message         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT    _nx_mdns_packet_rr_add(NX_PACKET *packet_ptr, NX_MDNS_RR *rr, UINT op, UINT packet_type)
{

USHORT      size = 0;
USHORT      index = 0;
USHORT      rr_size = 0;
USHORT      rdata_length_index = 0;
UCHAR       *data_ptr = packet_ptr -> nx_packet_append_ptr;
UINT        rr_name_length;


    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)rr -> nx_mdns_rr_name, &rr_name_length, NX_MDNS_NAME_MAX))
    {
        return (NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Check whether set the resource record send flag. */
    if(op ==  NX_MDNS_PACKET_ADD_RR_QUESTION)
    {

        /* The name size is should add the '.' and '\0',  Calcuate the resource record size. QNAME, QTYPE, QCLASS.  */
        rr_size = (USHORT)(rr_name_length + 2 + 4);

        if ((data_ptr + rr_size) > packet_ptr -> nx_packet_data_end)
        {
            return(NX_MDNS_PACKET_ERROR);
        }

        /* Encode and add the name.  */
        size = (USHORT)_nx_mdns_name_string_encode(data_ptr, rr -> nx_mdns_rr_name);

        /* Updat the index.  */
        index = (USHORT)(index + size);

        /* Add the type.  */
        if(packet_type == NX_MDNS_PACKET_PROBING)
        {
            _nx_mdns_short_to_network_convert(data_ptr + index, NX_MDNS_RR_TYPE_ALL);
        }
        else
        {
            _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_type);
        }
        index = (USHORT)(index + 2);

        /* Add the class.  */
        _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_class);
        index = (USHORT)(index + 2);
    }

    /* Check whether set the resource record send flag. */
    else
    {
        
        /* Calcuate the resource record size. NAME, TYPE, CLASS, TTL, RDLENGTH, RDATA.  */
        rr_size = (USHORT)(rr_name_length + 2 + 10 + rr -> nx_mdns_rr_rdata_length);

        if ((data_ptr + rr_size) > packet_ptr -> nx_packet_data_end)
        {
            return(NX_MDNS_PACKET_ERROR);
        }

        /* Encode and add the name.  */
        size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_name);

        /* Updat the index.  */
        index = (USHORT)(index + size);

        /* Add the type and class.  */
        _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_type);
        index = (USHORT)(index + 2);
        
        /* Add the class.  */
        if ((!(rr -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_PEER)) &&
            (rr -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE))
        {
            _nx_mdns_short_to_network_convert(data_ptr + index, ((rr -> nx_mdns_rr_class) | NX_MDNS_RR_CLASS_TOP_BIT));
        }
        else
        {
            
            _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_class);
        }
        index = (USHORT)(index + 2);
        
        /* Add the ttl.  */
        /* Check the resource record owner.  */
        if (rr -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_PEER)
        {
            /* Add the remaining ttl of peer resource reocrd.  */
            _nx_mdns_long_to_network_convert(data_ptr + index, (rr -> nx_mdns_rr_remaining_ticks / NX_IP_PERIODIC_RATE));
        }
        else
        {
            if (rr -> nx_mdns_rr_state == NX_MDNS_RR_STATE_GOODBYE)
            {
                _nx_mdns_long_to_network_convert(data_ptr + index, 0);
            }
            else
            {
                _nx_mdns_long_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_ttl);
            }
        }
        index = (USHORT)(index + 4);
    
        /* Compare the RDATA. */
        switch (rr -> nx_mdns_rr_type)
        {
            
            case NX_MDNS_RR_TYPE_A:
            {

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + index, 4);
                index = (USHORT)(index + 2);

                /* Add the rdata.  */
                _nx_mdns_long_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address);
                index = (USHORT)(index + 4);

                break;
            }
            case NX_MDNS_RR_TYPE_AAAA:
            {

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + index, 16);
                index = (USHORT)(index + 2);

                /* Add the rdata.  */
                _nx_mdns_long_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[0]);
                index = (USHORT)(index + 4);
                _nx_mdns_long_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[1]);
                index = (USHORT)(index + 4);
                _nx_mdns_long_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[2]);
                index = (USHORT)(index + 4);
                _nx_mdns_long_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[3]);
                index = (USHORT)(index + 4);

                break;
            }
            case NX_MDNS_RR_TYPE_PTR:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);

                /* Encode and add the name.  */
                size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name);
                index = (USHORT)(index + size);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, size);

                break;
            }
            case NX_MDNS_RR_TYPE_SRV:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);

                /* Add the prority.  */                    
                _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_priority);
                index = (USHORT)(index + 2);

                /* Add the weights.  */                    
                _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_weights);
                index = (USHORT)(index + 2);

                /* Add the port.  */
                _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_port);
                index = (USHORT)(index + 2);

                /* Encode and add the name.  */
                size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target);
                index = (USHORT)(index + size);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, (USHORT)(size + 6));

                break;
            } 
            case NX_MDNS_RR_TYPE_TXT:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);
                
                if (rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data)
                {

                    /* Encode and add the text.  */
                    size = (USHORT)_nx_mdns_txt_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data);
                }
                else
                {

                    /* Encode the null.  */
                    *(data_ptr + index) = 0;
                    size = 1;
                }
                index = (USHORT)(index + size);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, size);

                break;
            }

#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
            case NX_MDNS_RR_TYPE_CNAME:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);

                /* Encode and add the name.  */
                size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_cname.nx_mdns_rr_cname_name);
                index = (USHORT)(index + size);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, size);

                break;
            }
            case NX_MDNS_RR_TYPE_NS:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);

                /* Encode and add the name.  */
                size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ns.nx_mdns_rr_ns_name);
                index = (USHORT)(index + size);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, size);

                break;
            }
            case NX_MDNS_RR_TYPE_MX:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);

                /* Add the preference.  */                    
                _nx_mdns_short_to_network_convert(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_preference);
                index = (USHORT)(index + 2);

                /* Encode and add the name.  */
                size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_name);
                index = (USHORT)(index + size);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, (USHORT)(size + 2));

                break;
            }
#endif /* NX_MDNS_ENABLE_EXTENDED_RR_TYPES  */

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
            case NX_MDNS_RR_TYPE_NSEC:
            {

                /* Record the rdata length index then skip the rdata length.  */
                rdata_length_index = index;
                index = (USHORT)(index + 2);

                /* Encode and add the next domain name.  */
                size = (USHORT)_nx_mdns_name_string_encode(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_next_domain);
                index = (USHORT)(index + size);
                                         
                /* Add the window block.  */ 
                *(data_ptr + index) = rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_window_block;
                index++;
                
                /* Add the bitmap length.  */   
                *(data_ptr + index) = rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length;
                index++;

                /* Add the type bit maps.  */
                memcpy(data_ptr + index, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap, rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length); /* Use case of memcpy is verified. */
                index = (USHORT)(index + rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length);

                /* Add the rdata length.  */
                _nx_mdns_short_to_network_convert(data_ptr + rdata_length_index, (USHORT)(size + 2 + rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length));

                break;
            }
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
        }
    }        

    /* Update the append pointer and packet length.  */
    packet_ptr -> nx_packet_append_ptr += index;
    packet_ptr -> nx_packet_length += index;    

    return(NX_MDNS_SUCCESS);    
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_rr_set                              PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the resource record info from packet.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance.     */ 
/*    packet_ptr                            Pointer to the packet.        */ 
/*    data_ptr                              Pointer to the data.          */  
/*    rr_ptr                                Pointer to the record         */
/*    op                                    Operation:                    */ 
/*                                            set question or set answer. */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */  
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_name_string_decode           Decode the name string        */ 
/*    _nx_mdns_cache_add_string             Add the name string           */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_packet_rr_set(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, NX_MDNS_RR *rr_ptr, UINT op, UINT interface_index)
{

UCHAR          *cache_ptr;
UINT            cache_size;
UINT            cache_type;
USHORT          record_class;
UINT            status;
UINT            temp_string_length;


    /* Initialize.  */
    memset(rr_ptr, 0, sizeof(NX_MDNS_RR));

    /* Get cache type and size. */
    if ((op == NX_MDNS_RR_OP_LOCAL_SET_QUESTION) ||
        (op == NX_MDNS_RR_OP_LOCAL_SET_ANSWER))
    {
        cache_ptr = mdns_ptr -> nx_mdns_local_service_cache;
        cache_size = mdns_ptr -> nx_mdns_local_service_cache_size;
        cache_type = NX_MDNS_CACHE_TYPE_LOCAL;
    }
    else
    {
        cache_ptr = mdns_ptr -> nx_mdns_peer_service_cache;
        cache_size = mdns_ptr -> nx_mdns_peer_service_cache_size;
        cache_type = NX_MDNS_CACHE_TYPE_PEER;
    }

    /* Check cache pointer and cache size.  */
    if ((cache_ptr == NX_NULL) || (cache_size == 0))
    {
        return(NX_MDNS_ERROR);
    }

    /* Set the interface.  */
    rr_ptr -> nx_mdns_rr_interface_index = (UCHAR)interface_index;

    /* Process the name string.  */
    if (_nx_mdns_name_string_decode(packet_ptr -> nx_packet_prepend_ptr, 
                                    (UINT)(data_ptr - packet_ptr -> nx_packet_prepend_ptr),
                                    packet_ptr -> nx_packet_length,
                                    temp_string_buffer, NX_MDNS_NAME_MAX))
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)temp_string_buffer, &temp_string_length, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

         /* Add the string .  */
        if(_nx_mdns_cache_add_string(mdns_ptr, cache_type, temp_string_buffer, temp_string_length,
                                    (VOID **)(&rr_ptr -> nx_mdns_rr_name), NX_TRUE, NX_TRUE) != NX_MDNS_SUCCESS)
        {
            return(NX_MDNS_ERROR);
        }
    }
    else
    {
        return(NX_MDNS_ERROR);
    }

    /* Plus 4 for 2 bytes type and 2 bytes class. */
    temp_string_length = _nx_mdns_name_size_calculate(data_ptr, packet_ptr);
    if ((temp_string_length == 0) || ((data_ptr + temp_string_length + 4) > packet_ptr -> nx_packet_append_ptr))
    {
        return(NX_MDNS_ERROR);
    }

    /* Set the resource record type. */
    rr_ptr -> nx_mdns_rr_type = NX_MDNS_GET_USHORT_DATA(data_ptr + temp_string_length);

    /* Get the resource record class.*/
    record_class = NX_MDNS_GET_USHORT_DATA(data_ptr + temp_string_length + 2);

    /* Remote RR, set the RR owner flag.*/
    if (cache_type == NX_MDNS_CACHE_TYPE_PEER)
        rr_ptr -> nx_mdns_rr_word |= NX_MDNS_RR_FLAG_PEER;

    /* Unique RR, Set the RR set flag.  */
    if (record_class & (~NX_MDNS_TOP_BIT_MASK))
    {
        rr_ptr -> nx_mdns_rr_word |= NX_MDNS_RR_FLAG_UNIQUE;
    }

    /* Check the operation and update the record class.  */
    if((op == NX_MDNS_RR_OP_LOCAL_SET_QUESTION) || (op == NX_MDNS_RR_OP_PEER_SET_QUESTION))
    {
        rr_ptr -> nx_mdns_rr_class = record_class;
        return(NX_MDNS_SUCCESS);
    }
    else
    {
        rr_ptr -> nx_mdns_rr_class = record_class & NX_MDNS_TOP_BIT_MASK;
    }

    /* Set the rdata information for answer record.  */
    status = _nx_mdns_packet_rr_data_set(mdns_ptr, packet_ptr, data_ptr, rr_ptr, op);

    /* Return success.  */
    return (status);
}


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_rr_process                          PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the resource record of packet, and add the  */
/*    record into peer cache.                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance.     */ 
/*    packet_ptr                            Pointer to the packet.        */ 
/*    data_ptr                              Pointer to the data.          */ 
/*    insert_ptr                            Pointer to the insert record  */
/*    op                                    Operation: add answer, set    */ 
/*                                            question or set answer.     */ 
/*    cache_ptr                             The record buffer.            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */  
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_name_string_decode           Decode the name string        */ 
/*    _nx_mdns_cache_add_string             Add the name string           */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_packet_rr_process(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UINT interface_index)
{

UINT            status;
USHORT          record_class;
UCHAR           is_present;
NX_MDNS_RR      rr_ptr;
NX_MDNS_RR     *insert_ptr;
NX_MDNS_RR     *p;
ULONG          *head;
UCHAR          *service_name,*service_type,*service_domain;
UINT            temp_string_length;
UINT            rr_name_length;


    /* Check the peer cache.  */
    if ((mdns_ptr -> nx_mdns_peer_service_cache == NX_NULL) ||
        (mdns_ptr -> nx_mdns_peer_service_cache_size == 0))
    {
        return(NX_MDNS_ERROR);
    }

    /* Initialize.  */
    memset(&rr_ptr, 0, sizeof(NX_MDNS_RR));

    /* Process the name string.  */
    if (_nx_mdns_name_string_decode(packet_ptr -> nx_packet_prepend_ptr, 
                                    (UINT)(data_ptr - packet_ptr -> nx_packet_prepend_ptr),
                                    packet_ptr -> nx_packet_length,
                                    temp_string_buffer, NX_MDNS_NAME_MAX))
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)temp_string_buffer, &temp_string_length, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

         /* Add the string .  */
        if(_nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, temp_string_buffer, temp_string_length,
                                     (VOID **)(&rr_ptr.nx_mdns_rr_name), NX_FALSE, NX_TRUE))
        {
            return(NX_MDNS_ERROR);
        }
    }
    else
    {        
        return(NX_MDNS_ERROR);
    }

    /* Plus 4 for 2 bytes type and 2 bytes class. */
    temp_string_length = _nx_mdns_name_size_calculate(data_ptr, packet_ptr);
    if ((temp_string_length == 0) || ((data_ptr + temp_string_length + 4) > packet_ptr -> nx_packet_append_ptr))
    {
        return(NX_MDNS_ERROR);
    }

    /* Set the resource record type. */
    rr_ptr.nx_mdns_rr_type = NX_MDNS_GET_USHORT_DATA(data_ptr + temp_string_length);

    /* Get the resource record class.*/
    record_class = NX_MDNS_GET_USHORT_DATA(data_ptr + temp_string_length + 2);

    /* Set the resource record class.  */
    rr_ptr.nx_mdns_rr_class = record_class & NX_MDNS_TOP_BIT_MASK;
    
    /* Remote RR, set the RR owner flag.*/
    rr_ptr.nx_mdns_rr_word |= NX_MDNS_RR_FLAG_PEER;

    /* Unique RR, Set the RR set flag.  */
    if (record_class & (~NX_MDNS_TOP_BIT_MASK))
    {
        rr_ptr.nx_mdns_rr_word |= NX_MDNS_RR_FLAG_UNIQUE;
    }

    /* Set the interface index.  */
    rr_ptr.nx_mdns_rr_interface_index = (UCHAR)interface_index;

    /* Set the rdata information for answer record.  */
    status = _nx_mdns_packet_rr_data_set(mdns_ptr, packet_ptr, data_ptr, &rr_ptr, NX_MDNS_RR_OP_PEER_ADD_ANSWER);

    /* Check status.  */
    if(status)
    {

        /* Delete the name strings. */
        _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, rr_ptr.nx_mdns_rr_name, 0);
        return (NX_MDNS_ERROR);
    }

    /* Check if need to ignore this source reocrd.  */
    if ((mdns_ptr -> nx_mdns_service_ignore_mask) &&
        ((rr_ptr.nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) ||
         (rr_ptr.nx_mdns_rr_type == NX_MDNS_RR_TYPE_TXT) ||
         (rr_ptr.nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR)))
    {

        /* Get the service.  */
        if (rr_ptr.nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR)
        {

            /* Check string length.  */
            if (_nx_utility_string_length_check((CHAR *)rr_ptr.nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, &rr_name_length, NX_MDNS_NAME_MAX))
            {
                _nx_mdns_cache_delete_rr_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER,  &rr_ptr);
                return(NX_MDNS_DATA_SIZE_ERROR);
            }
            memcpy((CHAR *)temp_string_buffer, (const char *)rr_ptr.nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name, rr_name_length + 1); /* Use case of memcpy is verified. */
        }
        else
        {

            /* Check string length.  */
            if (_nx_utility_string_length_check((CHAR *)rr_ptr.nx_mdns_rr_name, &rr_name_length, NX_MDNS_NAME_MAX))
            {
                _nx_mdns_cache_delete_rr_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER,  &rr_ptr);
                return(NX_MDNS_DATA_SIZE_ERROR);
            }
            memcpy((CHAR *)temp_string_buffer, (const char *)rr_ptr.nx_mdns_rr_name, rr_name_length + 1); /* Use case of memcpy is verified. */
        }

        /* Resolve the service type.  */
        if (_nx_mdns_service_name_resolve(temp_string_buffer, &service_name, &service_type, &service_domain))
        {
            _nx_mdns_cache_delete_rr_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER,  &rr_ptr);
            return (status);
        }

        /* Check if this service type should be ignored.  */
        if (_nx_mdns_service_mask_match(mdns_ptr, service_type, mdns_ptr -> nx_mdns_service_ignore_mask) == NX_MDNS_SUCCESS)
        {
            _nx_mdns_cache_delete_rr_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER,  &rr_ptr);
            return (NX_MDNS_ERROR);
        }
    }

    /* Add the resource record into remote buffer.  */
    if (_nx_mdns_cache_add_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, &rr_ptr, &insert_ptr, &is_present))
    {

        /* Delete the name strings. */
        _nx_mdns_cache_delete_rr_string(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, &rr_ptr);
        return (NX_MDNS_ERROR);
    }

    /* Check the ttl.  */
    if (insert_ptr -> nx_mdns_rr_ttl)
    {

        /* Step1. Process answer record.  */
        /* Update the state.  */
        insert_ptr -> nx_mdns_rr_state = NX_MDNS_RR_STATE_VALID;

        /* Update the timer count. The querier should not include records in the Known-Answer list whose remaining TTL is less than half of their original TTL.  */
        insert_ptr -> nx_mdns_rr_timer_count = insert_ptr -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * 50 / 100;
                    
        /* Record the remaining ticks of resource record. */
        if (insert_ptr -> nx_mdns_rr_ttl >= NX_MDNS_RR_MAX_TTL)
        {
            /* If the ttl of resource resource record is outside the range, set as 0xFFFFFFFF.  */
            insert_ptr -> nx_mdns_rr_remaining_ticks = 0xFFFFFFFF;
        }
        else
        {
            /* Convert units(seconds to ticks). */
            insert_ptr -> nx_mdns_rr_remaining_ticks = insert_ptr -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE;
        }

        /* Step2. Process query record.  */
        /* Set the pointer.  */
        head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
        head = (ULONG*)(*head);

        /* Check the remote resource record state.  */
        for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_peer_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
        {

            /* Check the state.  */
            if (p -> nx_mdns_rr_state != NX_MDNS_RR_STATE_QUERY)
                continue;

            /* Check the interface index.  */
            if (p -> nx_mdns_rr_interface_index != insert_ptr -> nx_mdns_rr_interface_index)
                continue;

            /* Check the name.  */
            if (p -> nx_mdns_rr_name != insert_ptr -> nx_mdns_rr_name)
                continue;

            /* Check the type.  */
            if ((p -> nx_mdns_rr_type != insert_ptr -> nx_mdns_rr_type) &&
                (p -> nx_mdns_rr_type != NX_MDNS_RR_TYPE_ALL))
                continue;

            /* Check the class.  */
            if (p -> nx_mdns_rr_class != insert_ptr -> nx_mdns_rr_class)
                continue;

            /* Check the query type.  */
            if (!(p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_CONTINUOUS_QUERY))
            {

                /* Determine if we need to wake a thread suspended.  */
                if (mdns_ptr -> nx_mdns_rr_receive_suspension_list)
                {

                    /* Resume suspended thread.  */
                    _nx_mdns_query_thread_resume(&(mdns_ptr -> nx_mdns_rr_receive_suspension_list), mdns_ptr, insert_ptr);
                }

                /* Get the answer, we need not send the question again. Delete the resource record.  */
                _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_PEER, p);

            }
            else
            {

                /* There is no need for the querier to continue issuing a stream of queries when a mDNS response 
                    is received containing a unique answer.  mDNS querier should send the next query at 80%-82% of the 
                    record's TTL. RFC6762, Section5.2,Page11.  */
                if ((insert_ptr -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE) && (insert_ptr -> nx_mdns_rr_ttl))
                {

                    /* Set the timer count. */
                    p -> nx_mdns_rr_timer_count = insert_ptr -> nx_mdns_rr_ttl * NX_IP_PERIODIC_RATE * (ULONG)(80 + (((ULONG)NX_RAND()) % 3)) / 100;

                    /* Set the mDNS timer.  */
                    _nx_mdns_timer_set(mdns_ptr, p, p -> nx_mdns_rr_timer_count);
                }

                /* Set the updating flag to update the resource record at 80% of the record's TTL.  */
                insert_ptr -> nx_mdns_rr_word |= NX_MDNS_RR_FLAG_UPDATING;
            }
        }
    }
    else
    {

        /* Update the state to delete this record from peer cache. .  */
        insert_ptr -> nx_mdns_rr_state = NX_MDNS_RR_STATE_DELETE;

        /* Delete the resource record one second later.  */
        insert_ptr -> nx_mdns_rr_timer_count = NX_MDNS_RR_DELETE_DELAY_TIMER_COUNT;
    }

    /* Set the mDNS timer for insert RR.  */
    _nx_mdns_timer_set(mdns_ptr, insert_ptr, insert_ptr -> nx_mdns_rr_timer_count);

    /* Service name registered success, invoke the notify function.  */
    if (mdns_ptr -> nx_mdns_service_change_notify)
    {

        /* Process the service notify function.  */
        _nx_mdns_service_change_notify_process(mdns_ptr, insert_ptr, is_present);
    }

    /* Return success.  */
    return (NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_CLIENT  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_rr_data_set                         PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the packet, and sets the data of            */
/*    resource record.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance.     */ 
/*    packet_ptr                            Pointer to the packet.        */ 
/*    data_ptr                              Pointer to the data.          */ 
/*    rr_ptr                                Pointer to the record         */
/*    op                                    Operation:                    */ 
/*                                            set answer in local cache.  */ 
/*                                            or add answer in peer cache.*/ 
/*    cache_ptr                             The record buffer.            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */  
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_mdns_name_string_decode           Decode the name string        */ 
/*    _nx_mdns_cache_add_string             Add the name string           */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_packet_rr_data_set(NX_MDNS *mdns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, NX_MDNS_RR *rr_ptr, UINT op)
{

UCHAR          *string_search = NX_NULL;
UINT            cache_type;
UCHAR           find_string;
UCHAR           continue_process = NX_FALSE;
UINT            status = NX_MDNS_UNSUPPORTED_TYPE;
UINT            temp_string_length;


    /* Get cache type and size. */
    if (op == NX_MDNS_RR_OP_LOCAL_SET_ANSWER)
    {
        cache_type = NX_MDNS_CACHE_TYPE_LOCAL;
        find_string = NX_TRUE;
    }
    else
    {
        cache_type = NX_MDNS_CACHE_TYPE_PEER;
        find_string = NX_FALSE;
    }

    /* Plus 10 for 2 bytes type, 2 bytes class, 4 bytes ttl and 2 bytes rdata length. */
    temp_string_length = _nx_mdns_name_size_calculate(data_ptr, packet_ptr);
    if ((temp_string_length == 0) || ((data_ptr + temp_string_length + 10) > packet_ptr -> nx_packet_append_ptr))
    {
        return(NX_MDNS_ERROR);
    }

    /* Set the resource record time to live.*/
    rr_ptr -> nx_mdns_rr_ttl = NX_MDNS_GET_ULONG_DATA(data_ptr + temp_string_length + 4);

    /* Set the resource record rdata length.  */
    rr_ptr -> nx_mdns_rr_rdata_length = NX_MDNS_GET_USHORT_DATA(data_ptr + temp_string_length + 8);;

    /* Update the pointer to point at the resource data.  */
    data_ptr = data_ptr + temp_string_length + 10;

    /* Check the type.  */
    switch (rr_ptr -> nx_mdns_rr_type)
    {
        case NX_MDNS_RR_TYPE_A:
        {

            /* 4 bytes IP address. */
            if (data_ptr + 4 > packet_ptr -> nx_packet_append_ptr)
            {
                return(NX_MDNS_ERROR);
            }

            /* Get the rdata.  */
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address = NX_MDNS_GET_ULONG_DATA(data_ptr);
            rr_ptr -> nx_mdns_rr_rdata_length = 4;

            /* Update the status.  */
            status = NX_MDNS_SUCCESS;
            break;
        }
        case NX_MDNS_RR_TYPE_AAAA:
        {

            /* 16 bytes IPv6 address. */
            if (data_ptr + 16 > packet_ptr -> nx_packet_append_ptr)
            {
                return(NX_MDNS_ERROR);
            }

            /* Get the rdata.  */
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[0] = NX_MDNS_GET_ULONG_DATA(data_ptr);
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[1] = NX_MDNS_GET_ULONG_DATA(data_ptr + 4);
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[2] = NX_MDNS_GET_ULONG_DATA(data_ptr + 8);
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[3] = NX_MDNS_GET_ULONG_DATA(data_ptr + 12);
            rr_ptr -> nx_mdns_rr_rdata_length = 16;

            /* Update the status.  */
            status = NX_MDNS_SUCCESS;
            break;
        }
        case NX_MDNS_RR_TYPE_TXT:
        {

            /* Process the txt string. An empty TXT record contaning zero strings is not allowed. RFC6763, Page12.  */
            if (rr_ptr -> nx_mdns_rr_rdata_length == 1)
            {

                /* Update the status.  */
                status = NX_MDNS_SUCCESS;
            }
            else if (rr_ptr -> nx_mdns_rr_rdata_length > 1)
            {

                if (data_ptr + rr_ptr -> nx_mdns_rr_rdata_length > packet_ptr -> nx_packet_append_ptr)
                {
                    return(NX_MDNS_ERROR);
                }

                /* Add the txt string.  */
                if (_nx_mdns_txt_string_decode(data_ptr, rr_ptr -> nx_mdns_rr_rdata_length, temp_string_buffer, NX_MDNS_NAME_MAX) == NX_MDNS_SUCCESS)
                {

                    /* Check string length.  */
                    if (_nx_utility_string_length_check((CHAR *)temp_string_buffer, &temp_string_length, NX_MDNS_NAME_MAX))
                    {
                        return(NX_MDNS_DATA_SIZE_ERROR);
                    }

                    /* Find the same string .  */
                    if(_nx_mdns_cache_add_string(mdns_ptr, cache_type, temp_string_buffer, temp_string_length,
                                                 (VOID **)&string_search, find_string, NX_TRUE) == NX_MDNS_SUCCESS)
                    {

                        /* Update the name pointer.  */
                        rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data = string_search;

                        /* Calculate the text string.inlcude the one byte label.  */
                        rr_ptr -> nx_mdns_rr_rdata_length = (USHORT)(temp_string_length + 1);

                        /* Update the status.  */
                        status = NX_MDNS_SUCCESS;
                    }
                }
            }

            break;
        }
        case NX_MDNS_RR_TYPE_SRV:
        {

            /* Plus 6 bytes for 2 bytes priority, 2 bytes weights and 2 bytes port. */
            if (data_ptr + 6 > packet_ptr -> nx_packet_append_ptr)
            {
                return(NX_MDNS_ERROR);
            }

            /* Get the priority.  */
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_priority = NX_MDNS_GET_USHORT_DATA(data_ptr);
            data_ptr += 2;

            /* Get the weights.  */
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_weights = NX_MDNS_GET_USHORT_DATA(data_ptr);
            data_ptr += 2;

            /* Get the port.  */
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_port = NX_MDNS_GET_USHORT_DATA(data_ptr);
            data_ptr += 2;

            /* Update the rdata length.  */
            rr_ptr -> nx_mdns_rr_rdata_length = 6;

            /* Continue to process target name.  */
            continue_process = NX_TRUE;
            break;
        }
        case NX_MDNS_RR_TYPE_PTR: 
#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
        case NX_MDNS_RR_TYPE_CNAME:
        case NX_MDNS_RR_TYPE_NS:
#endif
        {

            /* Update the rdata length.  */
            rr_ptr -> nx_mdns_rr_rdata_length = 0;

            /* Continue to process the domain name.  */
            continue_process = NX_TRUE;
            break;
        }
#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
        case NX_MDNS_RR_TYPE_MX:
        {

            /* Plus 2 bytes for preference. */
            if (data_ptr + 2 > packet_ptr -> nx_packet_append_ptr)
            {
                return(NX_MDNS_ERROR);
            }

            /* Set the preference.  */
            rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_preference = NX_MDNS_GET_USHORT_DATA(data_ptr);
            data_ptr += 2;

            /* Update the rdata length.  */
            rr_ptr ->  nx_mdns_rr_rdata_length = 2;

            /* Continue to process the domain name.  */
            continue_process = NX_TRUE;
            break;
        }
#endif
    }

    /* Check if need to continue process rdata.  */
    if(continue_process == NX_TRUE)
    {

        /* Process the target/domain name string.  */
        if (_nx_mdns_name_string_decode(packet_ptr -> nx_packet_prepend_ptr, 
                                       (UINT)(data_ptr - packet_ptr -> nx_packet_prepend_ptr),
                                        packet_ptr -> nx_packet_length,
                                       temp_string_buffer, NX_MDNS_NAME_MAX))
        {

            /* Check string length.  */
            if (_nx_utility_string_length_check((CHAR *)temp_string_buffer, &temp_string_length, NX_MDNS_NAME_MAX))
            {
                return(NX_MDNS_DATA_SIZE_ERROR);
            }

            /* Add the string.  */
            if(_nx_mdns_cache_add_string(mdns_ptr, cache_type, temp_string_buffer, temp_string_length,
                                         (VOID **)&string_search, find_string, NX_TRUE) == NX_MDNS_SUCCESS)
            {

                /* Update the name pointer for SRV, PTR, etc.  */
                rr_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target = string_search;

                /* Calculate the text string. Inlcude the one byte label '.' and null '\0'.  */
                rr_ptr -> nx_mdns_rr_rdata_length = (USHORT)(rr_ptr -> nx_mdns_rr_rdata_length + (temp_string_length + 2));

                /* Update the status.  */
                status = NX_MDNS_SUCCESS;
            }
        }
    }

    /* Return success.  */
    return (status);
}


#ifdef NX_MDNS_ENABLE_ADDRESS_CHECK
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_packet_address_check                       PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks the IP address and port of received packet.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to mDNS packet        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    none                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_packet_address_check(NX_PACKET *packet_ptr)
{

USHORT              mdns_flags;
UINT                src_port;
ULONG              *udp_header;

#if !defined NX_DISABLE_IPV4 || defined NX_MDNS_ENABLE_IPV6
NXD_ADDRESS         src_address;
NXD_ADDRESS         des_address;
#endif /* !NX_DISABLE_IPV4 || NX_MDNS_ENABLE_IPV6  */

#ifndef NX_DISABLE_IPV4
NX_IPV4_HEADER     *ipv4_header;
#endif /* NX_DISABLE_IPV4  */

#ifdef NX_MDNS_ENABLE_IPV6
NX_IPV6_HEADER     *ipv6_header;
#endif /* NX_MDNS_ENABLE_IPV6  */


    /* 2 bytes ID and 2 bytes flags. */
    if (packet_ptr -> nx_packet_length < 4)
    {
        return(NX_MDNS_ERROR);
    }

    /* Extract the message type which should be the first byte.  */
    mdns_flags = NX_MDNS_GET_USHORT_DATA(packet_ptr -> nx_packet_prepend_ptr + NX_MDNS_FLAGS_OFFSET);

#ifndef NX_DISABLE_IPV4
    /* Get the ip address.  */
    if(packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
    {

        /* Set the IPv4 header.  */
        ipv4_header = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Set the source address.  */
        src_address.nxd_ip_version = NX_IP_VERSION_V4;
        src_address.nxd_ip_address.v4 = ipv4_header -> nx_ip_header_source_ip;

        /* Set the destination address.  */
        des_address.nxd_ip_version = NX_IP_VERSION_V4;
        des_address.nxd_ip_address.v4 = ipv4_header -> nx_ip_header_destination_ip;
    }
    else
#endif /* NX_DISABLE_IPV4  */

#ifdef NX_MDNS_ENABLE_IPV6
    if(packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
    {

        /* Set the IPv6 header.  */
        ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Set the source address.  */
        src_address.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_source_ip, src_address.nxd_ip_address.v6);

        /* Set the destination address.  */
        des_address.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_destination_ip, des_address.nxd_ip_address.v6);
    }
    else
#endif /* NX_MDNS_ENABLE_IPV6  */
    {

        /* Invalid IP version . */
        return(NX_MDNS_ERROR);
    }

    /* Pickup the pointer to the head of the UDP packet.  */
    udp_header =  (ULONG *) (packet_ptr -> nx_packet_prepend_ptr - 8);

    /* Get the source port and destination port.  */
    src_port = (UINT) ((*udp_header) >> NX_SHIFT_BY_16);

    /* Check the source UDP port.  RFC6762, Section6, Page15.  */
    if (src_port != NX_MDNS_UDP_PORT)
    {
        return(NX_MDNS_UDP_PORT_ERROR);
    }

    /* Check the QR bit. */
    if ((mdns_flags & NX_MDNS_RESPONSE_FLAG) == NX_MDNS_RESPONSE_FLAG)
    {

        /* Check the response code.  */
        if (mdns_flags & NX_MDNS_ERROR_MASK)
        {

            /* Release the source packet.  */
            return(NX_MDNS_AUTH_ERROR);
        }

        /* In response messages for Multicast domains, the AA bit Must be set to one, RFC6762, Page48. */
        if ((mdns_flags & NX_MDNS_AA_FLAG) != NX_MDNS_AA_FLAG)
        {
            return (NX_MDNS_ERROR);
        }

#ifndef NX_DISABLE_IPV4
        /* Check the interface and destination address. RFC6762, Section 11, Page38,39. */
        if (des_address.nxd_ip_version == NX_IP_VERSION_V4)
        {

            /* Local link check, Multicast Address or (I & M) == (P & M).  */
            if ((des_address.nxd_ip_address.v4 != NX_MDNS_IPV4_MULTICAST_ADDRESS) &&
                (packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_address & packet_ptr ->nx_packet_ip_interface -> nx_interface_ip_network_mask) != 
                (src_address.nxd_ip_address.v4 & packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_network_mask))
            {            
                return (NX_MDNS_NOT_LOCAL_LINK);
            }
        }
        else
#endif /* NX_DISABLE_IPV4  */

#ifdef NX_MDNS_ENABLE_IPV6
        if(des_address.nxd_ip_version == NX_IP_VERSION_V6)
        {

            /* Destination address is not FF02::FB address or source address not on link, RFC6762, Section 11, Page39. */
            /* Not check the source address.  */
            if ((des_address.nxd_ip_address.v6[0] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[0]) ||
                (des_address.nxd_ip_address.v6[1] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[1]) ||
                (des_address.nxd_ip_address.v6[2] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[2]) ||
                (des_address.nxd_ip_address.v6[3] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[3]))
            {                
                return (NX_MDNS_DEST_ADDRESS_ERROR);
            }
        }
        else
#endif /* NX_MDNS_ENABLE_IPV6  */
        {

            /* Invalid IP version. */
            return(NX_MDNS_ERROR);
        }
    }
    else
    {

#ifndef NX_DISABLE_IPV4
        /* Check the interface and destination address. RFC6762, Section 11, Page38,39. */
        if (des_address.nxd_ip_version == NX_IP_VERSION_V4)
        {
            if (des_address.nxd_ip_address.v4 != NX_MDNS_IPV4_MULTICAST_ADDRESS)
            {
                return (NX_MDNS_DEST_ADDRESS_ERROR);
            }
        }
        else
#endif /* NX_DISABLE_IPV4  */

#ifdef NX_MDNS_ENABLE_IPV6
        if(des_address.nxd_ip_version == NX_IP_VERSION_V6)
        {
            if ((des_address.nxd_ip_address.v6[0] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[0]) ||
                (des_address.nxd_ip_address.v6[1] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[1]) ||
                (des_address.nxd_ip_address.v6[2] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[2]) ||
                (des_address.nxd_ip_address.v6[3] != NX_MDNS_IPV6_MULTICAST_ADDRESS.nxd_ip_address.v6[3]))
            {                
                return (NX_MDNS_DEST_ADDRESS_ERROR);
            }
        }
        else
#endif /* NX_MDNS_ENABLE_IPV6  */
        {

            /* Invalid IP version . */
            return(NX_MDNS_ERROR);
        }
    }

    return(NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_ENABLE_ADDRESS_CHECK  */


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_address_change_process                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the address change event.                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_copy                        Copy the mDNS packet          */ 
/*    nx_packet_release                     Release the mDNS packet       */ 
/*    nx_udp_socket_send                    Send the udp packet           */
/*    nxd_udp_socket_send                   Send the udp packet           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_thread_entry                 Processing thread for mDNS    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID    _nx_mdns_address_change_process(NX_MDNS *mdns_ptr)
{

ULONG       *head;
NX_MDNS_RR  *p;
UINT        i;


    /* Register the host name.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Check if the interface is enabled.  */
        if (!mdns_ptr -> nx_mdns_interface_enabled[i])
            continue;

        /* Get the local buffer head. */
        head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;

        if (head)
        {

            /* Set the pointer.  */
            head = (ULONG*)(*head);

            /* Check the resource record.  */
            for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
            {

                /* Check whether the resource record is valid. */
                if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
                    continue;

                /* Check the interface index.  */
                if (p -> nx_mdns_rr_interface_index != i)
                    continue;

                /* If any of a host's IP addresses change, it MUST re-announce those address records. RFC6762, Section8.4, Page31.  */
                if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A) ||
                    (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_AAAA))
                {

                    /* Send the "goodbye " announcement with RR TTL zero. RFC6762, Section8.4, Page31.  */     
                    p -> nx_mdns_rr_state = NX_MDNS_RR_STATE_GOODBYE;

                    /* Clear the retransmit count.  */
                    p -> nx_mdns_rr_retransmit_count = NX_MDNS_GOODBYE_RETRANSMIT_COUNT;

                    /* Set the timer count. 250ms.  */
                    p -> nx_mdns_rr_timer_count = NX_MDNS_GOODBYE_TIMER_COUNT;
                
                    /* Set the delete flag.  */
                    p -> nx_mdns_rr_word = (p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_DELETE);

                    /* Set the mDNS timer.  */
                    _nx_mdns_timer_set(mdns_ptr, p, p -> nx_mdns_rr_timer_count);
                }

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC)
                    _nx_mdns_cache_delete_resource_record(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, p);
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
            }
        }

        /* Second register the host name, the host does not need to repeat the Probing step, Only Announcing the A/AAAA.  */
        _nx_mdns_host_name_register(mdns_ptr, NX_FALSE, i);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_conflict_process                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the mDNS conflict. RFC6762, Section9, Page31  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    record_rr                             The resource record           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_mdns_cache_add_string             Add the string                */ 
/*    _nx_mdns_cache_delete_string          Delete the string             */ 
/*    _nx_mdns_srv_name_resolve             Reslove the service name      */
/*    nx_mdns_rr_change_notify              Resource record notify        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_conflict_process(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_rr)
{
    
UINT        status;
UCHAR       *old_name;
UCHAR       *name;
UCHAR       *type = NX_NULL;
UCHAR       *domain = NX_NULL;
UINT        i;
ULONG       *head;
NX_MDNS_RR  *p; 
UCHAR       is_host_type;
UINT        temp_string_length;
UINT        rr_name_length;


    /* Initialize the value.  */
    i = 0;
    memset(&temp_string_buffer[0], 0, NX_MDNS_NAME_MAX + 1);
    memset(&target_string_buffer[0], 0, NX_MDNS_NAME_MAX + 1);

    /* Get rr type. */
    if((record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) ||
       (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_TXT))
    {
        is_host_type = NX_FALSE;

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)(record_rr -> nx_mdns_rr_name), &rr_name_length, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Store the service name.  */
        memcpy((CHAR *)&target_string_buffer[0], (const char*)(record_rr -> nx_mdns_rr_name), rr_name_length + 1); /* Use case of memcpy is verified. */

        /* Get Name, Type, Domain.  */                    
        _nx_mdns_service_name_resolve(&target_string_buffer[0], &name, &type, &domain);
    }
    else if((record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A) ||
            (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_AAAA))
    {
        is_host_type = NX_TRUE;
        name = mdns_ptr -> nx_mdns_host_name;
    }
    else
    {

        /* This is an unsupported or unknown type. */
        return(NX_MDNS_UNSUPPORTED_TYPE); 
    }

    /* Check the conflict count.  */
    if (record_rr -> nx_mdns_rr_conflict_count >= NX_MDNS_CONFLICT_COUNT)
    {

        /* Yes, Receive the confilictiong mDNS, Probing failure.  */
        if ((record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) ||
            (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A))
        {

            /* Service name has been registered , invoke the notify function.  */
            if (mdns_ptr -> nx_mdns_probing_notify)
            {
                (mdns_ptr -> nx_mdns_probing_notify)(mdns_ptr, name, NX_MDNS_LOCAL_SERVICE_REGISTERED_FAILURE);
            }
        }

        /* Return.  */
        return (NX_MDNS_ERROR);
    }

    /* Record the old name.  */
    old_name = record_rr -> nx_mdns_rr_name;

    /* Set the new name and probing it.  */
    while ((*name) != '\0')
    {
        temp_string_buffer[i++] = *name++;
    }

    /* The first conflict, only add the " (2)".  */
    if (record_rr -> nx_mdns_rr_conflict_count == 0)
    {
        temp_string_buffer[i++] = ' ';
        temp_string_buffer[i++] = '(';
        temp_string_buffer[i++] = '2';
        temp_string_buffer[i++] = ')';
    }

    /* Modify the count, eg: " (2)" change to " (3)".  */
    else
    {
        temp_string_buffer[i-2] = (UCHAR)(temp_string_buffer[i-2] + 1);
    }


    if(is_host_type == NX_TRUE)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)temp_string_buffer, &temp_string_length, NX_MDNS_HOST_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Construct the target host.  */
        memcpy((CHAR *)(mdns_ptr -> nx_mdns_host_name), (CHAR *)(temp_string_buffer), temp_string_length); /* Use case of memcpy is verified. The NX_MDNS_HOST_NAME_MAX is limited in nxd_mdns.h. */

        temp_string_buffer[i++] = '.';

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)mdns_ptr -> nx_mdns_domain_name, &temp_string_length, NX_MDNS_DOMAIN_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Add the domain.  */
        memcpy((CHAR *)(&temp_string_buffer[i]), (CHAR *)mdns_ptr -> nx_mdns_domain_name, temp_string_length); /* Use case of memcpy is verified. The NX_MDNS_DOMAIN_NAME_MAX is limited in nxd_mdns.h. */
    }
    else
    {
        temp_string_buffer[i++] = '.';

        /* Set the Type.  */
        while ((*type) != '\0')
        {
            temp_string_buffer[i++] = *type++;
        }

        temp_string_buffer[i++] = '.';

        /* Set the Domain.  */
        while ((*domain) != '\0')
        {
            temp_string_buffer[i++] = *domain++;
        }
    }

    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)&temp_string_buffer[0], &temp_string_length, NX_MDNS_NAME_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Add the new resource records. */
    status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL,
                                       &temp_string_buffer[0], temp_string_length,
                                       (VOID **)(&record_rr -> nx_mdns_rr_name), NX_FALSE, NX_TRUE);

    if (status )
    {

        /* No, return error status.  */
        return(status);
    }

    /* Update the state.  */
    record_rr -> nx_mdns_rr_state = NX_MDNS_RR_STATE_PROBING;
    record_rr -> nx_mdns_rr_timer_count = mdns_ptr -> nx_mdns_first_probing_delay;
    record_rr -> nx_mdns_rr_retransmit_count = NX_MDNS_PROBING_RETRANSMIT_COUNT;
    record_rr -> nx_mdns_rr_conflict_count ++;

    /* Set the mDNS timer.  */
    _nx_mdns_timer_set(mdns_ptr, record_rr, record_rr -> nx_mdns_rr_timer_count);

    /* Update the PTR/SRV data name.  */
    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
    head = (ULONG*)(*head);

    for (p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {
        if ((((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) && (is_host_type == NX_FALSE)) ||
             ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) && (is_host_type == NX_TRUE))) &&
            (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name == old_name))
        {

            /* Add the new resource records. */
            status = _nx_mdns_cache_add_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, &temp_string_buffer[0], temp_string_length,
                                               (VOID **)(&(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name)), NX_FALSE, NX_TRUE);

            /* Delete the rdata name.  */
            _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, old_name, 0);
        }
    }

    /* Delete the old name.  */
    _nx_mdns_cache_delete_string(mdns_ptr, NX_MDNS_CACHE_TYPE_LOCAL, old_name, 0);

    return(NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_SERVER  */


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_change_notify_process              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the mDNS service change operation, when the */ 
/*    service information change, notify the application.                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    record_rr                             The resource record           */ 
/*    is_present                            The service type              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_service_change_notify_process(NX_MDNS *mdns_ptr, NX_MDNS_RR *new_rr, UCHAR is_present)
{

UINT            notify_status = 0;
ULONG           *head;
NX_MDNS_RR      *p; 
NX_MDNS_SERVICE temp_service; 
UINT            rr_name_length;


    /* Initialize the struct.  */
    memset(&temp_service, 0, sizeof(NX_MDNS_SERVICE));

    /* Compare the RDATA. */
    switch (new_rr -> nx_mdns_rr_type)
    {
        case NX_MDNS_RR_TYPE_A:
        case NX_MDNS_RR_TYPE_AAAA:
        {

            /* Get the remote buffer head. */
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
            head = (ULONG*)(*head);

            /* Check the resource record.  */
            for(p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_peer_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
            {

                /* Check whether the resource record is valid. */
                if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID) || 
                    (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY))
                    continue;

                /* Check the interface.  */
                if (p -> nx_mdns_rr_interface_index != new_rr -> nx_mdns_rr_interface_index)
                    continue;

                if (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV)
                {
                    if (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target == new_rr -> nx_mdns_rr_name)
                    {

                        /* Check string length.  */
                        if (_nx_utility_string_length_check((CHAR *)(p -> nx_mdns_rr_name), &rr_name_length, NX_MDNS_NAME_MAX))
                        {
                            break;
                        }

                        /* Store the Service name.  */
                        memcpy((CHAR *)(&temp_string_buffer[0]), (CHAR *)(p -> nx_mdns_rr_name), rr_name_length + 1); /* Use case of memcpy is verified. */

                        /* Check if the address is updated.  */
                        if (((is_present == NX_FALSE) && (new_rr -> nx_mdns_rr_ttl != 0)) ||
                            (((is_present == NX_TRUE) && (new_rr -> nx_mdns_rr_ttl == 0))))
                        {
                            notify_status = NX_MDNS_PEER_SERVICE_UPDATED;
                        }
                        break;
                    }
                }
            }

            break;
        }
        case NX_MDNS_RR_TYPE_SRV:
        {

            /* Check string length.  */
            if (_nx_utility_string_length_check((CHAR *)(new_rr -> nx_mdns_rr_name), &rr_name_length, NX_MDNS_NAME_MAX))
            {
                break;
            }

            /* Store the Service name.  */
            memcpy((CHAR *)&temp_string_buffer[0], (CHAR *)(new_rr -> nx_mdns_rr_name), rr_name_length + 1); /* Use case of memcpy is verified. */

            /* Check if the service is new.  */
            if ((is_present == NX_FALSE) && (new_rr -> nx_mdns_rr_ttl != 0))
            {
                notify_status = NX_MDNS_PEER_SERVICE_RECEIVED;
            }
            else if ((is_present == NX_TRUE) && (new_rr -> nx_mdns_rr_ttl == 0))
            {
                notify_status = NX_MDNS_PEER_SERVICE_DELETED;
            }
            break;
        }        
        default:
        {

            /* This is an unsupported or unknown type. */
            return; 
        }
    }

    /* Check if need to call then notify function.  */
    if (notify_status == 0)
        return;

    /* Store the service name.  */
    memcpy((CHAR *)(&temp_service.buffer[0]), (CHAR *)(&temp_string_buffer[0]), rr_name_length); /* Use case of memcpy is verified. */

    /* Reslove the service name.  */
    if (_nx_mdns_service_name_resolve(&temp_service.buffer[0], &(temp_service.service_name), &(temp_service.service_type), &(temp_service.service_domain)))
        return;

    /* Compare the service change type.  */
    if (_nx_mdns_service_mask_match(mdns_ptr, temp_service.service_type, mdns_ptr -> nx_mdns_service_notify_mask))
        return;

    /* Get the additional information.  */
    _nx_mdns_service_addition_info_get(mdns_ptr, &temp_string_buffer[0], &temp_service, new_rr -> nx_mdns_rr_interface_index);

    /* Call the notify function.  */
    (mdns_ptr -> nx_mdns_service_change_notify)(mdns_ptr, &temp_service, notify_status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_addition_info_get                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS resource record into remote buffer,     */ 
/*    mDNS thread send the query message using continuous type.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The service instance          */ 
/*    type                                  The service type              */
/*    domain                                The domain name               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*    _nx_mdns_query_check                  Check the query RR            */ 
/*    _nx_mdns_cache_add_string             Add the string into buffer    */ 
/*    _nx_mdns_cache_delete_string          Delete the string from buffer */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                            into buffer                 */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_service_addition_info_get(NX_MDNS *mdns_ptr, UCHAR *srv_name, NX_MDNS_SERVICE *service, UINT interface_index)
{
    
UINT        index = 0;
ULONG       current_time; 
ULONG       *head, *tail;
UCHAR       i;
NX_MDNS_RR  *p;
NX_MDNS_RR  *p1;
UINT        srv_name_length;
UINT        temp_length;


    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)srv_name, &srv_name_length, NX_MDNS_NAME_MAX))
    {
        return(NX_MDNS_ERROR);
    }

    /* Get the current time.  */
    current_time = tx_time_get();
                                 
    for(i = 0; i < 2; i++)
    {

        /* Set the pointer. */
        if(i == NX_MDNS_CACHE_TYPE_LOCAL)
        {
#ifndef NX_MDNS_DISABLE_SERVER
            head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
#else
            continue;
#endif /* NX_MDNS_DISABLE_SERVER */
        }
        else
        {
#ifndef NX_MDNS_DISABLE_CLIENT
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
#else
            continue;
#endif /* NX_MDNS_DISABLE_CLIENT  */
        }
          
        if(head == NX_NULL)
            continue;

        /* Set the pointer.  */
        tail = (ULONG*)(*head);

        /* Check the resource record.  */
        for(p = (NX_MDNS_RR*)((UCHAR*)(head + 1)); (ULONG*)p < tail; p++)
        {

            /* Check whether the resource record is valid. */
            if ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID) || 
                (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY))
                continue;

            /* Check the interface index.  */
            if (p -> nx_mdns_rr_interface_index != interface_index)
                continue;

            /* Check the type and name. */
            if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) &&
                (!_nx_mdns_name_match(p -> nx_mdns_rr_name, srv_name, srv_name_length)))
            {

                /* Update the elasped timer.  */
                p -> nx_mdns_rr_elapsed_time = current_time;

                /* Set the service priority, weight, port and target. */
                service -> service_priority = p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_priority;
                service -> service_weight = p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_weights;
                service -> service_port = p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_port;

                /* Check string length.  */
                if (_nx_utility_string_length_check((CHAR *)(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target), &temp_length, NX_MDNS_HOST_NAME_MAX))
                {
                    return(NX_MDNS_ERROR);
                }

                /* Store the target host.  */
                memcpy((CHAR *)(service -> service_host), /* Use case of memcpy is verified. */
                       (CHAR *)(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target),
                       temp_length + 1);

                /* All address records (type "A" and "AAAA") named in the SRV rdata */
                for (p1 = (NX_MDNS_RR*)((UCHAR*)(head + 1)); (ULONG*)p1 < tail; p1++)
                {

                    /* Update the elasped timer.  */
                    p1 -> nx_mdns_rr_elapsed_time = current_time;

                    /* Check the interface index.  */
                    if (p1 -> nx_mdns_rr_interface_index != interface_index)
                        continue;

                    /* Find the "A" records.  */
                    if ((p1 -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A) &&
                        (p1 -> nx_mdns_rr_name == p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target))
                    {

                        /* Set the IPv4 address.  */
                        service -> service_ipv4 = p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address;
                    } 

                    /* Find the "AAAA" records.  */
                    if ((p1 -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_AAAA) &&
                        (p1 -> nx_mdns_rr_name == p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target))
                    {                    
                        if (index < NX_MDNS_IPV6_ADDRESS_COUNT)
                        {

                            /* Set the IPv6 address.  */                        
                            service -> service_ipv6[index][0] = p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[0];
                            service -> service_ipv6[index][1] = p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[1];
                            service -> service_ipv6[index][2] = p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[2];
                            service -> service_ipv6[index][3] = p1 -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address[3];
                            index ++;
                        }
                    }
                }
            } 

            /* Check the type and name. */
            if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_TXT) &&
                (!_nx_mdns_name_match(p -> nx_mdns_rr_name, srv_name, srv_name_length)))
            {

                /* Update the elasped timer.  */
                p -> nx_mdns_rr_elapsed_time = current_time;

                /* Set the text valid flag.  */
                service -> service_text_valid = 1;

                if (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data)
                {

                    /* Check string length.  */
                    if (_nx_utility_string_length_check((CHAR *)(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data), &temp_length, NX_MDNS_NAME_MAX))
                    {
                        continue;
                    }

                    /* Store the txt.  */
                    memcpy((CHAR *)(service -> service_text), (const char*)(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_txt.nx_mdns_rr_txt_data), temp_length + 1); /* Use case of memcpy is verified. */
                }
            }
        }
    }

    /* Set the interface index.  */
    service -> interface_index = (UCHAR)interface_index;

    /* Return a error status.  */
    return(NX_MDNS_SUCCESS);
}
#endif /* NX_MDNS_DISABLE_CLIENT */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_initialize                         PORTABLE C       */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the mDNS remote buffer and local buffer.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    local_cache_ptr                       Pointer to local cache        */ 
/*    local_cache_size                      The size of local cache       */ 
/*    peer_cache_ptr                        Pointer to peer cache         */ 
/*    peer_cache_size                       The size of peer cache        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_create                       Initialize the mDNS structure */ 
/*    _nx_mdns_disable                      Disable the mDNS function     */
/*    _nx_mdns_peer_cache_clear             Clear the peer cache          */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_initialize(NX_MDNS *mdns_ptr, VOID *local_cache_ptr, UINT local_cache_size, 
                               VOID *peer_cache_ptr, UINT peer_cache_size)
{
        
ULONG *head;
ULONG *tail;


#ifndef NX_MDNS_DISABLE_SERVER
    /* Check the cache.  */
    if (local_cache_ptr)
    {

        /* Zero out the cache. */
        memset(local_cache_ptr, 0, local_cache_size);

        /* Set the head. */
        head = (ULONG*)local_cache_ptr;
        *head = (ULONG)((ULONG*)local_cache_ptr + 1);

        /* Set the tail. */
        tail = (ULONG*)local_cache_ptr + (local_cache_size >> 2) - 1;
        *tail = (ULONG)tail;

        /* Record the info.  */
        mdns_ptr -> nx_mdns_local_service_cache = (UCHAR*)local_cache_ptr;
        mdns_ptr -> nx_mdns_local_service_cache_size = local_cache_size;

        /* Clear the count.  */
        mdns_ptr -> nx_mdns_local_rr_count = 0;    
        mdns_ptr -> nx_mdns_local_string_count = 0;
        mdns_ptr -> nx_mdns_local_string_bytes = 0;
    }
#else
    NX_PARAMETER_NOT_USED(local_cache_ptr);
    NX_PARAMETER_NOT_USED(local_cache_size);
#endif /* NX_MDNS_DISABLE_SERVER */
    
#ifndef NX_MDNS_DISABLE_CLIENT
    /* Check the cache.  */
    if (peer_cache_ptr)
    {

        /* Zero out the cache. */
        memset(peer_cache_ptr, 0, peer_cache_size);

        /* Set the head. */
        head = (ULONG*)peer_cache_ptr;
        *head = (ULONG)((ULONG*)peer_cache_ptr + 1);

        /* Set the tail. */
        tail = (ULONG*)peer_cache_ptr + (peer_cache_size >> 2) - 1;
        *tail = (ULONG)tail;

        /* Record the info.  */
        mdns_ptr -> nx_mdns_peer_service_cache = (UCHAR*)peer_cache_ptr;
        mdns_ptr -> nx_mdns_peer_service_cache_size = peer_cache_size; 

        /* Clear the count.  */
        mdns_ptr -> nx_mdns_peer_rr_count = 0;    
        mdns_ptr -> nx_mdns_peer_string_count = 0;
        mdns_ptr -> nx_mdns_peer_string_bytes = 0;
    }
#else
    NX_PARAMETER_NOT_USED(peer_cache_ptr);
    NX_PARAMETER_NOT_USED(peer_cache_size);
#endif /* NX_MDNS_DISABLE_CLIENT */

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_add_resource_record                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the mDNS  resource record into record buffer.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    cache_type                            Cache type: local or peer     */ 
/*    record_ptr                            Pointer to resource record    */ 
/*    insert_rr                             Pointer to insert resource    */ 
/*                                            record                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_mdns_cache_find_resource_record   Find the resource record      */ 
/*    _nx_mdns_cache_delete_string          Delete the string from buffer */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_rr_a_aaaa_add                Add A/AAAA resource record    */
/*    _nx_mdns_rr_ptr_add                   Add PTR resource record       */
/*    _nx_mdns_rr_srv_add                   Add SRV resource record       */
/*    _nx_mdns_rr_txt_add                   Add TXT resource record       */
/*    _nx_mdns_query                        Send query                    */
/*    _nx_mdns_cache_delete_rr_string       Delete the rr string          */
/*    _nx_mdns_packet_rr_process            Process resource record       */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_add_resource_record(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_ptr, NX_MDNS_RR **insert_ptr, UCHAR *is_present)
{

UCHAR       *cache_ptr;
UINT        cache_size;
ULONG       *tail;
ULONG       *head;
NX_MDNS_RR  *p;
NX_MDNS_RR  *rr;
UINT        rr_name_length;

#ifndef NX_MDNS_DISABLE_CLIENT
ULONG       elapsed_time;
ULONG       current_time;
ULONG       min_elapsed_time;
#endif /* NX_MDNS_DISABLE_CLIENT  */


#ifndef NX_MDNS_DISABLE_CLIENT
    /* Initialize the parameters.  */
    min_elapsed_time = 0;
    current_time = tx_time_get();
#endif /* NX_MDNS_DISABLE_CLIENT  */

    /* Initialize.  */
    if (is_present)
        *is_present = NX_FALSE;

    /* Check the RR with same rname, rtype, rclass and rdata.  */
    if (_nx_mdns_cache_find_resource_record(mdns_ptr, cache_type, record_ptr, NX_MDNS_RR_MATCH_ALL, &rr) == NX_MDNS_SUCCESS)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)(rr -> nx_mdns_rr_name), &rr_name_length, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Copy other informations of record_ptr into insert_rr resource record.  */
        memcpy(rr, record_ptr, sizeof(NX_MDNS_RR)); /* Use case of memcpy is verified. */
        
        /* Special process for _services._dns-sd._udp.local which pointer to same service type.  */
        if ((rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR) &&
            (!_nx_mdns_name_match(rr -> nx_mdns_rr_name, (UCHAR *)_nx_mdns_dns_sd, rr_name_length)))
        {
            rr -> nx_mdns_rr_count ++;
        }

        /* Delete the resource record same strings. */
        _nx_mdns_cache_delete_rr_string(mdns_ptr, cache_type, rr);

        /* Set the insert ptr.  */
        if(insert_ptr != NX_NULL)
            *insert_ptr = rr;

        /* Set the flag.  */
        if (is_present)
            *is_present = NX_TRUE;

        return(NX_MDNS_SUCCESS);
    }

    /* Check the cache type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {
        cache_ptr = mdns_ptr -> nx_mdns_local_service_cache;
        cache_size = mdns_ptr -> nx_mdns_local_service_cache_size;
    }
    else
    {
        cache_ptr = mdns_ptr -> nx_mdns_peer_service_cache;
        cache_size = mdns_ptr -> nx_mdns_peer_service_cache_size;
    }

    /* Check cache pointer and cache size.  */
    if ((cache_ptr == NX_NULL) || (cache_size == 0))
    {
        return(NX_MDNS_ERROR);
    }

    /* Get head and tail. */
    tail = (ULONG*)cache_ptr + (cache_size >> 2) - 1;
    tail = (ULONG*)(*tail);
    head = (ULONG*)cache_ptr;
    head = (ULONG*)(*head);

    /* Set the pointer.  */
    rr = NX_NULL;

    /* Find an empty entry before head. */
    for(p = (NX_MDNS_RR*)((ULONG*)cache_ptr + 1); p < (NX_MDNS_RR*)head; p++)
    {
        if(p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
        {
            rr = p;
            break; 
        }
    }

    if (!rr)
    {

        /* Check whether the cache is full. */
        if((head + (sizeof(NX_MDNS_RR) >> 2)) > tail) 
        {
            if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
            {
                return(NX_MDNS_CACHE_ERROR); 
            }
            else
            {
                
#ifndef NX_MDNS_DISABLE_CLIENT
                /* Find an aging resource reocrd and repalce it.  */
                for(p = (NX_MDNS_RR*)((ULONG*)cache_ptr + 1); p < (NX_MDNS_RR*)head; p++)
                {

                    if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY)
                        continue;

                    /* Calculate the elapsed time.  */
                    elapsed_time = current_time - p -> nx_mdns_rr_elapsed_time;

                    /* Check the elapsed time to find the aging resource record.  */
                    if (elapsed_time >= min_elapsed_time)
                    {
                        rr = p;
                        min_elapsed_time = elapsed_time;
                    }
                }

                if (rr)
                {
                    
                    /* Delete this record.  */
                    _nx_mdns_cache_delete_resource_record(mdns_ptr, cache_type, rr);

                    /* Update the head.  */
                    head = (ULONG*)cache_ptr;
                    head = (ULONG*)(*head);
                }
                else
                {
                    return(NX_MDNS_CACHE_ERROR);
                }
#endif /* NX_MDNS_DISABLE_CLIENT */
            }
        }
        else
        {
            rr = (NX_MDNS_RR*)head;
        }
    }

    /* Just copy it to cache_ptr. */
    memcpy(rr, record_ptr, sizeof(NX_MDNS_RR)); /* Use case of memcpy is verified. */

    /* Check the type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {

        /* Increase the count.  */
        mdns_ptr -> nx_mdns_local_rr_count ++;
    }
    else
    {

        /* Increase the count.  */
        mdns_ptr -> nx_mdns_peer_rr_count ++;
    }

#ifndef NX_MDNS_DISABLE_CLIENT
    /* Update the peer resource record elapsed time.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_PEER)
    {

        /* Get the current time to set the elapsed time.  */
        rr -> nx_mdns_rr_elapsed_time = current_time;
    }
#endif /* NX_MDNS_DISABLE_CLIENT */

    /* Set the insert ptr.  */
    if(insert_ptr != NX_NULL)
        *insert_ptr = rr;

    if((ULONG*)rr >= head)
    {

        /* Update HEAD when new record is added. */
        head = (ULONG*)cache_ptr;
        *head = (ULONG)(rr + 1);
    }

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_delete_resource_record               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the mDNS resource record from record buffer.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance.     */ 
/*    cache_type                            Cache type: local or peer     */ 
/*    record_ptr                            Pointer to resource record.   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_mdns_cache_delete_string          Delete the string from buffer */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_rr_delete                    Delete the resource record    */
/*    _nx_mdns_query                        Send the one-shot query       */
/*    _nx_mdns_timer_event_process          Process the timer event       */
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*    _nx_mdns_cache_delete_rr_string       Delete the rr string          */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_delete_resource_record(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_ptr)
{

UCHAR       *cache_ptr;
ULONG       *head;


    /* Delete the resource record strings. */
    _nx_mdns_cache_delete_rr_string(mdns_ptr, cache_type, record_ptr);
    
    /* Zero out the record. */
    memset(record_ptr, 0, sizeof(NX_MDNS_RR));

    /* Check the type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {

        /* Decrease the count.  */
        mdns_ptr -> nx_mdns_local_rr_count --;

        /* Set the cache.  */
        cache_ptr = mdns_ptr -> nx_mdns_local_service_cache;
    }
    else
    {

        /* Decrease the count.  */
        mdns_ptr -> nx_mdns_peer_rr_count --;

        /* Set the cache.  */
        cache_ptr = mdns_ptr -> nx_mdns_peer_service_cache;
    }

    if (cache_ptr == NX_NULL)
    {
        return(NX_MDNS_CACHE_ERROR);
    }
    
    /* Get head. */
    head = (ULONG*)cache_ptr;
    head = (ULONG*)(*head);

    /* Move HEAD if the last RR is deleted. */
    if(record_ptr == ((NX_MDNS_RR*)head - 1))
    {
        while(record_ptr -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
        {
            record_ptr--;
            if(record_ptr < (NX_MDNS_RR*)cache_ptr)
                break;
        }
        *((ULONG*)cache_ptr) = (ULONG)(record_ptr + 1);
    }

    return(NX_MDNS_SUCCESS);    
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_find_resource_record                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds the mDNS resource record in record buffer.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance.     */ 
/*    cache_type                            Cache type: local or peer     */ 
/*    record_ptr                            Resource record.              */ 
/*    match_type                            Match type.                   */ 
/*    search_rr                             The search resource record.   */  
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */  
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_find_resource_record(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_ptr, UINT match_type, NX_MDNS_RR **search_rr)
{

UCHAR       *cache_ptr;
ULONG       *head;
NX_MDNS_RR  *p;
UINT        same_rdata;


    /* Check the cache type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {
        cache_ptr = mdns_ptr -> nx_mdns_local_service_cache;
    }
    else
    {
        cache_ptr = mdns_ptr -> nx_mdns_peer_service_cache;
    }

    /* Get head. */
    head = (ULONG*)cache_ptr;
    head = (ULONG*)(*head);

    /* Find the same record.  */
    for(p = (NX_MDNS_RR*)((UCHAR*)cache_ptr + sizeof(ULONG)); (ULONG*)p < head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != record_ptr -> nx_mdns_rr_interface_index)
            continue;

        /* Check whether the resource record is valid. */
        if(p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
            continue;

        /* Check whether the same record it is. RFC6762, Section6, Page13. */
        /* The rules: rrname must match the question name.
                      rrtype must match the question qtype unless the qtype is "ANY" or the rrtype is "CNAME". 
                      rrclass must match the question qclass unless the qclass is "ANY". */
        if (p -> nx_mdns_rr_name != record_ptr -> nx_mdns_rr_name)
            continue;

        /* In the probing state, we need not match the type.  */
        if (p -> nx_mdns_rr_state != NX_MDNS_RR_STATE_PROBING)
        {
            if ((p -> nx_mdns_rr_type != record_ptr -> nx_mdns_rr_type) &&
                (record_ptr -> nx_mdns_rr_type != NX_MDNS_RR_TYPE_ALL) &&
                (p -> nx_mdns_rr_type != NX_MDNS_RR_TYPE_CNAME))
                continue;
        }
        
        /* Check the RR class, Ignore the top bit.  */
        if ((p -> nx_mdns_rr_class != (record_ptr -> nx_mdns_rr_class & NX_MDNS_TOP_BIT_MASK)) &&
            ((record_ptr -> nx_mdns_rr_class & NX_MDNS_TOP_BIT_MASK) != NX_MDNS_RR_CLASS_ALL))
            continue;

        /* Check if just match the basic info.  */
        if (match_type == NX_MDNS_RR_MATCH_EXCEPT_RDATA)
        {

            /* Find the same record.  */
            *search_rr = p;
            return(NX_MDNS_SUCCESS);
        }
        else
        {

            /* Yes, match all information of resource record.  */

            /* Check the state.  */
            if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY)
                continue;

            /* Reset the value.  */
            same_rdata = NX_FALSE;

            /* Compare the RDATA. */
            switch (p -> nx_mdns_rr_type)
            {
                case NX_MDNS_RR_TYPE_A:
                {
                    if(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_a.nx_mdns_rr_a_address)
                    {
                        same_rdata = NX_TRUE;
                    }
                    break;
                }
                case NX_MDNS_RR_TYPE_SRV:
                {
                    if((p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_priority == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_priority) &&
                        (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_weights == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_weights)   &&
                        (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_port == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_port)         &&
                        (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target))
                    {
                        same_rdata = NX_TRUE;
                    }
                    break;
                }
                case NX_MDNS_RR_TYPE_AAAA:
                {
                    if(!memcmp(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address, record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_aaaa.nx_mdns_rr_aaaa_address, 16))
                    {
                        same_rdata = NX_TRUE;
                    }
                    break;
                }
                case NX_MDNS_RR_TYPE_PTR:
                case NX_MDNS_RR_TYPE_TXT:

#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
                case NX_MDNS_RR_TYPE_CNAME:
                case NX_MDNS_RR_TYPE_NS:
#endif
                {

                    /* Check the rdata. */
                    if(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name)
                    {
                        same_rdata = NX_TRUE;
                    }
                    break;
                }
#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
                case NX_MDNS_RR_TYPE_MX:
                {
                    if((p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_name == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_name) &&
                        (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_preference == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_mx.nx_mdns_rr_mx_preference))
                    {
                        same_rdata = NX_TRUE;
                    }
                    break;
                }                            
#endif /* NX_MDNS_ENABLE_EXTENDED_RR_TYPES  */

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
                case NX_MDNS_RR_TYPE_NSEC:
                {
                    if ((p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_next_domain == record_ptr ->nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_next_domain) &&
                        (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_window_block == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_window_block) &&
                        (p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length) &&
                        (!memcmp(&(p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[0]),
                                 &(record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap[0]), 
                                 record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_nsec.nx_mdns_rr_nsec_bitmap_length))) 
                    {     
                        same_rdata = NX_TRUE;
                    }
                    break;
                }
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
                default:
                {

                    /* This is an unsupported or unknown type. */
                    return(NX_MDNS_UNSUPPORTED_TYPE); 
                }
            }

            /* Check the same RR.  */
            if (same_rdata == NX_TRUE)
            {

                /* Find the same record.  */
                *search_rr = p;
                return(NX_MDNS_SUCCESS);
            }
        }
    }

    return(NX_MDNS_ERROR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_add_string                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds or finds the mDNS string in the record buffer.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    cache_type                            Cache type: local or peer     */ 
/*    memory_ptr                            Pointer to the string         */ 
/*    memory_size                           The size of string.           */ 
/*    insert_rr                             Pointer to insert string.     */ 
/*    find_string                           If set, just find the existed */ 
/*                                            string, otherwise, insert   */ 
/*                                            this string if it does not  */ 
/*                                            exist.                      */ 
/*    add_name                              If set, use name match        */ 
/*                                            to check the string,        */ 
/*                                            otherwise, use memcmp.      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    none                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_rr_a_aaaa_add                Add A/AAAA resource record    */
/*    _nx_mdns_rr_ptr_add                   Add PTR resource record       */
/*    _nx_mdns_rr_srv_add                   Add SRV resource record       */
/*    _nx_mdns_rr_txt_add                   Add TXT resource record       */
/*    _nx_mdns_query                        Send query                    */
/*    _nx_mdns_conflict_process             Process the conflict          */
/*    _nx_mdns_packet_rr_process            Process resource record       */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_add_string(NX_MDNS *mdns_ptr, UINT cache_type, VOID *memory_ptr, UINT memory_size, VOID **insert_ptr, UCHAR find_string, UCHAR add_name)
{

UCHAR   *cache_ptr;
UINT    cache_size;
ULONG   *tail;
ULONG   *head;
UINT    memory_len;
UINT    used_cache_size;
USHORT  len, cnt;
USHORT  min_len = 0xFFFF;
UCHAR   *p, *available, *start;


    /* Check the cache type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {
        cache_ptr = mdns_ptr -> nx_mdns_local_service_cache;
        cache_size = mdns_ptr -> nx_mdns_local_service_cache_size;
    }
    else
    {
        cache_ptr = mdns_ptr -> nx_mdns_peer_service_cache;
        cache_size = mdns_ptr -> nx_mdns_peer_service_cache_size;
    }

    /* Check cache pointer and cache size.  */
    if ((cache_ptr == NX_NULL) || (cache_size == 0))
    {            
        return(NX_MDNS_ERROR);
    }

    /* Get head and tail. */
    tail = (ULONG*)cache_ptr + (cache_size >> 2) - 1;
    p = (UCHAR*)tail;
    tail = (ULONG*)(*tail);
    head = (ULONG*)cache_ptr;
    head = (ULONG*)(*head);

    /* Calculate the amount of memory needed to store this string, including CNT and LEN fields. */
    
    /* Make the length 4 bytes align. */
    memory_len = memory_size;
    
    /* Add the length of CNT and LEN fields.  */
    memory_len = ((memory_len & 0xFFFFFFFC) + 8) & 0xFFFFFFFF;
    
    available = (UCHAR*)tail;
    while(p > (UCHAR*)tail)
    {

        /* Get len and cnt. */
        len = *((USHORT*)(p - 2));
        cnt = *((USHORT*)(p - 4));
        start = p - len;

        if((len == memory_len) &&
           (((add_name == NX_TRUE) &&
             (!_nx_mdns_name_match(start, memory_ptr, memory_size))) ||
            ((add_name == NX_FALSE) &&
             (!memcmp(start, memory_ptr, memory_size)))))
        {

            /* The same string exists in the string table. */
            if(insert_ptr)
                *insert_ptr = start;

            if(find_string == NX_FALSE)
            {

                /* Increase the use count CNT. */
                cnt++;
                *((USHORT*)(p - 4)) = cnt;
            }

            return(NX_MDNS_SUCCESS);
        }

        /* This slot is not being used. The size of the slot is a smaller
           fit for this string. */
        if((cnt == 0) && (len >= memory_len) && (len < min_len))
        {

            /* This place is better to insert. */
            available = p;
            min_len = len;
        }

        /* Move to the next string. */
        p = start;
    }

    if(find_string == NX_TRUE)
        return(NX_MDNS_ERROR);

    /* If we reach this point, the string needs to be added to the string table. */
    if(available == (UCHAR*)tail)
    {

        /* Make sure the service cache still has room to add this string 
           (without overwriting the RR area.) */
        if(((UCHAR*)tail - memory_len) < (UCHAR*)head )
        {

            /* This service cache does not have room for the string table to grow. */
            /* Invoke user-installed cache full notify function .*/
            if(mdns_ptr -> nx_mdns_cache_full_notify)
            {

                /* Calculate the fragment size, RR size, string size, Head and Tail. */
                if(cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
                {
                    used_cache_size = mdns_ptr -> nx_mdns_local_rr_count * sizeof(NX_MDNS_RR) + mdns_ptr -> nx_mdns_local_string_bytes + 2 * sizeof(ULONG);
                }
                else
                {
                    used_cache_size = mdns_ptr -> nx_mdns_peer_rr_count * sizeof(NX_MDNS_RR) + mdns_ptr -> nx_mdns_peer_string_bytes + 2 * sizeof(ULONG);
                }

                /* Check the cache.  */
                if ((used_cache_size + memory_len) <= cache_size)
                {
                    (mdns_ptr -> nx_mdns_cache_full_notify)(mdns_ptr, NX_MDNS_CACHE_STATE_FRAGMENTED, cache_type);
                }
                else
                {
                    (mdns_ptr -> nx_mdns_cache_full_notify)(mdns_ptr, NX_MDNS_CACHE_STATE_FULL, cache_type);
                }                
            }

            /* The buffer is full. */
            return(NX_MDNS_CACHE_ERROR);
        }
        
        /* Update TAIL. */
        *((ULONG*)cache_ptr + (cache_size >> 2) - 1) = (ULONG)(available - memory_len);

    }
    else if(memory_len < min_len)
    {

        /* Set the LEN for remaining space. */
        *((USHORT*)(available - 2 - memory_len)) = (USHORT)(min_len - memory_len);
    }

    /* Set LEN and CNT. */
    *((USHORT*)(available - 2)) = (USHORT)memory_len;
    *((USHORT*)(available - 4)) = 1;

    /* Clear last 4 bytes. */
    *((ULONG*)(available - 8)) = 0;

    /* Insert string to cache. */
    memcpy(available - memory_len, memory_ptr, memory_size); /* Use case of memcpy is verified. */

    /* Set end character 0. */
    *(available - memory_len + memory_size) = 0;
    
    
    /* Check the type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {

        /* Increase the count and bytes.  */
        mdns_ptr -> nx_mdns_local_string_count ++;
        mdns_ptr -> nx_mdns_local_string_bytes += memory_len;
    }
    else
    {
        
        /* Increase the count and bytes.  */
        mdns_ptr -> nx_mdns_peer_string_count ++;
        mdns_ptr -> nx_mdns_peer_string_bytes += memory_len;
    }

    if(insert_ptr)
        *insert_ptr = available - memory_len;

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_delete_string                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the mDNS string from the record buffer.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    cache_type                            Cache type: local or peer     */ 
/*    string_ptr                            Pointer to the string         */ 
/*    string_len                            The length of string          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_rr_a_aaaa_add                Add A/AAAA resource record    */
/*    _nx_mdns_rr_ptr_add                   Add PTR resource record       */
/*    _nx_mdns_rr_srv_add                   Add SRV resource record       */
/*    _nx_mdns_rr_txt_add                   Add TXT resource record       */
/*    _nx_mdns_query                        Send query                    */
/*    _nx_mdns_conflict_process             Process the conflict          */
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*    _nx_mdns_packet_rr_process            Process resource record       */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_cache_delete_string(NX_MDNS *mdns_ptr, UINT cache_type, VOID *string_ptr, UINT string_len)
{

UCHAR   *cache_ptr;
UINT    cache_size;
ULONG   *tail;
ULONG   *end;
USHORT  cnt;


    /* Check the cache type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {
        cache_ptr = mdns_ptr -> nx_mdns_local_service_cache;
        cache_size = mdns_ptr -> nx_mdns_local_service_cache_size;
    }
    else
    {
        cache_ptr = mdns_ptr -> nx_mdns_peer_service_cache;
        cache_size = mdns_ptr -> nx_mdns_peer_service_cache_size;
    }

    /* Check cache pointer and cache size.  */
    if ((cache_ptr == NX_NULL) || (cache_size == 0))
    {            
        return(NX_MDNS_ERROR);
    }

    /* Validate input parameter. */
    if(string_ptr == NX_NULL)
        return(NX_MDNS_PARAM_ERROR);

    /* Check if input the string length.  */
    if (string_len == 0)
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check(string_ptr, &string_len, NX_MDNS_NAME_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }
    }

    /* Add the length of CNT and LEN fields.  */
    /* Also make the total length 4 bytes align. */
    string_len = ((string_len & 0xFFFFFFFC) + 8) & 0xFFFFFFFF;

    end = (ULONG*)((UCHAR*)string_ptr + string_len);

    /* Get tail. */

    /* Validate the string table. */
    tail = (ULONG*)cache_ptr + (cache_size >> 2) - 1;
    if(end > tail)
    {

        /* The end of string exceeds cache_ptr. */
        return(NX_MDNS_CACHE_ERROR);
    }
    tail = (ULONG*)(*tail);
    if((UCHAR*)string_ptr < (UCHAR*)tail)
    {

        /* The end of string exceeds cache_ptr. */
        return(NX_MDNS_CACHE_ERROR);
    }

    /* Decrease the usage counter value. */
    cnt = *((USHORT*)((UCHAR*)end - 4));
    cnt--;
    *((USHORT*)((UCHAR*)end - 4)) = cnt;

    /* Clear the memory if cnt is zero. */
    if(cnt == 0)
    {
        memset(string_ptr, 0, string_len - 2);
        
        /* Check the type.  */
        if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
        {

            /* Increase the count and bytes.  */
            mdns_ptr -> nx_mdns_local_string_count --;
            mdns_ptr -> nx_mdns_local_string_bytes -= string_len;
        }
        else
        {
            
            /* Increase the count and bytes.  */
            mdns_ptr -> nx_mdns_peer_string_count --;
            mdns_ptr -> nx_mdns_peer_string_bytes -= string_len;
        }

        /* Update the tail pointer if the string at the tail is deleted. */
        if(string_ptr == tail)
        {
            tail = end;
        
            while((end < ((ULONG*)cache_ptr + (cache_size >> 2) - 1)))
            {
                
                /* Set the string pt and string length.  */
                string_ptr = end;

                /* Check string length.  */
                if (_nx_utility_string_length_check(string_ptr, &string_len, NX_MDNS_NAME_MAX))
                {
                    return(NX_MDNS_DATA_SIZE_ERROR);
                }

                /* Check the string length.  */
                if(string_len == 0)
                {
                    
                    /* This slot is cleared. */
                    while(*((ULONG*)string_ptr) == 0)
                        string_ptr = (UCHAR*)string_ptr + 4;
                    
                    end = (ULONG*)string_ptr + 1;
                    cnt = *((USHORT*)string_ptr);
                }
                else
                {
                    
                    /* Make the length 4 bytes align and add the length of CNT and LEN fields.  */
                    string_len = ((string_len & 0xFFFFFFFC) + 8) & 0xFFFFFFFF;
                    
                    end = (ULONG*)((UCHAR*)string_ptr + string_len);
                    cnt = *((USHORT*)((UCHAR*)end - 4));
                }
                
                /* Check whether this slot is never referenced. */
                if(cnt == 0)
                    tail = end;
                else
                    break;
            }
            *((ULONG*)cache_ptr + (cache_size >> 2) - 1) = (ULONG)tail;
        }
    }

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_cache_delete_rr_string                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the mDNS string from the record buffer.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    cache_type                            Cache type: local or peer     */ 
/*    string_ptr                            Pointer to the string         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_mdns_cache_delete_string          Delete the string from cache  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_cache_add_resource_record    Add the resource record       */
/*    _nx_mdns_cache_delete_resource_record Delete the resource record    */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID _nx_mdns_cache_delete_rr_string(NX_MDNS *mdns_ptr, UINT cache_type, NX_MDNS_RR *record_rr)
{

    /* Delete the  name strings. */
    _nx_mdns_cache_delete_string(mdns_ptr, cache_type, record_rr -> nx_mdns_rr_name, 0);

    /* Check if need to release the rdata string.  */
    if (record_rr -> nx_mdns_rr_state != NX_MDNS_RR_STATE_QUERY)
    {
        
        if (((record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) ||
             (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_TXT) ||
             (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_PTR)
#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
             ||
             (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC)
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
             ||
             (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_CNAME) ||
             (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NS) ||
             (record_rr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_MX)
#endif /* NX_MDNS_ENABLE_EXTENDED_RR_TYPES  */
            ) &&
            (record_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target))
        {

            /* Delete the same strings. */
            _nx_mdns_cache_delete_string(mdns_ptr, cache_type, record_rr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target, 0);
        }
    }
}


#ifndef NX_MDNS_DISABLE_SERVER
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_additional_resource_record_find            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function find the mDNS additional resource record.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    record_ptr                            Pointer to record instance    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_response_send                Send response message         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_mdns_additional_resource_record_find(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_ptr)
{
    
ULONG           *head;
NX_MDNS_RR      *p;

    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
    head = (ULONG*)(*head);

    /* Find the additional resource records according to the type.  */
    switch (record_ptr -> nx_mdns_rr_type )
    {   
        
        case NX_MDNS_RR_TYPE_A:
        case NX_MDNS_RR_TYPE_AAAA:
        {
            
            /* Recommends AAAA records in the additional section when responding
               to rrtype "A" queries, and vice versa. RFC6762, Section19, Page51.  */
            _nx_mdns_additional_a_aaaa_find(mdns_ptr, record_ptr -> nx_mdns_rr_name, record_ptr -> nx_mdns_rr_interface_index);

            break;
        }        

        case NX_MDNS_RR_TYPE_PTR:
        {

            /* Find the additional resource record.  */
            for (p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
            {   

                /* Check the interface.  */
                if (p -> nx_mdns_rr_interface_index != record_ptr -> nx_mdns_rr_interface_index)
                    continue;

                /* The SRV record(s) named in the PTR rdata.
                   The TXT record(s) named in the PTR rdata.
                   All address records (type "A" and "AAAA") named in the SRV rdata. RFC6763, Section12.1, Page30.*/

                /* The SRV records named in the PTR rdata */
                if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV) &&
                    (p -> nx_mdns_rr_name == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name) && 
                    ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                     (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING)) &&
                    (p -> nx_mdns_rr_send_flag == NX_MDNS_RR_SEND_FLAG_CLEAR) &&
                    (!(p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_ANSWER)))
                {

                    /* Set the flag to add this resource records into additional records.  */
                    p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_ADDITIONAL;

                    /* All address records (type "A" and "AAAA") named in the SRV rdata */
                    _nx_mdns_additional_a_aaaa_find(mdns_ptr, p -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target, record_ptr -> nx_mdns_rr_interface_index);
                } 

                /* The TXT records named in the PTR rdata */
                if (((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_TXT) ||
                     (p ->nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC)) &&
                    (p -> nx_mdns_rr_name == record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_ptr.nx_mdns_rr_ptr_name) && 
                    ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                     (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING)) &&
                    (p -> nx_mdns_rr_send_flag == NX_MDNS_RR_SEND_FLAG_CLEAR) &&
                    (!(p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_ANSWER)))
                {
                    
                    /* Set the flag to add this resource records into additional records.  */
                    p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_ADDITIONAL;
                }
            }

            break;
        }
            
        case NX_MDNS_RR_TYPE_SRV:
        case NX_MDNS_RR_TYPE_TXT:
        {
                    
#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
            /* Find the additional resource record.  */
            for (p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
            {

                /* Check the interface.  */
                if (p -> nx_mdns_rr_interface_index != record_ptr -> nx_mdns_rr_interface_index)
                    continue;

                /* The NSEC records named same as SRV/TXT name.  */
                if ((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC) &&
                    (p -> nx_mdns_rr_name == record_ptr -> nx_mdns_rr_name) && 
                    ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
                     (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING)) &&
                    (p -> nx_mdns_rr_send_flag == NX_MDNS_RR_SEND_FLAG_CLEAR) &&
                    (!(p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_ANSWER)))
                {

                    /* Set the flag to add this resource records into additional records.  */
                    p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_ADDITIONAL;
                } 
            }
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */
                                                
            /* All address records (type "A" and "AAAA") named in the SRV rdata */
            if (record_ptr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_SRV)
                _nx_mdns_additional_a_aaaa_find(mdns_ptr, record_ptr -> nx_mdns_rr_rdata.nx_mdns_rr_rdata_srv.nx_mdns_rr_srv_target, record_ptr -> nx_mdns_rr_interface_index);
            break;
        }
    }

    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_additional_a_aaaa_find                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function find the mDNS additional A/AAAA resource record.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    record_ptr                            Pointer to record instance    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_additional_resource_record_find                            */ 
/*                                          Find additional record        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_mdns_additional_a_aaaa_find(NX_MDNS *mdns_ptr, UCHAR *name, UINT interface_index)
{
    
ULONG           *head;
NX_MDNS_RR      *p;

    head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
    head = (ULONG*)(*head);

    /* Recommends AAAA records in the additional section when responding
       to rrtype "A" queries, and vice versa. RFC6762, Section19, Page51.  */
    for (p = (NX_MDNS_RR*)((UCHAR*)mdns_ptr -> nx_mdns_local_service_cache + sizeof(ULONG)); (ULONG*)p < head; p++)
    {

        /* Check the interface index.  */
        if (p -> nx_mdns_rr_interface_index != interface_index)
            continue;

        /* Find the "A" records.  */
        if (((p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_A) ||
             (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_AAAA ||
             (p -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_NSEC))) &&
            (p -> nx_mdns_rr_name == name) && 
            ((p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_VALID) ||
            (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_ANNOUNCING)) &&
            (p -> nx_mdns_rr_send_flag == NX_MDNS_RR_SEND_FLAG_CLEAR) &&
            (!(p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_ANSWER)))
        {

            /* Set the flag to add this resource records into additional records.  */
            p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_ADDITIONAL;
        }             
    }
}
#endif /* NX_MDNS_DISABLE_SERVER */


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_known_answer_find                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds the mDNS known answer.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */
/*    record_ptr                            Pointer to record instance    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_query_send                   Send query message            */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_known_answer_find(NX_MDNS *mdns_ptr, NX_MDNS_RR *record_ptr)
{
   
UINT            status = NX_MDNS_NO_KNOWN_ANSWER;
ULONG           *head, *tail;
NX_MDNS_RR      *p;
UINT            i;
UINT            name_length;
#ifndef NX_MDNS_DISABLE_SERVER
UINT            cache_count = 2;
#else
UINT            cache_count = 1;
#endif

    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)(record_ptr -> nx_mdns_rr_name), &name_length, NX_MDNS_NAME_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    /* Loop to find the known answer in cache.  */
    for (i = 0; i < cache_count; i++)
    {

        /* Set the pointer. */
        if (i == 0)
        {
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;
        }
#ifndef NX_MDNS_DISABLE_SERVER
        else
        {
            head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
        }
#endif /* NX_MDNS_DISABLE_SERVER  */
        tail = (ULONG*)(*head);

        /* Whether this mDNS message response to query. */
        for(p = (NX_MDNS_RR*)(head + 1); (ULONG*)p < tail; p++)
        {

            /* Check the state.  */
            if(p -> nx_mdns_rr_state != NX_MDNS_RR_STATE_VALID)
                continue;

            /* Check the interface index.  */
            if (p -> nx_mdns_rr_interface_index != record_ptr -> nx_mdns_rr_interface_index)
                continue;

            /* Check whether the Known Answer record it is. */
            if ((!_nx_mdns_name_match(p -> nx_mdns_rr_name, record_ptr -> nx_mdns_rr_name, name_length)) &&
                (p -> nx_mdns_rr_class == record_ptr -> nx_mdns_rr_class))
            {

                if ((record_ptr -> nx_mdns_rr_type == NX_MDNS_RR_TYPE_ALL) ||
                    (p -> nx_mdns_rr_type == record_ptr -> nx_mdns_rr_type))
                {

                    /* Set the additional flag.  */
                    p -> nx_mdns_rr_word = p -> nx_mdns_rr_word | NX_MDNS_RR_FLAG_KNOWN_ANSWER;

                    /* Yes, find the known answer.  */
                    status = NX_MDNS_SUCCESS;
                }
            }
        }
    }

    return(status);
}
#endif /* NX_MDNS_DISABLE_CLIENT */


#ifndef NX_MDNS_DISABLE_CLIENT
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_query_check                                PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks whether mDNS querier already has one Unique    */ 
/*    record in the local buffer/remote buffer or one same query in the   */ 
/*    remote buffer.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    name                                  The resource record name      */ 
/*    type                                  The resource record type      */ 
/*    interface_index                       The interface index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */   
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_query                        Send the query                */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_query_check(NX_MDNS *mdns_ptr, UCHAR *name, USHORT type, UINT one_shot, NX_MDNS_RR **search_rr, UINT interface_index)
{
    
ULONG       *head;
NX_MDNS_RR  *p;
UCHAR       i;
UCHAR       same_query = NX_FALSE;
NX_MDNS_RR  *rr = NX_NULL;
UCHAR       srv_flag = NX_FALSE;
UCHAR       txt_flag = NX_FALSE;
UINT        name_length;


    /* Check string length.  */
    if (_nx_utility_string_length_check((CHAR *)name, &name_length, NX_MDNS_NAME_MAX))
    {
        return(NX_MDNS_DATA_SIZE_ERROR);
    }

    for(i = 0; i < 2; i++)
    {
        if(i == NX_MDNS_CACHE_TYPE_LOCAL)
            head = (ULONG*)mdns_ptr -> nx_mdns_local_service_cache;
        else
            head = (ULONG*)mdns_ptr -> nx_mdns_peer_service_cache;

        if(head)
        {
            for(p = (NX_MDNS_RR*)(head + 1); (ULONG)p < *head; p++)
            {

                /* Check whether the resource record is valid. */
                if (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_INVALID)
                    continue;

                /* Check the interface index.  */
                if (p -> nx_mdns_rr_interface_index != interface_index)
                    continue;

                if (!_nx_mdns_name_match(p -> nx_mdns_rr_name, name, name_length))
                {    
                    if ((p -> nx_mdns_rr_type == type) && (p -> nx_mdns_rr_state == NX_MDNS_RR_STATE_QUERY))
                    {

                        /* Set the query.  */
                        same_query = NX_TRUE; 

                        /* Record the RR potiner.  */
                        rr = p;
                    }

                    /* Check the rr type.  */
                    if (p -> nx_mdns_rr_type  == NX_MDNS_RR_TYPE_SRV)
                        srv_flag = NX_TRUE;       
                    if (p -> nx_mdns_rr_type  == NX_MDNS_RR_TYPE_TXT)
                        txt_flag = NX_TRUE;

                    if ((p -> nx_mdns_rr_type == type) ||
                        ((type == NX_MDNS_RR_TYPE_ALL) && (srv_flag == NX_TRUE) && (txt_flag == NX_TRUE)))
                    {
                        if (search_rr)
                            *search_rr = p;

                        /* Check the query type.  */
                        if (one_shot == NX_TRUE)
                            return(NX_MDNS_EXIST_SHARED_RR);

                        if (p -> nx_mdns_rr_word & NX_MDNS_RR_FLAG_UNIQUE) 
                            return(NX_MDNS_EXIST_UNIQUE_RR);
                    }
                }
            }
        }
    }

    /* Check the same query.  */
    if (same_query == NX_TRUE)
    {

        /* A multicast DNS querier should also delay the first query of the series by 
           a randomly chosen amount in the range 20-120ms.  */
        rr -> nx_mdns_rr_timer_count = (ULONG)(NX_MDNS_QUERY_DELAY_MIN + (((ULONG)NX_RAND()) % NX_MDNS_QUERY_DELAY_RANGE));
        rr -> nx_mdns_rr_retransmit_lifetime = NX_MDNS_TIMER_COUNT_RANGE;

        /* Set the mDNS timer.  */
        _nx_mdns_timer_set(mdns_ptr, rr, rr -> nx_mdns_rr_timer_count);    

        return (NX_MDNS_EXIST_SAME_QUERY);
    }         

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_query_cleanup                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes mDNS One-shot query timeout and thread      */ 
/*    terminate actions that require the mDNS data structures to be       */ 
/*    cleaned up.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to suspended thread's */ 
/*                                            control block               */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_resume              Resume thread service         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_thread_timeout                    Thread timeout processing     */ 
/*    _tx_thread_terminate                  Thread terminate processing   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_mdns_query_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER)
{

TX_INTERRUPT_SAVE_AREA

NX_MDNS     *mdns_ptr;                

   
    NX_CLEANUP_EXTENSION
 
    /* Setup pointer to UDP socket control block.  */
    mdns_ptr =  (NX_MDNS *) thread_ptr -> tx_thread_suspend_control_block;

    /* Disable interrupts to remove the suspended thread from the UDP socket.  */
    TX_DISABLE

    /* Determine if the cleanup is still required.  */
    if ((thread_ptr -> tx_thread_suspend_cleanup) && (mdns_ptr) &&
        (mdns_ptr -> nx_mdns_id == NX_MDNS_ID))
    {

        /* Yes, we still have thread suspension!  */

        /* Clear the suspension cleanup flag.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Remove the suspended thread from the list.  */

        /* See if this is the only suspended thread on the list.  */
        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            mdns_ptr -> nx_mdns_rr_receive_suspension_list =  NX_NULL;
        }
        else
        {

            /* At least one more thread is on the same suspension list.  */

            /* Update the list head pointer.  */
            mdns_ptr -> nx_mdns_rr_receive_suspension_list =  thread_ptr -> tx_thread_suspended_next;

            /* Update the links of the adjacent threads.  */
            (thread_ptr -> tx_thread_suspended_next) -> tx_thread_suspended_previous = thread_ptr -> tx_thread_suspended_previous;
            (thread_ptr -> tx_thread_suspended_previous) -> tx_thread_suspended_next = thread_ptr -> tx_thread_suspended_next;
        } 
 
        /* Decrement the suspension count.  */
        mdns_ptr -> nx_mdns_rr_receive_suspended_count--;

        /* Now we need to determine if this cleanup is from a terminate, timeout,
           or from a wait abort.  */
        if (thread_ptr -> tx_thread_state == TX_TCP_IP)
        {

            /* Thread still suspended on the UDP socket.  Setup return error status and
               resume the thread.  */

            /* Setup return status.  */
            thread_ptr -> tx_thread_suspend_status = NX_MDNS_NO_RR;

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Resume the thread!  Check for preemption even though we are executing 
               from the system timer thread right now which normally executes at the 
               highest priority.  */
            _tx_thread_system_resume(thread_ptr);

            /* Finished, just return.  */
            return;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_mdns_query_thread_suspend                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function suspends a thread on a mDNS client query service      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    suspension_list_head                  Pointer to the suspension list*/
/*    mutex_ptr                             Pointer to mutex to release   */
/*    suspend_cleanup                       Suspension cleanup routine    */
/*    wait_option                           Optional timeout value        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_put                          Release protection            */
/*    _tx_thread_system_suspend             Suspend thread                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_mdns_query_thread_suspend(TX_THREAD **suspension_list_head, VOID (*suspend_cleanup)(TX_THREAD * NX_CLEANUP_PARAMETER),
                                    NX_MDNS *mdns_ptr, NX_MDNS_RR **rr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;


    /* Lockout interrupts.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* Setup suspension list.  */
    if (*suspension_list_head)
    {

        /* This list is not NULL, add current thread to the end. */
        thread_ptr -> tx_thread_suspended_next = *suspension_list_head;
        thread_ptr -> tx_thread_suspended_previous = (*suspension_list_head) -> tx_thread_suspended_previous;
        ((*suspension_list_head) -> tx_thread_suspended_previous) -> tx_thread_suspended_next = thread_ptr;
        (*suspension_list_head) -> tx_thread_suspended_previous = thread_ptr;
    }
    else
    {

        /* No other threads are suspended.  Setup the head pointer and
            just setup this threads pointers to itself.  */
        *suspension_list_head = thread_ptr;
        thread_ptr -> tx_thread_suspended_next = thread_ptr;
        thread_ptr -> tx_thread_suspended_previous = thread_ptr;
    }

    /* Setup return status.  */
    thread_ptr -> tx_thread_suspend_status = NX_MDNS_NO_RR;

    /* Setup cleanup routine pointer.  */
    thread_ptr -> tx_thread_suspend_cleanup = suspend_cleanup;

    /* Setup cleanup information, i.e. this pool control block.  */
    thread_ptr -> tx_thread_suspend_control_block = (void *)mdns_ptr;

    /* Save the return RR pointer address as well.  */
    thread_ptr -> tx_thread_additional_suspend_info = (void *)rr;

    /* Increment the suspended thread count.  */
    mdns_ptr -> nx_mdns_rr_receive_suspended_count++;

    /* Set the state to suspended.  */
    thread_ptr -> tx_thread_state = TX_TCP_IP;

    /* Set the suspending flag.  */
    thread_ptr -> tx_thread_suspending = TX_TRUE;

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Save the timeout value.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks = wait_option;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Call actual thread suspension routine.  */
    _tx_thread_system_suspend(thread_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_query_thread_resume                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resumes a thread suspended on a mDNS Client query     */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to suspended thread's */ 
/*                                            control block               */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_resume              Resume thread service         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_thread_timeout                    Thread timeout processing     */ 
/*    _tx_thread_terminate                  Thread terminate processing   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_mdns_query_thread_resume(TX_THREAD **suspension_list_head, NX_MDNS *mdns_ptr, NX_MDNS_RR *rr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the thread pointer.  */
    thread_ptr =  *suspension_list_head;

    /* Determine if there still is a thread suspended.  */
    if (thread_ptr)
    {

        /* Remove the suspended thread from the list.  */

        /* See if this is the only suspended thread on the list.  */
        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            *suspension_list_head = TX_NULL;
        }
        else
        {

            /* At least one more thread is on the same expiration list.  */

            /* Update the list head pointer.  */
            *suspension_list_head = thread_ptr -> tx_thread_suspended_next;

            /* Update the links of the adjacent threads.  */
            (thread_ptr -> tx_thread_suspended_next) -> tx_thread_suspended_previous = thread_ptr -> tx_thread_suspended_previous;
            (thread_ptr -> tx_thread_suspended_previous) -> tx_thread_suspended_next = thread_ptr -> tx_thread_suspended_next;
        } 

        /* Decrement the suspension count.  */
        mdns_ptr -> nx_mdns_rr_receive_suspended_count--;

        /* Prepare for resumption of the first thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Return this block pointer to the suspended thread waiting for
           a block.  */
        *((NX_MDNS_RR **) thread_ptr -> tx_thread_additional_suspend_info) =  rr;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Put return status into the thread control block.  */
        thread_ptr -> tx_thread_suspend_status =  NX_MDNS_SUCCESS;

        /* Resume thread.  */
        _tx_thread_system_resume(thread_ptr);
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_service_mask_match                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function matchs the mDNS service mask to listen the service    */ 
/*    and notify the application, or ignore the service.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mdns_ptr                              Pointer to mDNS instance      */ 
/*    service_type                          Pointer to Service type       */ 
/*    service_mask                          The Service mask              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the mDNS mutex            */ 
/*    tx_mutex_put                          Put the mDNS mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_service_mask_match(NX_MDNS *mdns_ptr, UCHAR *service_type, ULONG service_mask)
{
   
UINT        i;
UINT        found;
ULONG       mask;
UINT        type_length;


    NX_PARAMETER_NOT_USED(mdns_ptr);

    /* Initialize the parameters.  */
    i = 0;
    found = 0;
    mask = 0x00000001;

    /* Compare the service type.  */
    while (nx_mdns_service_types[i])
    {

        /* Check string length.  */
        if (_nx_utility_string_length_check((CHAR *)nx_mdns_service_types[i], &type_length, NX_MDNS_TYPE_MAX))
        {
            return(NX_MDNS_DATA_SIZE_ERROR);
        }

        /* Find the same service type.  */
        if (!_nx_mdns_name_match(service_type, (UCHAR *)(nx_mdns_service_types[i]), type_length))
        {
            found = 1;
            break;
        }
        i ++;
    } 

    /* Check the result.  */
    if (found)
    {

        /* Check service mask.  */
        if (service_mask & (mask << i))
        {
              return(NX_MDNS_SUCCESS);
        }
    }

    /* Return status.  */
    return(NX_MDNS_SERVICE_TYPE_MISMATCH);
}
#endif /* NX_MDNS_DISABLE_CLIENT */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_name_match                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function name string match, the lowercase letters "a" to "z"   */ 
/*    match their uppercase equivalents "A" to "Z".                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to destination        */ 
/*    name                                  Source name string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_rr_add                Add the RR into packet        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_mdns_name_match(UCHAR *src, UCHAR *dst, UINT length)
{

UINT    index = 0;
   

    /* Check the name.  */
    while (*dst != '\0')
    {
        if((*src) != (*dst))
        {
            if((((*src) | 0x20) >= 'a') && (((*src) | 0x20) <= 'z'))
            {
                /* Match the characters, case in-sensitive. */
                if(((*src) | 0x20) != ((*dst) | 0x20))
                    return(NX_MDNS_NAME_MISMATCH);
            }
            else
            {
                return(NX_MDNS_NAME_MISMATCH);
            }
        }
        src ++;
        dst ++;
        index ++;
    }

    /* Check the scan length.  */
    if (index != length)
    {
        return (NX_MDNS_NAME_MISMATCH);
    }

    /* Return success.  */
    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_txt_string_encode                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function encodes the txt string and adds the txt string into   */ 
/*    packet.the TXT records are formatted in a "key=value" notation      */
/*    with ";" acting as separator when more then one key is available.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to destination        */ 
/*    name                                  Source name string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_rr_add                Add the RR into packet        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_mdns_txt_string_encode(UCHAR *ptr, UCHAR *txt)
{

UCHAR   *length;
UINT    count =  1;


    /* Point to the first character position in the buffer.  This will point
       to the length of the following name.  */
    length =  ptr++;

    /* Default the length to zero.  */
    *length =  0;

    /* Move through string, copying bytes and updating length.
       Whenever a "." is found, start a new string by updating the
       pointer to the length and setting the length to zero.  */
    while (*txt) 
    {

        /* Is a dot been found?  */
        if (*txt == ';')
        { 


            /* Yes, setup a new length pointer. */
            length =  ptr++;

            /* Default the length to zero. */
            *length =  0;
            txt++;
        }
        else
        {

            /* Copy a character to the destination.  */
            *ptr++ =  *txt++;

            /* Adjust the length of the current segment.  */
            (*length)++;
        }

        /* Increment the total count here.  */
        count++;
    }

    /* Return the count.  */
    return(count);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_txt_string_decode                          PORTABLE C      */ 
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function decode the txt string and adds the txt string into    */ 
/*    buffer.the TXT records are formatted in a "key=value"  notation     */
/*    with ";" acting as separator when more then one key is available.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data                                  Pointer to buffer to decode   */ 
/*    data_length                           The length of data            */
/*    buffer                                Pointer to decoded data       */ 
/*    size                                  Size of data buffer to decode */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_mdns_packet_process               Process mDNS packet           */
/*    _nx_mdns_packet_rr_process            Process resource record       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_txt_string_decode(UCHAR *data, UINT data_length, UCHAR *buffer, UINT size)
{
    

    /* Check the buffer size.  */
    if (data_length >= size)
    {
        return (NX_MDNS_CACHE_ERROR);
    }

    /* decode  the data.   */
    while (data_length)
    {

    UINT  labelSize =  *data++;

        /* Is this a compression pointer or a count.  */
        if (labelSize <= NX_MDNS_LABEL_MAX)
        {

            /* Simple count, check for space and copy the label.  */
            while ((labelSize > 0) && (data_length > 1))
            {

                *buffer++ =  *data++;
                labelSize--;
                data_length--;
            }
      
            /* Now add the ';' */
            *buffer++ =  ';';
            data_length--;
        }
        else
        {
            return(NX_MDNS_EXCEED_MAX_LABEL);
        }
    }

    /* Done copying the data, set the last . to a trailing null */
    if (*(buffer - 1) == ';')
    {

        buffer--;
    }
    
    /* Null terminate name.  */
    *buffer =  '\0';

    /* Return name size.  */
    return(NX_MDNS_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_name_size_calculate                        PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calculates the size of the name.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    name                                  Pointer to the name           */ 
/*    packet_ptr                            Pointer to received packet    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    UINT                                  Size of name                  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_mdns_name_size_calculate(UCHAR *name, NX_PACKET *packet_ptr)
{

UINT size =  0;


    /* As long as we haven't found a zero length terminating label */
    while (*name != '\0')
    {

        UINT  labelSize =  *name++;

        /* Is this a compression pointer or a count.  */
        if (labelSize <= NX_MDNS_LABEL_MAX)
        {

            if (name + labelSize >= packet_ptr -> nx_packet_append_ptr)
            {

                /* If name buffer is OOB, just fail. */
                return(0);
            }

            /* Simple count, adjust size and skip the label.  */
            size +=  labelSize + 1;
            name +=  labelSize;
        }
        else if ((labelSize & NX_MDNS_COMPRESS_MASK) == NX_MDNS_COMPRESS_VALUE)
        {

            /* This is a pointer size is 2 bytes and this is the end of this name */
            return(size + 2);
        }
        else
        {

            /* Not defined, just fail */
            return(0);
        }
    }

    /* Adjust size for the final NULL.  */
    return(size + 1);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_name_string_encode                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a string containing the name as a list of    */ 
/*    labels separated by dots to the encoded list of labels specified    */ 
/*    in RFC1035 for DNS servers. This conversion doesn't handle          */ 
/*    compression and doesn't check on the lengths of the labels or the   */ 
/*    entire name.                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to destination        */ 
/*    name                                  Source name string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    count                                 Count of characters encoded   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_mdns_name_string_encode(UCHAR *ptr, UCHAR *name)
{

UCHAR   *length;
UINT    count =  1;


    /* Point to the first character position in the buffer.  This will point
       to the length of the following name.  */
    length =  ptr++;

    /* Default the length to zero.  */
    *length =  0;

    /* Move through string, copying bytes and updating length.
       Whenever a "." is found, start a new string by updating the
       pointer to the length and setting the length to zero.  */
    while (*name) 
    {

        /* Is a dot been found?  */
        if (*name == '.')
        { 

            /* Yes, setup a new length pointer. */
            length =  ptr++;

            /* Default the length to zero. */
            *length =  0;
            name++;
        }
        else
        {

            /* Copy a character to the destination.  */
            *ptr++ =  *name++;

            /* Adjust the length of the current segment.  */
            (*length)++;
        }

        /* Increment the total count here.  */
        count++;
    }

    /* Add the final zero length, like a NULL terminator.  */
    *ptr =  0;

    /* Increment the total count.  */
    count++;

    /* Return the count.  */
    return(count);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_name_string_decode                       PORTABLE C        */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts from the encoded list of labels as specified */ 
/*    in RFC 1035 to a string containing the name as a list of labels     */ 
/*    separated by dots.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data                                  Pointer to buffer to decode   */ 
/*    start                                 Location of start of data     */
/*    data_length                           Length of data buffer         */
/*    buffer                                Pointer to decoded data       */ 
/*    size                                  Size of data buffer to decode */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Size of decoded data                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            prevented infinite loop in  */
/*                                            name compression,           */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_mdns_name_string_decode(UCHAR *data, UINT start, UINT data_length, UCHAR *buffer, UINT size)
{

UCHAR   *character =  data + start;
UINT    length = 0;
UINT    offset;
UINT    pointer_count = 0;
UINT    labelSize;

  
    /* As long as there is space in the buffer and we haven't 
       found a zero terminating label */
    while (1)
    {

        if (character >= data + data_length)
        {
            return(0);
        }

        if ((size <= length) || (*character == '\0'))
        {
            break;
        }

        labelSize =  *character++;

        /* Is this a compression pointer or a count.  */
        if (labelSize <= NX_MDNS_LABEL_MAX)
        {

            /* Simple count, check for space and copy the label.  */
            while ((size > length) && (labelSize > 0))
            {
                if (character >= data + data_length)
                {
                    return(0);
                }

                *buffer++ =  *character++;
                length++;
                labelSize--;
            }
      
            /* Now add the '.' */
            *buffer++ =  '.';
            length++;
        }
        else if ((labelSize & NX_MDNS_COMPRESS_MASK) == NX_MDNS_COMPRESS_VALUE)
        {

            if (character >= data + data_length)
            {
                return(0);
            }

            /* This is a pointer, just adjust the source.  */
            offset = ((labelSize & NX_MDNS_LABEL_MAX) << 8) + *character;

            /* Make sure offset is in the buffer.  */
            if (offset >= data_length)
            {
                return(0);
            }

            /* Pointer must not point back to itself. */
            if ((data + offset == character) || (data + offset == character - 1))
            {
                return(0);
            }

            /* Prevent infinite loop with compression pointers. */
            pointer_count++;
            if (pointer_count > NX_MDNS_MAX_COMPRESSION_POINTERS)
            {
                return(0);
            }

            character =  data + offset;
        }
        else
        {

            /* Not defined, just fail */
            return(0);
        }
    }

    /* Done copying the data, set the last . to a trailing null */
    if ((length > 0) && (*(buffer - 1) == '.'))
    {

        buffer--;
        length --;
    }
    
    /* Null terminate name.  */
    *buffer =  '\0';

    /* Return name size.  */
    return(length);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_rr_size_get                                PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets the size of resource record.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    resource                              Pointer to the resource       */ 
/*    packet_ptr                            Pointer to received packet    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    size of data                                                        */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_mdns_rr_size_get(UCHAR *resource, NX_PACKET *packet_ptr)
{

UINT    name_size;
UINT    data_size;

    /* Resource size is 
        name size + data size + 2 bytes for type, 2 for class, 4 for time to live and 2 for data length
        i.e. name size + data size + 10 bytes overhead.
    */
    name_size = _nx_mdns_name_size_calculate(resource, packet_ptr);

    if (resource + name_size + 8 + 2 > packet_ptr -> nx_packet_append_ptr)
    {
        return(0);
    }
    data_size = NX_MDNS_GET_USHORT_DATA(resource + name_size + 8);

    /* Return resource size.  */
    return(name_size + data_size + 10);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_short_to_network_convert                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts an unsigned short to network byte order,     */ 
/*    which is big endian.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to the destination    */ 
/*    value                                 Source value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    DNS component                                                       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static void  _nx_mdns_short_to_network_convert(UCHAR *ptr, USHORT value)
{
    
    *ptr++ =  (UCHAR)(value >> 8);
    *ptr   =  (UCHAR)value;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_mdns_long_to_network_convert                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts an unsigned long to network format, which    */ 
/*    big endian.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to the destination    */ 
/*    value                                 Source value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    DNS component                                                       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static void _nx_mdns_long_to_network_convert(UCHAR *ptr, ULONG value)
{
  
    *ptr++ =  (UCHAR)(value >> 24);
    *ptr++ =  (UCHAR)(value >> 16);
    *ptr++ =  (UCHAR)(value >> 8);
    *ptr   =  (UCHAR)value;
}
