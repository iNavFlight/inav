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
/**   Domain Name System (DNS)                                            */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_DNS_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nxd_dns.h"
#include    "stdio.h"
#include    "string.h"
#include    "nx_system.h"

/* Define the resource record section.  */
#define NX_DNS_RR_ANSWER_SECTION        1
#define NX_DNS_RR_AUTHORITY_SECTION     2
#define NX_DNS_RR_ADDITIONAL_SECTION    3

/* Internal DNS functions. */  
static UINT        _nx_dns_header_create(UCHAR *buffer_ptr, USHORT id, USHORT flags);
static UINT        _nx_dns_new_packet_create(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *name, USHORT type);
static UINT        _nx_dns_name_size_calculate(UCHAR *name, NX_PACKET *packet_ptr);
static UINT        _nx_dns_name_string_encode(UCHAR *ptr, UCHAR *name);
static UINT        _nx_dns_name_string_unencode(NX_PACKET *packet_ptr, UCHAR *data, UCHAR *buffer, UINT buffer_size);
static USHORT      _nx_dns_network_to_short_convert(UCHAR *ptr);
static ULONG       _nx_dns_network_to_long_convert(UCHAR *ptr);
static UINT        _nx_dns_question_add(NX_PACKET *packet_ptr, UCHAR *name, USHORT type);
static UCHAR *     _nx_dns_resource_data_address_get(UCHAR *resource, NX_PACKET *packet_ptr);
static UINT        _nx_dns_resource_data_length_get(UCHAR *resource, NX_PACKET *packet_ptr, UINT *length);
static UINT        _nx_dns_resource_type_get(UCHAR *resource, NX_PACKET *packet_ptr, UINT *resource_type);
static UINT        _nx_dns_resource_size_get(UCHAR *resource, NX_PACKET *packet_ptr, UINT *resource_size);
static VOID        _nx_dns_short_to_network_convert(UCHAR *ptr, USHORT value);
#ifndef NX_DISABLE_IPV4
static UINT        _nx_dns_number_to_ascii_convert(UINT number, CHAR *buffstring);
#endif /* NX_DISABLE_IPV4 */
static UINT        _nx_dns_host_resource_data_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, UCHAR *record_buffer, UINT buffer_size, 
                                                          UINT *record_count, UINT lookup_type, ULONG wait_option);
static UINT        _nx_dns_response_receive(NX_DNS *dns_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
static UINT        _nx_dns_response_process(NX_DNS *dns_ptr, UCHAR *host_name, NX_PACKET *packet_ptr, UCHAR *record_buffer, UINT buffer_size, UINT *record_count);
static UINT        _nx_dns_process_a_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, UINT *record_count, UINT rr_location);
static UINT        _nx_dns_process_aaaa_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, UINT *record_count, UINT rr_location);
   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
static UINT        _nx_dns_resource_name_real_size_calculate(UCHAR *data, UINT start, UINT data_length);
static UINT        _nx_dns_process_cname_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR *record_buffer, UINT buffer_size, UINT *record_count);
static UINT        _nx_dns_process_txt_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR *record_buffer, UINT buffer_size, UINT *record_count);
static UINT        _nx_dns_process_ns_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, UINT *record_count);
static UINT        _nx_dns_process_mx_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, UINT *record_count);
static UINT        _nx_dns_process_srv_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, UINT *record_count);
static UINT        _nx_dns_process_soa_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR *record_buffer, UINT buffer_size, UINT *record_count);
#endif                

#ifdef NX_DNS_CACHE_ENABLE   
static UINT        _nx_dns_name_match(UCHAR *src, UCHAR *dst, UINT length);
static UINT        _nx_dns_cache_add_rr(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, NX_DNS_RR *record_ptr, NX_DNS_RR **insert_ptr);     
static UINT        _nx_dns_cache_find_answer(NX_DNS *dns_ptr, VOID *cache_ptr, UCHAR *query_name, USHORT query_type, UCHAR *buffer, UINT buffer_size, UINT *record_count);
static UINT        _nx_dns_cache_delete_rr(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, NX_DNS_RR *record_ptr);   
static UINT        _nx_dns_cache_delete_rr_string(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, NX_DNS_RR *record_ptr);
static UINT        _nx_dns_cache_add_string(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, VOID *string_ptr, UINT string_size, VOID **insert_ptr);
static UINT        _nx_dns_cache_delete_string(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, VOID *string_ptr, UINT string_len);  
static UINT        _nx_dns_resource_time_to_live_get(UCHAR *resource, NX_PACKET *packet_ptr, ULONG *rr_ttl);
#endif /* NX_DNS_CACHE_ENABLE  */

#ifdef FEATURE_NX_IPV6
static VOID        _nxd_dns_build_an_ipv6_question_string(NXD_ADDRESS *ip_address, UCHAR *buffer, UINT len);
#endif                                         
static UINT        _nx_dns_server_add_internal(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);
static UINT        _nx_dns_server_remove_internal(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);      
static UINT        _nx_dns_server_get_internal(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *server_address);
static UINT        _nx_dns_host_by_address_get_internal(NX_DNS *dns_ptr, NXD_ADDRESS *host_address_ptr, UCHAR *host_name_ptr, 
                                                        UINT host_name_buffer_size, ULONG wait_option); 
static UINT        _nx_dns_send_query_by_address(NX_DNS *dns_ptr, NXD_ADDRESS *dns_server, UCHAR *ip_question, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
static UINT        _nx_dns_send_query_get_rdata_by_name(NX_DNS *dns_ptr, NXD_ADDRESS *server_address, UCHAR *host_name, UCHAR *record_buffer, 
                                                         UINT buffer_size, UINT *record_count, UINT dns_record_type, ULONG wait_option);

/* static VOID        _nx_dns_long_to_network_convert(UCHAR *ptr, ULONG value); */
/* static UINT        _nx_dns_resource_class_get(UCHAR *resource); */
/* static UINT        _nx_dns_resource_time_to_live_get(UCHAR *resource, NX_PACKET *packet_ptr, ULONG *rr_ttl); */
/* static UINT        _nx_dns_resource_name_get(UCHAR *buffer, UINT start, UCHAR *destination, UINT size); */


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

UCHAR lookup_end[] =  "IN-ADDR.ARPA";

#ifdef NX_DNS_CACHE_ENABLE  
static NX_DNS_RR   temp_rr;
#endif /* NX_DNS_CACHE_ENABLE  */

static UCHAR       temp_string_buffer[NX_DNS_NAME_MAX + 1];

/* Record the temp host name,*/
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
#define TEMP_SRV_BUFFER_SIZE    (NX_DNS_NAME_MAX + 1 + sizeof (NX_DNS_SRV_ENTRY))
#endif

NX_DNS *_nx_dns_instance_ptr;

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_create                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS create function call.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    domain_name                           DNS name                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_create                        Actual DNS create function    */ 
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
UINT  _nxe_dns_create(NX_DNS *dns_ptr, NX_IP *ip_ptr, UCHAR *domain_name)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id == NX_DNS_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS create service.  */
    status =  _nx_dns_create(dns_ptr, ip_ptr, domain_name);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_create                                      PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a DNS instance for the specified IP. This     */ 
/*    involves creating a UDP DNS socket, and a mutex for mutual exclusion*/
/*    of DNS requests, and binding the DNS socket to the appropriate port.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    domain_name                           DNS name                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_create                  Create DNS UDP socket         */ 
/*    nx_udp_socket_delete                  Delete DNS UDP socket         */ 
/*    tx_mutex_create                       Create a ThreadX mutex        */
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
/*  02-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            randomized the source port, */
/*                                            resulting in version 6.1.4  */
/*  07-29-2022     Jidesh Veeramachaneni    Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_dns_create(NX_DNS *dns_ptr, NX_IP *ip_ptr, UCHAR *domain_name)
{

UINT            status;

    /* If the host does intend to supply their own packet pool, create one here. */
#ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Create the DNS packet pool.  */
    status =  nx_packet_pool_create(&(dns_ptr -> nx_dns_pool), "DNS Pool", NX_DNS_PACKET_PAYLOAD, 
                                    dns_ptr -> nx_dns_pool_area,  NX_DNS_PACKET_POOL_SIZE);

    /* Check status of packet pool create.  */
    if (status != NX_SUCCESS)
    {

        /* Return an error.  */
        return(NX_DNS_ERROR);
    }

    /* Set an internal packet pool pointer to the newly created packet pool. */
    dns_ptr -> nx_dns_packet_pool_ptr = &dns_ptr -> nx_dns_pool;

#endif

    /* Create the DNS UDP socket.  */
    nx_udp_socket_create(ip_ptr, &(dns_ptr -> nx_dns_socket), "DNS Socket",
                         NX_DNS_TYPE_OF_SERVICE, NX_DNS_FRAGMENT_OPTION, NX_DNS_TIME_TO_LIVE, NX_DNS_QUEUE_DEPTH);

    /* Create a DNS mutex for multi-thread access protection.  */
    status =  tx_mutex_create(&dns_ptr -> nx_dns_mutex, "DNS Mutex", TX_NO_INHERIT);

    /* Check status of mutex create.  */
    if (status != TX_SUCCESS)
    {

#ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

       /* Delete the packet pool. */
        nx_packet_pool_delete(dns_ptr -> nx_dns_packet_pool_ptr);
#endif

        /* Delete the socket.  */
        nx_udp_socket_delete(&(dns_ptr -> nx_dns_socket));

        /* Return the threadx error.  */
        return(status);
    }
    
    /* Save the IP structure pointer.  */
    dns_ptr -> nx_dns_ip_ptr =   ip_ptr;
    dns_ptr -> nx_dns_domain = domain_name;

    /* Initialize the rest of the DNS structure.  */
    dns_ptr -> nx_dns_id =      NX_DNS_ID;

    /* Clear memory except for client DNS server array. */
    memset(&dns_ptr -> nx_dns_server_ip_array[0], 0, NX_DNS_MAX_SERVERS * sizeof(NXD_ADDRESS));

    /* Setup the maximum retry.  */
    dns_ptr -> nx_dns_retries =  NX_DNS_MAX_RETRIES;

    /* Is the client's network gateway also the primary DNS server? */   
#if defined(NX_DNS_IP_GATEWAY_AND_DNS_SERVER) && !defined(NX_DISABLE_IPV4)

    /* Verify we have a non zero gateway IP address. */
    if(ip_ptr -> nx_ip_gateway_address != 0)
    {

        /* Yes; Assign the IP's gateway address to the first entry in the Client DNS server list. */       
        dns_ptr -> nx_dns_server_ip_array[0].nxd_ip_version = NX_IP_VERSION_V4;
        dns_ptr -> nx_dns_server_ip_array[0].nxd_ip_address.v4 = ip_ptr -> nx_ip_gateway_address; 
    }

#endif /* defined(NX_DNS_IP_GATEWAY_AND_DNS_SERVER) && !defined(NX_DISABLE_IPV4) */

    _nx_dns_instance_ptr = dns_ptr;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_packet_pool_set                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the set the DNS Client    */
/*    packet pool service.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    packet_pool_ptr                       Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    NX_NOT_ENABLED                        DNS client not enabled for    */ 
/*                                             user create packet pool    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_packet_pool_set               Actual set packet pool service*/ 
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
UINT  _nxe_dns_packet_pool_set(NX_DNS *dns_ptr, NX_PACKET_POOL *packet_pool_ptr)
{

#ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(packet_pool_ptr);

    /* Client not configured for the user creating the packet pool. Return an error status. */
    return NX_NOT_ENABLED;
#else

UINT  status;


    /* Check for invalid pointer input. */
    if ((dns_ptr == NX_NULL) || (packet_pool_ptr == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    status = _nx_dns_packet_pool_set(dns_ptr, packet_pool_ptr);

    return status;
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_packet_pool_set                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the DNS Client packet pool by passing in a       */
/*    packet pool pointer to packet pool already create.  The             */
/*    NX_DNS_CLIENT_USER_CREATE_PACKET_POOL must be set. For guidelines on*/
/*    packet payload and packet pool size, see the                        */
/*    nx_packet_pool_create call in nx_dns_create.                        */ 
/*                                                                        */
/*    Note that the DNS Client nx_dns_delete service will delete this     */
/*    packet pool when the DNS Client is deleted.                         */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    packet_pool_ptr                       Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    NX_NOT_ENABLED                        Setting DNS Client packet     */ 
/*                                             pool not enabled           */ 
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_dns_packet_pool_set(NX_DNS *dns_ptr, NX_PACKET_POOL *packet_pool_ptr)
{


#ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(packet_pool_ptr);

    /* No, return the error status. */
    return NX_NOT_ENABLED;  
#else
    /* Check the payload size.  */
    if (packet_pool_ptr -> nx_packet_pool_payload_size < NX_DNS_PACKET_PAYLOAD)
        return(NX_DNS_PARAM_ERROR);

    /* Set the Client packet pool to the supplied packet pool. */
    dns_ptr -> nx_dns_packet_pool_ptr = packet_pool_ptr;

    return NX_SUCCESS;
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_delete                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS delete function call.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_delete                        Actual DNS delete function    */ 
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
UINT  _nxe_dns_delete(NX_DNS *dns_ptr)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS delete service.  */
    status =  _nx_dns_delete(dns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_delete                                      PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created DNS instance for the     */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_delete                  Delete DNS UDP socket         */ 
/*    tx_mutex_delete                       Delete DNS mutex              */ 
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
/*  02-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            randomized the source port, */
/*                                            resulting in version 6.1.4  */
/*  07-29-2022     Jidesh Veeramachaneni    Modified comment(s),          */
/*                                            removed error checking for  */
/*                                            nx_packet_pool_delete,      */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dns_delete(NX_DNS *dns_ptr)
{

UINT    status;


    /* Delete the DNS UDP socket.  */
    status =  nx_udp_socket_delete(&dns_ptr -> nx_dns_socket);

    if (status != NX_SUCCESS)
    {
        /* Return the socket delete error. */
        return status;
    }

#ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Delete the DNS packet pool.  */
    nx_packet_pool_delete(dns_ptr -> nx_dns_packet_pool_ptr);

#endif

    /* Delete the DNS mutex.  */
    status =  tx_mutex_delete(&dns_ptr -> nx_dns_mutex);

    if (status != NX_SUCCESS)
    {
        /* Return the mutex delete error. */
        return status;
    }

    /* Cleanup entries in the DNS structure.  */
    dns_ptr -> nx_dns_id =      0;
    dns_ptr -> nx_dns_ip_ptr =  NX_NULL;

    /* Return successful completion status.  */
    return(NX_SUCCESS);
}
               

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_server_add                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS add server function      */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    server_address                        DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_add                    Actual DNS add server function*/ 
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
UINT  _nxe_dns_server_add(NX_DNS *dns_ptr, ULONG server_address)
{

#ifndef NX_DISABLE_IPV4
UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID))
    {

        return(NX_PTR_ERROR);
    }

    /* Check for invalid server IP address.  */
    if (server_address == 0)
    {

        return(NX_DNS_BAD_ADDRESS_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS server add service.  */
    status =  _nx_dns_server_add(dns_ptr, server_address);

    /* Return status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_add                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the actual service to add DNS server address.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    server_address                        DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_add_internal           Actual add DNS server service */ 
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
UINT  _nx_dns_server_add(NX_DNS *dns_ptr, ULONG server_address)
{

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS dns_server_address;


    memset(&dns_server_address, 0, sizeof(NXD_ADDRESS));

    /* Construct an IP address structure, and fill in IPv4 address information. */
    dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;
    dns_server_address.nxd_ip_address.v4 = server_address;

    /* Invoke the real function.  */
    return (_nx_dns_server_add_internal(dns_ptr, &dns_server_address));
#else
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

                          
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_dns_server_add                                PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS add server function      */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    server_address                        DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_dns_server_add                   Actual DNS add server function*/ 
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
/*  07-29-2022     Jidesh Veeramachaneni    Modified comment(s) and       */
/*                                            simplified some branches,   */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nxde_dns_server_add(NX_DNS *dns_ptr, NXD_ADDRESS *server_address)
{

UINT    status;


    /* Check for invalid pointer input.  */
    if ((dns_ptr == NX_NULL)|| (server_address == NX_NULL)) 
    {

        return(NX_PTR_ERROR);
    }

    /* Check for invalid non pointer input or invalid server attributes. */
    if ((dns_ptr -> nx_dns_id != NX_DNS_ID) || (server_address -> nxd_ip_version == NX_NULL))        
    {

        return NX_DNS_PARAM_ERROR;
    }

    /* Check if the server address is unspecified (::). */
    if (server_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Check for invalid or null address input*/
#ifdef FEATURE_NX_IPV6
        if(CHECK_UNSPECIFIED_ADDRESS(&server_address -> nxd_ip_address.v6[0]))
        {

            /* Null address input. */
            return NX_DNS_BAD_ADDRESS_ERROR;
        }
#else

        /* Unsupported address type. */
        return NX_DNS_INVALID_ADDRESS_TYPE;
#endif

    }
    else if (server_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

#ifndef NX_DISABLE_IPV4
        /* Check for null address. */
        if(server_address -> nxd_ip_address.v4 == 0)
        {

            return NX_DNS_BAD_ADDRESS_ERROR;
        }
#else

        /* Unsupported address type. */
        return NX_DNS_INVALID_ADDRESS_TYPE;
#endif /* NX_DISABLE_IPV4 */
    }
    else
    {
        return NX_DNS_INVALID_ADDRESS_TYPE;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS add server service.  */
    status =  _nxd_dns_server_add(dns_ptr, server_address);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_server_add                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the specified DNS server to this DNS (client)    */
/*    instance's list of DNS server. DNS servers can be IPv4 or IPv6      */
/*    servers.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS client         */ 
/*    dns_server_address                    DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */    
/*    _nx_dns_server_add_internal           Actual add DNS server service */  
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
UINT _nxd_dns_server_add(NX_DNS *dns_ptr, NXD_ADDRESS *dns_server_address)
{
 
                     
    /* Invoke the real function.  */
    return (_nx_dns_server_add_internal(dns_ptr, dns_server_address)); 
}

         
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_add_internal                         PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the specified DNS server to this DNS (client)    */
/*    instance's list of DNS server.                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS client         */ 
/*    dns_server_address                    DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
/*  07-29-2022     Jidesh Veeramachaneni    Modified comment(s) and       */
/*                                            simplified branches,        */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_dns_server_add_internal(NX_DNS *dns_ptr, NXD_ADDRESS *server_address)
{

UINT        status;
UINT        i;    

    /* Get the protection mutex to make sure no other thread interferes.  */
    status =  tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);

    /* Determine if there was an error getting the mutex.  */
    if (status != TX_SUCCESS)
    {

        /* Return the completion status error.  */
        return(status);
    }

    /* Check for a duplicate entry. */
    i =  0;

    /* Check all entries for a either a matching entry or an empty slot is found.  */
    while (i < NX_DNS_MAX_SERVERS) 
    {

        /* The dns server IP is IPv4 address. */
        if (dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_version == NX_IP_VERSION_V4)
        {

#ifndef NX_DISABLE_IPV4
            /* Is there a match? */
            if (server_address -> nxd_ip_version == NX_IP_VERSION_V4) 
            {
                if (dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_address.v4 == server_address -> nxd_ip_address.v4)
                {

                    /* Error, release the mutex and return.  */
                    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

                    /* Yes, no need to add to the table, just return the 'error' status. */
                    return NX_DNS_DUPLICATE_ENTRY;
                }
            }
#else
            /* Error, release the mutex and return.  */
            tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

            return NX_DNS_PARAM_ERROR;
#endif /* NX_DISABLE_IPV4 */
        }

        /* The dns server IP is IPv6 address. */
        else if (dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_version == NX_IP_VERSION_V6)
        {

#ifdef FEATURE_NX_IPV6
            /* Is there a match? */
            if (server_address -> nxd_ip_version == NX_IP_VERSION_V6) 
            {
                if (CHECK_IPV6_ADDRESSES_SAME(&dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_address.v6[0], 
                                          &(server_address -> nxd_ip_address.v6[0])))
                {

                    /* Error, release the mutex and return.  */
                    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

                    /* Yes, no need to add to the table, just return the 'error' status. */
                    return NX_DNS_DUPLICATE_ENTRY;
                }
            }
#else
            /* Error, release the mutex and return.  */
            tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

            return NX_DNS_PARAM_ERROR;
#endif
        }

        else
        {
            /* Yes, find one empty slot in the DNS server array. */
            break;
        }
        
        i++;
    }

    if(i == NX_DNS_MAX_SERVERS)
    {

        /* Can not find one empty slot. Release the mutex and return.  */
        tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

        return(NX_NO_MORE_ENTRIES);
    }

    /* Add the new entry here.  */
    if (server_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

#ifdef FEATURE_NX_IPV6
        COPY_NXD_ADDRESS(server_address, &dns_ptr -> nx_dns_server_ip_array[i]);
#else
        /* Error, release the mutex and return.  */
        tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

        return NX_DNS_IPV6_NOT_SUPPORTED;
#endif
    }
    else
    {

#ifndef NX_DISABLE_IPV4
        dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_address.v4 = server_address -> nxd_ip_address.v4;
        dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_version = NX_IP_VERSION_V4;
#else
        /* Error, release the mutex and return.  */
        tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

        return NX_DNS_INVALID_ADDRESS_TYPE;
#endif /* NX_DISABLE_IPV4 */
    }

    /* Success, release the mutex and return.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}   



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_server_remove                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS remove server function   */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    server_address                        DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_remove                 Actual DNS remove server      */ 
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
UINT  _nxe_dns_server_remove(NX_DNS *dns_ptr, ULONG server_address)
{

#ifndef NX_DISABLE_IPV4
UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid server IP address.  */
    if (server_address == 0)
    {
        return(NX_DNS_BAD_ADDRESS_ERROR); 
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS remove server service.  */
    status =  _nx_dns_server_remove(dns_ptr, server_address);

    /* Return status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_remove                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls actual service to remove specified DNS Server.  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS client         */ 
/*    server_address                        DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_remove_internal        Actual DNS server remove      */ 
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
UINT  _nx_dns_server_remove(NX_DNS *dns_ptr, ULONG server_address)
{

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS dns_server_address;

    /* Construct an IP address structure, and fill in IPv4 address information. */
    dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;
    dns_server_address.nxd_ip_address.v4 = server_address;

    /* Invoke the real function.  */
    return (_nx_dns_server_remove_internal(dns_ptr, &dns_server_address));
#else
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

                                           
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_dns_server_remove                             PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS remove server function   */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS client          */ 
/*    server_address                       DNS Server NXD_ADDRESS instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                               Completion status              */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_remove                 Actual DNS remove server      */ 
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
/*  07-29-2022     Jidesh Veeramachaneni    Modified comment(s) and       */
/*                                            simplified check for        */ 
/*                                            invalid address types,      */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nxde_dns_server_remove(NX_DNS *dns_ptr, NXD_ADDRESS *server_address)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (server_address == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check if the server address is unspecified (::). */
    if (server_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Check for invalid or null address input*/
#ifdef FEATURE_NX_IPV6
        if(CHECK_UNSPECIFIED_ADDRESS(&server_address -> nxd_ip_address.v6[0]))
        {

            /* Null address input. */
            return NX_DNS_BAD_ADDRESS_ERROR;
        }
#else

        /* Unsupported address type. */
        return NX_DNS_INVALID_ADDRESS_TYPE;
#endif

    }
    else if (server_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

#ifndef NX_DISABLE_IPV4
        /* Check for null address. */
        if(server_address -> nxd_ip_address.v4 == 0)
        {

            return NX_DNS_BAD_ADDRESS_ERROR;
        }
#else

        /* Unsupported address type. */
        return NX_DNS_INVALID_ADDRESS_TYPE;
#endif /* NX_DISABLE_IPV4 */
    }
    else
    {
        return NX_DNS_INVALID_ADDRESS_TYPE;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS remove server service.  */
    status =  _nxd_dns_server_remove(dns_ptr, server_address);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_server_remove                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function removes the specified DNS server to this DNS          */ 
/*    instance. If the server is not found among the client list of DNS   */
/*    servers, the function returns an error. When a server is removed,   */
/*    the servers above the removed server's index are moved down one to  */
/*    fill the empty slot.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS client          */ 
/*    server_address                       DNS Server NXD_ADDRESS isntance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
UINT  _nxd_dns_server_remove(NX_DNS *dns_ptr, NXD_ADDRESS *server_address)
{
                     

    /* Invoke the real function.  */
    return (_nx_dns_server_remove_internal(dns_ptr, server_address));  
} 

        
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_remove_internal                      PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function removes the specified DNS server to this DNS          */ 
/*    instance. If the server is not found among the client list of DNS   */
/*    servers, the function returns an error. When a server is removed,   */
/*    the servers above the removed server's index are moved down one to  */
/*    fill the empty slot.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS client          */ 
/*    server_address                       DNS Server address             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
/*  07-29-2022     Jidesh Veeramachaneni    Modified comment(s) and       */
/*                                            removed null IP checks,     */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_server_remove_internal(NX_DNS *dns_ptr, NXD_ADDRESS *server_address)
{

NXD_ADDRESS     *DNSserver_array;
UINT            status;
UINT            i;
UINT            found_match;

    /* Initialize local variables. */
    found_match = NX_FALSE;
    i =  0;

    /* Calculate the start of the DNS server array.  */
    DNSserver_array =  dns_ptr -> nx_dns_server_ip_array;

    /* Get the protection mutex to make sure no other thread interferes.  */
    status =  tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);

    /* Determine if there was an error getting the mutex.  */
    if (status != TX_SUCCESS)
    {

        /* Return an error.  */
        return(status);
    }

    /* Search through the array to see if we can find the server address.  */
    do
    {

        /* Is there an IPv6 address in this slot? */
        if (DNSserver_array[i].nxd_ip_version == NX_IP_VERSION_V6)
#ifndef FEATURE_NX_IPV6
        {

            /* Error, release the mutex and return.  */
            tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

             return NX_DNS_IPV6_NOT_SUPPORTED;
        }
#else
        {
            /* Determine if this entry matches the specified DNS server.  */
            if (CHECK_IPV6_ADDRESSES_SAME(&DNSserver_array[i].nxd_ip_address.v6[0], &(server_address -> nxd_ip_address.v6[0])))
            {
    
                found_match = NX_TRUE;
                break;
            }
        }
#endif         
        else if (DNSserver_array[i].nxd_ip_version == NX_IP_VERSION_V4)
#ifndef NX_DISABLE_IPV4
        {
            /* Determine if this entry matches the specified DNS server.  */
            if (DNSserver_array[i].nxd_ip_address.v4 == server_address -> nxd_ip_address.v4)
            {
    
                found_match = NX_TRUE;
                break;
            }
        }
#else
        {

            /* Error, release the mutex and return.  */
            tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

             return NX_DNS_IPV6_NOT_SUPPORTED;
        }
#endif /* NX_DISABLE_IPV4 */

        /* Check the next slot.  */
        i++;
    
    } while (i < NX_DNS_MAX_SERVERS);

    if (!found_match)
    {

        /* Release the mutex and return.  */
        tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

        /* Return the error.  */
        return(NX_DNS_SERVER_NOT_FOUND);
    }

    /* We found the DNS server entry. Remove it from the table.  */            
    while (i < NX_DNS_MAX_SERVERS - 1)
    {

        /* Move them up one slot. */

#ifdef FEATURE_NX_IPV6
         /* This will handle IPv6 and IPv4 addresses if IPv6 enabled in NetX Duo. */
         COPY_NXD_ADDRESS(&DNSserver_array[i+1], &DNSserver_array[i]);
#else
         /* IPv6 is not enabled, so first verify this is not an IPv6 address. */
         if (DNSserver_array[i+1].nxd_ip_version == NX_IP_VERSION_V6)
         {

             /* Error, release the mutex and return.  */
             tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

             /* This should not happen.  return an error. */
             return NX_DNS_IPV6_NOT_SUPPORTED;
         }

         /* This is an IPv4 address. Copy to the slot above directly. */
         DNSserver_array[i].nxd_ip_version = DNSserver_array[i+1].nxd_ip_version;
         DNSserver_array[i].nxd_ip_address.v4 = DNSserver_array[i+1].nxd_ip_address.v4;
#endif

        i++;
    }

    /* Terminate the last slot. */
    memset(&dns_ptr -> nx_dns_server_ip_array[NX_DNS_MAX_SERVERS - 1], 0, sizeof(NXD_ADDRESS));

    /* Release the mutex and return.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_server_remove_all                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS remove all               */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_remove_all             Actual DNS remove all function*/ 
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
UINT  _nxe_dns_server_remove_all(NX_DNS *dns_ptr)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS remove server service.  */
    status =  _nx_dns_server_remove_all(dns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_remove_all                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function removes all DNS servers.                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
UINT  _nx_dns_server_remove_all(NX_DNS *dns_ptr)
{

UINT    status;

    /* Get the protection mutex to make sure no other thread interferes.  */
    status =  tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);

    /* Determine if there was an error getting the mutex.  */
    if (status != TX_SUCCESS)
    {
        /* Return an error.  */
        return(NX_DNS_ERROR);
    }

    /* Remove all DNS servers.  */
    memset(&dns_ptr -> nx_dns_server_ip_array[0], 0, NX_DNS_MAX_SERVERS * sizeof(NXD_ADDRESS));

    /* Release the mutex and return.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}                                                   


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_get_serverlist_size                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the get DNS server list size     */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    size                                  Pointer to server size        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_get_serverlist_size          Actual get DNS server list size*/ 
/*                                          service                       */
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
UINT  _nxe_dns_get_serverlist_size(NX_DNS *dns_ptr, UINT *size)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID) || (size == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get server size service.  */
    status =  _nx_dns_get_serverlist_size(dns_ptr, size);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_get_serverlist_size                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns the number of DNS servers in the Client list. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS client         */ 
/*    size                                  Pointer to server list size   */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_dns_get_serverlist_size(NX_DNS *dns_ptr, UINT *size)
{

    /* Initialize the list size to zero. */
    *size = 0;

    /* Loop through the list till we hit the first empty slot. */
    while (*size < NX_DNS_MAX_SERVERS)
    {

        /* Is this a valid IP address entry in this slot? */
        if (dns_ptr -> nx_dns_server_ip_array[*size].nxd_ip_version == 0)
        {

            /* This is an empty slot, so we're at the end of the list. */
            break;
        }

        (*size)++;
    }

    return NX_SUCCESS;         
}
                                                                                  

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_server_get                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the get DNS server service.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    index                                 Index into server list        */
/*    dns_server_address                    DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_get                    Actual get DNS server service */ 
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
UINT  _nxe_dns_server_get(NX_DNS *dns_ptr, UINT index, ULONG *dns_server_address)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID) || (dns_server_address == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller. */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual get DNS server service.  */
    status =  _nx_dns_server_get(dns_ptr, index, dns_server_address);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_get                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls actual service to get the DNS Server address.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS client         */ 
/*    index                                 Index into server list        */
/*    dns_server_address                    DNS Server IP address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_get                    Get DNS server service        */ 
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
UINT  _nx_dns_server_get(NX_DNS *dns_ptr, UINT index, ULONG *dns_server_address)
{

#ifndef NX_DISABLE_IPV4
UINT        status;
NXD_ADDRESS server_address;


    /* Initialize to not found. */
    *dns_server_address = 0x0;

    /* Invoke the real function. */
    status = _nx_dns_server_get_internal(dns_ptr, index, &server_address);

    if (status != NX_SUCCESS)
    {
        return status;
    }

    /* Verify this is an IPv4 address. */
    if (server_address.nxd_ip_version != NX_IP_VERSION_V4)
    {
         return NX_DNS_INVALID_ADDRESS_TYPE;
    }

    /* Return the IPv4 address information. */
    *dns_server_address = server_address.nxd_ip_address.v4;

    return NX_SUCCESS;     
#else
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(index);
    NX_PARAMETER_NOT_USED(dns_server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_dns_server_get                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the get DNS server service.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS client          */ 
/*    index                                Index into server list         */
/*    dns_server_address                   Pointer to Server address      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                               Completion status              */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_server_remove                 Actual DNS return server      */ 
/*                                            service                     */ 
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
UINT  _nxde_dns_server_get(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *dns_server_address)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_server_address == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual get DNS server service.  */
    status =  _nxd_dns_server_get(dns_ptr, index, dns_server_address);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_server_get                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retreives the DNS server at the specified index into  */
/*    the DNS server list.  If the index exceeds the size of the list or  */
/*    an empty slot is at that index, the function returns an error.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS client          */ 
/*    index                                Index into server list         */
/*    dns_server_address                   Pointer to Server IP address   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
UINT  _nxd_dns_server_get(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *dns_server_address)
{
                  
    /* Invoke the real function. */
    return(_nx_dns_server_get_internal(dns_ptr, index, dns_server_address)); 
}      

                  
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_server_get_internal                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retreives the DNS server at the specified index into  */
/*    the DNS server list.  If the index exceeds the size of the list or  */
/*    an empty slot is at that index, the function returns an error.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS client          */ 
/*    index                                Index into server list         */
/*    dns_server_address                   Pointer to Server IP address   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
static UINT  _nx_dns_server_get_internal(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *server_address)
{
           
UINT            status;


    /* Get the protection mutex to make sure no other thread interferes.  */
    status =  tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);

    /* Determine if there was an error getting the mutex.  */
    if (status != TX_SUCCESS)
    {

        /* Return an error.  */
        return(status);
    }
              
    /* Check for an invalid index. The max list size is NX_DNS_MAX_SERVERS.  */
    if (index >= NX_DNS_MAX_SERVERS)
    {
    
        return NX_DNS_PARAM_ERROR;
    }

    /* Is there a valid IP address entry in this slot? */
    if (dns_ptr -> nx_dns_server_ip_array[index].nxd_ip_version == 0)
    {

        /* No, release the mutex and return.  */
        tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

        return NX_DNS_SERVER_NOT_FOUND;
    }
#ifndef NX_DISABLE_IPV4
    /* Check for a null IPv4 address if this is an IPv4 type. */
    else if ((dns_ptr -> nx_dns_server_ip_array[index].nxd_ip_version == NX_IP_VERSION_V4) && 
             (dns_ptr -> nx_dns_server_ip_array[index].nxd_ip_address.v4 == 0))
    {

        /* Error, release the mutex and return.  */
        tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

        return NX_DNS_BAD_ADDRESS_ERROR;
    }
#endif /* NX_DISABLE_IPV4 */
    else if (dns_ptr -> nx_dns_server_ip_array[index].nxd_ip_version == NX_IP_VERSION_V6)
#ifndef FEATURE_NX_IPV6
    {
    
         /* IPv6 not enabled, return an error status. */

         /* Release the mutex and return.  */
         tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

         return NX_DNS_INVALID_ADDRESS_TYPE;
    }
#else
    {

        /* Check for unspecified (null) IPv6 address. */
         if (CHECK_UNSPECIFIED_ADDRESS(&(dns_ptr -> nx_dns_server_ip_array[index].nxd_ip_address.v6[0])))
         {

             /* Error, release the mutex and return.  */
             tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

             return NX_DNS_BAD_ADDRESS_ERROR;
         }
     }
#endif

    /* Set a pointer to the DNS server in the list. */
    *server_address = (dns_ptr -> nx_dns_server_ip_array[index]);

    /* Release the mutex.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_host_by_name_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up host IP address  */ 
/*    by name service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    host_address_ptr                      Pointer to DNS host IP address*/ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_by_name_get              Actual DNS host IP address    */
/*                                            lookup by host name service */ 
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
UINT  _nxe_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (host_name == NX_NULL) || (host_address_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host by name service.  */
    status =  _nx_dns_host_by_name_get(dns_ptr, host_name, host_address_ptr, wait_option);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_host_by_name_get                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls service to get the host address by name service */
/*    as an A record (IPv4) lookup query.                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    host_address_ptr                      Pointer to DNS host IP address*/ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Actual DNS get host address   */ 
/*                                            by name service             */ 
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
UINT  _nx_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, ULONG wait_option)
{

UINT        status;
UINT        record_count = 0;

    /* Keep the API consistency. Invoke the real DNS query call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, (VOID*)host_address_ptr, sizeof(ULONG), &record_count, NX_DNS_RR_TYPE_A, wait_option);
 
    /* Record_count set to 1 indicates the call is successful. */
    if(record_count == 1)
    {
        /* Set the return value. */
        status = NX_SUCCESS;
    }

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_dns_host_by_name_get                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS get host by name         */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    host_address_ptr                      Pointer to destination of     */ 
/*                                            host IP address             */ 
/*    wait_option                           Timeout value                 */ 
/*    lookup_type                           Lookup for which IP version   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_dns_host_by_name_get             Actual DNS get host by name   */ 
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
UINT  _nxde_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, NXD_ADDRESS *host_address_ptr, 
                                 ULONG wait_option, UINT lookup_type)
{

UINT    status;

    /* Check for invalid pointer input.  */
    if ((dns_ptr == NX_NULL) || (host_name == NX_NULL) || (host_address_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if ((dns_ptr -> nx_dns_id != NX_DNS_ID) || (lookup_type == 0))
    {
        return NX_DNS_PARAM_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host by name service.  */
    status =  _nxd_dns_host_by_name_get(dns_ptr, host_name, host_address_ptr, wait_option, lookup_type);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_host_by_name_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS get host by name         */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    host_address_ptr                      Pointer to host IP address    */ 
/*    wait_option                           Timeout value                 */ 
/*    lookup_type                           Lookup for which IP version   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Actual DNS get host by name   */ 
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
UINT  _nxd_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, NXD_ADDRESS *host_address_ptr, 
                                ULONG wait_option, UINT lookup_type)
{

UINT        status;
UINT        record_count = 0;
        
                                 
    /* Is this a AAAA record lookup (e.g. expect an IPv6 address)? If no type is specified default to AAAA type. */
    if((lookup_type == NX_IP_VERSION_V6) || (lookup_type == 0))
    {

#ifdef FEATURE_NX_IPV6
        /* Send AAAA type message, and keep the API consistency. Invoke the real connection call. */
        status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, (UCHAR *)&host_address_ptr -> nxd_ip_address.v6[0], 16, &record_count, NX_DNS_RR_TYPE_AAAA, wait_option);

        /* Record_count being set indicates the query gets a valid answer. */
        if (record_count)
        {

            /* Have recorded the IPv6 address, set the IP version.  */     
            host_address_ptr -> nxd_ip_version = NX_IP_VERSION_V6;

            /* Modify the return status.*/
            status = NX_SUCCESS;
        }
#else
        return NX_DNS_IPV6_NOT_SUPPORTED;
#endif
    }
    else if(lookup_type == NX_IP_VERSION_V4)
    {

#ifndef NX_DISABLE_IPV4
        /* Keep the API consistency. Invoke the real connection call. */
        status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, (UCHAR *)&host_address_ptr -> nxd_ip_address.v4, sizeof(ULONG), &record_count, NX_DNS_RR_TYPE_A, wait_option);
        
        /* Record_count being set indicates the query gets a valid answer. */
        if (record_count)
        {

            /* Have recorded the IPv4 address, set the IP version.  */     
            host_address_ptr -> nxd_ip_version = NX_IP_VERSION_V4;

            /* Modify the return status.*/
            status = NX_SUCCESS;
        }
#else
        return NX_DNS_BAD_ADDRESS_ERROR;
#endif
    }

    else
    {
        return NX_DNS_BAD_ADDRESS_ERROR;
    }

    /* Return status.  */
    return(status);
}     
                                            
                     
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_info_by_name_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS get info by name         */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    host_address_ptr                      Pointer to destination of     */ 
/*                                            host IP address             */ 
/*    host_port_ptr                         Pointer to destination of host*/
/*                                            source port                 */
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_info_by_name_get              Actual DNS get info by name   */ 
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
UINT  _nxe_dns_info_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, USHORT *host_port_ptr, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (dns_ptr -> nx_dns_id != NX_DNS_ID) || 
        (host_name == NX_NULL) || (host_address_ptr == NX_NULL) || (host_port_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host by name service.  */
    status =  _nx_dns_info_by_name_get(dns_ptr, host_name, host_address_ptr, host_port_ptr, wait_option);

    /* Return status.  */
    return(status);
}
#endif

                         
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_info_by_name_get                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the Service record associated with the*/ 
/*    specified host name.  If a service rec cannot be found, this        */ 
/*    routine returns a zero IP address in the input address pointer and a*/
/*    non zero error status return to signal an error.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    host_address_ptr                      Pointer to destination of     */ 
/*                                            host IP address             */ 
/*    host_port_ptr                         Pointer to destination of     */ 
/*                                            host port                   */ 
/*    wait_option                           Timeout value                 */ 
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
UINT  _nx_dns_info_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, 
                                       USHORT *host_port_ptr, ULONG wait_option)
{

NX_DNS_SRV_ENTRY    *dns_srv_entry;
UINT                record_count;
UINT                status;
UCHAR               temp_buffer[TEMP_SRV_BUFFER_SIZE];

    /* Call the service get function to get information. */
    status =  _nx_dns_domain_service_get(dns_ptr, host_name, (VOID *)temp_buffer, TEMP_SRV_BUFFER_SIZE, &record_count, wait_option);

    /* Check erros. */
    if(status)
    {
        return status;
    }

    /* Set the srv pointer.  */
    dns_srv_entry = (NX_DNS_SRV_ENTRY *)temp_buffer;
    
    /* record the info returned to user. */ 
    *host_address_ptr = dns_srv_entry -> nx_dns_srv_ipv4_address;
    *host_port_ptr = dns_srv_entry -> nx_dns_srv_port_number;

    return NX_SUCCESS;
}
#endif                             


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_ipv4_address_by_name_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up host IP address  */ 
/*    by name service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name_ptr                        Name of host to search on      */
/*    record_buffer                        Buffer space for storing       */ 
/*                                           IPv4 addresses               */ 
/*    buffer_size                          Size of the record_buffer      */
/*    record_count                         The count of IPv4 addresses    */
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_ipv4_address_by_name_get      Actual DNS host IP address    */
/*                                            lookup by host name service */ 
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
UINT  _nxe_dns_ipv4_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name_ptr, VOID *record_buffer, 
                                        UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((!dns_ptr) || (!host_name_ptr) || (!record_buffer) || (!record_count))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Make sure record_buffer is 4-byte aligned. */
    if(((ALIGN_TYPE)record_buffer & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host ipv4 address by name service.  */
    status =  _nx_dns_ipv4_address_by_name_get(dns_ptr, host_name_ptr, record_buffer, buffer_size, record_count, wait_option);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_ipv4_address_by_name_get                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */    
/*    This function calls service to get the host address by name service */
/*    as an A record (IPv4) lookup query.                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */
/*    record_buffer                         Buffer space for storing      */ 
/*                                            IPv4 addresses              */ 
/*    buffer_size                           Size of the record_buffer     */
/*    record_count                          The count of IPv4 addresses   */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Actual DNS get host rdata     */ 
/*                                               by name service          */ 
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
UINT  _nx_dns_ipv4_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name_ptr, VOID *buffer, 
                                       UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT        status;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name_ptr, buffer, buffer_size, record_count, NX_DNS_RR_TYPE_A, wait_option);

    /* Return completion status. */
    return status;
}

                                
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_dns_ipv6_address_by_name_get                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up host IPv6 address*/ 
/*    by name service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name_ptr                        Name of host to search on      */
/*    record_buffer                        Buffer space for storing IPv6  */
/*                                           addresses.                   */ 
/*    buffer_size                          Size of record_buffer          */
/*    record_count                         The count of IPv6 addresses    */ 
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_ipv6_address_by_name_get      Actual DNS host IPv6 address  */
/*                                            lookup by host name service */ 
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
UINT  _nxde_dns_ipv6_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name_ptr, VOID *record_buffer, 
                                         UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((!dns_ptr) || (!host_name_ptr) || (!record_buffer) || (!record_count))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Make sure record_buffer is 4-byte aligned. */
    if(((ALIGN_TYPE)record_buffer & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host ipv6 address by name service.  */
    status =  _nxd_dns_ipv6_address_by_name_get(dns_ptr, host_name_ptr, record_buffer, buffer_size, record_count, wait_option);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_ipv6_address_by_name_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates an NXD_ADDRESS instance from the specified    */
/*    IPv6 address and submits it to the actual get host by name service  */
/*    as an AAAA record (IPv6) lookup query.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to resolve        */ 
/*    record_buffer                        Buffer space for storing IPv6  */
/*                                           addresses.                   */ 
/*    buffer_size                          Size of record_buffer          */
/*    record_count                         The count of IPv6 addresses    */ 
/*    wait_option                          Timeout value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Actual NetX Duo DNS get       */ 
/*                                          host rdata by name service    */ 
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
UINT  _nxd_dns_ipv6_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name_ptr, VOID *buffer, 
                                        UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT        status;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name_ptr, buffer, buffer_size, record_count, NX_DNS_RR_TYPE_AAAA, wait_option);

    /* Return completion status. */
    return status;
}

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_cname_get                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up host cname       */ 
/*    by name service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for storing       */ 
/*                                           cname response.              */
/*    buffer_size                          Size of record_buffer          */ 
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_cname_get                Actual DNS host cname         */
/*                                          lookup by host name service   */ 
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
UINT  _nxe_dns_cname_get(NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer,
                         UINT buffer_size, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if (!dns_ptr || !host_name || !record_buffer)
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host by name service.  */
    status =  _nx_dns_cname_get(dns_ptr, host_name, record_buffer, buffer_size, wait_option);

    /* Return status.  */
    return(status);
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cname_get                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the host cname associated with        */
/*    the specified host name. If host cname cannot be found, this        */ 
/*    routine returns zero for the string size to signal an error.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to resolve        */ 
/*    record_buffer                        Buffer space for storing       */ 
/*                                           cname response.              */
/*    buffer_size                          Size of record_buffer          */ 
/*    wait_option                          Timeout value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                        Actual DNS get host cname       */ 
/*                                             by name service            */ 
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
UINT  _nx_dns_cname_get(NX_DNS *dns_ptr, UCHAR *host_name, UCHAR *record_buffer, 
                        UINT buffer_size, ULONG wait_option)
{

UINT        status;
UINT        record_count = 0;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, record_buffer, buffer_size, 
                                                    &record_count, NX_DNS_RR_TYPE_CNAME, wait_option);

    /* Return completion status. */
    return status;
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_domain_name_server_get                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up the authoritative*/ 
/*    name servers in a given domain.                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for storing       */ 
/*                                           the data structure that      */
/*                                           holds name server information*/
/*    buffer_size                          Size of the record_buffer      */
/*    record_count                         The number of records stored   */
/*                                           in the record_buffer         */
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                               Completion status              */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_domain_name_server_get       Actual DNS host NS             */
/*                                           lookup by host name service  */ 
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
UINT  _nxe_dns_domain_name_server_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, 
                                      UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((!dns_ptr) || (!host_name) || (!record_buffer) || (!record_count))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Make sure record_buffer is 4-byte aligned. */
    if(((ALIGN_TYPE)record_buffer & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get the authoritative name server by name service.  */
    status =  _nx_dns_domain_name_server_get(dns_ptr, host_name, record_buffer, buffer_size, record_count, wait_option);

    /* Return status.  */
    return(status);
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_domain_name_server_get                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the authoritative name server         */
/*    associated with the specified host name. If host cname cannot       */ 
/*    be found, this routine returns zero for the string size to          */ 
/*    signal an error.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for storing       */ 
/*                                           the data structure that      */
/*                                           holds name server information*/
/*    buffer_size                          Size of the record_buffer      */
/*    record_count                         The number of records stored   */
/*                                           in the record_buffer         */
/*    wait_option                          Timeout value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Actual DNS get the            */ 
/*                                          authoritaive name server      */ 
/*                                            by name service             */ 
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
UINT  _nx_dns_domain_name_server_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, 
                                     UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT        status;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, record_buffer, buffer_size, record_count, NX_DNS_RR_TYPE_NS, wait_option);

    /* Return completion status. */
    return status;
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_host_text_get                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up the text strings */ 
/*    by name service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for storing       */ 
/*                                           the host text string         */
/*    buffer_size                          Size of record_buffer.         */
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    _nx_dns_host_text_get                  Actual DNS host text strings */
/*                                          lookup by host name service   */ 
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
UINT  _nxe_dns_host_text_get(NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer,
                             UINT buffer_size, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if (!dns_ptr || !host_name || !record_buffer)
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get the text strings by name service.  */
    status =  _nx_dns_host_text_get(dns_ptr, host_name, record_buffer, buffer_size, wait_option);

    /* Return status.  */
    return(status);
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_host_text_get                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the text strings associated with      */
/*    the specified host name. If host cname cannot be found,             */ 
/*    this routine returns zero for the string size to  signal an error.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to resolve        */ 
/*    record_buffer                        Buffer space for storing       */ 
/*                                           the host text string         */
/*    buffer_size                          Size of record_buffer.         */ 
/*    wait_option                          Timeout value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                         Actual DNS get the text        */ 
/*                                         strings by name service        */ 
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
UINT  _nx_dns_host_text_get(NX_DNS *dns_ptr, UCHAR *host_name, UCHAR *record_buffer, 
                            UINT buffer_size, ULONG wait_option)
{

UINT        status;
UINT        record_count = 0;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, record_buffer, buffer_size, &record_count, NX_DNS_RR_TYPE_TXT, wait_option);

    /* Return completion status. */
    return status;
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_domain_mail_exchange_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up the mail         */ 
/*    exchange by name service.                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for recording     */ 
/*                                           data structure that holds    */
/*                                           the mail exchange information*/
/*    buffer_size                          Size of record_buffer.         */
/*    record_count                         The count of mail exchange     */
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_domain_mail_exchange_get                                    */
/*                                          Actual DNS host MX lookup     */ 
/*                                           by host name service   */ 
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
UINT  _nxe_dns_domain_mail_exchange_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, 
                                        UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((!dns_ptr) || (!host_name) || (!record_buffer) || (!record_count))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Make sure record_buffer is 4-byte aligned. */
    if(((ALIGN_TYPE)record_buffer & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get the mail exchange by name service.  */
    status =  _nx_dns_domain_mail_exchange_get(dns_ptr, host_name, record_buffer, buffer_size, record_count, wait_option);

    /* Return status.  */
    return(status);
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_domain_mail_exchange_get                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the mail exchange associated with     */
/*    the specified host name. If host cname cannot be found,             */ 
/*    this routine returns zero for the string size to signal an error.   */  
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to resolve        */
/*    record_buffer                        Buffer space for recording     */ 
/*                                           data structure that holds    */
/*                                           the mail exchange information*/
/*    buffer_size                          Size of record_buffer.         */
/*    record_count                         The count of mail exchange     */ 
/*    wait_option                          Timeout value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_get                                      */ 
/*                                          Actual DNS get the mail       */ 
/*                                           exchange by name service     */ 
/*                                                                        */ 
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
UINT  _nx_dns_domain_mail_exchange_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, 
                                       UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT        status;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, record_buffer, buffer_size, 
                                                    record_count, NX_DNS_RR_TYPE_MX, wait_option);

    /* Return completion status. */
    return status;
}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_domain_service_get                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up the service      */ 
/*    by name service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for storing the   */ 
/*                                           the data structures that     */
/*                                           hold the service information */
/*    buffer_size                          size of record_buffer          */
/*    record_count                         The count of services stored   */
/*                                           in record_buffer             */
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_domain_service_get                                          */
/*                                          Actual DNS host SRV           */ 
/*                                          lookup by host name service   */ 
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
UINT  _nxe_dns_domain_service_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, 
                                  UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((!dns_ptr) || (!host_name) || (!record_buffer) || (!record_count))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Make sure record_buffer is 4-byte aligned. */
    if(((ALIGN_TYPE)record_buffer & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get the service by name service.  */
    status =  _nx_dns_domain_service_get(dns_ptr, host_name, record_buffer, buffer_size, record_count, wait_option);

    /* Return status.  */
    return(status);
}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_domain_service_get                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the service associated with           */
/*    the specified host name. If host cname cannot be found,             */ 
/*    this routine returns zero for the string size to signal an error.   */  
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */
/*    record_buffer                        Buffer space for storing the   */ 
/*                                           the data structures that     */
/*                                           hold the service information */
/*    buffer_size                          size of record_buffer          */
/*    record_count                         The count of services stored   */
/*                                           in record_buffer             */
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Route that sends out the      */ 
/*                                          query and process response.   */ 
/*                                                                        */ 
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
UINT  _nx_dns_domain_service_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, 
                                 UINT buffer_size, UINT *record_count, ULONG wait_option)
{

UINT        status;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, record_buffer, buffer_size, 
                                                    record_count, NX_DNS_RR_TYPE_SRV, wait_option);

    /* Return completion status. */
    return status;
}
#endif
   

#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_authority_zone_start_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS look up the start of     */ 
/*    a zone of authority by name service.                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to search on      */
/*    record_buffer                        Buffer space for storing       */ 
/*                                           the data structures that     */
/*                                           hold the SOA information     */
/*    buffer_size                          Size of record_buffer.         */
/*    wait_option                          Time to wait on server response*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    _nx_dns_authority_zone_start_get       Actual DNS host text strings */
/*                                          lookup by host name service   */ 
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
UINT  _nxe_dns_authority_zone_start_get(NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer,
                                        UINT buffer_size, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if (!dns_ptr || !host_name || !record_buffer)
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }
    
    /* Make sure record_buffer is 4-byte aligned. */
    if(((ALIGN_TYPE)record_buffer & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get the start of zone authority by name service.  */
    status =  _nx_dns_authority_zone_start_get(dns_ptr, host_name, record_buffer, buffer_size, wait_option);

    /* Return status.  */
    return(status);
}
#endif

   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_authority_zone_start_get                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the start of a zone of authority      */
/*    associated with the specified host name. If it cannot be found,     */ 
/*    this routine returns zero for the string size to signal an error.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                              Pointer to DNS instance        */ 
/*    host_name                            Name of host to resolve        */ 
/*    record_buffer                        Buffer space for storing       */ 
/*                                           the data structures that     */
/*                                           hold the SOA information     */
/*    buffer_size                          Size of record_buffer.         */ 
/*    wait_option                          Timeout value                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                         Actual DNS get the authority   */ 
/*                                         zone by name service           */ 
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
UINT  _nx_dns_authority_zone_start_get(NX_DNS *dns_ptr, UCHAR *host_name, UCHAR *record_buffer, 
                                       UINT buffer_size, ULONG wait_option)
{

UINT        status;
UINT        record_count = 0;

    /* Invoke the real connection call. */
    status = _nx_dns_host_resource_data_by_name_get(dns_ptr, host_name, record_buffer, buffer_size, &record_count, NX_DNS_RR_TYPE_SOA, wait_option);

    /* Return completion status. */
    return status;
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get               PORTABLE C     */ 
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to find the IP address associated with the   */ 
/*    specified host name.  If a DNS server responds but does not have    */
/*    the IP address, this function skips to the next server in the       */
/*    DNS client list.  Otherwise it will resend the same query up to the */
/*    DNS client's max retry times before skipping to the next server.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_name                             Name of host to resolve       */ 
/*    record_buffer                         Buffer for record ipv4 address*/ 
/*    buffer_size                           Buffer size for ipv4 adress   */
/*    record_count                          The count of ipv4 address     */ 
/*    wait_option                           Timeout value                 */ 
/*    lookup_type                           Lookup for which IP version   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_send_query_get_rdata_by_name                                */
/*                                          Creates and transmits a DNS   */ 
/*                                            query on supplied host name */
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
/*    nx_udp_socket_bind                    Bind DNS UDP socket to port   */ 
/*    nx_udp_socket_unbind                  Unbind DNS UDP socket         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the timeout of first query, */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            randomized the source port, */
/*                                            resulting in version 6.1.4  */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            receiving dns response,     */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_host_resource_data_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, 
                                                    UCHAR *buffer, UINT buffer_size, 
                                                    UINT *record_count, UINT lookup_type, ULONG wait_option)
{

UINT        status;
UINT        retries;
UINT        i;


    /* Get the protection mutex to make sure no other thread interferes.  */
    status =  tx_mutex_get(&(dns_ptr -> nx_dns_mutex), wait_option);

    /* Check status.  */
    if (status != TX_SUCCESS)
    {

        /* The mutex was not granted in the time specified.  Return the threadx error.  */
        return(status);
    }

    /* Clear the record name buffer and record count.  This way
      if the query fails, the API returns zero record. */
    memset(buffer, 0, buffer_size);

    /* Initialize the record count.  */
    *record_count = 0;

#ifdef NX_DNS_CACHE_ENABLE

    /* Find the answer in local cache.  */
    if(_nx_dns_cache_find_answer(dns_ptr, dns_ptr -> nx_dns_cache, host_name, (USHORT)lookup_type, buffer, buffer_size, record_count) == NX_DNS_SUCCESS)
    {           

        /* Put the DNS mutex.  */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

        return (NX_DNS_SUCCESS);
    }
#endif /*NX_DNS_CACHE_ENABLE.  */

    /* Determine if there is at least one DNS server. Is there anything in the first slot? */
    if (dns_ptr -> nx_dns_server_ip_array[0].nxd_ip_version == 0)
    {

        /* No, this means the list is empty. Release the DNS Client lock. */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

        /* At least one DNS server is required - return an error.  */
        return(NX_DNS_NO_SERVER);
    }

    /* Bind the UDP socket to random port for each query.  */
    status =  nx_udp_socket_bind(&(dns_ptr -> nx_dns_socket), NX_ANY_PORT, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status != TX_SUCCESS)
    {

        /* Release the DNS Client lock.  */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);
        return(status);
    }

    /* Limit the timeout to NX_DNS_MAX_RETRANS_TIMEOUT.  */
    if (wait_option > NX_DNS_MAX_RETRANS_TIMEOUT)
    {
        wait_option = NX_DNS_MAX_RETRANS_TIMEOUT;
    }

    /* Keep sending queries to all DNS Servers till the retry count expires.  */
    for (retries = 0; retries < dns_ptr -> nx_dns_retries; retries++)
    {

        /* The client should try other servers and server addresses before repeating a query to a specific address of a server.  
           RFC1035, Section4.2.1 UDP usage, Page32.  */
        /*  Attempt host name resolution from each DNS server till one if found. */        
        for (i = 0; (i < NX_DNS_MAX_SERVERS) && (dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_version != 0); i ++)
        {

            /* Send the DNS query. */
            status = _nx_dns_send_query_get_rdata_by_name(dns_ptr, &dns_ptr -> nx_dns_server_ip_array[i], host_name, 
                                                          buffer, buffer_size, record_count, lookup_type, wait_option);

            /* Check the status.  */
            if (status == NX_SUCCESS)
            {

                /* Unbind the socket.  */
                nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));

                /* Release the mutex */
                tx_mutex_put(&dns_ptr -> nx_dns_mutex);

                /* Yes, have done, just return success.  */
                return NX_SUCCESS;
            }
            else
            {

                /* Let application controls query retransmission for non-blocking.  */
                if (wait_option == NX_NO_WAIT)
                {

                    /* Check if the query is sent out.  */
                    if (status == NX_IN_PROGRESS)
                    {

                        /* No need to release mutex and unbind the socket for non-blocking since
                           _nx_dns_response_get will receive the response and release the resource.  */
                        return(status);
                    }
                    else
                    {

                        /* Unbind the socket.  */
                        nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));

                        /* Release the mutex */
                        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

                        return(status);
                    }
                }
            }
        }

        /* Timed out for querying all DNS servers in this cycle, double the timeout, limited to NX_DNS_MAX_RETRANS_TIMEOUT.  */
        if (wait_option <= (NX_DNS_MAX_RETRANS_TIMEOUT >> 1))
            wait_option =  (wait_option << 1);
        else
            wait_option =  NX_DNS_MAX_RETRANS_TIMEOUT;
    }

    /* Unbind the socket.  */
    nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));

    /* Release protection.  */
    tx_mutex_put(&dns_ptr -> nx_dns_mutex);

    /* Failed on all servers, return DNS lookup failed status.  */
    return(NX_DNS_QUERY_FAILED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_send_query_by_address                       PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the name of a host from the specified */
/*    IP address associated.  If the IP address cannot be found, this     */ 
/*    routine returns a NULL host name                    .               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    dns_server                            Pointer to DNS server to use  */ 
/*    ip_question                           Buffer pointer to IP address  */
/*                                               in ascii to lookup       */ 
/*    host_name_ptr                         Buffer pointer to host name   */ 
/*    host_name_buffer_size                 Buffer size for host name     */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*    _nx_dns_network_to_long_convert       Convert to unsigned long      */ 
/*    _nx_dns_network_to_short_convert      Convert to unsigned short     */ 
/*    _nx_dns_new_packet_create             Create new DNS packet         */ 
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_resource_data_length_get      Get length of resource        */ 
/*    _nx_dns_resource_type_get             Get resource type             */ 
/*    nx_packet_copy                        Copy packet                   */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_receive                 Receive DNS UDP packet        */  
/*    nxd_udp_socket_send                   Send DNS UDP packet           */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), updated  */
/*                                            resource get function and   */
/*                                            status check to improve     */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            improved id generation,     */
/*                                            improved the logic of       */
/*                                            receiving dns response,     */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_send_query_by_address(NX_DNS *dns_ptr, NXD_ADDRESS *dns_server, UCHAR *ip_question, UCHAR *host_name_ptr, 
                                          UINT host_name_buffer_size, ULONG wait_option)
{

UINT        status;
USHORT      answerCount;
UCHAR       *data_ptr;
NX_PACKET   *packet_ptr;
NX_PACKET   *receive_packet_ptr;
UINT        ip_question_size;
UINT        name_size;
UINT        resource_type;
UINT        resource_size;
#ifdef NX_DNS_CACHE_ENABLE 
ULONG       rr_ttl;
#endif /* NX_DNS_CACHE_ENABLE  */



    /* Check for IP question.  */
    if (_nx_utility_string_length_check((CHAR *)ip_question, &ip_question_size, NX_DNS_IP_LOOKUP_SIZE))
    {
        return(NX_DNS_SIZE_ERROR);
    }

    /* Allocate a packet.  */
    status = nx_packet_allocate(dns_ptr -> nx_dns_packet_pool_ptr, &packet_ptr, NX_UDP_PACKET, NX_DNS_PACKET_ALLOCATE_TIMEOUT);

    /* Check the allocate status.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Create a request */
    status  =  _nx_dns_new_packet_create(dns_ptr, packet_ptr, ip_question, NX_DNS_RR_TYPE_PTR);

    /* Check the DNS packet create status.  */
    if (status != NX_SUCCESS)
    {

        nx_packet_release(packet_ptr);

        /* Return error status.  */
        return(status);
    }

    /* We will use the time spent sleeping to clear broadcast DNS packets from a previous query
       from the DNS receive queue. This will prevent ensure the most recent DNS response is
       processed and avoid the situation of valid DNS response packets overflowing the DNS socket
       queue. */
#ifdef NX_DNS_CLIENT_CLEAR_QUEUE
    do
    {

        /* Is there any packets on the queue?  */
        status = nx_udp_socket_receive(&(dns_ptr -> nx_dns_socket), &receive_packet_ptr, NX_NO_WAIT); 

        /* Yes, we received a packet on the DNS port! */
        if (status == NX_SUCCESS)
        {

            /* But we don't want it. Release it! */
            nx_packet_release(receive_packet_ptr);
        }

        /* Keep checking till the queue becomes empty. */
    } while(status == NX_SUCCESS);
#endif /* NX_DNS_CLIENT_CLEAR_QUEUE */

    /* Send the DNS packet out.  */
    status =  nxd_udp_socket_send(&dns_ptr -> nx_dns_socket, packet_ptr, dns_server, NX_DNS_PORT);

    /* Check the completion of the send.  */
    if (status != NX_SUCCESS)
    {

        /* Unsuccessful, release the packet.  */
        nx_packet_release(packet_ptr);

        return status;
    }

    /* Wait for a DNS response.  */
    status = _nx_dns_response_receive(dns_ptr, &receive_packet_ptr, wait_option);

    /* Check status.  */
    if (status == NX_SUCCESS)
    {

#ifndef NX_DISABLE_PACKET_CHAIN
        if (receive_packet_ptr -> nx_packet_next)
        {
            
            /* Chained packet is not supported. */
            nx_packet_release(receive_packet_ptr);
            return(NX_INVALID_PACKET);
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* We received a response. First, check that there is a valid header.  */
        if (receive_packet_ptr -> nx_packet_length <= NX_DNS_QDSECT_OFFSET)
        {
            /* No; Release the new packet.  */ 
            nx_packet_release(receive_packet_ptr);

            /* Return error status. */
            return NX_DNS_MALFORMED_PACKET;
        }
        
        /* Packet is long enough. Check the IDs in the DNS header match.  */
        if (_nx_dns_network_to_short_convert(receive_packet_ptr -> nx_packet_prepend_ptr + NX_DNS_ID_OFFSET) != dns_ptr -> nx_dns_transmit_id)
        {

            /* No; Release the packet.  */ 
            nx_packet_release(receive_packet_ptr);

            /* Return error status. */
            return NX_DNS_BAD_ID_ERROR;
        }

        /* We have a response with matching ID, check that it is a response
           and is successful and has a response record.  */
        status =  _nx_dns_network_to_short_convert(receive_packet_ptr -> nx_packet_prepend_ptr + NX_DNS_FLAGS_OFFSET);

        /* Check for indication of DNS server error (cannot authenticate answer or authority portion 
           of the DNS data. */
        if ((status & NX_DNS_ERROR_MASK) == NX_DNS_ERROR_MASK)
        {

            /* No; Release the packet.  */ 
            nx_packet_release(receive_packet_ptr);

            return NX_DNS_SERVER_AUTH_ERROR;
        }

        answerCount = _nx_dns_network_to_short_convert(receive_packet_ptr -> nx_packet_prepend_ptr + NX_DNS_ANCOUNT_OFFSET);

        /* Is it for our question?  */
        if (((status & NX_DNS_QUERY_MASK) == NX_DNS_RESPONSE_FLAG)
            && ((status & NX_DNS_RCODE_MASK) == NX_DNS_RCODE_SUCCESS)
            && (answerCount >= 1))
        {

            /* Yes, set a point to the start of the question to find the response record.  */
            data_ptr =  receive_packet_ptr -> nx_packet_prepend_ptr + NX_DNS_QDSECT_OFFSET;

            /* Determine if there is a question still in the server's response.  */
            if (_nx_dns_network_to_short_convert(receive_packet_ptr -> nx_packet_prepend_ptr + NX_DNS_QDCOUNT_OFFSET) == 1)
            {

                /* Get name size */
                name_size = _nx_dns_name_size_calculate(data_ptr, receive_packet_ptr);

                if (!name_size)
                {

                    /* Release the packet. */
                    nx_packet_release(receive_packet_ptr);

                    /* NULL-terminate the host name string.  */
                    *host_name_ptr =  NX_NULL;

                    /* Return an error!  */
                    return(NX_DNS_MALFORMED_PACKET);
                }

                /* Yes, the question is present in the response, skip it!  */
                data_ptr +=  name_size + 4;
            }

            /* Check all the response records */
            while (answerCount-- > 0)
            {

                /* Check for valid data_ptr.  */
                if (data_ptr >= receive_packet_ptr -> nx_packet_append_ptr)
                {

                    /* Release the packet. */
                    nx_packet_release(receive_packet_ptr);

                    /* NULL-terminate the host name string.  */
                    *host_name_ptr =  NX_NULL;

                    /* Return an error!  */
                    return(NX_DNS_SIZE_ERROR);
                }

                /* Get resource type. */
                status = _nx_dns_resource_type_get(data_ptr, receive_packet_ptr, &resource_type);
                if (status)
                {
                    /* Release the packet. */
                    nx_packet_release(receive_packet_ptr);

                    /* NULL-terminate the host name string.  */
                    *host_name_ptr =  NX_NULL;

                    /* Return an error!  */
                    return(NX_DNS_MALFORMED_PACKET);
                }

                /* Check that the answer has a name and there is space for it.  */
                if (resource_type == NX_DNS_RR_TYPE_PTR)
                {

#ifdef NX_DNS_CACHE_ENABLE    
                    /* Get the resource record ttl.  */
                    status = _nx_dns_resource_time_to_live_get(data_ptr, receive_packet_ptr, &rr_ttl);
                    if (status)
                    {

                        /* Release the packet. */
                        nx_packet_release(receive_packet_ptr);

                        /* NULL-terminate the host name string.  */
                        *host_name_ptr =  NX_NULL;

                        /* Return an error!  */
                        return(NX_DNS_MALFORMED_PACKET);
                    }
#endif /* NX_DNS_CACHE_ENABLE  */

                    /* Update the pointer to point at the response data and get the name.  */
                    data_ptr = _nx_dns_resource_data_address_get(data_ptr, receive_packet_ptr);
                    if (!data_ptr)
                    {

                        /* Release the packet. */
                        nx_packet_release(receive_packet_ptr);

                        /* NULL-terminate the host name string.  */
                        *host_name_ptr =  NX_NULL;

                        /* Return an error!  */
                        return(NX_DNS_MALFORMED_PACKET);
                    }

                    /* Determine if there is room for the name - one less for NULL termination.  */
                    name_size = _nx_dns_name_string_unencode(receive_packet_ptr, data_ptr, host_name_ptr, host_name_buffer_size - 1);
                    if (name_size)
                    {

                        /* Yes; We're done! */

                        /* Need to release the packet. */
                        nx_packet_release(receive_packet_ptr);

#ifdef NX_DNS_CACHE_ENABLE 
                        /* Set the resource record type.  */
                        temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_PTR;     

                        /* Set the resource record ttl.  */
                        temp_rr.nx_dns_rr_ttl = rr_ttl;

                        /* Add the name string.  */
                        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, ip_question, ip_question_size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

                        /* Check the status.  */
                        if(status)
                            return (NX_SUCCESS);

                        /* Add the PTR string.  */
                        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, host_name_ptr, name_size, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_ptr.nx_dns_rr_ptr_name)));

                        /* Check the status.  */
                        if(status)
                        {
                            _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
                            return (NX_SUCCESS);
                        }

                        /* Add the resource record.  */
                        status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);

                        /* Check the status.  */
                        if(status)
                        {

                            /* Delete the resource record.  */
                            _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
                        } 
#endif /* NX_DNS_CACHE_ENABLE  */

                        /* Return success!  */
                        return(NX_SUCCESS);
                    }
                    else
                    {

                        /* Nope, Our destination string is too small.  Release the packet. */
                        nx_packet_release(receive_packet_ptr);

                        /* NULL-terminate the host name string.  */
                        *host_name_ptr =  NX_NULL;

                        /* Return an error!  */
                        return(NX_DNS_SIZE_ERROR);
                    }
                }
                else
                {
                    /* This response isn't a name, just skip it. */
                    status = _nx_dns_resource_size_get(data_ptr, receive_packet_ptr, &resource_size);
                    if (status)
                    {
                        /* Nope, Our destination string is too small.  Release the packet. */
                        nx_packet_release(receive_packet_ptr);

                        /* NULL-terminate the host name string.  */
                        *host_name_ptr =  NX_NULL;

                        /* Return an error!  */
                        return(NX_DNS_SIZE_ERROR);
                    }
                    data_ptr += resource_size;
                }

            } /* and check the next answer record */
        }

        /* We got a packet, but did it supply name resolution? */
        if (answerCount == 0)
        {

            /* No, set the failed query status. */
            status = NX_DNS_QUERY_FAILED;
        }

        /* Release the packet.  */  
        nx_packet_release(receive_packet_ptr);
    }

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_send_query_get_rdata_by_name                 PORTABLE C     */ 
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allocates and sends a new DNS query on the specific   */   
/*    information.  On receiving a response, this function also invokes   */
/*    a process function to parse the resposne.                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    server_address                        The DNS server address        */
/*    host_name                             Name of host to resolve       */ 
/*    record_buffer                         Buffer for resource data      */ 
/*    buffer_size                           Buffer size for resource data */
/*    record_count                          The count of resource data    */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a new packet         */
/*    _nx_dns_new_packet_create             Create new DNS packet         */
/*    nx_packet_release                     Release packet                */ 
/*    nxd_udp_socket_send                   Send DNS UDP packet           */ 
/*    _nx_dns_response_get                  Get DNS response              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_host_resource_data_by_name_get                              */ 
/*                                          Get the resource data by name */ 
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
/*                                            improved the logic of       */
/*                                            receiving dns response,     */
/*                                            resulting in version 6.1.4  */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            receiving dns response,     */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_send_query_get_rdata_by_name(NX_DNS *dns_ptr, NXD_ADDRESS *dns_server_address, 
                                                 UCHAR *host_name, UCHAR *record_buffer, UINT buffer_size, 
                                                 UINT *record_count, UINT dns_record_type, ULONG wait_option)
{

UINT                status;
NX_PACKET           *packet_ptr;
#ifdef NX_DNS_CLIENT_CLEAR_QUEUE
NX_PACKET           *receive_packet_ptr;
#endif /* NX_DNS_CLIENT_CLEAR_QUEUE */
 
    /* Allocate a packet.  */
    status =  nx_packet_allocate(dns_ptr -> nx_dns_packet_pool_ptr, &packet_ptr, NX_UDP_PACKET, NX_DNS_PACKET_ALLOCATE_TIMEOUT);

    /* Check the allocate status.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Create a request */
    status =  _nx_dns_new_packet_create(dns_ptr, packet_ptr, host_name, (USHORT)dns_record_type);

    /* Check the DNS packet create status.  */
    if (status != NX_SUCCESS)
    {

        nx_packet_release(packet_ptr);

        /* Return error status.  */
        return(status);
    }

    /* We will use the time spent sleeping to clear broadcast DNS packets from a previous query
       from the DNS receive queue. This will prevent ensure the most recent DNS response is
       processed and avoid the situation of valid DNS response packets overflowing the DNS socket
       queue. */
#ifdef NX_DNS_CLIENT_CLEAR_QUEUE
    do
    {

        /* Is there any packets on the queue?  */
        status = nx_udp_socket_receive(&(dns_ptr -> nx_dns_socket), &receive_packet_ptr, NX_NO_WAIT); 

        /* Yes, we received a packet on the DNS port! */
        if (status == NX_SUCCESS)
        {

            /* But we don't want it. Release it! */
            nx_packet_release(receive_packet_ptr);
        }

        /* Keep checking till the queue becomes empty. */
    } while(status == NX_SUCCESS);
#endif /* NX_DNS_CLIENT_CLEAR_QUEUE */

    /* Send the DNS packet out.  */
    status =  nxd_udp_socket_send(&dns_ptr -> nx_dns_socket, packet_ptr, dns_server_address, NX_DNS_PORT);

    /* Check the completion of the send.  */
    if (status != NX_SUCCESS)
    {

        /* Unsuccessful, release the packet.  */
        nx_packet_release(packet_ptr);

        return status;
    }

    /* Check for non-blocking.  */
    if (wait_option == NX_NO_WAIT)
    {
        return(NX_IN_PROGRESS);
    }

    /* Wait for a DNS response.  */
    status = _nx_dns_response_get(dns_ptr, host_name, record_buffer, buffer_size, record_count, wait_option);

    /* Return completion status. */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_response_get                                 PORTABLE C     */ 
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets dns response.                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    host_name                             Name of host to resolve       */ 
/*    record_buffer                         Buffer for resource data      */ 
/*    buffer_size                           Buffer size for resource data */
/*    record_count                          The count of resource data    */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_response_receive              Receive DNS response          */ 
/*    _nx_dns_response_process              Process the DNS respondse     */
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_send_query_get_rdata_by_name  Get the resource data by name */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-02-2021     Yuxin Zhou               Initial Version 6.1.5         */
/*                                                                        */
/**************************************************************************/
UINT _nx_dns_response_get(NX_DNS *dns_ptr, UCHAR *host_name, UCHAR *record_buffer, 
                          UINT buffer_size, UINT *record_count, ULONG wait_option)
{
UINT        status;
NX_PACKET  *packet_ptr;


    /* Wait for a DNS response.  */
    status = _nx_dns_response_receive(dns_ptr, &packet_ptr, wait_option);

    /* Check status.  */
    if (status == NX_SUCCESS)
    {

#ifndef NX_DISABLE_PACKET_CHAIN
        if (packet_ptr -> nx_packet_next)
        {
            
            /* Chained packet is not supported. */
            nx_packet_release(packet_ptr);

            /* Release the resource obtained in _nx_dns_host_resource_data_by_name_get for non-blocking.  */
            if (wait_option == NX_NO_WAIT)
            {

                /* Unbind the socket.  */
                nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));
                tx_mutex_put(&dns_ptr -> nx_dns_mutex);
            }

            return(NX_INVALID_PACKET);
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Call the function to process the DNS packet.  */
        status = _nx_dns_response_process(dns_ptr, host_name, packet_ptr, record_buffer, buffer_size, record_count);
    }

    /* Release the resource obtained in _nx_dns_host_resource_data_by_name_get for non-blocking.  */
    if (wait_option == NX_NO_WAIT)
    {

        /* Unbind the socket.  */
        nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);
    }

    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_response_receive                             PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function receives dns response.                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to UDP packet pointer */
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_time_get                           Get the system time           */ 
/*    nx_udp_socket_receive                 Receive DNS UDP packet        */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_send_query_get_rdata_by_name  Get the resource data by name */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  02-02-2021     Yuxin Zhou               Initial Version 6.1.4         */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_response_receive(NX_DNS *dns_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT                status;
ULONG               start_time;
ULONG               current_time;
ULONG               elapsed_time;
ULONG               time_remaining;


    /* Initialize the value.  */
    start_time =  tx_time_get();
    elapsed_time = 0;
    time_remaining = wait_option;

    do
    {

        /* Receive udp packet. */
        status = nx_udp_socket_receive(&(dns_ptr -> nx_dns_socket), packet_ptr, time_remaining);

        /* Determine if this one is for us. */
        if (status == NX_SUCCESS)
        {
   
            /* Check the IDs in the DNS header match.  */
            if (((*packet_ptr) -> nx_packet_length >= sizeof(USHORT)) &&
                (_nx_dns_network_to_short_convert((*packet_ptr) -> nx_packet_prepend_ptr + NX_DNS_ID_OFFSET) == dns_ptr -> nx_dns_transmit_id))
            {

                /* They do. We can stop receiving packets and process this one. */
                break;
            }
            else
            {

                /* They do not. Discard the packet! */
                nx_packet_release((*packet_ptr));

                /* Continue to receive next packet.  */
                if (time_remaining == 0)
                {
                    continue;
                }
            }
        }

        /* Get the current time. */
        current_time = tx_time_get();

        /* Has the time wrapped? */
        if (current_time >= start_time)
        {

            /* No, simply subtract to get the elapsed time.   */
            elapsed_time = current_time - start_time;
        }
        else
        {

            /* Yes it has. Time has rolled over the 32-bit boundary.  */
            elapsed_time = (((ULONG) 0xFFFFFFFF) - start_time) + current_time;
        }

        /* Update the time remaining with the elapsed time. */
        if (time_remaining > elapsed_time)
        {
            time_remaining -= elapsed_time;
        }
        else
        {
            time_remaining = 0;
        }

    } while(time_remaining > 0);

    /* Return completion status. */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_response_process                             PORTABLE C     */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes a DNS respond packet. If the reply packet   */
/*    includes multiple answers. this service records as many answers     */
/*    into the record_buffer.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    host_name                             Name of host to resolve       */ 
/*    packet_ptr                            Pointer to received packet    */ 
/*    record_buffer                         Buffer for resource data      */ 
/*    buffer_size                           Buffer size for resource data */
/*    record_count                          The count of resource data    */ 
/*    lookup_type                           The DNS query type            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*    _nx_dns_network_to_short_convert      Convert to unsigned short     */ 
/*    _nx_dns_resource_type_get             Get the value of the resource */
/*                                            type.                       */
/*    _nx_dns_process_cname_type            Process the CNAME type record */ 
/*    _nx_dns_process_txt_type              Process the TXT type record   */ 
/*    _nx_dns_process_ns_type               Process the NS type record    */ 
/*    _nx_dns_process_mx_type               Process the MX type record    */ 
/*    _nx_dns_process_srv_type              Process the SRV type record   */
/*    _nx_dns_process_a_type                Process the A type record     */
/*    _nx_dns_process_aaaa_type             Process the AAAA type record  */
/*    _nx_dns_process_soa_type              Process the SOA type record   */
/*    nx_packet_release                     Release the packet.           */
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
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            receiving dns response,     */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_response_process(NX_DNS *dns_ptr, UCHAR *host_name, NX_PACKET *packet_ptr, 
                                     UCHAR *record_buffer, UINT buffer_size, UINT *record_count)
{

UINT                status;
USHORT              answerCount;
USHORT              answerRRCount;
USHORT              authorityRRCount;
USHORT              additionalRRCount;
UCHAR               *data_ptr;
UINT                response_type;
UCHAR               *buffer_prepend_ptr;
UCHAR               *buffer_append_ptr;
UINT                rrIndex;
UINT                rr_location; 
UINT                answer_found = NX_FALSE;
UINT                resource_size;
UINT                name_size;
UINT                host_name_size;

    /* Set the buffer pointer.  */
    buffer_prepend_ptr = record_buffer;
    buffer_append_ptr = record_buffer + buffer_size;

    /* We received a response. Is there a valid header?  */
    if (packet_ptr -> nx_packet_length <= NX_DNS_QDSECT_OFFSET)
    {

        /* No; Release the new packet.  */ 
        nx_packet_release(packet_ptr);

        /* Return error status. */
        return NX_DNS_MALFORMED_PACKET;
    }
    
    /* Does the incoming DNS header ID match the query ID we sent out?  */
    if (_nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_ID_OFFSET) != dns_ptr -> nx_dns_transmit_id)
    {

        /* No; Release the packet.  */ 
        nx_packet_release(packet_ptr);

        /* Return error status. */
        return NX_DNS_BAD_ID_ERROR;
    }

    /* Check that the packet has a valid response record.  */
    status =  _nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_FLAGS_OFFSET);

    /* Check for indication of DNS server error (cannot authenticate answer or authority portion 
       of the DNS data. */
    if ((status & NX_DNS_ERROR_MASK) == NX_DNS_ERROR_MASK)
    {

        /* Release the source packet.  */
        nx_packet_release(packet_ptr);

        return NX_DNS_SERVER_AUTH_ERROR;
    }

    /* Determine if we have any 'answers' to our DNS query. */
    answerRRCount = _nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_ANCOUNT_OFFSET);
    answerCount = answerRRCount;

    /* Also check if there are any 'hints' from the Authoritative nameserver. */
    authorityRRCount = _nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_NSCOUNT_OFFSET);
    answerCount = (USHORT)(answerCount + authorityRRCount);

    /* Include Additional section as well */
    additionalRRCount = _nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_ARCOUNT_OFFSET);
    answerCount = (USHORT)(answerCount + additionalRRCount);

    /* Are there answers in this resposne? */
    if (((status & NX_DNS_QUERY_MASK) == NX_DNS_RESPONSE_FLAG)
        && ((status & NX_DNS_RCODE_MASK) == NX_DNS_RCODE_SUCCESS)
        && (answerCount >= 1))
    {


        /* This looks like the response to our question, now find the response record.  */
        /* Point at the start of the question.  */
        data_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_DNS_QDSECT_OFFSET; 

        /* Determine if there is a question still in the server's response.  */
        if (_nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_QDCOUNT_OFFSET) == 1)
        {

            /* Get name size */
            name_size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

            if (!name_size)
            {

                /* Release the source packet.  */
                nx_packet_release(packet_ptr);

                return NX_DNS_MALFORMED_PACKET;
            }

            /* Check for name.  */
            if (_nx_utility_string_length_check((CHAR *)host_name, &host_name_size, name_size) ||
                (name_size != host_name_size) ||
                (memcmp(host_name, temp_string_buffer, name_size) != 0))
            {
                
                /* Release the source packet.  */
                nx_packet_release(packet_ptr);

                /* This was not what the Client requested. Return error status.  */
                return(NX_DNS_MISMATCHED_RESPONSE);
            }

            /* Get the length of name field.  */
            name_size = _nx_dns_name_size_calculate(data_ptr, packet_ptr);

            /* Check if the data pointer is valid.  */
            if (data_ptr + name_size + 4 >= packet_ptr -> nx_packet_append_ptr)
            {

                /* Release the source packet.  */
                nx_packet_release(packet_ptr);

                return(NX_DNS_MALFORMED_PACKET);
            }

            /* Check the type and class.  */
            if ((_nx_dns_network_to_short_convert(data_ptr + name_size) != dns_ptr -> nx_dns_lookup_type) || 
                (_nx_dns_network_to_short_convert(data_ptr + name_size + 2) != NX_DNS_RR_CLASS_IN))
            {

                /* Release the source packet.  */
                nx_packet_release(packet_ptr);

                /* This was not what the Client requested. Return error status.  */
                return(NX_DNS_MISMATCHED_RESPONSE);
            }

            /* Yes, the question is present in the response, skip it!  */
            data_ptr +=  name_size + 4;
        }

        /* Set the status.  */
        status = NX_SUCCESS;

        /* Check all the response records */
        for(rrIndex = 0; rrIndex < answerCount; rrIndex++)
        {

            /* Set the section class.  */
            if(answerRRCount && (rrIndex < answerRRCount))
                rr_location = NX_DNS_RR_ANSWER_SECTION;
            else if(authorityRRCount && (rrIndex < (UINT)(answerRRCount + authorityRRCount)))
                rr_location = NX_DNS_RR_AUTHORITY_SECTION;
            else
                rr_location = NX_DNS_RR_ADDITIONAL_SECTION;

            /* Check for valid data_ptr.  */
            if (data_ptr >= packet_ptr -> nx_packet_append_ptr)
            {

                /* Process error.  */
                break;
            }

            /* Process the server response. */
            status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
            if (status)
            {

                /* Process error.  */
                break;
            }

            /* Is this an A Type? */
            if(response_type == NX_DNS_RR_TYPE_A)
            {
                
                /* Call the function process the A type message. */
                status = _nx_dns_process_a_type(dns_ptr, packet_ptr, data_ptr,  &buffer_prepend_ptr, &buffer_append_ptr, record_count, rr_location);
                
            }
            
            /* Is this an AAAA Type? */
            else if(response_type == NX_DNS_RR_TYPE_AAAA)
            {
                
                /* Call the function process the AAAA type message. */
                status = _nx_dns_process_aaaa_type(dns_ptr, packet_ptr, data_ptr,  &buffer_prepend_ptr, &buffer_append_ptr, record_count, rr_location);
                
            }

#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 

            /* Is this a Type NX_DNS_RR_TYPE_CNAME?  */
            else if(response_type == NX_DNS_RR_TYPE_CNAME)
            {

                /* Call the function process the CNAME type message. */
                status = _nx_dns_process_cname_type(dns_ptr, packet_ptr, data_ptr, record_buffer, buffer_size, record_count);

            }

            /* Is this a Type NX_DNS_RR_TYPE_TXT?  */
            else if(response_type == NX_DNS_RR_TYPE_TXT)
            {                      

                /* Call the function process the TXT type message. */                
                status = _nx_dns_process_txt_type(dns_ptr, packet_ptr, data_ptr, record_buffer, buffer_size, record_count);
            }

            /* Is this a Type NX_DNS_RR_TYPE_NS?  */
            else if(response_type == NX_DNS_RR_TYPE_NS)
            {

                /* Call the function process the NS type message. */
                status = _nx_dns_process_ns_type(dns_ptr, packet_ptr, data_ptr, &buffer_prepend_ptr, &buffer_append_ptr, record_count);
            }


            /* Is this a Type NX_DNS_RR_TYPE_MX?  */
            else if(response_type == NX_DNS_RR_TYPE_MX)
            {

                /* Call the function process the MX type message. */
                status = _nx_dns_process_mx_type(dns_ptr, packet_ptr, data_ptr, &buffer_prepend_ptr, &buffer_append_ptr, record_count);
            }

            /* Is this a Type NX_DNS_RR_TYPE_SRV?  */
            else if(response_type == NX_DNS_RR_TYPE_SRV)
            {

                /* Call the function process the SRV type message. */
                status = _nx_dns_process_srv_type(dns_ptr, packet_ptr, data_ptr, &buffer_prepend_ptr, &buffer_append_ptr, record_count);
            }
            
            /* Is this a Type NX_DNS_RR_TYPE_SOA?  */
            else if(response_type == NX_DNS_RR_TYPE_SOA)
            {

                /* Call the function process the SOA type message. */
                status = _nx_dns_process_soa_type(dns_ptr, packet_ptr, data_ptr, record_buffer, buffer_size, record_count);

            }
#endif

            /* Check the process status.  */
            if(status != NX_SUCCESS)
            {

                /* Process error.  */
                break;
            }
            else
            {

                /* Did we get a correct answer?  */
                if ((answer_found == NX_FALSE) &&
                    (response_type == dns_ptr -> nx_dns_lookup_type))
                {
                    answer_found = NX_TRUE;
                }
            }

            status = _nx_dns_resource_size_get(data_ptr, packet_ptr, &resource_size);
            if (status)
            {

                /* Process error, reset answer is not found.  */
                break;
            }
            data_ptr += resource_size;
        }  
    }
    
    /* Release the packet.  */ 
    nx_packet_release(packet_ptr);

    /* Check the answer found flag.  */
    if (answer_found)
        status = NX_SUCCESS;
    else
        status = NX_DNS_QUERY_FAILED;

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_a_type                               PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the A record type.   If the DNS look up type  */ 
/*    was NS, MX, or SRV type, this function also parses the additional   */ 
/*    section looking for A type information.                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */ 
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    buffer_prepend_ptr                    Pointer to the starting       */
/*                                            address of available buffer */ 
/*    buffer_append_ptr                     Pointer to the ending address */ 
/*                                            of available buffer         */ 
/*    record_count                          The count of the IPv4 address */ 
/*    rr_location                           The DNS resource data section */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_a_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, 
                                   UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, 
                                   UINT *record_count, UINT rr_location)
{

UINT            response_type;
ULONG           ipv4_address;
UINT            status;
UINT            data_length;
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
UCHAR           *buffer_header_ptr;
#endif
#ifdef NX_DNS_CACHE_ENABLE 
UINT            size; 
ULONG           rr_ttl;
#endif /* NX_DNS_CACHE_ENABLE  */

#if !defined(NX_DNS_CACHE_ENABLE) && !defined(NX_DNS_ENABLE_EXTENDED_RR_TYPES)
    NX_PARAMETER_NOT_USED(packet_ptr);
#endif

#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));

    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    } 
 
    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Process the A type message in the answer section.*/
    if((rr_location == NX_DNS_RR_ANSWER_SECTION)||
       (rr_location == NX_DNS_RR_AUTHORITY_SECTION))
    {

        /* Verify this is what the DNS Client was requesting. */
        if (response_type != dns_ptr -> nx_dns_lookup_type)
        {

            /* No, this was not what the Client requested. Return error status. 
            This should not happen so return error to the host application,
            might be a problem with the query or the server. */
            return NX_DNS_MISMATCHED_RESPONSE;
        }

        /* Yes, make sure it has the correct data size.  */
        status = _nx_dns_resource_data_length_get(data_ptr, packet_ptr, &data_length);
        if (status)
        {

            /* Return!  */
            return(NX_DNS_MALFORMED_PACKET);
        }

        if (data_length == 4)
        {

            /* Get data address and check if it is valid. */ 
            data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
            if ((!data_ptr) || ((data_ptr + sizeof(ULONG)) > packet_ptr -> nx_packet_append_ptr))
            {

                /* Return!  */
                return(NX_DNS_MALFORMED_PACKET);
            }

            /* Finally, get the address!!! */
            ipv4_address =  _nx_dns_network_to_long_convert(data_ptr);

            /* Check the buffer space.  */
            if (*buffer_prepend_ptr + 4 > *buffer_append_ptr)
            {

                /* The buffer space is not enough.  */
                return(NX_DNS_NEED_MORE_RECORD_BUFFER);
            }

            /* Set the return IP address.  */
            memcpy(*buffer_prepend_ptr,&ipv4_address,4); /* Use case of memcpy is verified. */

            /* Update the record buffer pointer. */
            *buffer_prepend_ptr +=4;

            /* Update the count of ipv4 address.  */
            (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 
            /* Set the resource record type.  */
            temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_A;     

            /* Set the resource record ttl.  */
            temp_rr.nx_dns_rr_ttl = rr_ttl;

            /* Add the name string.  */
            status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

            /* Check the status.  */
            if(status)
                return (NX_SUCCESS);

            /* Add the IPv4 address.  */
            temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_a.nx_dns_rr_a_address = ipv4_address;

            /* Add the resource record.  */
            status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);
                           
            /* Check the status.  */
            if(status)
            {

                /* Delete the resource record.  */
                _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
            }
#endif /* NX_DNS_CACHE_ENABLE  */

            /* Success!  Return success to caller!  */
            return(NX_SUCCESS);
        }
        else
        {

            /* Return.*/
            return(NX_DNS_MALFORMED_PACKET);
        }
    }
   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
    /* Process the A type message in the additional section.  */
    else if(rr_location == NX_DNS_RR_ADDITIONAL_SECTION)
    {
        if(dns_ptr -> nx_dns_lookup_type == NX_DNS_RR_TYPE_NS)
        {
            NX_DNS_NS_ENTRY *ns_entry;
            UINT            entry_index;
            UINT            name_size;

            /* Processing NS query, and encountered A type in the additional section. */
            /* This means the record_buffer should already contain name server information,
               and the name server IP address is in this resource record. */
            
            /* First obtain the string. */
            temp_string_buffer[0] = 0;
            name_size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

            /* Check the string correct.  */
            if(!name_size)
            {

                /* Return !*/
                return(NX_DNS_MALFORMED_PACKET);
            }

            /* Get the first address of record buffer.  */
            buffer_header_ptr = *buffer_prepend_ptr - ((*record_count) * sizeof(NX_DNS_NS_ENTRY));
            
            /* Go through all the records and match the string. */
            for(entry_index = 0; entry_index < *record_count; entry_index++)
            {

                /* Set the NS entry.  */
                ns_entry = (NX_DNS_NS_ENTRY*)(buffer_header_ptr);

                /* Check the ipv4 address.*/
                if( ns_entry -> nx_dns_ns_ipv4_address != IP_ADDRESS(0, 0, 0, 0))
                {
                    
                    /* Update the buffer prepend pointer, and match the next NS entry.  */
                    buffer_header_ptr += sizeof(NX_DNS_NS_ENTRY);

                    continue;
                }

                /* The nx_dns_ns_hostname_ptr is set internally with null termination. */
                status = _nx_utility_string_length_check((CHAR *)(ns_entry -> nx_dns_ns_hostname_ptr), &data_length, name_size);

                if((status == NX_SUCCESS) &&
                   (data_length == name_size) &&
                   ((memcmp(ns_entry -> nx_dns_ns_hostname_ptr, &temp_string_buffer[0], name_size)) == 0))
                {

                    /* This A type record contains the IPv4 address for the NS entry.  */
                    
                    /* Yes, make sure it has the correct data size.  */
                    status = _nx_dns_resource_data_length_get(data_ptr, packet_ptr, &data_length);
                    if (status)
                    {

                        /* Return!  */
                        return(NX_DNS_MALFORMED_PACKET);
                    }

                    if (data_length == 4)
                    {

                        /* Get data address and check if it is valid. */ 
                        data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
                        if ((!data_ptr) || ((data_ptr + sizeof(ULONG)) > packet_ptr -> nx_packet_append_ptr))
                        {

                            /* Return!  */
                            return(NX_DNS_MALFORMED_PACKET);
                        }

                        /* Finally, get the address!!! */
                        ipv4_address =  _nx_dns_network_to_long_convert(data_ptr);
                        
                        /* Record the NS entry ipv4 address . */
                        ns_entry -> nx_dns_ns_ipv4_address = ipv4_address;
                        
                        /* Success!  Return success to caller!  */
                        return(NX_SUCCESS);
                    }
                }
                else
                {

                    /* Update the buffer prepend pointer, and match the next NS entry.  */
                    buffer_header_ptr += sizeof(NX_DNS_NS_ENTRY);
                }
            }
        }
            
        else if(dns_ptr -> nx_dns_lookup_type == NX_DNS_RR_TYPE_MX)
        {
            NX_DNS_MX_ENTRY *mx_entry;
            UINT entry_index;
            UINT name_size;

            /* Processing MX query, and encountered A type in the additional section. */
            /* This means the record_buffer should already contain mail exchange information,
               and the mail server IP address is in this resource record. */
            
            /* First obtain the string. */
            temp_string_buffer[0] = 0;
            name_size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

            /* Check the string correct.  */
            if(!name_size)
            {

                /* Return !*/
                return(NX_DNS_MALFORMED_PACKET);
            }

            /* Set the record buffer prepend.  */
            buffer_header_ptr = *buffer_prepend_ptr - ((*record_count) * sizeof(NX_DNS_MX_ENTRY));
            
            /* Go through all the records and match the string. */
            for(entry_index = 0; entry_index < *record_count; entry_index++)
            {

                /* Set the MX entry pointer.  */
                mx_entry = (NX_DNS_MX_ENTRY*)(buffer_header_ptr);
                
                /* Check the ipv4 address, If the ipv4 address has been set, skip it..*/
                if( mx_entry -> nx_dns_mx_ipv4_address != IP_ADDRESS(0, 0, 0, 0))
                {

                    /* Update the buffer prepend pointer, and match the next MX entry.  */
                    buffer_header_ptr += sizeof(NX_DNS_MX_ENTRY);
                    continue;
                }

                /* The nx_dns_mx_hostname_ptr is set internally with null termination. */
                status = _nx_utility_string_length_check((CHAR *)(mx_entry -> nx_dns_mx_hostname_ptr), &data_length, name_size);

                if((status == NX_SUCCESS) &&
                   (data_length == name_size) &&
                   ((memcmp(mx_entry -> nx_dns_mx_hostname_ptr, &temp_string_buffer[0], name_size)) == 0))
                {

                    /* This A type record contains the IPv4 address for the MX entry.  */
                    
                    /* Yes, make sure it has the correct data size.  */
                    status = _nx_dns_resource_data_length_get(data_ptr, packet_ptr, &data_length);
                    if (status)
                    {

                        /* Return!  */
                        return(NX_DNS_MALFORMED_PACKET);
                    }

                    if (data_length == 4)
                    {

                        /* Get data address and check if it is valid. */ 
                        data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
                        if ((!data_ptr) || ((data_ptr + sizeof(ULONG)) > packet_ptr -> nx_packet_append_ptr))
                        {

                            /* Return!  */
                            return(NX_DNS_MALFORMED_PACKET);
                        }

                        /* Finally, get the address!!! */
                        ipv4_address =  _nx_dns_network_to_long_convert(data_ptr);
                        
                        /* Record the MX entry ipv4 address . */
                        mx_entry -> nx_dns_mx_ipv4_address = ipv4_address;
                        
                        /* Success!  Return success to caller!  */
                        return(NX_SUCCESS);
                    }
                }
                else
                {

                    /* Update the buffer prepend pointer, and match the next MX entry.  */
                    buffer_header_ptr += sizeof(NX_DNS_MX_ENTRY);
                }
            }
        }
        else if(dns_ptr -> nx_dns_lookup_type == NX_DNS_RR_TYPE_SRV)
        {
            NX_DNS_SRV_ENTRY *srv_entry;
            UINT entry_index;
            UINT name_size;

            /* Processing SRV query, and encountered A type in the additional section. */
            /* This means the record_buffer should already contain SRV information,
               and the mail server IP address is in this resource record. */
            
            /* First obtain the string. */
            temp_string_buffer[0] = 0;
            name_size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

            /* Check the string correct.  */
            if(!name_size)
            {

                /* Return !*/
                return(NX_DNS_MALFORMED_PACKET);
            }

            /* Set the record buffer prepend.  */
            buffer_header_ptr = *buffer_prepend_ptr - ((*record_count) * sizeof(NX_DNS_SRV_ENTRY));
            
            /* Go through all the records and match the string. */
            for(entry_index = 0; entry_index < *record_count; entry_index++)
            {

                /* Set the MX entry pointer.  */
                srv_entry = (NX_DNS_SRV_ENTRY*)(buffer_header_ptr);
                
                /* Check the ipv4 address, If the ipv4 address has been set, skip it..*/
                if( srv_entry -> nx_dns_srv_ipv4_address != IP_ADDRESS(0, 0, 0, 0))
                {

                    /* Update the buffer prepend pointer, and match the next MX entry.  */
                    buffer_header_ptr += sizeof(NX_DNS_SRV_ENTRY);
                    continue;
                }

                /* The nx_dns_srv_hostname_ptr is set internally with null termination. */
                status = _nx_utility_string_length_check((CHAR *)(srv_entry -> nx_dns_srv_hostname_ptr), &data_length, name_size);

                if((status == NX_SUCCESS) &&
                   (data_length == name_size) &&
                   ((memcmp(srv_entry -> nx_dns_srv_hostname_ptr, &temp_string_buffer[0], name_size)) == 0))
                {

                    /* This A type record contains the IPv4 address for the MX entry.  */
                    
                    /* Yes, make sure it has the correct data size.  */
                    status = _nx_dns_resource_data_length_get(data_ptr, packet_ptr, &data_length);
                    if (status)
                    {

                        /* Return!  */
                        return(NX_DNS_MALFORMED_PACKET);
                    }

                    if (data_length == 4)
                    {

                        /* Get data address and check if it is valid. */ 
                        data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
                        if ((!data_ptr) || ((data_ptr + sizeof(ULONG)) > packet_ptr -> nx_packet_append_ptr))
                        {

                            /* Return!  */
                            return(NX_DNS_MALFORMED_PACKET);
                        }

                        /* Finally, get the address!!! */
                        ipv4_address =  _nx_dns_network_to_long_convert(data_ptr);
                        
                        /* Record the SRV entry ipv4 address . */
                        srv_entry -> nx_dns_srv_ipv4_address = ipv4_address;
                        
                        /* Success!  Return success to caller!  */
                        return(NX_SUCCESS);
                    }
                }
                else
                {

                    /* Update the buffer prepend pointer, and match the next MX entry.  */
                    buffer_header_ptr += sizeof(NX_DNS_SRV_ENTRY);
                }
            }
        }      
    }
#endif

    /* Return.*/
    return(NX_SUCCESS);         
}                         


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_aaaa_type                            PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the AAAA record type.                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */  
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    buffer_prepend_ptr                    Pointer to the starting       */
/*                                            address of available buffer */ 
/*    buffer_append_ptr                     Pointer to the ending address */ 
/*                                            of available buffer         */ 
/*    record_count                          The count of the IPv6 address */ 
/*    rr_location                           The DNS resource data section */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_aaaa_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, 
                                      UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, 
                                      UCHAR **buffer_append_ptr, UINT *record_count, 
                                      UINT rr_location)
{
    
UINT                    response_type;
UINT                    i;
ULONG                   ipv6_address;
NX_DNS_IPV6_ADDRESS     *ipv6_address_ptr;
UINT                    status;
UINT                    data_length;
#ifdef NX_DNS_CACHE_ENABLE 
UINT                    size;
ULONG                   rr_ttl;  
#endif /* NX_DNS_CACHE_ENABLE  */

#ifndef NX_DNS_CACHE_ENABLE
    NX_PARAMETER_NOT_USED(packet_ptr);
#endif

#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));

    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
 
    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
    
    /* Process the A type message in the answer section.*/
    if((rr_location == NX_DNS_RR_ANSWER_SECTION)||
       (rr_location == NX_DNS_RR_AUTHORITY_SECTION))
    {

        /* Verify this is what the DNS Client was requesting. */
        if (response_type != dns_ptr -> nx_dns_lookup_type)
        {

            /* No, this was not what the Client requested. Return error status. 
            This should not happen so return error to the host application,
            might be a problem with the query or the server. */
            return NX_DNS_MISMATCHED_RESPONSE;
        }

        /* Yes, make sure it has the correct data size.  */
        status = _nx_dns_resource_data_length_get(data_ptr, packet_ptr, &data_length);
        if (status)
        {

            /* Return!  */
            return(NX_DNS_MALFORMED_PACKET);
        }

        if (data_length == 16)
        {
                        
            /* Check the buffer space.  */
            if ((*buffer_prepend_ptr + sizeof(NX_DNS_IPV6_ADDRESS)) > *buffer_append_ptr)
            {

                /* The buffer space is not enough.  */
                return(NX_DNS_NEED_MORE_RECORD_BUFFER);
            }

            /* Update the pointer to the ipv6 address.  */
            data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
            if ((!data_ptr) || ((data_ptr + (4 * sizeof(ULONG))) > packet_ptr -> nx_packet_append_ptr))
            {

                /* Return!  */
                return(NX_DNS_MALFORMED_PACKET);
            }

            ipv6_address_ptr = (NX_DNS_IPV6_ADDRESS*)(*buffer_prepend_ptr);
            
            /* Finally, Record the ipv6 address to record buffer.  */
            for(i = 0; i < 4; i++)
            {

                ipv6_address = _nx_dns_network_to_long_convert(data_ptr);

                ipv6_address_ptr -> ipv6_address[i] = ipv6_address;

                data_ptr +=4;

            }

            *buffer_prepend_ptr = *buffer_prepend_ptr + sizeof(NX_DNS_IPV6_ADDRESS);
                        
            /* Update the count of ipv6 address.  */
            (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 
            /* Set the resource record type.  */
            temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_AAAA;     

            /* Set the resource record ttl.  */
            temp_rr.nx_dns_rr_ttl = rr_ttl;
                 
            /* Add the name string.  */
            status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

            /* Check the status.  */
            if(status)
                return (NX_SUCCESS);
                                       
            /* Add the IPv6 address string.  */
            status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, ipv6_address_ptr, 16, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_aaaa.nx_dns_rr_aaaa_address)));

            /* Check the status.  */
            if(status)
            {
                _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
                return (NX_SUCCESS);
            }

            /* Add the resource record.  */
            status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);
                           
            /* Check the status.  */
            if(status)
            {

                /* Delete the resource record.  */
                _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
            }   
#endif /* NX_DNS_CACHE_ENABLE  */

            /* Success!  */
            return(NX_SUCCESS);
        }
        else
        {

            /* Return.*/
            return(NX_DNS_MALFORMED_PACKET);
        }
        
    }

    /* Return.*/
    return(NX_SUCCESS);

}


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_cname_type                           PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the CNAME record type.                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */ 
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    record_buffer                         Buffer space for storing      */ 
/*                                            the host cname              */
/*    buffer_size                           Size of record_buffer.        */
/*    record_count                          Record count                  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_cname_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, 
                                       UCHAR *record_buffer, UINT buffer_size, UINT *record_count)
{
UINT            response_type;
UINT            name_size;
UINT            status;
#ifdef NX_DNS_CACHE_ENABLE  
UINT            size;
ULONG           rr_ttl;
#endif /* NX_DNS_CACHE_ENABLE  */
                
#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));
                             
    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
 
    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
    
    /* Verify this is what the DNS Client was requesting. */
    if(response_type == dns_ptr -> nx_dns_lookup_type)
    { 

        /* Update the pointer to point at the resource data.  */
        data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
        if (!data_ptr)
        {
            /* Return!  */
            return(NX_DNS_MALFORMED_PACKET);
        }

        /* Determine if there is room for the name - one less for NULL termination.  */
        name_size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, record_buffer, buffer_size - 1);
        if (name_size)
        {

            /* Yes, got the canonical name successfully,and record the information!  */

            /* Update the count.  */
            (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 
            /* Set the resource record type.  */
            temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_CNAME;     

            /* Set the resource record ttl.  */
            temp_rr.nx_dns_rr_ttl = rr_ttl;

            /* Add the name string.  */
            status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

            /* Check the status.  */
            if(status)
                return (NX_SUCCESS);
                                                
            /* Add the cname string.  */
            status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, record_buffer, name_size, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_cname.nx_dns_rr_cname_name)));

            /* Check the status.  */
            if(status)         
            {
                _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
                return (NX_SUCCESS);
            }

            /* Add the resource record.  */
            status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);
                           
            /* Check the status.  */
            if(status)
            {

                /* Delete the resource record.  */
                _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
            } 
#endif /* NX_DNS_CACHE_ENABLE  */

            return(NX_SUCCESS);
        }
        else
        {

            /* Return !*/
            return(NX_DNS_MALFORMED_PACKET);
        }
    }
    
    /* If send A type query, should also process the CNAME + A type response.
       RFC1034, section3.6.2, page15 and RFC10.5 section 3.3.1, page14. */
    else if (dns_ptr -> nx_dns_lookup_type == NX_DNS_RR_TYPE_A)
    {
        
        /* Only get the A type message, skip the CNAME answer process, 
           and return success to process next A type answer. 
           The CNAME record to be implemented.  */        
        return(NX_SUCCESS);
    }

    /* Error lookup response*/
    else
    {
        /* Return error status.  */
        return NX_DNS_MISMATCHED_RESPONSE;
    }

}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_txt_type                             PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function process the TXT DNS type packet and record text string*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */ 
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    record_buffer                         Buffer space for storing      */ 
/*                                            the host text string        */
/*    buffer_size                           Size of record_buffer.        */
/*    record_count                          Record count                  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_txt_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, 
                                     UCHAR *record_buffer, UINT buffer_size, UINT *record_count)
{
UINT            response_type;
UINT            text_data_length; 
UINT            status;
#ifdef NX_DNS_CACHE_ENABLE    
UINT            size;  
ULONG           rr_ttl;
#endif /* NX_DNS_CACHE_ENABLE  */  
              
#ifdef NX_DNS_CACHE_ENABLE                
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));
                             
    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
                              
    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
#else
    NX_PARAMETER_NOT_USED(packet_ptr);
#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Verify this is what the DNS Client was requesting. */
    if (response_type != dns_ptr -> nx_dns_lookup_type)
    {

        /* No, this was not what the Client requested. Return error status. 
        This should not happen so return error to the host application,
        might be a problem with the query or the server. */
        return NX_DNS_MISMATCHED_RESPONSE;
    }

    /* Update the pointer to point at the response data.  */
    data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
    if (!data_ptr)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the text resource data length.  */
    text_data_length = (UINT) (*data_ptr++); 

    /* Judge the resource data buffer space.  */
    if ((text_data_length > buffer_size - 1) || ((data_ptr + text_data_length) > packet_ptr -> nx_packet_append_ptr))
    {

        /* Return error, and release the packet in repsonse*/
        return(NX_DNS_MALFORMED_PACKET);

    }
    else
    {

        /* Record the text string to the buffer.  */
        memcpy(&record_buffer[0],data_ptr,text_data_length); /* Use case of memcpy is verified. */

        /* Null terminate text.  */
        record_buffer[text_data_length] =  '\0';

        /* Update the count.  */
        (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 
        /* Set the resource record type.  */
        temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_TXT;     

        /* Set the resource record ttl.  */
        temp_rr.nx_dns_rr_ttl = rr_ttl;

        /* Add the name string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

        /* Check the status.  */
        if(status)
            return (NX_SUCCESS);

        /* Add the txt string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, record_buffer, text_data_length, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_txt.nx_dns_rr_txt_data)));

        /* Check the status.  */
        if(status)         
        {
            _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
            return (NX_SUCCESS);
        }

        /* Add the resource record.  */
        status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);

        /* Check the status.  */
        if(status)
        {

            /* Delete the resource record.  */
            _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
        }          
#endif /* NX_DNS_CACHE_ENABLE  */

        /* Yes; We're done! Return success!  */
        return(NX_SUCCESS);
    }  
}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_ns_type                              PORTABLE C     */ 
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the NS record type.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    buffer_prepend_ptr                    Pointer to the starting       */
/*                                            address of available buffer */ 
/*    buffer_append_ptr                     Pointer to the ending address */ 
/*                                            of available buffer         */ 
/*    record_count                          The count of the name server  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*    _nx_dns_resource_name_real_size_calculate                           */ 
/*                                          Calculate real size of name   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), updated  */
/*                                            input parameter of the API  */
/*                                            to get the real size of     */
/*                                            resource name,              */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_ns_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, 
                                    UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, 
                                    UINT *record_count)
{
NX_DNS_NS_ENTRY     *nx_dns_ns_entry_ptr;    
UINT                response_type;
UINT                name_buffer_size;
UINT                status;
#ifdef NX_DNS_CACHE_ENABLE 
UINT                size;
ULONG               rr_ttl;
#endif /* NX_DNS_CACHE_ENABLE  */
            
#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));
                             
    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Verify this is what the DNS Client was requesting. */
    if (response_type != dns_ptr -> nx_dns_lookup_type)
    {

        /* No, this was not what the Client requested. Return error status. 
        This should not happen so return error to the host application,
        might be a problem with the query or the server. */
        return NX_DNS_MISMATCHED_RESPONSE;
    }

    /* Update the pointer to point at the resource data.  */
    data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
    if (!data_ptr)
    {
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the real size of the name, and set the name buffer size.*/
    name_buffer_size = _nx_dns_resource_name_real_size_calculate(packet_ptr -> nx_packet_prepend_ptr, (UINT)(data_ptr - packet_ptr -> nx_packet_prepend_ptr), packet_ptr -> nx_packet_length);

    /* Check the buffer space.  */
    if ((*buffer_append_ptr - name_buffer_size - 1 ) < (*buffer_prepend_ptr + sizeof(NX_DNS_NS_ENTRY)))
    {

        /* The buffer space is not enough.  */
        return(NX_DNS_NEED_MORE_RECORD_BUFFER);
    }

    /* Set the ns entry pointer.  */
    nx_dns_ns_entry_ptr = (NX_DNS_NS_ENTRY *)(*buffer_prepend_ptr);

    /* Initialize the  variables to NULL. */
    memset(nx_dns_ns_entry_ptr, 0, sizeof(NX_DNS_NS_ENTRY));

    /* Update the buffer pointer.*/
    *buffer_prepend_ptr += sizeof(NX_DNS_NS_ENTRY);

    /* Update the store pointer. include the null flag '\0'.  */
    *buffer_append_ptr -= (name_buffer_size + 1);

    /* Determine if there is room for the name - one less for NULL termination.  */
    if (_nx_dns_name_string_unencode(packet_ptr, data_ptr, *buffer_append_ptr, name_buffer_size))
    {

        /* Yes,record the name server successfully.  */

        /* Update the name server pointer.  */
        nx_dns_ns_entry_ptr -> nx_dns_ns_hostname_ptr = *buffer_append_ptr;

        /* Update the count of ns.  */
        (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 
        /* Set the resource record type.  */
        temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_NS;     

        /* Set the resource record ttl.  */
        temp_rr.nx_dns_rr_ttl = rr_ttl;

        /* Add the name string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

        /* Check the status.  */
        if(status)
            return (NX_SUCCESS);

        /* Add the ns string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, nx_dns_ns_entry_ptr -> nx_dns_ns_hostname_ptr, name_buffer_size, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_ns.nx_dns_rr_ns_name)));

        /* Check the status.  */
        if(status)         
        {
            _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
            return (NX_SUCCESS);
        }

        /* Add the resource record.  */
        status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);

        /* Check the status.  */
        if(status)
        {

            /* Delete the resource record.  */
            _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
        }   
#endif /* NX_DNS_CACHE_ENABLE  */

        return(NX_SUCCESS);
    }
    else
    {
        /* Ruturn.  */
        return(NX_DNS_MALFORMED_PACKET);
    }
}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_mx_type                              PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the MX record type.                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    buffer_prepend_ptr                    Pointer to the starting       */
/*                                            address of available buffer */ 
/*    buffer_append_ptr                     Pointer to the ending address */ 
/*                                            of available buffer         */ 
/*    record_count                          The count of the mail exchange*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*    _nx_dns_resource_name_real_size_calculate                           */ 
/*                                          Calculate real size of name   */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), updated  */
/*                                            input parameter of the API  */
/*                                            to get the real size of     */
/*                                            resource name,              */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_mx_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, 
                                    UCHAR **buffer_prepend_ptr, UCHAR **buffer_append_ptr, 
                                    UINT *record_count)
{
NX_DNS_MX_ENTRY     *nx_dns_mx_entry_ptr;    
UINT                response_type;
UINT                name_buffer_size;
USHORT              mx_preference;
UINT                name_length;
UINT                status;
#ifdef NX_DNS_CACHE_ENABLE  
ULONG               rr_ttl;
UINT                size;
#endif /* NX_DNS_CACHE_ENABLE  */
           
#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));
                             
    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
                        
    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Verify this is what the DNS Client was requesting. */
    if (response_type != dns_ptr -> nx_dns_lookup_type)
    {
        
        /* No, this was not what the Client requested. Return error status. 
        This should not happen so return error to the host application,
        might be a problem with the query or the server. */
        return NX_DNS_MISMATCHED_RESPONSE;
    }

    /* Update the pointer to point at the resource data.  */
    data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
    if ((!data_ptr) || ((data_ptr + sizeof(USHORT)) >= packet_ptr -> nx_packet_append_ptr))
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the preference data of the resource data.  */
    mx_preference = _nx_dns_network_to_short_convert(data_ptr);

    /* Skip the MX preference, and update the pointer to point at the mail exchange data.  */
    data_ptr += 2;

    /* Get the real size of the name, and set the name buffer size.*/
    name_buffer_size = _nx_dns_resource_name_real_size_calculate(packet_ptr -> nx_packet_prepend_ptr, (UINT)(data_ptr - packet_ptr -> nx_packet_prepend_ptr), packet_ptr -> nx_packet_length);

    /* Check the buffer space.  */
    if ((*buffer_append_ptr - name_buffer_size - 1 ) < (*buffer_prepend_ptr + sizeof(NX_DNS_MX_ENTRY)))
    {

        /* The buffer space is not enough.  */
        return(NX_DNS_NEED_MORE_RECORD_BUFFER);
    }

    /* Set the ns entry pointer.  */
    nx_dns_mx_entry_ptr = (NX_DNS_MX_ENTRY *)(*buffer_prepend_ptr);

    /* Initialize the  variables to NULL. */
    memset(nx_dns_mx_entry_ptr, 0, sizeof(NX_DNS_MX_ENTRY));

    /* Record the MX preference.  */
    nx_dns_mx_entry_ptr -> nx_dns_mx_preference = mx_preference;

    /* Update the buffer pointer.*/
    *buffer_prepend_ptr += sizeof(NX_DNS_MX_ENTRY);

    /* Update the store pointer. include the null flag '\0'.  */
    *buffer_append_ptr -= (name_buffer_size + 1);

    /* Determine if there is room for the name - one less for NULL termination.  */
    name_length = _nx_dns_name_string_unencode(packet_ptr, data_ptr, *buffer_append_ptr, name_buffer_size);

    /* Check the length.  */
    if (name_length)
    {

        /* Yes,record the name server successfully.  */

        /* Update the name server pointer.  */
        nx_dns_mx_entry_ptr -> nx_dns_mx_hostname_ptr = *buffer_append_ptr;

        /* Update the count of mx.  */
        (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 
        
        /* Check the temp buffer size.  */
        if (name_length + 2 > NX_DNS_NAME_MAX + 1)  
            return (NX_SUCCESS);

        /* Set the resource record type.  */
        temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_MX;     

        /* Set the resource record ttl.  */
        temp_rr.nx_dns_rr_ttl = rr_ttl;       

        /* Add the name string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

        /* Check the status.  */
        if(status)
            return (NX_SUCCESS);

        /* Set the MX rdata preference string.  */
        *(USHORT *)(&temp_string_buffer[0]) = mx_preference;

        /* Set the MX rdata string.  */
        memcpy((char*)&temp_string_buffer[2], (const char*)nx_dns_mx_entry_ptr -> nx_dns_mx_hostname_ptr, name_length); /* Use case of memcpy is verified. */

        /* Add the MX string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, name_length + 2, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_mx.nx_dns_rr_mx_rdata)));
                             
        /* Check the status.  */
        if(status)         
        {
            _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
            return (NX_SUCCESS);
        }

        /* Add the resource record.  */
        status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);

        /* Check the status.  */
        if(status)
        {

            /* Delete the resource record.  */
            _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
        }    
#endif /* NX_DNS_CACHE_ENABLE  */

        return(NX_SUCCESS);
    }
    else
    {

        /* Return.  */
        return(NX_DNS_MALFORMED_PACKET);
    }
}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_srv_type                             PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function process the DNS SRV record type.                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    buffer_prepend_ptr                    Pointer to the starting       */
/*                                            address of available buffer */ 
/*    buffer_append_ptr                     Pointer to the ending address */ 
/*                                            of available buffer         */ 
/*    record_count                          The count of the services     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*    _nx_dns_resource_name_real_size_calculate                           */ 
/*                                          Calculate real size of name   */  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), updated  */
/*                                            input parameter of the API  */
/*                                            to get the real size of     */
/*                                            resource name,              */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_srv_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, UCHAR **buffer_prepend_ptr, 
                                     UCHAR **buffer_append_ptr, UINT *record_count)
{
NX_DNS_SRV_ENTRY    *nx_dns_srv_entry_ptr;    
UINT                response_type;
UINT                name_buffer_size;
USHORT              srv_priority;
USHORT              srv_weight;
USHORT              srv_port_number;
UINT                name_length;
UINT                status;
#ifdef NX_DNS_CACHE_ENABLE  
ULONG               rr_ttl;
UINT                size;
#endif /* NX_DNS_CACHE_ENABLE  */
               
#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));
                             
    /* First obtain the string. */
    size = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!size)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the resource record ttl.  */
    status =  _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Verify this is what the DNS Client was requesting. */
    if (response_type != dns_ptr -> nx_dns_lookup_type)
    {
        
        /* No, this was not what the Client requested. Return error status. 
        This should not happen so return error to the host application,
        might be a problem with the query or the server. */
        return NX_DNS_MISMATCHED_RESPONSE;
    }

    /* Update the pointer to point at the resource data.  */
    data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);

    /* Plus 6 for 2 bytes priority, 2 bytes weight and 2 bytes port. */
    if ((!data_ptr) || ((data_ptr + 6) >= packet_ptr -> nx_packet_append_ptr))
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the priority data of the resource data.  */
    srv_priority = _nx_dns_network_to_short_convert(data_ptr);

    /* Skip the SRV preference, and update the pointer to point at the weight data.  */
    data_ptr += 2;

    /* Get the weight data of the resource data.  */
    srv_weight = _nx_dns_network_to_short_convert(data_ptr);

    /* Skip the SRV weight, and update the pointer to point at the port data.  */
    data_ptr += 2;

    /* Get the port data of the resource data.  */
    srv_port_number = _nx_dns_network_to_short_convert(data_ptr);

    /* Skip the SRV port, and update the pointer to point at the target data.  */
    data_ptr += 2;

    /* Get the real size of the name, and set the name buffer size.*/
    name_buffer_size = _nx_dns_resource_name_real_size_calculate(packet_ptr -> nx_packet_prepend_ptr, (UINT)(data_ptr - packet_ptr -> nx_packet_prepend_ptr), packet_ptr -> nx_packet_length);

    /* Check the buffer space.  */
    if ((*buffer_append_ptr - name_buffer_size - 1 ) < (*buffer_prepend_ptr + sizeof(NX_DNS_MX_ENTRY)))
    {

        /* The buffer space is not enough.  */
        return(NX_DNS_NEED_MORE_RECORD_BUFFER);
    }

    /* Set the SRV entry pointer.  */
    nx_dns_srv_entry_ptr = (NX_DNS_SRV_ENTRY *)(*buffer_prepend_ptr);

    /* Initialize the  variables to NULL. */
    memset(nx_dns_srv_entry_ptr, 0, sizeof(NX_DNS_SRV_ENTRY));

    /* Record the SRV options data.  */
    nx_dns_srv_entry_ptr -> nx_dns_srv_priority = srv_priority;
    nx_dns_srv_entry_ptr -> nx_dns_srv_weight = srv_weight;
    nx_dns_srv_entry_ptr -> nx_dns_srv_port_number = srv_port_number;

    /* Update the buffer pointer.*/
    *buffer_prepend_ptr += sizeof(NX_DNS_SRV_ENTRY);

    /* Update the store pointer. include the null flag '\0'.  */
    *buffer_append_ptr -= (name_buffer_size + 1);

    /* Determine if there is room for the name - one less for NULL termination.  */
    name_length = _nx_dns_name_string_unencode(packet_ptr, data_ptr, *buffer_append_ptr, name_buffer_size);
    
    /* Check the length.  */
    if (name_length)
    {

        /* Yes,record the name server successfully.  */

        /* Update the name server pointer.  */
        nx_dns_srv_entry_ptr -> nx_dns_srv_hostname_ptr = *buffer_append_ptr;

        /* Update the count of srv.  */
        (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 

        /* Check the temp buffer size.  */
        if (name_length + 6 > NX_DNS_NAME_MAX + 1)  
            return (NX_SUCCESS);

        /* Set the resource record type.  */
        temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_SRV;     

        /* Set the resource record ttl.  */
        temp_rr.nx_dns_rr_ttl = rr_ttl;         

        /* Add the name string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, size, (VOID **)(&(temp_rr.nx_dns_rr_name)));

        /* Check the status.  */
        if(status)
            return (NX_SUCCESS);   

        /* Set the SRV priority, weight and port number.  */
        *(USHORT *)(&temp_string_buffer[0]) = srv_priority;   
        *(USHORT *)(&temp_string_buffer[2]) = srv_weight;
        *(USHORT *)(&temp_string_buffer[4]) = srv_port_number;

        /* Set the SRV rdata string.  */
        memcpy((char*)&temp_string_buffer[6], (const char*)nx_dns_srv_entry_ptr -> nx_dns_srv_hostname_ptr, name_length); /* Use case of memcpy is verified. */

        /* Add the srv string.  */
        status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, name_length + 6, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata)));
                             
        /* Check the status.  */
        if(status)         
        {
            _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
            return (NX_SUCCESS);
        }

        /* Add the resource record.  */
        status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);

        /* Check the status.  */
        if(status)
        {

            /* Delete the resource record.  */
            _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
        }     
#endif /* NX_DNS_CACHE_ENABLE  */   

        return(NX_SUCCESS);
    }
    else
    {

        /* Return.  */
        return(NX_DNS_MALFORMED_PACKET);
    }
}
#endif


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_process_soa_type                             PORTABLE C     */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the SOA record type.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */
/*    packet_ptr                            Pointer to received packet    */ 
/*    data_ptr                              Pointer to resource data      */ 
/*                                            section                     */
/*    record_buffer                         Buffer space for storing      */ 
/*                                            the data structures that    */
/*                                            hold the SOA information    */
/*    buffer_size                           Size of record_buffer.        */
/*    record_count                          Record count                  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_resource_type_get             Get resource type             */  
/*    _nx_dns_resource_data_address_get     Get address of data           */ 
/*    _nx_dns_name_string_unencode          Unencode the name and get it. */
/*    _nx_dns_name_size_calculate           Calculate size of name field  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_response_process                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            and verified buffer size,   */
/*                                            updated resource get APIs to*/
/*                                            improve buffer bound check, */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_process_soa_type(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *data_ptr, 
                                     UCHAR *record_buffer, UINT buffer_size, UINT *record_count)
{
NX_DNS_SOA_ENTRY    *nx_dns_soa_entry_ptr;    
UINT                response_type;
ULONG               mname_length;  
ULONG               rname_length;
UCHAR               *buffer_start;  
UINT                status;
UINT                name_size;
#ifdef NX_DNS_CACHE_ENABLE    
ULONG               name_length;   
ULONG               rr_ttl;
#endif /* NX_DNS_CACHE_ENABLE  */
                   
#ifdef NX_DNS_CACHE_ENABLE 
    /* Initialize the value.  */
    memset(temp_string_buffer, 0, NX_DNS_NAME_MAX + 1);
    memset(&temp_rr, 0, sizeof (NX_DNS_RR));
                             
    /* First obtain the string. */
    name_length = _nx_dns_name_string_unencode(packet_ptr, data_ptr, temp_string_buffer, NX_DNS_NAME_MAX);

    /* Check the string correct.  */
    if(!name_length)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the resource record ttl.  */
    status = _nx_dns_resource_time_to_live_get(data_ptr, packet_ptr, &rr_ttl);
    if (status)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
#endif /* NX_DNS_CACHE_ENABLE  */

    /* Process the server response and get it. */
    status = _nx_dns_resource_type_get(data_ptr, packet_ptr, &response_type);
    if (status)
    {
        
        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Verify this is what the DNS Client was requesting. */
    if (response_type != dns_ptr -> nx_dns_lookup_type)
    {

        /* No, this was not what the Client requested. Return error status. 
        This should not happen so return error to the host application,
        might be a problem with the query or the server. */
        return NX_DNS_MISMATCHED_RESPONSE;
    }
    
    /* Set the SRV entry pointer.  */
    nx_dns_soa_entry_ptr = (NX_DNS_SOA_ENTRY *)(record_buffer);

    if (buffer_size <= sizeof(NX_DNS_SOA_ENTRY))
    {
        /* The buffer size is not enough.  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Update the start address of available buffer and the buffer size.  */
    buffer_start = record_buffer + sizeof(NX_DNS_SOA_ENTRY);
    buffer_size -= sizeof(NX_DNS_SOA_ENTRY);

    /* Update the pointer to point at the resource data.  */
    data_ptr = _nx_dns_resource_data_address_get(data_ptr, packet_ptr);
    if (!data_ptr)
    {

        /* Return!  */
        return(NX_DNS_MALFORMED_PACKET);
    }
    /* Get the primary name server and record it in buffer.  */
    mname_length = _nx_dns_name_string_unencode(packet_ptr, data_ptr, buffer_start, buffer_size - 1);

    /* Check the name length.  */
    if (mname_length)
    {

        /* Yes, got the primary name server successfully. set the mname ptr pointer.  */
        nx_dns_soa_entry_ptr -> nx_dns_soa_host_mname_ptr = buffer_start;
        
        /* Get name size */
        name_size = _nx_dns_name_size_calculate(data_ptr, packet_ptr);

        if (!name_size)
        {
            /* Return!  */
            return(NX_DNS_MALFORMED_PACKET);
        }

        /* Update the pointer to point at the rname data.  */ 
        data_ptr += name_size;

        /* Update the start address of available buffer and the buffer size. 1 is the Null terminate.  */
        buffer_start += mname_length + 1;
        buffer_size -= mname_length + 1;
    }
    else
    {

        /* Return !*/
        return(NX_DNS_MALFORMED_PACKET);
    }

    if (!buffer_size)
    {
        /* The buffer size is not enough.  */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the responsible mail address and record it in buffer.  */
    rname_length = _nx_dns_name_string_unencode(packet_ptr, data_ptr, buffer_start, buffer_size - 1);

    if (rname_length)
    {

        /* Yes, got the primary name server successfully. set the mname ptr pointer.  */
        nx_dns_soa_entry_ptr -> nx_dns_soa_host_rname_ptr = buffer_start;
        
        /* Get name size */
        name_size = _nx_dns_name_size_calculate(data_ptr, packet_ptr);

        /* 20 bytes for 4 bytes serial, 4 bytes refresh, 4 bytes retry, 4 bytes expire and 4 bytes minmum. */
        if ((!name_size) || ((data_ptr + name_size + 20) > packet_ptr -> nx_packet_append_ptr))
        {
            /* Return!  */
            return(NX_DNS_MALFORMED_PACKET);
        }

        /* Update the pointer to point at the rname data.  */ 
        data_ptr += name_size;

        /* Update the start address of available buffer and the buffer size.1 is the Null terminate.  */
        buffer_start +=  rname_length + 1;
        buffer_size -= rname_length + 1;
    }
    else
    {

        /* Return !*/
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Get the serial number of the resource data.  */
    nx_dns_soa_entry_ptr -> nx_dns_soa_serial = _nx_dns_network_to_long_convert(data_ptr);

    /* Skip the serial number, and update the pointer to point at the refresh data.  */
    data_ptr += 4;
    
    /* Get the refresh number of the resource data.  */
    nx_dns_soa_entry_ptr -> nx_dns_soa_refresh = _nx_dns_network_to_long_convert(data_ptr);

    /* Skip the refresh number, and update the pointer to point at the retry data.  */
    data_ptr += 4;
    
    /* Get the retry number of the resource data.  */
    nx_dns_soa_entry_ptr -> nx_dns_soa_retry = _nx_dns_network_to_long_convert(data_ptr);

    /* Skip the retry number, and update the pointer to point at the expire data.  */
    data_ptr += 4;
    
    /* Get the expire number of the resource data.  */
    nx_dns_soa_entry_ptr -> nx_dns_soa_expire = _nx_dns_network_to_long_convert(data_ptr);

    /* Skip the expire number, and update the pointer to point at the minmum data.  */
    data_ptr += 4;
        
    /* Get the minmum number of the resource data.  */
    nx_dns_soa_entry_ptr -> nx_dns_soa_minmum = _nx_dns_network_to_long_convert(data_ptr);

    /* Skip the serial number, and update the pointer to point at the refresh data.  */
    data_ptr += 4;

    /* Update the count.  */
    (*record_count) ++;

#ifdef NX_DNS_CACHE_ENABLE 

    /* Check the temp buffer size.  */
    if (mname_length + rname_length + 22 > NX_DNS_NAME_MAX + 1)  
        return (NX_SUCCESS);

    /* Set the resource record type.  */
    temp_rr.nx_dns_rr_type = NX_DNS_RR_TYPE_SOA;     

    /* Set the resource record ttl.  */
    temp_rr.nx_dns_rr_ttl = rr_ttl;         

    /* Add the name string.  */
    status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, name_length, (VOID **)(&(temp_rr.nx_dns_rr_name)));

    /* Check the status.  */
    if(status)
        return (NX_SUCCESS);   

    /* Set the SOA MNAME.  */
    memcpy((char*)&temp_string_buffer[0], (char*)nx_dns_soa_entry_ptr -> nx_dns_soa_host_mname_ptr, mname_length); /* Use case of memcpy is verified. */
    temp_string_buffer[mname_length] = '\0';
                                                 
    /* Set the SOA RNAME.  */
    memcpy((char*)&temp_string_buffer[mname_length + 1], (char*)nx_dns_soa_entry_ptr -> nx_dns_soa_host_rname_ptr, rname_length); /* Use case of memcpy is verified. */
    temp_string_buffer[mname_length + 1 + rname_length] = '\0';

    /* Set the SOA Serial, Refresh, Retry, Expire, Minmum.  */ 
    *(ULONG *)(&temp_string_buffer[mname_length + rname_length + 2]) = nx_dns_soa_entry_ptr -> nx_dns_soa_serial;
    *(ULONG *)(&temp_string_buffer[mname_length + rname_length + 6]) = nx_dns_soa_entry_ptr -> nx_dns_soa_refresh; 
    *(ULONG *)(&temp_string_buffer[mname_length + rname_length + 10]) = nx_dns_soa_entry_ptr -> nx_dns_soa_retry; 
    *(ULONG *)(&temp_string_buffer[mname_length + rname_length + 14]) = nx_dns_soa_entry_ptr -> nx_dns_soa_expire; 
    *(ULONG *)(&temp_string_buffer[mname_length + rname_length + 18]) = nx_dns_soa_entry_ptr -> nx_dns_soa_minmum;  

    /* Add the SOA string, mname length, '\0', rname length, '\0', Refresh, Retry, Expire, Minmum.  */
    status = _nx_dns_cache_add_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_string_buffer, mname_length + rname_length + 22, (VOID **)(&(temp_rr.nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata)));

    /* Check the status.  */
    if(status)         
    {
        _nx_dns_cache_delete_string(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, temp_rr.nx_dns_rr_name, 0); 
        return (NX_SUCCESS);
    }

    /* Add the resource record.  */
    status = _nx_dns_cache_add_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr,  NX_NULL);

    /* Check the status.  */
    if(status)
    {

        /* Delete the resource record.  */
        _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, &temp_rr);
    }     
#endif /* NX_DNS_CACHE_ENABLE  */

    return (NX_SUCCESS);
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_host_by_address_get                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS get host by address      */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    ip_address                            IP address to get host name   */ 
/*    host_name                             Destination for host name     */ 
/*    host_name_buffer_size                 Buffer size of host name      */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_by_address_get           Actual DNS get host by address*/ 
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
UINT  _nxe_dns_host_by_address_get(NX_DNS *dns_ptr, ULONG host_address, UCHAR *host_name, UINT host_name_buffer_size, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) || (host_name == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid non pointer input.  */
    if (!host_address || dns_ptr -> nx_dns_id != NX_DNS_ID || (host_name_buffer_size == 0))
        return(NX_DNS_PARAM_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS get host by address service.  */
    status =  _nx_dns_host_by_address_get(dns_ptr, host_address, host_name, host_name_buffer_size, wait_option);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_host_by_address_get                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the host name associated with the     */ 
/*    specified IP address.  If a host name cannot be found, this         */ 
/*    routine returns zero for the string size to signal an error.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    dns_address                           DNS server IP address         */ 
/*    host_name                             Destination for host name     */ 
/*    host_name_buffer_size                 Buffer size for host name     */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_host_by_address_get_internal  Actual host address           */
/*                                               get service              */
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
UINT  _nx_dns_host_by_address_get(NX_DNS *dns_ptr, ULONG dns_address, UCHAR *host_name, UINT host_name_buffer_size, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS host_address;


    /* Check for null address input. */
    if (dns_address == 0)
    {
        return NX_DNS_BAD_ADDRESS_ERROR;
    }

    /* Construct an IP address structure, and fill in IPv4 address information. */
    host_address.nxd_ip_version = NX_IP_VERSION_V4;
    host_address.nxd_ip_address.v4 = dns_address;

    /* Invoke the real function. */
    return(_nx_dns_host_by_address_get_internal(dns_ptr, &host_address, host_name, host_name_buffer_size, wait_option));
#else
    NX_PARAMETER_NOT_USED(dns_ptr);
    NX_PARAMETER_NOT_USED(dns_address);
    NX_PARAMETER_NOT_USED(host_name);
    NX_PARAMETER_NOT_USED(host_name_buffer_size);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_dns_host_by_address_get                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the NetX duo compatible DNS get  */
/*    host by address function call.                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    ip_address                            IP address to get host name   */ 
/*    host_name_ptr                         Destination for host name     */ 
/*    host_name_buffer_size                 Size of host name buffer      */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_dns_host_by_address_get          Actual DNS get host by address*/ 
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
UINT  _nxde_dns_host_by_address_get(NX_DNS *dns_ptr, NXD_ADDRESS *host_address, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((dns_ptr == NX_NULL) ||  (host_name_ptr == NX_NULL) || (host_address == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid host name size.  */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID || (host_name_buffer_size == 0))
        return(NX_DNS_PARAM_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DNS service get host IP address by host name.  */
    status =  _nxd_dns_host_by_address_get(dns_ptr, host_address, host_name_ptr, host_name_buffer_size, wait_option);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_host_by_address_get                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the host name associated with the     */ 
/*    specified IP address.  If a host name cannot be found, this         */ 
/*    routine returns zero for the string size to signal an error.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_address_ptr                      NXD_ADDRESS instance with the */
/*                                            IP address to search for    */
/*                                            host name                   */ 
/*    host_name_ptr                         Destination for host name     */ 
/*    host_name_buffer_size                 Buffer size for host name     */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_dns_build_an_ipv6_question_string Create the DNS query         */
/*    _nx_dns_send_query_by_address          Create and transmit the DNS  */
/*                                             query packet               */
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
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
UINT  _nxd_dns_host_by_address_get(NX_DNS *dns_ptr, NXD_ADDRESS *host_address_ptr, UCHAR *host_name_ptr, 
                                   UINT host_name_buffer_size, ULONG wait_option)
{
              

    /* Invoke the real function. */
    return(_nx_dns_host_by_address_get_internal(dns_ptr, host_address_ptr, host_name_ptr, host_name_buffer_size, wait_option));
}   

            
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_host_by_address_get_internal                PORTABLE C      */
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function uses DNS to get the host name associated with the     */ 
/*    specified IP address.  If a host name cannot be found, this         */ 
/*    routine returns zero for the string size to signal an error.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    host_address_ptr                      Pointer to host address       */
/*    host_name_ptr                         Destination for host name     */ 
/*    host_name_buffer_size                 Buffer size for host name     */ 
/*    wait_option                           Timeout value                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_dns_build_an_ipv6_question_string Create the DNS query         */
/*    _nx_dns_send_query_by_address          Create and transmit the DNS  */
/*                                             query packet               */
/*    tx_mutex_get                          Get DNS protection mutex      */ 
/*    tx_mutex_put                          Release DNS protection mutex  */ 
/*    nx_udp_socket_bind                    Bind DNS UDP socket to port   */ 
/*    nx_udp_socket_unbind                  Unbind DNS UDP socket         */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the timeout of first query, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            randomized the source port, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_host_by_address_get_internal(NX_DNS *dns_ptr, NXD_ADDRESS *host_address_ptr, UCHAR *host_name_ptr, 
                                                  UINT host_name_buffer_size, ULONG wait_option)
{

UINT        retries;
UINT        status;
UCHAR       ip_question[NX_DNS_IP_LOOKUP_SIZE + 1];
UINT        i;
#ifndef NX_DISABLE_IPV4
UCHAR       dot = '.';
UINT        value;
UINT        length, index;
#endif /* NX_DISABLE_IPV4 */


    /* Check for an invalid address type. */
    if ((host_address_ptr -> nxd_ip_version != NX_IP_VERSION_V4) && 
        (host_address_ptr -> nxd_ip_version != NX_IP_VERSION_V6))
    {

        return NX_DNS_INVALID_ADDRESS_TYPE;
    }

    if (host_address_ptr -> nxd_ip_version == NX_IP_VERSION_V6)     
#ifdef FEATURE_NX_IPV6  
    {
    
        /* Check for Null address input. */
        if (CHECK_UNSPECIFIED_ADDRESS(&(host_address_ptr ->nxd_ip_address.v6[0])))
        {
            return NX_DNS_BAD_ADDRESS_ERROR;
        }
    }
#else
    {
        /* IPv6 not supported. */
        return NX_DNS_INVALID_ADDRESS_TYPE;
    }
#endif
    else if (host_address_ptr -> nxd_ip_version == NX_IP_VERSION_V4)
    {
    
#ifndef NX_DISABLE_IPV4
        /* Check for Null address input. */
        if (host_address_ptr -> nxd_ip_address.v4== 0)
#endif /* NX_DISABLE_IPV4 */
        {

            return NX_DNS_BAD_ADDRESS_ERROR;
        }
    }
    else
    {
        return NX_DNS_INVALID_ADDRESS_TYPE;
    }

    /* Check for an invalid buffer size. */
    if (host_name_buffer_size == 0)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Clear the host name buffer.  */
    memset(host_name_ptr, 0, sizeof(host_name_buffer_size));

    /* Get the protection mutex to make sure no other thread interferes.  */
    status =  tx_mutex_get(&(dns_ptr -> nx_dns_mutex), wait_option);

    /* Check status.  */
    if (status != TX_SUCCESS)
    {

        /* The mutex was not granted in the time specified.  Return an error.  */
        return(NX_DNS_TIMEOUT);
    }

    /* Determine if there is at least one DNS server. Is the first slot empty?  */
    if (dns_ptr -> nx_dns_server_ip_array[0].nxd_ip_version == 0)
    {

        /* No, this means the list is empty. Release the DNS Client lock. */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

        /* At least one DNS server is required - return an error.  */
        return(NX_DNS_NO_SERVER);
    }

    /* Is this an IPv6 address we are looking up? */
    if (host_address_ptr -> nxd_ip_version == NX_IP_VERSION_V6)
    {

#ifndef FEATURE_NX_IPV6
        /* Release the mutex and return. */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

        return NX_DNS_IPV6_NOT_SUPPORTED;
#else
        /* Yes; build the PTR query string containing an IPv6 address. */
        _nxd_dns_build_an_ipv6_question_string(host_address_ptr, &ip_question[0], NX_DNS_IP_LOOKUP_SIZE);
#endif
    }
    else
    {

#ifndef NX_DISABLE_IPV4
        memset(ip_question, 0, NX_DNS_IP_LOOKUP_SIZE);
        value = host_address_ptr -> nxd_ip_address.v4 & 0xff;
        length = _nx_dns_number_to_ascii_convert(value, (CHAR *)&ip_question[0]);
        ip_question[length++] = dot;
        index = length;

        value = (host_address_ptr -> nxd_ip_address.v4 >> 8) & 0xff;
        length += _nx_dns_number_to_ascii_convert(value, (CHAR *)&ip_question[index]);
        ip_question[length++] = dot;
        index = length;

        value = (host_address_ptr -> nxd_ip_address.v4 >> 16) & 0xff;
        length += _nx_dns_number_to_ascii_convert(value, (CHAR *)&ip_question[index]) ;
        ip_question[length++] = dot;
        index = length;

        value = (host_address_ptr -> nxd_ip_address.v4 >> 24) & 0xff;
        length += _nx_dns_number_to_ascii_convert(value, (CHAR *)&ip_question[index]);
        ip_question[length++] = dot;
        index = length;

        memcpy(&ip_question[length], &lookup_end[0], 12); /* Use case of memcpy is verified. */
#else
        /* Release the mutex and return. */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

        return NX_DNS_BAD_ADDRESS_ERROR;
#endif /* NX_DISABLE_IPV4 */
    }
    
#ifdef NX_DNS_CACHE_ENABLE
                          
    /* Find the answer in local cache.  */
    if(_nx_dns_cache_find_answer(dns_ptr, dns_ptr -> nx_dns_cache, ip_question, NX_DNS_RR_TYPE_PTR, host_name_ptr, host_name_buffer_size, NX_NULL) == NX_DNS_SUCCESS)
    {           

        /* Release the mutex. */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);

        return (NX_DNS_SUCCESS);
    }
#endif /*NX_DNS_CACHE_ENABLE.  */  

    /* Bind the UDP socket to random port for each query.  */
    status =  nx_udp_socket_bind(&(dns_ptr -> nx_dns_socket), NX_ANY_PORT, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status != TX_SUCCESS)
    {

        /* Release the DNS Client lock.  */
        tx_mutex_put(&dns_ptr -> nx_dns_mutex);
        return(status);
    }

    /* Limit the timeout to NX_DNS_MAX_RETRANS_TIMEOUT.  */
    if (wait_option > NX_DNS_MAX_RETRANS_TIMEOUT)
    {
        wait_option = NX_DNS_MAX_RETRANS_TIMEOUT;
    }

    /* Keep sending queries to all DNS Servers till the retry count expires.  */
    for (retries = 0; retries < dns_ptr -> nx_dns_retries; retries++)
    {
                                                                                      
        /* The client should try other servers and server addresses before repeating a query to a specific address of a server.  
           RFC1035, Section4.2.1 UDP usage, Page32.  */
        /*  Attempt host name resolution from each DNS server till one if found. */      
        for (i = 0; (i < NX_DNS_MAX_SERVERS) && (dns_ptr -> nx_dns_server_ip_array[i].nxd_ip_version != 0); i ++)
        {

            /* Send the PTR/reverse lookup query. */
            status = _nx_dns_send_query_by_address(dns_ptr, &dns_ptr -> nx_dns_server_ip_array[i], &ip_question[0], 
                                                   host_name_ptr, host_name_buffer_size, wait_option);
                      
            /* Check the status.  */
            if (status == NX_SUCCESS)
            {

                /* Unbind the socket.  */
                nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));

                /* Release the mutex */
                tx_mutex_put(&dns_ptr -> nx_dns_mutex);

                /* Yes, have done, just return success.  */
                return NX_SUCCESS;
            }
        }

        /* Timed out for querying all DNS servers in this cycle, double the timeout, limited to NX_DNS_MAX_RETRANS_TIMEOUT.  */
        if (wait_option <= (NX_DNS_MAX_RETRANS_TIMEOUT >> 1))
            wait_option = (wait_option << 1);
        else
            wait_option = NX_DNS_MAX_RETRANS_TIMEOUT;
    }

    /* Unbind the socket.  */
    nx_udp_socket_unbind(&(dns_ptr -> nx_dns_socket));

    /* Release protection.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));
                                                       
    /* Failed on all servers, return DNS lookup failed status.  */
    return(NX_DNS_QUERY_FAILED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_new_packet_create                           PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine fills in the header and question in a DNS queyr packet,*/ 
/*    and updates the size and the question count fields in the header.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                               Pointer to DNS instance       */ 
/*    packet_ptr                            Packet allocated for message  */
/*    name                                  Question e.g. host name       */ 
/*    type                                  DNS message type e.g. A, AAAA */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DNS_PACKET_CREATE_ERROR            Error creating header or query*/ 
/*    NX_SUCCESS                            Successful compltion          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_header_create                 Create a DNS header           */ 
/*    _nx_dns_question_add                  Add the DNS question to packet*/ 
/*    nx_packet_allocate                    Allocate a new DNS packet     */ 
/*    nx_packet_release                     Release DNS packet            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_host_by_name_get              Get IP address with name      */ 
/*    _nx_host_by_address_get               Get name from IP address      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved id generation,     */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_new_packet_create(NX_DNS *dns_ptr, NX_PACKET *packet_ptr, UCHAR *name, USHORT type)
{
USHORT id;
UINT size;

    /* Generate a random ID based on name. */
    id = (USHORT)NX_RAND();

    /* Add the DNS header.  */
    size =  _nx_dns_header_create(packet_ptr -> nx_packet_append_ptr, id, NX_DNS_QUERY_FLAGS);

    /* Determine if there was an error.  */
    if (size == 0)
    {

        /* Return error status. */
        return NX_DNS_PACKET_CREATE_ERROR; 
    }

    /* Save the DNS  transmit id and lookup type.  */
    dns_ptr -> nx_dns_transmit_id = id;
    dns_ptr -> nx_dns_lookup_type = type;

    /* Setup the packet pointers.  */
    packet_ptr -> nx_packet_append_ptr +=  size;
    packet_ptr -> nx_packet_length +=      size;

    /* Add the DNS question.  */
    size =  _nx_dns_question_add(packet_ptr, name, type);

    /* Determine if there was an error.  */
    if (size == 0)
    {

        /* Return the error status.  */
        return NX_DNS_PACKET_CREATE_ERROR; 
    }

    /* Successful completion.  */
    return NX_SUCCESS; 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_header_create                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a standard DNS header and returns the size of */ 
/*    header.                                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    buffer_ptr                            Pointer to header area        */ 
/*    id                                    Identification                */ 
/*    flags                                 Flags                         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    size                                  Size of DNS header            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_short_to_network_convert      Convert from short to network */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_new_packet_create             Create new DNS packet         */ 
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
static UINT  _nx_dns_header_create(UCHAR *buffer_ptr, USHORT id, USHORT flags)
{

    /* Transaction ID.  */
    _nx_dns_short_to_network_convert(buffer_ptr + NX_DNS_ID_OFFSET, id);

    /* Flags and Command.  */   
    _nx_dns_short_to_network_convert(buffer_ptr + NX_DNS_FLAGS_OFFSET, flags);
  
    /* Initialize counts to 0.  */  
    memset(buffer_ptr + NX_DNS_QDCOUNT_OFFSET, 0, 8);
    
    /* Return the size of the DNS header.  */
    return(NX_DNS_QDSECT_OFFSET);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_question_add                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds a question to the packet buffer at the end of    */ 
/*    the current data and updates the size and the question count        */ 
/*    (assuming that there is a dns header at the start if the packet     */ 
/*    buffer). The question class is assumed to be INET.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to header area        */ 
/*    name                                  Host name or IP address that  */
/*                                           is target of the DNS query   */ 
/*    type                                  DNS record type (e.g. A, AAAA)*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    size                                  Total size of packet          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_string_encode            Encode the supplied string    */ 
/*    _nx_dns_network_to_short_convert      Convert from network to short */ 
/*    _nx_dns_short_to_network_convert      Convert from short to network */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_new_packet_create             Create new DNS packet         */ 
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
static UINT  _nx_dns_question_add(NX_PACKET *packet_ptr, UCHAR *name, USHORT type)
{

UINT    name_size;
UINT    size;
USHORT  value;


    /* Check for name.  */
    if (_nx_utility_string_length_check((CHAR *)name, &name_size, NX_DNS_NAME_MAX))
    {

        /* Name error, release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return a size of 0 to indicate an error.  */
        return(0);
    }

    /* The question will take the size of the string plus 6 bytes, is there space?  */
    if ((name_size + 6) > (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr))
    {

        /* Size error, release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return a size of 0 to indicate an error.  */
        return(0);
    }

    /* Encode and add the name.  */
    size =  _nx_dns_name_string_encode(packet_ptr -> nx_packet_append_ptr, name);

    /* Add the type and class.  */
    _nx_dns_short_to_network_convert(packet_ptr -> nx_packet_append_ptr + size, type);
    size +=  2;
    _nx_dns_short_to_network_convert(packet_ptr -> nx_packet_append_ptr + size, NX_DNS_RR_CLASS_IN);
    size +=  2;

    /* Update the packet size and end.  */
    packet_ptr -> nx_packet_length +=      size;
    packet_ptr -> nx_packet_append_ptr +=  size;

    /* Get the question count. */
    value = _nx_dns_network_to_short_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_QDCOUNT_OFFSET);

    /* Increment question count by one. */
    value++;
    _nx_dns_short_to_network_convert(packet_ptr -> nx_packet_prepend_ptr + NX_DNS_QDCOUNT_OFFSET, value);
                                     
    /* Return the size.  */
    return(packet_ptr -> nx_packet_length);
}


#ifdef FEATURE_NX_IPV6
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dns_build_an_ipv6_question_string              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates the IP address in text form to go into the DNS*/ 
/*    lookup query ("Name"), including the ip6.arpa tag on the end.       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_address                            NXD_ADDRESS instance with the */
/*                                            IP address to search for    */
/*                                            host name                   */ 
/*    buffer                                Pointer to host name to lookup*/ 
/*    len                                   Buffer size for host name     */ 
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
/*    _nxd_dns_host_by_address_get         Lookup host by address service */ 
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
static VOID _nxd_dns_build_an_ipv6_question_string(NXD_ADDRESS *ip_address, UCHAR *buffer, UINT len)
{
INT i,  j;
ULONG temp;

    memset(buffer, 0, len);
    for (j = 3; j >= 0; j--)
    {
        i = 0;
        while (i <= 7)
        {

            temp = ip_address -> nxd_ip_address.v6[j];
            temp = temp >> (4 * i++);
            temp = (USHORT)temp & 0xf;
            if(temp >= 10)
                *buffer = (UCHAR)('a' + ((UCHAR)temp - 10));
            else
                *buffer = (UCHAR)('0' + (UCHAR)temp);
            buffer ++;
            *buffer = '.';
            buffer++;
        }
    }

    memcpy(buffer, "ip6.arpa", sizeof("ip6.arpa")); /* Use case of memcpy is verified. */

    return;
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_name_string_encode                          PORTABLE C      */ 
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
/*    _nx_dns_question_add                  Add question to DNS packet    */ 
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
static UINT  _nx_dns_name_string_encode(UCHAR *ptr, UCHAR *name)
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
/*    _nx_dns_name_string_unencode                        PORTABLE C      */ 
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
/*    buffer                                Pointer to decoded data       */ 
/*    buffer_size                           Size of data buffer to decode */ 
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
/*    _nx_dns_send_query_by_address         Send reverse lookup query     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            compression pointer check,  */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), prevented*/
/*                                            infinite loop in name       */
/*                                            compression, resulting in   */
/*                                            version 6.1.3               */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_name_string_unencode(NX_PACKET *packet_ptr, UCHAR *data, UCHAR *buffer, UINT buffer_size)
{

UCHAR   *character;
UCHAR   *message_start;
UINT    label_size;
UINT    offset;
UINT    length;
UINT    pointer_count = 0;

    /* Initialize the value.  */
    character = data;
    message_start = packet_ptr -> nx_packet_prepend_ptr;
    length = 0;

    /* As long as there is space in the buffer and we haven't 
       found a zero terminating label */
    while (1)
    {

        if (character >= packet_ptr -> nx_packet_append_ptr)
        {
            return(0);
        }

        if (*character == '\0')
        {
            break;
        }

        /* Check the buffer size.  */
        if (buffer_size > length)
        {

            /* Get the label size.  */
            label_size = *character++;

            /* Is this a compression pointer or a count.  */
            if (label_size <= NX_DNS_LABEL_MAX)
            {

                /* Simple count, check for space and copy the label.  */
                while ((buffer_size > length) && (label_size > 0))
                {

                    if ((character >= packet_ptr -> nx_packet_append_ptr) || (*character == '\0'))
                    {
                        return(0);
                    }

                    *buffer++ =  *character++;
                    length++;
                    label_size--;
                }

                /* Now add the '.' */
                *buffer++ =  '.';
                length++;
            }
            else if ((label_size & NX_DNS_COMPRESS_MASK) == NX_DNS_COMPRESS_VALUE)
            {

                /* Message compression.  */
                if (character >= packet_ptr -> nx_packet_append_ptr)
                {
                    return(0);
                }

                /* Get the offset.  */
                offset = ((label_size & NX_DNS_LABEL_MAX) << 8) + *character;

                /* Check the offset.  */
                if (offset >= packet_ptr -> nx_packet_length)
                {

                    /* This is malformed packet.  */
                    return(0);
                }
                else
                {

                    /* This is a pointer, just adjust the source.  */
                    if (character ==  message_start + offset)
                    {
                        /* If compression pointer equals the same offset currently being parsed,
                           it could lead to an infinite loop. */
                        return(0);
                    }
                    else
                    {
                        /* Prevent infinite loop with compression pointers. */
                        pointer_count++;
                        if (pointer_count > NX_DNS_MAX_COMPRESSION_POINTERS)
                        {

                            /* This is malformed packet.  */
                            return(0);
                        }
                        /* update valid pointer */
                        character =  message_start + offset;
                    }
                }
            }
            else
            {

                /* Not defined, just fail */
                return(0);
            }
        }
        else
        {   

            /* Size error , just fail */
            return(0);
        }
    }

    /* Check for invalid length.  */
    if (length == 0)
        return(length);

    /* Done copying the data, set the last . to a trailing null */
    if (*(buffer - 1) == '.')
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
/*    _nx_dns_name_size_calculate                         PORTABLE C      */ 
/*                                                           6.1          */
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
/*    _nx_dns_host_by_name_get              Get IP address with name      */ 
/*    _nx_host_by_address_get               Get name from IP address      */ 
/*    _nx_dns_resource_type_get             Get resource type             */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_name_size_calculate(UCHAR *name, NX_PACKET *packet_ptr)
{

UINT size =  0;


    /* As long as we haven't found a zero length terminating label */
    while (*name != '\0')
    {

        UINT  labelSize =  *name++;

        /* Is this a compression pointer or a count.  */
        if (labelSize <= NX_DNS_LABEL_MAX)
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
        else if ((labelSize & NX_DNS_COMPRESS_MASK) == NX_DNS_COMPRESS_VALUE)
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


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_resource_name_real_size_calculate           PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calculates the real size of the resouce name.         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data                                  Pointer to buffer to decode   */ 
/*    start                                 Location of start of data     */ 
/*    data_length                           Data buffer length            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Real size of a string                                               */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_send_query_by_name             Send reverse lookup query    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            compression pointer check,  */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            pointer check, prevented    */
/*                                            infinite loop in name       */
/*                                            compression, resulting in   */
/*                                            version 6.1.3               */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT    _nx_dns_resource_name_real_size_calculate(UCHAR *data, UINT start, UINT data_length)
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

        if (character >= (data + data_length))
        {
            return(0);
        }

        if (*character == '\0')
        {
            break;
        }

        labelSize =  *character++;

        /* Is this a compression pointer or a count.  */
        if (labelSize <= NX_DNS_LABEL_MAX)
        {

            /* Simple count, check for space and copy the label.  */
            while (labelSize > 0)
            {

                if (character >= (data + data_length))
                {
                    return(0);
                }

                character++;
                length++;
                labelSize--;
            }
      
            /* Now add the '.' space */
            length++;
        }
        else if ((labelSize & NX_DNS_COMPRESS_MASK) == NX_DNS_COMPRESS_VALUE)
        {

            if (character >= (data + data_length))
            {
                return(0);
            }

            /* Get the offset.  */
            offset = ((labelSize & NX_DNS_LABEL_MAX) << 8) + *character;

            /* Check the offset.  */
            if (offset >= data_length)
            {

                return(0);
            }

            /* This is a pointer, just adjust the source.  */
            if (character ==  data + offset)
            {
                /* If compression pointer equals the same offset currently being parsed,
                   it could lead to an infinite loop. */
                return(0);
            }
            else
            {

                /* Prevent infinite loop with compression pointers. */
                pointer_count++;

                if (pointer_count > NX_DNS_MAX_COMPRESSION_POINTERS)
                {

                    /* This is malformed packet.  */
                    return(0);
                }

                /* update valid pointer */
                character =  data + offset;
            }
        }
        else
        {

            /* Not defined, just fail */
            return(0);
        }
    }

    /* Reduce the last '.' string, update the length.  */
    if (length)
    {
        length --;
    }

    /* Return name size.  */
    return(length);
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_resource_type_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the resource type. It is a wrapper for the  */
/*    actual internal function that retrieves the name to maintain        */ 
/*    compatibility with the previous version of DNS Client, e.g. NetX DNS*/
/*    Client.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    resource                              Pointer to the resource       */ 
/*    packet_ptr                            Pointer to received packet    */
/*    resource_type                         Pointer to resource type      */
/*                                                                        */  
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Success or failure            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_size_calculate           Calculate name's size         */ 
/*    _nx_dns_network_to_short_convert      Actual get resource type      */
/*                                                service                 */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_host_by_name_get              Get IP address with name      */ 
/*    _nx_host_by_address_get               Get name from IP address      */ 
/*    _nx_dns_name_size_calculate           Calculate name's size         */ 
/*    _nx_dns_network_to_short_convert      Convert network to short      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_resource_type_get(UCHAR *resource, NX_PACKET *packet_ptr, UINT *resource_type)
{
UINT    name_size;

    name_size = _nx_dns_name_size_calculate(resource, packet_ptr);

    if (!name_size)
    {
        /* If name size is not valid, return a failure. */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* Resource type is 2 bytes long starting from resource + name_size, check if there is OOB. */
    if (resource + name_size + 2 >= packet_ptr -> nx_packet_append_ptr)
    {
        /* If there is OOB read, return a failure. */
        return(NX_OVERFLOW);
    }
    else
    {
        *resource_type = _nx_dns_network_to_short_convert(resource + name_size);
        return(NX_SUCCESS);
    }
}


#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_resource_time_to_live_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the resource time to live.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    resource                              Pointer to the resource       */ 
/*    packet_ptr                            Pointer to received packet    */
/*    rr_ttl                                Pointer to time to live       */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Status                                Success or failure            */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_resource_time_to_live_get(UCHAR *resource, NX_PACKET *packet_ptr, ULONG *rr_ttl)
{
UINT    name_size;

    name_size = _nx_dns_name_size_calculate(resource, packet_ptr);

    if (!name_size)
    {

        /* If name size is not valid, return a failure. */
        return(NX_DNS_MALFORMED_PACKET);
    }

    /* rr_ttl is 4 bytes long starting from resource + name_size + 4, check if there is OOB. */
    if (resource + name_size + 4 + 4 >= packet_ptr -> nx_packet_append_ptr)
    {

        /* if there is OOB read, return an invalid value. */
        return(NX_OVERFLOW);
    }
    else
    {
        *rr_ttl = _nx_dns_network_to_long_convert(resource + name_size + 4);
        return(NX_SUCCESS);
    }
}
#endif /* NX_DNS_CACHE_ENABLE  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_resource_data_length_get                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the resource data length.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    resource                              Pointer to the resource       */
/*    packet_ptr                            Pointer to received packet    */
/*    length                                Pointer to data length        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Success or failure            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_size_calculate           Calculate name's size         */ 
/*    _nx_dns_network_to_short_convert      Convert from network to short */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_host_by_name_get              Get IP address with name      */ 
/*    _nx_host_by_address_get               Get name from IP address      */ 
/*    _nx_dns_resource_size_get             Get size of resource          */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_resource_data_length_get(UCHAR *resource, NX_PACKET *packet_ptr, UINT *length)
{
UINT    name_size;

    name_size = _nx_dns_name_size_calculate(resource, packet_ptr);

    if (!name_size)
    {
        /* If name size is not valid, return a failure. */
        return(NX_DNS_MALFORMED_PACKET);
    }


    /* Resource length is 2 bytes long starting from resource + name_size + 8, check if there is OOB. */
    if (resource + name_size + 8 + 2 >= packet_ptr -> nx_packet_append_ptr)
    {
        /* if there is OOB read, return a failure. */
        return(NX_OVERFLOW);
    }
    else
    {
        *length = (_nx_dns_network_to_short_convert(resource + name_size + 8));
        return(NX_SUCCESS);
    }
    
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_resource_data_address_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the resource data address.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    resource                              Pointer to the resource       */
/*    packet_ptr                            Pointer to received packet    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    pointer to address                                                  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_size_calculate           Calculate name's size         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dns_host_by_name_get              Get IP address with name      */ 
/*    _nx_host_by_address_get               Get name from IP address      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UCHAR  *_nx_dns_resource_data_address_get(UCHAR *resource, NX_PACKET *packet_ptr)
{
UINT    name_size;

    name_size = _nx_dns_name_size_calculate(resource, packet_ptr);

    if (!name_size)
    {
        /* If name size is not valid, return an invalid NULL address. */
        return(0);
    }

    if (resource + name_size + 10 >= packet_ptr -> nx_packet_append_ptr)
    {
        /* if there is OOB read, return an invalid NULL address. */
        return(0);
    }
    else
    {
        return(resource + name_size + 10);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_resource_size_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the resource data size.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    resource                              Pointer to the resource       */
/*    packet_ptr                            Pointer to received packet    */
/*    resource size                         Pointer to resource size      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Success or failure            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_name_size_calculate           Calculate name's size         */ 
/*    _nx_dns_resource_data_length_get      Get resource data length      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_host_by_address_get               Get name from IP address      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer bound check,         */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dns_resource_size_get(UCHAR *resource, NX_PACKET *packet_ptr, UINT *resource_size)
{
UINT status;
UINT data_length;
UINT name_size;

    status = _nx_dns_resource_data_length_get(resource, packet_ptr, &data_length);
    if (status)
    {

        /* Return directly if failed to get resource data length. */
        return(status);
    }

    /* Resource size is 
    name size + data size + 2 bytes for type, 2 for class, 4 for time to live and 2 for data length
    i.e. name size + data size + 10 bytes overhead
    */
    name_size = _nx_dns_name_size_calculate(resource, packet_ptr);

    if (!name_size)
    {

        /* If name size is not valid, return a failure. */
        return(NX_DNS_MALFORMED_PACKET);
    }

    *resource_size = name_size + data_length + 10;
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_short_to_network_convert                    PORTABLE C      */ 
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
static void  _nx_dns_short_to_network_convert(UCHAR *ptr, USHORT value)
{
    
    *ptr++ =  (UCHAR)(value >> 8);
    *ptr   =  (UCHAR)value;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_network_to_short_convert                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts an unsigned short in network format, which   */ 
/*    big endian, to an unsigned short.                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to the source         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return value                                                        */ 
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
static USHORT _nx_dns_network_to_short_convert(UCHAR *ptr)
{

USHORT value =  *ptr++;
  
    value =  (USHORT)((value << 8) | *ptr);
  
    return(value);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_network_to_long_convert                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts an unsigned long in network format, which    */ 
/*    big endian, to an unsigned long.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to the source         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return value                                                        */ 
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
static ULONG  _nx_dns_network_to_long_convert(UCHAR *ptr)
{

ULONG value =  *ptr++;
   
    value =  (value << 8) | *ptr++;
    value =  (value << 8) | *ptr++;
    value =  (value << 8) | *ptr;
  
    return(value);  
}


#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_dns_number_to_ascii_convert                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts single digits into an ASCII character in an  */
/*    NULL terminated string.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Unsigned integer number       */
/*    buffstring                            Destination string            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Size                                  Number of bytes in string     */
/*                                           (0 implies an error)         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_dns_host_by_address_get           Send PTR query to DNS server  */
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
static UINT  _nx_dns_number_to_ascii_convert(UINT number, CHAR *buffstring)
{
UINT value;
UINT digit;
UINT index = 0;

    value = number;

    /* Is the value in the range of 100 and 255? */
    if(value >= 200)
    {
        buffstring[index++] = '2';
        value -= 200;
    }
    else if(value >= 100)
    {
        buffstring[index++] = '1';
        value -= 100;
    }

    digit = value % 10;
    value = value / 10;

    if(value == 0)
    {
        /* The 10s is zero.  However if we already recorded the hundreds,
           we need to insert 0 here. */
        if(index != 0)
            buffstring[index++] = '0';
    }
    else
        buffstring[index++] = (CHAR)('0' + value);
    
    /* Last print the digit. */
    buffstring[index++] = (CHAR)('0' + digit);

    return index;
}
#endif /* NX_DISABLE_IPV4 */
        

#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_cache_initialize                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS cache initialize         */  
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                           Pointer to DNS instance           */ 
/*    cache_ptr                         Pointer to cache memory           */ 
/*    cache_size                       The size of cache                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_cache_initialize         Actual cache initialize function   */
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
UINT _nxe_dns_cache_initialize(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!dns_ptr)
    {    
        return(NX_PTR_ERROR);
    }    

    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }    
                  
    /* Check for invalid input pointers.  */
    if (!cache_ptr)
    {    
        return(NX_DNS_CACHE_ERROR);
    } 
    
    /* Make sure peer cache is 4-byte aligned. */
    if ((((ALIGN_TYPE)cache_ptr & 0x3) != 0) ||
        ((cache_size & 0x3) != 0))
    {
        return(NX_DNS_ERROR);
    }             
    
    /* Call actual DNS cache initialize service.  */
    status =  _nx_dns_cache_initialize(dns_ptr, cache_ptr, cache_size);

    /* Return status.  */
    return(status);
} 


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_initialize                           PORTABLE C       */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the DNS cache.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                           Pointer to DNS instance           */ 
/*    cache_ptr                         Pointer to cache memory           */ 
/*    cache_size                       The size of cache                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                                Completion status             */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                      Get the DNS mutex                 */ 
/*    tx_mutex_put                      Put the DNS mutex                 */ 
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
UINT _nx_dns_cache_initialize(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size)
{

ALIGN_TYPE *head;
ALIGN_TYPE *tail;


    /* Get the mutex.  */
    tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);

    /* Zero out the cache. */
    memset(cache_ptr, 0, cache_size);

    /* Set the head. */
    head = (ALIGN_TYPE*)cache_ptr;
    *head = (ALIGN_TYPE)((ALIGN_TYPE*)cache_ptr + 1);

    /* Set the tail. */
    tail = (ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1;
    *tail = (ALIGN_TYPE)tail;

    /* Record the info.  */
    dns_ptr -> nx_dns_cache = (UCHAR*)cache_ptr;
    dns_ptr -> nx_dns_cache_size = cache_size; 

    /* Clear the count.  */
    dns_ptr -> nx_dns_rr_count = 0;    
    dns_ptr -> nx_dns_string_count = 0;
    dns_ptr -> nx_dns_string_bytes = 0;

    /* Put the DNS mutex.  */
    tx_mutex_put(&dns_ptr -> nx_dns_mutex);

    return(NX_SUCCESS);
}
#endif /* NX_DNS_CACHE_ENABLE  */
                  

#ifdef NX_DNS_CACHE_ENABLE          
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_cache_notify_set                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS cache full notify        */ 
/*    function call.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                           Pointer to DNS instance           */ 
/*    cache_full_notify                 Cache full notify function        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_cache_notify_set         Actual cache notify set function   */ 
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
UINT _nxe_dns_cache_notify_set(NX_DNS *dns_ptr, VOID (*cache_full_notify_cb)(NX_DNS *dns_ptr))
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!dns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Call actual DNS cache notify set function.  */
    status =  _nx_dns_cache_notify_set(dns_ptr, cache_full_notify_cb);

    /* Return status.  */
    return(status);
}

                               
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_notify_set                            PORTABLE C      */ 
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
/*    dns_ptr                           Pointer to DNS instance           */ 
/*    cache_full_notify                 Cache full notify function        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                      Get the DNS mutex                 */ 
/*    tx_mutex_put                      Put the DNS mutex                 */ 
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
UINT _nx_dns_cache_notify_set(NX_DNS *dns_ptr, VOID (*cache_full_notify_cb)(NX_DNS *dns_ptr))
{


    /* Get the DNS mutex.  */
    tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);
    
    /* Set the cache notify.  */
    dns_ptr -> nx_dns_cache_full_notify = cache_full_notify_cb;
    
    /* Release the DNS mutex.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

    return(NX_DNS_SUCCESS);
}   
#endif /* NX_DNS_CACHE_ENABLE  */


#ifdef NX_DNS_CACHE_ENABLE   
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dns_cache_notify_clear                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DNS cache full notify clear  */ 
/*    function call.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                           Pointer to DNS instance           */ 
/*    cache_full_notify                 Cache full notify function        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dns_cache_notify_clear       Actual cache notify clear function */ 
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
UINT _nxe_dns_cache_notify_clear(NX_DNS *dns_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!dns_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (dns_ptr -> nx_dns_id != NX_DNS_ID)
    {
        return(NX_DNS_PARAM_ERROR);
    }

    /* Call actual DNS cache notify clear function.  */
    status =  _nx_dns_cache_notify_clear(dns_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_notify_clear                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clear the cache full notify function.                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                           Pointer to DNS instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                      Get the DNS mutex                 */ 
/*    tx_mutex_put                      Put the DNS mutex                 */ 
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
UINT _nx_dns_cache_notify_clear(NX_DNS *dns_ptr)
{


    /* Get the DNS mutex.  */
    tx_mutex_get(&(dns_ptr -> nx_dns_mutex), TX_WAIT_FOREVER);
    
    /* Clear the cache notify.  */
    dns_ptr -> nx_dns_cache_full_notify = NX_NULL;
    
    /* Release the DNS mutex.  */
    tx_mutex_put(&(dns_ptr -> nx_dns_mutex));

    return(NX_DNS_SUCCESS);
}       
#endif /* NX_DNS_CACHE_ENABLE  */


#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_add_rr                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the DNS  resource record into record buffer.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_cache_add_rr(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, NX_DNS_RR *record_ptr, NX_DNS_RR **insert_ptr)
{

ALIGN_TYPE  *tail;
ALIGN_TYPE  *head;
NX_DNS_RR   *p;
NX_DNS_RR   *rr;       
ULONG       elapsed_time;
ULONG       current_time;
ULONG       max_elapsed_time;
                            
                                   
    /* Check the cache.  */
    if (cache_ptr == NX_NULL)
        return(NX_DNS_CACHE_ERROR);
                   
    /* Initialize the parameters.  */
    max_elapsed_time = 0;
    current_time = tx_time_get();

    /* Get head and tail. */
    tail = (ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1;
    tail = (ALIGN_TYPE*)(*tail);
    head = (ALIGN_TYPE*)cache_ptr;
    head = (ALIGN_TYPE*)(*head);
                 
    /* Set the pointer.  */
    rr = NX_NULL;

    /* Find an empty entry before head. */
    for(p = (NX_DNS_RR*)((ALIGN_TYPE*)cache_ptr + 1); p < (NX_DNS_RR*)head; p++)
    {
        if(!p -> nx_dns_rr_type)
        {
            rr = p;
            break; 
        }
    }

    /* Check the record ptr.  */
    if (!rr)
    {                                          
        /* Check whether the cache is full. */
        if((ALIGN_TYPE*)((UCHAR*)head + sizeof(NX_DNS_RR)) > tail) 
        {

            /* Find an aging resource reocrd and repalce it.  */
            for(p = (NX_DNS_RR*)((ALIGN_TYPE*)cache_ptr + 1); p < (NX_DNS_RR*)head; p++)
            {

                if (!p -> nx_dns_rr_name)
                    continue;

                /* Calculate the elapsed time.  */
                elapsed_time = current_time - p -> nx_dns_rr_last_used_time;

                /* Step1. Find the expired resource record.  */
                if ((elapsed_time / NX_IP_PERIODIC_RATE) >= p ->nx_dns_rr_ttl)
                {

                    /* Yes, find it.  */
                    rr = p;
                    break;
                }

                /* Step2. Find the aging resource record.  */
                if (elapsed_time >= max_elapsed_time)
                {
                    rr = p;
                    max_elapsed_time = elapsed_time;
                }
            }

            /* Check the replacement resource record.  */
            if (rr)
            {

                /* Delete this record.  */
                _nx_dns_cache_delete_rr(dns_ptr, cache_ptr, cache_size, rr);

                /* Update the head.  */
                head = (ALIGN_TYPE*)cache_ptr;
                head = (ALIGN_TYPE*)(*head);
            }
            else
            {
                return(NX_DNS_CACHE_ERROR);
            }
        }
        else
        {
            rr = (NX_DNS_RR*)head;
        }
    }

    /* Just copy it to cache_ptr. */
    memcpy(rr, record_ptr, sizeof(NX_DNS_RR)); /* Use case of memcpy is verified. */

    /* Update the resource record count.  */
    dns_ptr -> nx_dns_rr_count ++;

    /* Get the current time to set the elapsed time.  */
    rr -> nx_dns_rr_last_used_time = current_time;

    /* Set the insert ptr.  */
    if(insert_ptr != NX_NULL)
        *insert_ptr = rr;

    if((ALIGN_TYPE*)rr >= head)
    {

        /* Update HEAD when new record is added. */
        head = (ALIGN_TYPE*)cache_ptr;
        *head = (ALIGN_TYPE)(rr + 1);
    }

    return(NX_DNS_SUCCESS);
}    
#endif /* NX_DNS_CACHE_ENABLE  */      

                             
#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_find_answer                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds the answer of DNS query in record buffer.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    dns_ptr                           Pointer to DNS instance.          */ 
/*    cache_ptr                         Pointer to the record buffer      */  
/*    query_name                        RR Query name                     */ 
/*    query_type                        RR Query type                     */   
/*    buffer                            Pointer to buffer                 */ 
/*    buffer_size                       The size of record_buffer         */
/*    record_count                      The count of RR stored            */
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
static UINT _nx_dns_cache_find_answer(NX_DNS *dns_ptr, VOID *cache_ptr, UCHAR *query_name, USHORT query_type, UCHAR *buffer, UINT buffer_size, UINT *record_count)
{

ALIGN_TYPE          *head;
NX_DNS_RR           *p;      
ULONG               current_time;   
ULONG               elasped_ttl;    
UINT                old_count;
UINT                answer_count;
UCHAR               *buffer_prepend_ptr;
UINT                 query_name_length;
UINT                 name_string_length;
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES     
UCHAR               *buffer_append_ptr;
NX_DNS_NS_ENTRY     *nx_dns_ns_entry_ptr;  
NX_DNS_MX_ENTRY     *nx_dns_mx_entry_ptr;
NX_DNS_SRV_ENTRY    *nx_dns_srv_entry_ptr;  
NX_DNS_SOA_ENTRY    *nx_dns_soa_entry_ptr;
UINT                 rname_string_length;
UINT                 mname_string_length;
#endif /* NX_DNS_ENABLE_EXTENDED_RR_TYPES  */


    /* Check the query name string.  */
    if (_nx_utility_string_length_check((CHAR *)query_name, &query_name_length, NX_DNS_NAME_MAX))
    {

        /* Return.  */
        return(NX_DNS_CACHE_ERROR);
    }

    /* Check the cache.  */
    if (cache_ptr == NX_NULL)
        return(NX_DNS_CACHE_ERROR);

    /* Initialize the value.  */  
    old_count = 0;
    answer_count = 0;   
    if(record_count)
        *record_count = 0;
                          
    /* Set the buffer pointer.  */
    buffer_prepend_ptr = buffer;   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES    
    buffer_append_ptr = buffer + buffer_size;  
#endif /* NX_DNS_ENABLE_EXTENDED_RR_TYPES  */

    /* Get the current time.  */
    current_time = tx_time_get();

    /* Get head. */
    head = (ALIGN_TYPE*)cache_ptr;
    head = (ALIGN_TYPE*)(*head);

    /* Lookup the cache to delete the expired resource record and find the answer.  */ 
    for(p = (NX_DNS_RR*)((UCHAR*)cache_ptr + sizeof(ALIGN_TYPE)); (ALIGN_TYPE*)p < head; p++)
    {

        /* Check whether the resource record is valid. */
        if (!p -> nx_dns_rr_name)
            continue;  

        /* Calucate the elapsed time.  */
        elasped_ttl = (current_time - p -> nx_dns_rr_last_used_time) / NX_IP_PERIODIC_RATE;

        /* Compare the ttl.  */
        if (elasped_ttl >= p -> nx_dns_rr_ttl)
        {

            /* The resource record is expired, Delete the resource record.  */ 
            _nx_dns_cache_delete_rr(dns_ptr, dns_ptr -> nx_dns_cache, dns_ptr -> nx_dns_cache_size, p); 
            continue;
        }

        /* Check the resource record type.  */
        if (p -> nx_dns_rr_type != query_type)
            continue;

        /* Check the resource record name.  */
        if (_nx_dns_name_match(p -> nx_dns_rr_name, query_name, query_name_length))
            continue;      

        /* Update the elasped time and ttl.  */
        p -> nx_dns_rr_last_used_time = current_time;
        p -> nx_dns_rr_ttl -= elasped_ttl;
        
        /* Yes, get the answer.  */
        
        /* Check the type. */
        switch (query_type)
        {     
             
            case NX_DNS_RR_TYPE_A:
            {     

                /* Check the buffer size.  */
                if (buffer_size < 4)                   
                    break;

                /* Set the IPv4 address.  */
                *(ULONG *)buffer_prepend_ptr = p -> nx_dns_rr_rdata.nx_dns_rr_rdata_a.nx_dns_rr_a_address;

                /* Update the count and pointer.  */
                answer_count++;
                buffer_size -= 4;
                buffer_prepend_ptr += 4;    

                break;
            }
            case NX_DNS_RR_TYPE_AAAA:
            {
                        
                /* Check the buffer size.  */
                if (buffer_size < 16)                
                    break;

                /* Set the IPv6 address.  */
                *(ULONG *)buffer_prepend_ptr = p -> nx_dns_rr_rdata.nx_dns_rr_rdata_aaaa.nx_dns_rr_aaaa_address[0];  
                *(ULONG *)(buffer_prepend_ptr + 4) = p -> nx_dns_rr_rdata.nx_dns_rr_rdata_aaaa.nx_dns_rr_aaaa_address[1];
                *(ULONG *)(buffer_prepend_ptr + 8) = p -> nx_dns_rr_rdata.nx_dns_rr_rdata_aaaa.nx_dns_rr_aaaa_address[2];
                *(ULONG *)(buffer_prepend_ptr + 12) = p -> nx_dns_rr_rdata.nx_dns_rr_rdata_aaaa.nx_dns_rr_aaaa_address[3];   
                
                /* Update the count and pointer.  */ 
                answer_count++;
                buffer_size -= 16;
                buffer_prepend_ptr += 16;   

                break;
            }         
            case NX_DNS_RR_TYPE_PTR:   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES  
            case NX_DNS_RR_TYPE_CNAME:  
            case NX_DNS_RR_TYPE_TXT:    
#endif
            {

                /* PTR, CNAME, TXT record should be only one answer, union: ptr name, cname name, and txt data.  */  
                /* Check the name string.  */
                if (_nx_utility_string_length_check((CHAR *)p -> nx_dns_rr_rdata.nx_dns_rr_rdata_ptr.nx_dns_rr_ptr_name, &name_string_length, NX_DNS_NAME_MAX))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Make sure there is enough room to store the name and null-terminator.  */
                if (buffer_size < (name_string_length + 1))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Set the cname string.  */
                memcpy((char *)buffer_prepend_ptr, (char *)p -> nx_dns_rr_rdata.nx_dns_rr_rdata_ptr.nx_dns_rr_ptr_name, name_string_length); /* Use case of memcpy is verified. */

                /* Return success.  */
                return (NX_DNS_SUCCESS);
            }    
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
            case NX_DNS_RR_TYPE_NS:   
            {

                /* Check the name string.  */
                if (_nx_utility_string_length_check((CHAR *)p -> nx_dns_rr_rdata.nx_dns_rr_rdata_ns.nx_dns_rr_ns_name, &name_string_length, NX_DNS_NAME_MAX))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Make sure there is enough room to store the name and null-terminator.  */
                if (buffer_size < (sizeof(NX_DNS_NS_ENTRY) + name_string_length + 1))
                    break;

                /* Set the ns entry pointer.  */
                nx_dns_ns_entry_ptr = (NX_DNS_NS_ENTRY *)(buffer_prepend_ptr);

                /* Update the store pointer. include the null flag '\0'.  */
                buffer_append_ptr -= (name_string_length + 1);

                /* Set the ns string.  */
                memcpy((char *)buffer_append_ptr, (char *)p -> nx_dns_rr_rdata.nx_dns_rr_rdata_ns.nx_dns_rr_ns_name, name_string_length); /* Use case of memcpy is verified. */
                nx_dns_ns_entry_ptr -> nx_dns_ns_hostname_ptr = buffer_append_ptr;   

                /* Update the count and pointer.  */  
                answer_count++;
                buffer_size -= (sizeof(NX_DNS_NS_ENTRY) + name_string_length + 1);
                buffer_prepend_ptr += sizeof(NX_DNS_NS_ENTRY);  

                break;
            }
            case NX_DNS_RR_TYPE_MX: 
            {

                /* Check the name string.  */
                if (_nx_utility_string_length_check((CHAR *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_mx.nx_dns_rr_mx_rdata + 2), &name_string_length, NX_DNS_NAME_MAX))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Make sure there is enough room to store the name and null-terminator.  */
                if (buffer_size < (sizeof(NX_DNS_MX_ENTRY) + name_string_length + 1))
                    break;

                /* Set the mx entry pointer.  */
                nx_dns_mx_entry_ptr = (NX_DNS_MX_ENTRY *)(buffer_prepend_ptr);

                /* Set the mx preference.  */  
                nx_dns_mx_entry_ptr -> nx_dns_mx_preference = *(USHORT *)p -> nx_dns_rr_rdata.nx_dns_rr_rdata_mx.nx_dns_rr_mx_rdata;

                /* Update the store pointer. include the null flag '\0'.  */
                buffer_append_ptr -= (name_string_length + 1);

                /* Set the mx string.  */
                memcpy((char *)buffer_append_ptr, (char *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_mx.nx_dns_rr_mx_rdata + 2), name_string_length); /* Use case of memcpy is verified. */
                nx_dns_mx_entry_ptr -> nx_dns_mx_hostname_ptr = buffer_append_ptr;
                                                                                
                /* Update the count and pointer.  */        
                answer_count++;
                buffer_size -= (sizeof(NX_DNS_MX_ENTRY) + name_string_length + 1);
                buffer_prepend_ptr += sizeof(NX_DNS_MX_ENTRY);  

                break;
            }
            case NX_DNS_RR_TYPE_SRV:
            {

                /* Check the name string.  */
                if (_nx_utility_string_length_check((CHAR *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata + 6), &name_string_length, NX_DNS_NAME_MAX))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Make sure there is enough room to store the name and null-terminator.  */
                if (buffer_size < (sizeof(NX_DNS_SRV_ENTRY) + name_string_length + 1))
                    break;

                /* Set the srv entry pointer.  */
                nx_dns_srv_entry_ptr = (NX_DNS_SRV_ENTRY *)(buffer_prepend_ptr);
                                                  
                /* Set the priority, weight and port number.  */
                nx_dns_srv_entry_ptr -> nx_dns_srv_priority = *(USHORT *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata);  
                nx_dns_srv_entry_ptr -> nx_dns_srv_weight = *(USHORT *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata + 2);
                nx_dns_srv_entry_ptr -> nx_dns_srv_port_number = *(USHORT *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata + 4);  

                /* Update the store pointer. include the null flag '\0'.  */                                                             
                buffer_append_ptr -= (name_string_length + 1);

                /* Set the srv string.  */
                memcpy((char *)buffer_append_ptr, (char *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata + 6), name_string_length); /* Use case of memcpy is verified. */
                nx_dns_srv_entry_ptr -> nx_dns_srv_hostname_ptr = buffer_append_ptr;
                                                                        
                /* Update the count and pointer.  */    
                answer_count++;
                buffer_size -= (sizeof(NX_DNS_SRV_ENTRY) + name_string_length + 1);
                buffer_prepend_ptr += sizeof(NX_DNS_SRV_ENTRY);  

                break;
            }       
            case NX_DNS_RR_TYPE_SOA:
            {

                /* Check the mname string.  */
                if (_nx_utility_string_length_check((CHAR *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata), &mname_string_length, NX_DNS_NAME_MAX))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Check the rname string.  */
                if (_nx_utility_string_length_check((CHAR *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + 1), &rname_string_length, NX_DNS_NAME_MAX))
                {

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Make sure there is enough room to store the name and null-terminator.  */
                if (buffer_size < (sizeof(NX_DNS_SOA_ENTRY) + mname_string_length + rname_string_length + 2))
                {                 

                    /* Return.  */
                    return(NX_DNS_CACHE_ERROR);
                }

                /* Set the soa entry pointer.  */
                nx_dns_soa_entry_ptr = (NX_DNS_SOA_ENTRY *)(buffer_prepend_ptr);
                                                                                  
                /* Update the store pointer. include the null flag '\0'.  */                                                             
                buffer_append_ptr -= (mname_string_length + rname_string_length + 2);

                /* Set the soa mname string.  */
                memcpy((char *)(buffer_append_ptr), (char *)p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata, mname_string_length); /* Use case of memcpy is verified. */
                nx_dns_soa_entry_ptr -> nx_dns_soa_host_mname_ptr = buffer_append_ptr;

                /* Set the soa rname string.  */                                                                                           
                memcpy((char *)(buffer_append_ptr + mname_string_length + 1), (const char *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + 1), rname_string_length); /* Use case of memcpy is verified. */
                nx_dns_soa_entry_ptr -> nx_dns_soa_host_rname_ptr = (buffer_append_ptr + mname_string_length + 1);

                /* Set the SOA Serial, Refresh, Retry, Expire, Minmum.  */ 
                nx_dns_soa_entry_ptr -> nx_dns_soa_serial = *(ULONG *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + rname_string_length + 2); 
                nx_dns_soa_entry_ptr -> nx_dns_soa_refresh = *(ULONG *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + rname_string_length + 6); 
                nx_dns_soa_entry_ptr -> nx_dns_soa_retry = *(ULONG *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + rname_string_length + 10); 
                nx_dns_soa_entry_ptr -> nx_dns_soa_expire = *(ULONG *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + rname_string_length + 14); 
                nx_dns_soa_entry_ptr -> nx_dns_soa_minmum = *(ULONG *)(p -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + mname_string_length + rname_string_length + 18);  
                       
                /* Return success.  */
                return (NX_DNS_SUCCESS);
            }                         
#endif /* NX_DNS_ENABLE_EXTENDED_RR_TYPES  */
        }

        /* Check if the answer count is updated.  */
        if (old_count == answer_count)
        {       

            /* The answer count is not updated, means the buffer is not enough, stop finding.  */
            break;
        }
        else
        {
            /* Update the old count.  */
            old_count = answer_count;
        }
    }

    /* Check the answer count.  */
    if (answer_count)
    {

        /* Update the record count.  */
        if (record_count)
            *record_count = answer_count;
        return (NX_DNS_SUCCESS);
    }
    else
    {
        return(NX_DNS_ERROR);
    }
}   
#endif /* NX_DNS_CACHE_ENABLE  */

           
#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_delete_rr                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the DNS resource record from cache.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
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
static UINT _nx_dns_cache_delete_rr(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, NX_DNS_RR *record_ptr)
{

ALIGN_TYPE  *head;


    /* Check the cache.  */
    if (cache_ptr == NX_NULL)
        return(NX_DNS_CACHE_ERROR);

    /* Delete the resource record strings. */
    _nx_dns_cache_delete_rr_string(dns_ptr, cache_ptr,cache_size, record_ptr);
    
    /* Zero out the record. */
    memset(record_ptr, 0, sizeof(NX_DNS_RR));

    /* Update the resource record count.  */
    dns_ptr -> nx_dns_rr_count --;
            
    /* Get head. */
    head = (ALIGN_TYPE*)cache_ptr;
    head = (ALIGN_TYPE*)(*head);

    /* Move HEAD if the last RR is deleted. */
    if(record_ptr == ((NX_DNS_RR*)head - 1))
    {
        while(!record_ptr -> nx_dns_rr_type)
        {
            record_ptr--;
            if(record_ptr < (NX_DNS_RR*)cache_ptr)
                break;
        }
        *((ALIGN_TYPE*)cache_ptr) = (ALIGN_TYPE)(record_ptr + 1);
    }

    return(NX_SUCCESS);    
}     
#endif /* NX_DNS_CACHE_ENABLE  */                 


#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_delete_rr_string                      PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the DNS entry string from the record cache.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
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
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_dns_cache_delete_rr_string(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, NX_DNS_RR *record_ptr)
{

#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
UINT    string_len;
UINT    size;
#endif /* NX_DNS_ENABLE_EXTENDED_RR_TYPES */


    /* Check the cache.  */
    if (cache_ptr == NX_NULL)
        return(NX_DNS_CACHE_ERROR);

    /* Compare the resource record type.  */ 
    if((record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_PTR)
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
       ||
       (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_TXT) ||
       (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_CNAME) ||
       (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_NS)
#endif
        )
    {

        /* Delete the rdata name string. */                      
        _nx_dns_cache_delete_string(dns_ptr, cache_ptr, cache_size, record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_ptr.nx_dns_rr_ptr_name, 0);
    }
    else if (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_AAAA)
    {

        /* Delete the IPv6 address string. */                      
        _nx_dns_cache_delete_string(dns_ptr, cache_ptr, cache_size, record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_aaaa.nx_dns_rr_aaaa_address, 16);
    }
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES
    else if (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_SRV)
    {

        /* Compute the SRV rdata length.  */
        if (_nx_utility_string_length_check((CHAR *)(record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata), &size, NX_DNS_NAME_MAX))
        {

            /* Return.  */
            return(NX_DNS_CACHE_ERROR);
        }
        string_len = (UINT)(size + 6);

        /* Delete the SRV rdata string. */
        _nx_dns_cache_delete_string(dns_ptr, cache_ptr, cache_size, record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_srv.nx_dns_rr_srv_rdata, string_len);
    }
    else if (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_MX)
    {

        /* Compute the MX rdata length.  */
        if (_nx_utility_string_length_check((CHAR *)(record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_mx.nx_dns_rr_mx_rdata), &size, NX_DNS_NAME_MAX))
        {

            /* Return.  */
            return(NX_DNS_CACHE_ERROR);
        }
        string_len = (UINT)(size + 2);

        /* Delete the MX rdata string. */
        _nx_dns_cache_delete_string(dns_ptr, cache_ptr, cache_size, record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_mx.nx_dns_rr_mx_rdata, string_len);
    }
    else if (record_ptr -> nx_dns_rr_type == NX_DNS_RR_TYPE_SOA)
    {

        /* Compute the SOA rdata length.  */
        if (_nx_utility_string_length_check((CHAR *)(record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata), &size, NX_DNS_NAME_MAX))
        {

            /* Return.  */
            return(NX_DNS_CACHE_ERROR);
        }
        string_len = size; /* MNAME  */
        string_len += 1; /* '\0'  */

        if (_nx_utility_string_length_check((CHAR *)(record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata + string_len), &size, NX_DNS_NAME_MAX))
        {

            /* Return.  */
            return(NX_DNS_CACHE_ERROR);
        }
        string_len += size; /* RNAME  */
        string_len += 1;  /* '\0'  */
        string_len += 20; /* Serial, Refresh, Retry, Expire, Minmum.  */

        /* Delete the SRV rdata string. */
        _nx_dns_cache_delete_string(dns_ptr, cache_ptr, cache_size, record_ptr -> nx_dns_rr_rdata.nx_dns_rr_rdata_soa.nx_dns_rr_soa_rdata, string_len);
    }
#endif

    /* Delete the name string. */                      
    _nx_dns_cache_delete_string(dns_ptr, cache_ptr, cache_size, record_ptr -> nx_dns_rr_name, 0);

    return(NX_DNS_SUCCESS);
}     
#endif /* NX_DNS_CACHE_ENABLE  */

                        
#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_add_string                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds or finds the DNS string in the cache.            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
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
static UINT _nx_dns_cache_add_string(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, VOID *string_ptr, UINT string_size, VOID **insert_ptr)
{

ALIGN_TYPE  *tail;
ALIGN_TYPE  *head;
UINT        string_len;
USHORT      len, cnt;
USHORT      min_len = 0xFFFF;
UCHAR       *p, *available, *start;
                                 
                                     
    /* Check the cache.  */
    if (cache_ptr == NX_NULL)
        return(NX_DNS_CACHE_ERROR);

    /* Get head and tail. */
    tail = (ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1;
    p = (UCHAR*)tail;
    tail = (ALIGN_TYPE*)(*tail);
    head = (ALIGN_TYPE*)cache_ptr;
    head = (ALIGN_TYPE*)(*head);

    /* Calculate the amount of memory needed to store this string, including CNT and LEN fields. */
    
    /* Make the length 4 bytes align. */
    string_len = string_size;
    
    /* Add the length of CNT and LEN fields.  */
    string_len = ((string_len & 0xFFFFFFFC) + 8) & 0xFFFFFFFF;
    
    available = (UCHAR*)tail;
    while(p > (UCHAR*)tail)
    {

        /* Get len and cnt. */
        len = *((USHORT*)(p - 2));
        cnt = *((USHORT*)(p - 4));
        start = p - len;

        if((len == string_len) &&
           (!_nx_dns_name_match(start, string_ptr, string_size)))
        {

            /* The same string exists in the string table. */
            if(insert_ptr)
                *insert_ptr = start;

            /* Increase the use count CNT. */
            cnt++;
            *((USHORT*)(p - 4)) = cnt;

            return(NX_SUCCESS);
        }

        /* This slot is not being used. The size of the slot is a smaller
           fit for this string. */
        if((cnt == 0) && (len >= string_len) && (len < min_len))
        {

            /* This place is better to insert. */
            available = p;
            min_len = len;
        }

        /* Move to the next string. */
        p = start;
    }

    /* If we reach this point, the string needs to be added to the string table. */
    if(available == (UCHAR*)tail)
    {

        /* Make sure the service cache still has room to add this string 
           (without overwriting the RR area.) */
        if(((UCHAR*)tail - string_len) < (UCHAR*)head)
        {
                           
            /* This service cache does not have room for the string table to grow. */
            /* Invoke user-installed cache full notify function .*/
            if(dns_ptr -> nx_dns_cache_full_notify)
            {

                /* Call the notify function.  */
                (dns_ptr -> nx_dns_cache_full_notify)(dns_ptr);
            }

            /* The buffer is full. */
            return(NX_DNS_SIZE_ERROR);
        }
        
        /* Update TAIL. */
        *((ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1) = (ALIGN_TYPE)(available - string_len);

    }
    else if(string_len < min_len)
    {

        /* Set the LEN for remaining space. */
        *((USHORT*)(available - 2 - string_len)) = (USHORT)(min_len - string_len);
    }

    /* Set LEN and CNT. */
    *((USHORT*)(available - 2)) = (USHORT)string_len;
    *((USHORT*)(available - 4)) = 1;

    /* Clear last 4 bytes. */
    *((ULONG*)(available - 8)) = 0;

    /* Insert string to cache. */
    memcpy(available - string_len, string_ptr, string_size); /* Use case of memcpy is verified. */

    /* Set end character 0. */
    *(available - string_len + string_size) = 0;

    /* Update the string length and count .  */
    dns_ptr -> nx_dns_string_count ++;
    dns_ptr -> nx_dns_string_bytes += string_len;

    if(insert_ptr)
        *insert_ptr = available - string_len;

    return(NX_SUCCESS);
}     
#endif /* NX_DNS_CACHE_ENABLE  */

                   
#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_cache_delete_string                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the DNS string from the record buffer.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dns_ptr                           Pointer to DNS instance.          */ 
/*    cache_ptr                         Pointer to the record buffer      */ 
/*    cache_size                        The size of buffer.               */ 
/*    string_ptr                        Pointer to the string             */ 
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
static UINT _nx_dns_cache_delete_string(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size, VOID *string_ptr, UINT string_len)
{

ALIGN_TYPE  *tail;
ALIGN_TYPE  *end;
USHORT      cnt;


    /* Check the cache.  */
    if (cache_ptr == NX_NULL)
        return(NX_DNS_CACHE_ERROR);

    /* Validate input parameter. */
    if (string_ptr == NX_NULL)
        return(NX_DNS_PARAM_ERROR);

    /* Validate string. */
    if (_nx_utility_string_length_check((CHAR *)string_ptr, &string_len, NX_DNS_NAME_MAX))
        return(NX_DNS_SIZE_ERROR);

    /* Add the length of CNT and LEN fields.  */
    /* Also make the total length 4 bytes align. */
    string_len = ((string_len & 0xFFFFFFFC) + 8) & 0xFFFFFFFF;

    end = (ALIGN_TYPE*)((UCHAR*)string_ptr + string_len);

    /* Get tail. */

    /* Validate the string table. */
    tail = (ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1;
    if(end > tail)
    {

        /* The end of string exceeds cache_ptr. */
        return(NX_DNS_SIZE_ERROR);
    }
    tail = (ALIGN_TYPE*)(*tail);
    if((UCHAR*)string_ptr < (UCHAR*)tail)
    {

        /* The end of string exceeds cache_ptr. */
        return(NX_DNS_SIZE_ERROR);
    }

    /* Decrease the usage counter value. */
    cnt = *((USHORT*)((UCHAR*)end - 4));
    cnt--;
    *((USHORT*)((UCHAR*)end - 4)) = cnt;

    /* Clear the memory if cnt is zero. */
    if(cnt == 0)
    {
        memset(string_ptr, 0, string_len - 2);

        /* Update the string length and count .  */
        dns_ptr -> nx_dns_string_count --;
        dns_ptr -> nx_dns_string_bytes -= string_len;
                
        /* Update the tail pointer if the string at the tail is deleted. */
        if(string_ptr == tail)
        {
            tail = end;
        
            while(end < ((ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1))
            {
                
                /* Set the string pt and string length.  */
                string_ptr = end;

                /* Validate string. */
                if (_nx_utility_string_length_check((CHAR *)string_ptr, &string_len, NX_DNS_NAME_MAX))
                    return(NX_DNS_SIZE_ERROR);

                /* Check the string length.  */
                if(string_len == 0)
                {
                    
                    /* This slot is cleared. */
                    while(*((ULONG*)string_ptr) == 0)
                        string_ptr = (UCHAR*)string_ptr + 4;
                    
                    end = (ALIGN_TYPE*)((UCHAR*)string_ptr + 4);
                    cnt = *((USHORT*)string_ptr);
                }
                else
                {
                    
                    /* Make the length 4 bytes align and add the length of CNT and LEN fields.  */
                    string_len = ((string_len & 0xFFFFFFFC) + 8) & 0xFFFFFFFF;

                    end = (ALIGN_TYPE*)((UCHAR*)string_ptr + string_len);
                    cnt = *((USHORT*)((UCHAR*)end - 4));
                }
                
                /* Check whether this slot is never referenced. */
                if(cnt == 0)
                    tail = end;
                else
                    break;
            }
            *((ALIGN_TYPE*)((UCHAR*)cache_ptr + cache_size) - 1) = (ALIGN_TYPE)tail;
        }
    }

    return(NX_SUCCESS);
}    
#endif /* NX_DNS_CACHE_ENABLE  */
    

#ifdef NX_DNS_CACHE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dns_name_match                                  PORTABLE C      */ 
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
static UINT  _nx_dns_name_match(UCHAR *src, UCHAR *dst, UINT length)
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
                    return(NX_DNS_NAME_MISMATCH);
            }
            else
            {
                return(NX_DNS_NAME_MISMATCH);
            }
        }
        src ++;
        dst ++;
        index ++;
    }

    /* Check the scan length.  */
    if (index != length)
    {
        return (NX_DNS_NAME_MISMATCH);
    }

    /* Return success.  */
    return(NX_DNS_SUCCESS);
}
#endif /* NX_DNS_CACHE_ENABLE  */       
