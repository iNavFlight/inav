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
/** NetX Duo Component                                                    */
/**                                                                       */
/**   Network Address Translation Protocol (NAT)                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define NX_NAT_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

                    
#include "tx_api.h"  
#include "tx_thread.h"
#include "nx_api.h"
#include "nx_nat.h"
#include "nx_udp.h" 
#include "nx_tcp.h" 
#include "nx_icmp.h" 
#include "nx_packet.h" 
#include "nx_system.h"     
                    
#ifdef NX_NAT_ENABLE
/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

/* Define */
NX_NAT_DEVICE       *nat_server_ptr;     
                                                 
               
/* Define internal NAT services. */
                                     
static UINT    _nx_nat_process_packet(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT packet_process); 
static UINT    _nx_nat_process_inbound_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr);
static UINT    _nx_nat_process_inbound_TCP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);
static UINT    _nx_nat_process_inbound_UDP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);
static UINT    _nx_nat_process_inbound_ICMP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);
static UINT    _nx_nat_process_outbound_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr);
static UINT    _nx_nat_process_outbound_TCP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);
static UINT    _nx_nat_process_outbound_UDP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);
static UINT    _nx_nat_process_outbound_ICMP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);
static VOID    _nx_nat_ip_packet_send(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, UCHAR packet_type, ULONG next_hop_address);  
static UINT    _nx_nat_inbound_entry_find(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, NX_NAT_TRANSLATION_ENTRY **matched_entry_ptr, ULONG *next_hop_address);
static UINT    _nx_nat_outbound_entry_find(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, NX_NAT_TRANSLATION_ENTRY **matched_entry_ptr, ULONG *next_hop_address);   
static UINT    _nx_nat_entry_create(NX_NAT_DEVICE *nat_ptr, UCHAR protocol, ULONG local_ip_address, ULONG peer_ip_address, 
                                    USHORT local_port, USHORT  external_port, USHORT  peer_port, ULONG response_timeout, NX_NAT_TRANSLATION_ENTRY **match_entry_ptr);
static UINT    _nx_nat_entry_add(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr);  
static VOID    _nx_nat_entry_find(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_to_match, NX_NAT_TRANSLATION_ENTRY **match_entry_ptr, UCHAR direction, UINT skip_static_entries); 
static VOID    _nx_nat_entry_timeout_check(NX_NAT_DEVICE *nat_ptr);
static UINT    _nx_nat_packet_is_icmp_error_message(NX_PACKET *packet_ptr, UINT *is_icmp_error_msg);
static UINT    _nx_nat_find_available_port(NX_NAT_DEVICE *nat_ptr, UCHAR protocol, USHORT *port); 
static UINT    _nx_nat_entry_port_verify(NX_IP *ip_ptr, UINT protocol, UINT port);    
static UINT    _nx_nat_socket_port_verify(NX_IP *ip_ptr, UINT protocol, UINT port);      
static UINT    _nx_nat_utility_get_source_port(NX_PACKET *packet_ptr, UCHAR protocol, USHORT *source_port);
static UINT    _nx_nat_utility_get_destination_port(NX_PACKET *packet_ptr, UCHAR protocol, USHORT *destination_port); 
static VOID    _nx_nat_checksum_adjust(UCHAR *checksum, UCHAR *old_data, INT old_data_length, UCHAR *new_data, INT new_data_length);


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_create                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the create NAT service.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    ip_ptr                            Pointer to NAT IP instance        */
/*    global_interface_index            Index of global interface         */ 
/*    dynamic_cache_memory              Pointer to dynamic entry cache    */ 
/*    dynamic_cache_size                The size of dynamic entry cache   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    NX_NAT_PARAM_ERROR                 Invalid non pointer input        */
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_create                    Creates the NAT instance service  */ 
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
UINT  _nxe_nat_create(NX_NAT_DEVICE *nat_ptr, NX_IP *ip_ptr, UINT global_interface_index, VOID *dynamic_cache_memory, UINT dynamic_cache_size)                        
{

UINT status;


    /* Check for valid input pointers. */
    if ((!nat_ptr) || (!ip_ptr))
    {

       /* Return pointer error. */
       return(NX_PTR_ERROR);
    }

    /* Check for invalid non pointer input.  */
    if (ip_ptr -> nx_ip_id != NX_IP_ID)
    {

        /* Return pointer error. */
        return(NX_NAT_PARAM_ERROR);
    }  

    /* Check the interface index.  */
    if (global_interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
   
        /* Return pointer error. */
        return(NX_NAT_PARAM_ERROR);
    }             

    /* Make sure entry cache is 4-byte aligned. */
    if ((((UINT)dynamic_cache_memory & 0x3) != 0) ||
        ((dynamic_cache_size & 0x3) != 0))
    {

        /* Return error status.  */
        return(NX_NAT_CACHE_ERROR);
    }
    
    /* Check the cache size.  */
    if (dynamic_cache_size < (NX_NAT_MIN_ENTRY_COUNT * sizeof (NX_NAT_TRANSLATION_ENTRY)))
    {        

        /* Return error status.  */
        return(NX_NAT_CACHE_ERROR);
    }

    /* Call the actual NAT create service.  */
    status = _nx_nat_create(nat_ptr, ip_ptr, global_interface_index, dynamic_cache_memory, dynamic_cache_size);

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_create                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function creates the NetX NAT instance and various NAT          */
/*   configuration options, it also creates the NAT mutex.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    ip_ptr                            Pointer to NAT IP instance        */
/*    global_interface_index            Index of global interface         */
/*    dynamic_cache_memory              Pointer to dynamic entry cache    */ 
/*    dynamic_cache_size                The size of dynamic entry cache   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_SUCCESS                         Successful completion            */
/*    NX_NAT_OVERLAPPING_SUBNET_ERROR    Invalid NAT network interfaces   */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_create                    Create NAT flag group mutex      */
/*    memset                             Clear specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_nat_create(NX_NAT_DEVICE *nat_ptr, NX_IP *ip_ptr, UINT global_interface_index, VOID *dynamic_cache_memory, UINT dynamic_cache_size)  
{

UINT                          i;
UINT                          dynamic_entries;
NX_NAT_TRANSLATION_ENTRY      *entry_ptr;    
                      

    /* First clear memory for setting up the NAT server instance.  */
    memset(nat_ptr, 0, sizeof(NX_NAT_DEVICE));

    /* Bind the IP instance and related parameters. */
    nat_ptr -> nx_nat_ip_ptr = ip_ptr;
    nat_ptr -> nx_nat_global_interface_index = (UCHAR)global_interface_index;
                                                                                                                                 
    /* Clear the entry cache.  */
    memset((void *) dynamic_cache_memory, 0, dynamic_cache_size);

    /* Pickup starting address of the available entry list.  */
    entry_ptr = (NX_NAT_TRANSLATION_ENTRY *) dynamic_cache_memory;

    /* Determine how many NAT daynamic entries will fit in this cache area.  */
    dynamic_entries = dynamic_cache_size / sizeof(NX_NAT_TRANSLATION_ENTRY);         
               
    /* Initialize the pointers of available NAT entries.  */
    for (i = 0; i < (dynamic_entries - 1); i++)
    {
        /* Setup each entry to point to the next entry.  */
        entry_ptr -> next_entry_ptr = entry_ptr + 1;
        entry_ptr ++;
    }

    /* Setup the head pointers of the available and dynamic (active) lists in the NAT Device.  */    
    nat_ptr -> nx_nat_dynamic_available_entry_head = (NX_NAT_TRANSLATION_ENTRY *) dynamic_cache_memory; 
    nat_ptr -> nx_nat_dynamic_active_entry_head = NX_NULL;
    nat_ptr -> nx_nat_dynamic_available_entries = dynamic_entries;
    nat_ptr -> nx_nat_dynamic_active_entries = 0;
    nat_ptr -> nx_nat_static_active_entries = 0;       
                
    /* Load the NAT ID field in the NAT control block.  */
    nat_ptr -> nx_nat_id =  NX_NAT_ID;
                             
    /* Define a global NAT pointer. NetX forwards all packets to _nx_nat_process_packet if _nx_ip_packet_forward() is defined as such.
       The latter service can only send the current NX_IP and NX_PACKET pointer, so _nx_nat_process_packet()
       requries this global pointer. */
    /* Set the pointer of global variable NAT.  */
    nat_server_ptr = nat_ptr;

    /* Return successful completion. */
    return NX_SUCCESS;
} 


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_delete                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the delete NAT service.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to NAT instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_delete                    Deletes a NAT instance            */ 
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
UINT  _nxe_nat_delete(NX_NAT_DEVICE *nat_ptr) 
{

UINT status;

                  
    /* Check for invalid input pointers.  */
    if ((nat_ptr == NX_NULL) || (nat_ptr -> nx_nat_id != NX_NAT_ID))
        return(NX_PTR_ERROR);  

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual NAT delete service.  */
    status = _nx_nat_delete(nat_ptr); 

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_delete                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function deletes the specified NetX NAT instance.               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to NAT server to delete   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_delete                   Delete NAT mutex                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_nat_delete(NX_NAT_DEVICE *nat_ptr) 
{                             

    /* Clear the NAT ID to make it invalid.  */
    nat_ptr -> nx_nat_id =  0;     

    return NX_SUCCESS;
}

            
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_enable                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the resume NAT thread       */
/*   service.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                          Pointer to IP instance              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                          Actual completion status            */ 
/*    NX_PTR_ERROR                    Invalid pointer parameter           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_enable                  Actual enable NAT server            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_nat_enable(NX_NAT_DEVICE *nat_ptr)
{

UINT status;
            
    /* Check for invalid input pointers.  */
    if ((nat_ptr == NX_NULL) || (nat_ptr -> nx_nat_id != NX_NAT_ID))
        return(NX_PTR_ERROR);     

    /* Call the actual service. */
    status = _nx_nat_enable(nat_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_enable                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function enables the NAT server.                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                         Pointer to NAT server to resume     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Successful completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                    Get IP mutex                        */ 
/*    tx_mutex_put                    Put IP mutex                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_nat_enable(NX_NAT_DEVICE *nat_ptr)
{                     
             
                                     
    /* Get the IP mutex.  */
    tx_mutex_get(&(nat_ptr -> nx_nat_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup the IP nat processing routine pointer for NAT services.  */
    nat_ptr -> nx_nat_ip_ptr -> nx_ip_nat_packet_process =  _nx_nat_process_packet;    

    /* Setup the IP nat free port check for NAT services.  */
    nat_ptr -> nx_nat_ip_ptr -> nx_ip_nat_port_verify =  _nx_nat_entry_port_verify; 

    /* Release the IP mutex.  */
    tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));                                                                               

    /* Return successful completion.  */
    return(NX_SUCCESS);
}                       
                                      
            
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_disable                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the disable NAT service.    */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                         Pointer to NAT instance             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                          Actual completion status            */ 
/*    NX_PTR_ERROR                    Invalid pointer parameter           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_enable                  Actual enable NAT server            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_nat_disable(NX_NAT_DEVICE *nat_ptr)
{

UINT status;

                                      
    /* Check for invalid input pointers.  */
    if ((nat_ptr == NX_NULL) || (nat_ptr -> nx_nat_id != NX_NAT_ID))
        return(NX_PTR_ERROR);       

    /* Call the actual service. */
    status = _nx_nat_disable(nat_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_disable                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function disables the NAT server.                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                         Pointer to NAT server to resume     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Successful completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                    Get IP mutex                        */ 
/*    tx_mutex_put                    Put IP mutex                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_nat_disable(NX_NAT_DEVICE *nat_ptr)
{                                              
                        
    /* Get the IP mutex.  */
    tx_mutex_get(&(nat_ptr -> nx_nat_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
                                                                                    
    /* Clear the IP nat processing routine pointer.  */  
    nat_ptr -> nx_nat_ip_ptr -> nx_ip_nat_packet_process = NX_NULL;         

    /* Release the IP mutex.  */
    tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));                         
           
    /* Return successful completion.  */
    return(NX_SUCCESS);
}                  


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_cache_notify_set                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the NAT cache full notify        */ 
/*    function call.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    cache_full_notify                 Cache full notify function        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_cache_notify_set          Actual cache notify set  function */ 
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
UINT _nxe_nat_cache_notify_set(NX_NAT_DEVICE *nat_ptr, VOID (*cache_full_notify_cb)(NX_NAT_DEVICE *nat_ptr))
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (!nat_ptr)
    {    
        return(NX_PTR_ERROR);
    }
        
    /* Check for invalid non pointer input. */
    if (nat_ptr -> nx_nat_id != NX_NAT_ID)
    {
        return(NX_NAT_PARAM_ERROR);
    }

    /* Call actual NAT cache notify set function.  */
    status =  _nx_nat_cache_notify_set(nat_ptr, cache_full_notify_cb);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_cache_notify_set                            PORTABLE C      */ 
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
/*    nat_ptr                           Pointer to nat instance           */ 
/*    cache_full_notify                 Cache full notify function        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                      Get IP mutex                      */ 
/*    tx_mutex_put                      Put IP mutex                      */ 
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
UINT _nx_nat_cache_notify_set(NX_NAT_DEVICE *nat_ptr, VOID (*cache_full_notify_cb)(NX_NAT_DEVICE *nat_ptr))
{

                                  
    /* Get the IP mutex.  */
    tx_mutex_get(&(nat_ptr -> nx_nat_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
    
    /* Set the cache notify.  */
    nat_ptr -> nx_nat_cache_full_notify = cache_full_notify_cb;
                        
    /* Release the IP mutex.  */
    tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));   

    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_inbound_entry_create                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the create NAT translation  */
/*   inbound entry service.                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */    
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    entry_ptr                         Pointer to entry to add to table  */ 
/*    local_ip_address                  Entry local IP address            */
/*    external_port                     Entry external port               */  
/*    local_port                        Entry local port                  */
/*    protocol                          Entry network protocol            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    NX_NAT_PARAM_ERROR                 Invalid non pointer parameter    */ 
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_create     Calls actual create entry service  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_nat_inbound_entry_create(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr,   
                                    ULONG local_ip_address, USHORT external_port, USHORT local_port, UCHAR protocol)
{

UINT  status;

                                       
    /* Check for invalid input pointers.  */
    if ((nat_ptr == NX_NULL) || (nat_ptr -> nx_nat_id != NX_NAT_ID) || (!entry_ptr))
    {
        return (NX_PTR_ERROR);  
    }                    

    /* Check for invalid non pointer input. */
    if ((!local_ip_address) || (!protocol))
    {

        /* Return ptr error. */
        return (NX_NAT_PARAM_ERROR);
    }                       

    /* Call the actual service. */
    status = _nx_nat_inbound_entry_create(nat_ptr, entry_ptr, local_ip_address, external_port, local_port, protocol);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_create                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function creates a inbound NAT translation entry and adds it to */
/*   the NAT entry list. This is allows an  external host to initiate     */
/*   a session with a private host (typically a server).                  */
/*                                                                        */
/*   This entry can never expire. To remove the entry, use the            */
/*   nx_nat_inbound_entry_delete() service.                               */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */    
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    entry_ptr                         Pointer to entry to add to table  */ 
/*    local_ip_address                  Entry local IP address            */
/*    external_port                     Entry external port               */  
/*    local_port                        Entry local port                  */
/*    protocol                          Table entry network protocol      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    tx_mutex_get                      Get IP mutex                      */ 
/*    tx_mutex_put                      Put IP mutex                      */ 
/*    _nx_nat_entry_add                 Add entry to table linked list    */
/*    memset                            Clear specified area of memory    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_nat_inbound_entry_create(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr,   
                                   ULONG local_ip_address, USHORT external_port, USHORT local_port, UCHAR protocol)
{                    

UINT                     bound;
                                
    /* Get the IP mutex.  */
    tx_mutex_get(&(nat_ptr -> nx_nat_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
                                             
    /* Check whether this port has been used by NAT entry.  */
    bound = _nx_nat_entry_port_verify(nat_ptr -> nx_nat_ip_ptr, protocol, external_port); 

    /* Check the status.  */
    if (bound == NX_TRUE)
    {

        /* Release the IP mutex.  */
        tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));

        /* Return error status.  */
        return(NX_NAT_PORT_UNAVAILABLE);
    }

    /* Check whether this port has been used by NetXDuo socket.  */
    bound = _nx_nat_socket_port_verify(nat_ptr -> nx_nat_ip_ptr, protocol, external_port); 
           
    /* Check the status.  */
    if (bound == NX_TRUE)
    {

        /* Release the IP mutex.  */
        tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));

        /* Return error status.  */
        return(NX_NAT_PORT_UNAVAILABLE);
    }

    /* Initialize the entry to NULL. */
    memset(entry_ptr, 0, sizeof(NX_NAT_TRANSLATION_ENTRY));

    /* Assign the entry attributes. */
    entry_ptr -> local_ip_address = local_ip_address;
    entry_ptr -> external_port = external_port; 
    entry_ptr -> local_port = local_port;
    entry_ptr -> protocol = protocol;
                       
    /* Set the transaction type. */
    entry_ptr -> translation_type = NX_NAT_STATIC_ENTRY;                                                               

    /* Add the entry to the table. */
    _nx_nat_entry_add(nat_ptr, entry_ptr);     
                                                  
    /* Release the IP mutex.  */
    tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));   

    /* Return successful completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_nat_inbound_entry_delete                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the delete NAT translation  */
/*   inbound entry service.                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */      
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    remove_entry_ptr                  Pointer to NAT translation entry  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_delete      Actual table entry delete service */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_nat_inbound_entry_delete( NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *delete_entry_ptr)
{

UINT status;
               
    /* Check for invalid input pointers.  */
    if ((nat_ptr == NX_NULL) || (nat_ptr -> nx_nat_id != NX_NAT_ID) || (delete_entry_ptr == NX_NULL))
    {

        return(NX_PTR_ERROR); 
    }     
                 
    /* Check the entry tranlation type.  */
    if (delete_entry_ptr ->translation_type != NX_NAT_STATIC_ENTRY)
    {
        return(NX_NAT_ENTRY_TYPE_ERROR);
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual NAT entry delete service.  */
    status = _nx_nat_inbound_entry_delete(nat_ptr, delete_entry_ptr); 

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_delete                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function deletes the specified static entry from the NAT        */
/*   translation list.                                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */       
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    remove_entry_ptr                  Pointer to NAT translation entry  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memset                            Clear specified area of memory    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_nat_inbound_entry_delete(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *delete_entry_ptr)
{
                              
NX_NAT_TRANSLATION_ENTRY *entry_ptr;
NX_NAT_TRANSLATION_ENTRY *previous_ptr;
NX_NAT_TRANSLATION_ENTRY *next_entry_ptr;  

                            
    /* Get the IP mutex.  */
    tx_mutex_get(&(nat_ptr -> nx_nat_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);                 

    /* Get a pointer to the start of the entries in the translation table. */
    entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;

    /* Initialize the previous pointer.  */
    previous_ptr = NX_NULL;

    /* We need to loop through the translation table again so we
       can remove the entry to delete and connect the previous and
       next entries around it. */ 
    while (entry_ptr)
    {

        /* Set a pointer to the next entry in the table. */
        next_entry_ptr = entry_ptr -> next_entry_ptr;

        /* Is this the entry we want to delete? */
        if (entry_ptr == delete_entry_ptr)           
        {       

            /* Yes; check if this is the first entry in the list). */
            if (previous_ptr)
            {

                /* It is not, so link the previous entry around the entry we are deleting. */
                previous_ptr -> next_entry_ptr = next_entry_ptr;
            }
            else 
            {

                /* It is the first entry, so set the next pointer as the starting translation table entry. */
                nat_ptr -> nx_nat_dynamic_active_entry_head = next_entry_ptr;
            }           

            /* Update the static active entry count.  */      
            nat_ptr -> nx_nat_static_active_entries --;

            /* Release the IP mutex.  */
            tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));       

            /* We're done here. */
            return NX_SUCCESS;
        }

        /* We didn't delete the current entry, so now it is the previous entry. */
        previous_ptr = entry_ptr;
            
        /* Get the next entry in the table. */
        entry_ptr = next_entry_ptr;
    }                                                                                                    

    /* Release the IP mutex.  */
    tx_mutex_put(&(nat_ptr ->  nx_nat_ip_ptr -> nx_ip_protection));  

    /* Return error status. */
    return (NX_NAT_ENTRY_NOT_FOUND);
}                      


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_packet                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function receives packets to forward from both public and       */
/*   private networks on either side of NAT if IP forwarding is enabled   */
/*   and this function is specified as the packet handler.  It determines */
/*   packet direction (e.g. out to the public internet or inbound to a    */
/*   host on the private network) and forwards it to the appropriate      */
/*   handler.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                            Pointer to IP instance            */
/*    packet_ptr                        Pointer to packet to forward      */
/*    packet_process                    The flag for packet processing    */
/*                                        NX_TRUE or NX_FALSE             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_TRUE                          Packet is consumed by NAT          */ 
/*    NX_FALSE                         Packet is not consumed by NAT      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                Release the packet                 */
/*    _nx_ip_route_find                Find the outgoing interface        */
/*    _nx_nat_process_outbound_packet  Forward packet to public Internet  */
/*    _nx_nat_process_inbound_packet   Forward packet to private host     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ipv4_packet_receive          Netx forwards packet to NAT first  */ 
/*                                        if forwarding is enabled.       */
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
static UINT  _nx_nat_process_packet(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT packet_process)
{

UINT                            status;
UINT                            protocol;
ULONG                           header_size = sizeof(NX_IPV4_HEADER);
UINT                            global_interface_index;
ULONG                           next_hop_address;
NX_IPV4_HEADER                  *ip_header_ptr;
NX_INTERFACE                    *interface_ptr;
                                                          

    /* Check the NAT's IP instance.  */
    if (nat_server_ptr -> nx_nat_ip_ptr != ip_ptr)
    {                                              
                  
        /* Let IP packet receive process this packet. */
        return (NX_FALSE);  
    }                   

    /* Pickup the packet header.  */
    ip_header_ptr =  (NX_IPV4_HEADER *) packet_ptr -> nx_packet_prepend_ptr;                      
                                   
    /* Determine what protocol the current IP datagram is.  */
    protocol =  (ip_header_ptr -> nx_ip_header_word_2 >> 16) & 0xFF;   

    /* Check the protocol.  */
    if ((protocol != NX_PROTOCOL_ICMP) && (protocol != NX_PROTOCOL_UDP) && (protocol != NX_PROTOCOL_TCP))
    {                           
               
        /* Let IP packet receive process this packet. */
        return (NX_FALSE);  
    }

#ifndef NX_DISABLE_RX_SIZE_CHECKING
    /* Check the next protocol.  */
    if (protocol == NX_PROTOCOL_ICMP)
    {
        header_size += sizeof(NX_ICMP_HEADER);
    }
    else if (protocol == NX_PROTOCOL_UDP)
    {
        header_size += sizeof(NX_UDP_HEADER);
    }
    else
    {
        header_size += sizeof(NX_TCP_HEADER);
    }

    /* Check for valid packet length.  */
    if (packet_ptr -> nx_packet_length < header_size)
    {

#ifndef NX_DISABLE_IP_INFO
        /* Increment the IP invalid packet error.  */
        ip_ptr -> nx_ip_invalid_packets++;

        /* Increment the IP receive packets dropped count.  */
        ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

        /* Invalid packet length, just release it.  */
        _nx_packet_release(packet_ptr);
        
        /* Return NX_TRUE to indicate this packet has been processed.  */
        return (NX_TRUE);
    }
#endif /* NX_DISABLE_RX_SIZE_CHECKING */

    /* Set the packet interface pointer and NAT's global interface index.  */
    interface_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
    global_interface_index = nat_server_ptr -> nx_nat_global_interface_index;

    /* Check if the direction of the translation is inbound. */
    if (interface_ptr == &(ip_ptr -> nx_ip_interface[global_interface_index]))
    {                                                 
        
        /* Yes, this packet is received on NAT's external interface (an inbound packet). */
      
        /* Check the destination ip address against NAT's external IP address.  */
        if (ip_header_ptr -> nx_ip_header_destination_ip != interface_ptr -> nx_interface_ip_address)
        {      

            /* NAT cannot process this packet. Let IP packet receive process this packet. */
            return (NX_FALSE); 
        }

        /* Check if the caller wants NAT to process this packet. */
        if (packet_process == NX_TRUE)
        {

#ifndef NX_DISABLE_NAT_INFO
            /* Yes, so update the count.  */
            nat_server_ptr -> forwarded_packets_received ++;   
#endif

            /* Try to deliver the packet to the local host. */
            status = _nx_nat_process_inbound_packet(nat_server_ptr, packet_ptr);

            /* Check the status.  */ 
            if (status == NX_NAT_PACKET_CONSUMED_FAILED)
            {                       

                /* Let IP packet receive process this packet. */
                return (NX_FALSE); 
            }
        }
    }
    else
    {

        /* Check the destination ip address.  */
        if (

            /* Check for zero address.  */
            (ip_header_ptr -> nx_ip_header_destination_ip == 0) ||

            /* Check for limited broadcast.  */
            (ip_header_ptr -> nx_ip_header_destination_ip == NX_IP_LIMITED_BROADCAST) ||

            /* Check for multicast address*/
            ((ip_header_ptr -> nx_ip_header_destination_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE) ||

            /* Check for loopback address.  */
            ((ip_header_ptr -> nx_ip_header_destination_ip >= NX_IP_LOOPBACK_FIRST) &&
             (ip_header_ptr -> nx_ip_header_destination_ip <= NX_IP_LOOPBACK_LAST)))
        {

            /* Let IP packet receive process this packet. */
            return (NX_FALSE);
        }

        /* Clear the packet interface.  */
        interface_ptr = NX_NULL; 

        /* Find the suitable interface and next hop address according to the destination ip.  */
        if(_nx_ip_route_find(ip_ptr, ip_header_ptr -> nx_ip_header_destination_ip, &interface_ptr, &next_hop_address) != NX_SUCCESS)
        {

            /* Let IP packet receive process this packet. */
            return (NX_FALSE);  
        }

        /* Check the NAT forward interface.  */
        if (interface_ptr != &(ip_ptr -> nx_ip_interface[global_interface_index]))
        {

            /* Let IP packet receive process this packet. */
            return (NX_FALSE);  
        }
        
        /* Check for IP broadcast.  */
        if(((ip_header_ptr -> nx_ip_header_destination_ip & interface_ptr -> nx_interface_ip_network_mask) ==
            interface_ptr -> nx_interface_ip_network) &&
            ((ip_header_ptr -> nx_ip_header_destination_ip & ~(interface_ptr -> nx_interface_ip_network_mask)) ==
            ~(interface_ptr -> nx_interface_ip_network_mask)))
        {

            /* Let IP packet receive process this packet. */
            return (NX_FALSE);  
        }

        /* Check if NAT need to process this packet.  */
        if (packet_process == NX_TRUE)
        {

            /* Set the packet interface as global interface.  */
            packet_ptr -> nx_packet_address.nx_packet_interface_ptr = interface_ptr;

#ifndef NX_DISABLE_NAT_INFO
            /* Update the count.  */
            nat_server_ptr -> forwarded_packets_received ++;   
#endif                   

            /* Deliver the packet to the outbound packet handler. */
            _nx_nat_process_outbound_packet(nat_server_ptr, packet_ptr);
        }
    }

    /* Return NX_TRUE to indicate packet has been processed by NAT.  */
    return (NX_TRUE);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_packet                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function processes a packet from the external network for       */
/*   forwarding to the private network.  It sends each packet to a        */
/*   protocol handler for protocol specific processing.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                            Pointer to the NAT server        */ 
/*    packet_ptr                         Pointer to the packet to process */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_SUCCESS                         Successful completion status     */
/*    NX_NAT_INVALID_PROTOCOL            Unknown packet protocol          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_nat_process_inbound_TCP_packet                                   */ 
/*                                    Process external host's TCP packet  */
/*   _nx_nat_process_inbound_UDCP_packet                                  */ 
/*                                    Process external host's UDP packet  */
/*   _nx_nat_process_inbound_ICMP_packet                                  */ 
/*                                    Process external host's ICMP packet */
/*   nx_packet_release                Release the packet                  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_process_packet          Process packet from external host   */ 
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
static UINT  _nx_nat_process_inbound_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr)
{


UINT                        status = NX_SUCCESS;
UCHAR                       protocol;     
NX_IPV4_HEADER              *ip_header_ptr;
NX_NAT_TRANSLATION_ENTRY    translation_entry;
                                      
    
    /* Pick a pointer to the IP header. */
    ip_header_ptr =  (NX_IPV4_HEADER *) packet_ptr -> nx_packet_prepend_ptr;

    /* Determine what protocol the current IP datagram is.  */
    protocol =  (ip_header_ptr -> nx_ip_header_word_2 >> 16) & 0xFF; 
                   
    /* Set up the search criteria in the NAT translation table. */
    memset(&translation_entry, 0, sizeof(NX_NAT_TRANSLATION_ENTRY)); 
    translation_entry.peer_ip_address = ip_header_ptr -> nx_ip_header_source_ip;
    translation_entry.protocol = protocol;      

    /* Choose which packet handler by protocol. */
    switch (protocol)        
    {

        case  NX_PROTOCOL_TCP:
            status = _nx_nat_process_inbound_TCP_packet(nat_ptr, packet_ptr, &translation_entry);
            break;

        case  NX_PROTOCOL_UDP:
            status = _nx_nat_process_inbound_UDP_packet(nat_ptr, packet_ptr, &translation_entry);
            break;

        case  NX_PROTOCOL_ICMP:
            status = _nx_nat_process_inbound_ICMP_packet(nat_ptr, packet_ptr, &translation_entry);
            break;
    }

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_TCP_packet                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function finds the local IP address:port for an incoming packet */
/*   in the NAT translation table and replaces the global packet          */
/*   destination IP address and port with the local IP address and port.  */
/*   If none is found, it rejects the packet. Otherwise it updates the TCP*/
/*   checksum with the changed IP address and port. The packet is then    */
/*   sent to private host.                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                            Pointer to the NAT server        */ 
/*    packet_ptr                         Pointer to the packet to process */ 
/*    entry_ptr                          Pointer to the entry             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */
/*    NX_SUCCESS                         Successful completion status     */
/*                                        (packet is not necessarily      */
/*                                        forwarded)                      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find        Find inbound entry in entry list  */ 
/*    _nx_nat_ip_packet_send            Send packet to private interface  */
/*    nx_packet_release                 Release the packet                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_packet     Process inbound packets          */ 
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
static UINT  _nx_nat_process_inbound_TCP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{

UINT                            status; 
UINT                            entry_found;
USHORT                          old_port;
USHORT                          new_port;
ULONG                           old_address;
ULONG                           new_address;
USHORT                          checksum;
ULONG                           compute_checksum; 
ULONG                           next_hop_address;
NX_TCP_HEADER                   *tcp_header_ptr; 
NX_NAT_TRANSLATION_ENTRY        *record_entry;


    /* Initialize local variables. */ 
    compute_checksum = 1;
    record_entry = NX_NULL;
    entry_found = NX_TRUE;
                                                    
    /* Pickup the pointer to the head of the TCP packet.  */  
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length -= sizeof(NX_IPV4_HEADER);
    tcp_header_ptr =  (NX_TCP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);  
                 
    /* For little endian processors, adjust byte order for big endianness. */
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_sequence_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_acknowledgment_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_3);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);      

    /* Find the inbound entry, set the packet private interface and next hop address.  */
    status = _nx_nat_inbound_entry_find(nat_ptr, packet_ptr, entry_ptr, &record_entry, &next_hop_address);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

#ifndef NX_DISABLE_NAT_INFO
        /* Update the count.  */
        nat_ptr -> forwarded_packets_dropped++;
#endif

        /* Check the status.  */
        if (status == NX_NAT_ENTRY_NOT_FOUND)
        {

            /* Set the entry found flag.  */
            entry_found = NX_FALSE;
        }
        else
        {         

            /* Release the packet */
            nx_packet_release(packet_ptr);

            /* Return completion status. */
            return NX_SUCCESS;   
        }
    }  
    else
    {

        /* Check the inbound packet destination port, and update the TCP header. */
        if (record_entry -> external_port != record_entry -> local_port)
        {

            /* Replace the destination port with the local (source) port of the preceding outbound packet.   */
          
            tcp_header_ptr -> nx_tcp_header_word_0 = ((ULONG)(tcp_header_ptr -> nx_tcp_header_word_0 & ~NX_LOWER_16_MASK)) |
                                                      ((ULONG) record_entry -> local_port);
        }        

#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
        if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM)
            compute_checksum = 0;
        else
            compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
        if(compute_checksum)
        {

            /* Get the old checksum.  */
            checksum = (USHORT)(tcp_header_ptr -> nx_tcp_header_word_4 >> NX_SHIFT_BY_16);

            /* Check whether the port is updated.   */
            if (record_entry -> external_port != record_entry -> local_port)
            {

                /* Set the old port and new port.  */
                old_port = record_entry -> external_port;
                new_port = record_entry -> local_port;

                /* Adjust the checksum for port.  */
                _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_port, sizeof(USHORT), (UCHAR *)&new_port, sizeof(USHORT));
            }

            /* Set the old address and new address.  */
            old_address = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;
            new_address = record_entry -> local_ip_address;

            /* Adjust the checksum for address.  */
            _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_address, sizeof(LONG), (UCHAR *)&new_address, sizeof(LONG)); 

            /* OK to clear the TCP checksum field to zero before the checksum update. */
            tcp_header_ptr -> nx_tcp_header_word_4 = tcp_header_ptr -> nx_tcp_header_word_4 & NX_LOWER_16_MASK;  

            /* Place the checksum into the first header word.  */
            tcp_header_ptr -> nx_tcp_header_word_4 = tcp_header_ptr -> nx_tcp_header_word_4 | (ULONG)(checksum << NX_SHIFT_BY_16); 
        }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {

            /* OK to clear the TCP checksum field to zero before the checksum update. */
            tcp_header_ptr -> nx_tcp_header_word_4 = tcp_header_ptr -> nx_tcp_header_word_4 & NX_LOWER_16_MASK;  

            /* Set the flag.  */
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    }

    /* Swap endianness back before sending. */
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_sequence_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_acknowledgment_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_3);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);

    /* Check the entry found flag.  */
    if (entry_found == NX_FALSE)
    {

        /* Stop processing, recover packet length to Let NetXDuo process this packet.  */  
        packet_ptr -> nx_packet_prepend_ptr -= sizeof(NX_IPV4_HEADER);        
        packet_ptr -> nx_packet_length += sizeof(NX_IPV4_HEADER);
                                
        /* Let NetXDuo process this pcaket. */
        return(NX_NAT_PACKET_CONSUMED_FAILED);
    }                                                                             

    /* Send the TCP packet onto the private host. */
    _nx_nat_ip_packet_send(nat_ptr, packet_ptr, record_entry, NX_NAT_INBOUND_PACKET, next_hop_address); 

    /* Return completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_UDP_packet                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*   This function finds the local IP address:port for an incoming packet */
/*   in the NAT translation table and replaces the global packet          */
/*   destination IP address and port with the local IP address andport.   */
/*   If none is found, it rejects the packet. Otherwise it updates the UDP*/
/*   checksum with the changed IP address and port. The packet is then    */
/*   sent to private host.                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                            Pointer to the NAT server        */ 
/*    packet_ptr                         Pointer to the packet to process */ 
/*    entry_ptr                          Pointer to the entry             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_SUCCESS                         Successful completion status     */
/*    NX_NAT_INVALID_IP_HEADER           Invalid IP header                */
/*    NX_NAT_BAD_UDP_CHECKSUM            UDP checksum is invalid          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find        Find inbound entry in entry list  */ 
/*    _nx_nat_ip_packet_send            Send packet to private interface  */
/*    nx_packet_release                 Release the packet                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_packet     Process inbound packets          */ 
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
static UINT  _nx_nat_process_inbound_UDP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{

UINT                            status;     
UINT                            entry_found;
USHORT                          old_port;
USHORT                          new_port;
ULONG                           old_address;
ULONG                           new_address;
USHORT                          checksum;
ULONG                           compute_checksum; 
ULONG                           next_hop_address;
NX_UDP_HEADER                   *udp_header_ptr; 
NX_NAT_TRANSLATION_ENTRY        *record_entry;


    /* Initialize local variables. */
    compute_checksum = 1;
    record_entry = NX_NULL;  
    entry_found = NX_TRUE;                           

    /* Pickup the pointer to the head of the UDP packet.  */  
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length -= sizeof(NX_IPV4_HEADER);
    udp_header_ptr =  (NX_UDP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);

    /* For little endian processors, adjust byte order for big endianness. */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);                                                                                                                              

    /* Find the inbound entry, set the packet private interface and next hop address.  */
    status = _nx_nat_inbound_entry_find(nat_ptr, packet_ptr, entry_ptr, &record_entry, &next_hop_address);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {                

#ifndef NX_DISABLE_NAT_INFO
        /* Update the count.  */
        nat_ptr -> forwarded_packets_dropped++;
#endif
                          
        /* Check the status.  */
        if (status == NX_NAT_ENTRY_NOT_FOUND)
        {

            /* Set the entry found flag.  */
            entry_found = NX_FALSE;
        }
        else
        {         

            /* Release the packet */
            nx_packet_release(packet_ptr);

            /* Return completion status. */
            return NX_SUCCESS;   
        }
    } 
    else
    {     
                                                   
        /* Update the destination port if the NAT device mapped it to another port. */
        if (record_entry -> external_port != record_entry -> local_port)
        {

            /* Translate the destination UDP port to the private host port.  If translation 
               does not involve port number this will essentially be the original port number. */
            udp_header_ptr -> nx_udp_header_word_0 =  ((ULONG) (udp_header_ptr -> nx_udp_header_word_0 & ~NX_LOWER_16_MASK)) |
                                                       ((ULONG) record_entry -> local_port);
        }                

#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
        if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM)
            compute_checksum = 0;
        else
            compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
                             
        /* Get the checksum.  */
        checksum = udp_header_ptr -> nx_udp_header_word_1 & NX_LOWER_16_MASK;

        /*  UDP headers with 0 checksum should not be modified, RFC3022, Section4.1, Page8.  */
        if (checksum == 0)
            compute_checksum = 0;                                  

        /* Check the checksum.  */
        if(compute_checksum)
        {           

            /* Check whether the port is updated.   */
            if (record_entry -> external_port != record_entry -> local_port)
            {
                                     
                /* Set the old port and new port.  */
                old_port = record_entry -> external_port;
                new_port = record_entry -> local_port;

                /* Adjust the checksum for port.  */
                _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_port, sizeof(USHORT), (UCHAR *)&new_port, sizeof(USHORT));
            }
                                              
            /* Set the old address and new address.  */
            old_address = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;
            new_address = record_entry -> local_ip_address;     

            /* Adjust the checksum for address.  */
            _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_address, sizeof(LONG), (UCHAR *)&new_address, sizeof(LONG)); 

            /* OK to clear the UDP checksum field to zero before the checksum update. */
            udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 & ~NX_LOWER_16_MASK;  

            /* Place the checksum into the first header word.  */
            udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 | checksum; 
        }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {

            /* OK to clear the UDP checksum field to zero before the checksum update. */
            udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 & ~NX_LOWER_16_MASK; 

            /* Set the flag.  */
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    }

    /* Swap UDP header byte order back to big endian before sending. For big endian processors,
       this will have no effect. */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);                                                    
                          
    /* Check the packet consumed flag.  */
    if (entry_found == NX_FALSE)
    {

        /* Stop processing, recover packet length to Let NetXDuo process this packet.  */  
        packet_ptr -> nx_packet_prepend_ptr -= sizeof(NX_IPV4_HEADER);        
        packet_ptr -> nx_packet_length += sizeof(NX_IPV4_HEADER);
                          
        /* Let NetXDuo process this pcaket. */
        return(NX_NAT_PACKET_CONSUMED_FAILED);
    }
              
    /* Send the UDP packet onto the private host. */
    _nx_nat_ip_packet_send(nat_ptr, packet_ptr, record_entry, NX_NAT_INBOUND_PACKET, next_hop_address);     

    /* Return completion status. */
    return NX_SUCCESS;
}

    
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_ICMP_packet                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */   
/*   This function finds the local IP address:port for an incoming packet */
/*   in the NAT translation table and replaces the global packet          */
/*   IP address and queryID with the local IP address and queryID.        */
/*   If none is found, rejects the packet. Otherwise it updates the ICMP  */
/*   checksum with the changed IP address and port. The packet is then    */
/*   sent to private host.                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                            Pointer to the NAT server        */ 
/*    packet_ptr                         Pointer to the packet to process */  
/*    entry_ptr                          Pointer to the entry             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_SUCCESS                         Successful completion status     */
/*                                        (packet is not necessarily      */
/*                                        forwarded)                      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_nat_inbound_entry_find        Find inbound entry in entry list  */ 
/*    _nx_nat_ip_packet_send            Send packet to private interface  */
/*    nx_packet_release                 Release the packet                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_process_inbound_packet     Process inbound packets          */ 
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
static UINT  _nx_nat_process_inbound_ICMP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{
UINT                        status;          
UINT                        entry_found;
ULONG                       sequence;
UINT                        type;  
USHORT                      old_port;
USHORT                      new_port;
USHORT                      checksum;
ULONG                       compute_checksum;
ULONG                       next_hop_address;  
NX_ICMP_HEADER              *icmp_header_ptr;  
NX_NAT_TRANSLATION_ENTRY    *record_entry;


    /* Initialize local variables. */
    compute_checksum = 1;
    record_entry = NX_NULL; 
    entry_found = NX_TRUE;

    /* Get a pointer to the ICMP header. */      
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length -= sizeof(NX_IPV4_HEADER);
    icmp_header_ptr = (NX_ICMP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

    /* Adjust ICMP header byte order for endianness. */
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_1);                                                                       
    
    /* Extract the ICMP type and code. */
    type = icmp_header_ptr -> nx_icmp_header_word_0 >> 24;     
            
    /* Find the inbound entry, set the packet private interface and next hop address.  */
    status = _nx_nat_inbound_entry_find(nat_ptr, packet_ptr, entry_ptr, &record_entry, &next_hop_address);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

#ifndef NX_DISABLE_NAT_INFO
        /* Update the count.  */
        nat_ptr -> forwarded_packets_dropped++;
#endif
                    
        /* Check the status.  */
        if (status == NX_NAT_ENTRY_NOT_FOUND)
        {

            /* Set the entry found flag.  */
            entry_found = NX_FALSE;
        }
        else
        {         

            /* Release the packet */
            nx_packet_release(packet_ptr);

            /* Return completion status. */
            return NX_SUCCESS;   
        }
    } 
    else
    {     

        /* Check the type.  */
        if ((type != NX_ICMP_ECHO_REPLY_TYPE) &&
            (type != NX_ICMP_ECHO_REQUEST_TYPE))
        {         

#ifndef NX_DISABLE_NAT_INFO
            /* Unknown ICMP packet. Drop the packet and bail! */
            nat_ptr -> forwarded_packets_dropped++;
#endif

            nx_packet_release(packet_ptr);

            return NX_SUCCESS;
        }                            

        /* Now we have to translate the packet destination for private host address and 
           update packet header checksum. */  
        if ((type == NX_ICMP_ECHO_REPLY_TYPE) && (record_entry -> external_port != record_entry -> local_port))
        {                       

            /* Restore the local host ICMP Query ID.  */
            sequence = icmp_header_ptr -> nx_icmp_header_word_1 & NX_LOWER_16_MASK;

            /* Restore the local host ICMP Query ID from the 'source port' field in the NAT table entry. */
            icmp_header_ptr -> nx_icmp_header_word_1 = (ULONG)(record_entry -> local_port << NX_SHIFT_BY_16) | sequence;    

#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
            if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM)
                compute_checksum = 0;
            else
                compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
            if(compute_checksum)
            {

                /* Set the old checksum.  */
                checksum = icmp_header_ptr -> nx_icmp_header_word_0 & NX_LOWER_16_MASK;

                /* Set the old port and new port.  */
                old_port = entry_ptr -> external_port;
                new_port = record_entry -> local_port;

                /* Adjust the checksum.  */
                _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_port, sizeof(USHORT), (UCHAR *)&new_port, sizeof(USHORT)); 

                /* Ok to zero out the checksum because we'll replace it with an updated checksum. */
                icmp_header_ptr -> nx_icmp_header_word_0 = icmp_header_ptr -> nx_icmp_header_word_0 & ~NX_LOWER_16_MASK;

                /* Place the checksum into the first header word.  */
                icmp_header_ptr -> nx_icmp_header_word_0 = icmp_header_ptr -> nx_icmp_header_word_0 | checksum; 
            }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
            else
            {

                /* Set the checksum to zero. */
                icmp_header_ptr -> nx_icmp_header_word_0 = icmp_header_ptr -> nx_icmp_header_word_0 & ~NX_LOWER_16_MASK;

                /* Set the flag.  */
                packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM;
            }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
        }                  
    }                    

    /* If NX_LITTLE_ENDIAN is defined, the headers need to be swapped to match 
       that of the data area.  */
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_1);                                       
    
    /* Check the packet consumed flag.  */
    if (entry_found == NX_FALSE)
    {

        /* Stop processing, recover packet length to Let NetXDuo process this packet.  */  
        packet_ptr -> nx_packet_prepend_ptr -= sizeof(NX_IPV4_HEADER);        
        packet_ptr -> nx_packet_length += sizeof(NX_IPV4_HEADER);
                                    
        /* Let NetXDuo process this pcaket. */
        return(NX_NAT_PACKET_CONSUMED_FAILED);
    }
                             
    /* Send the ICMP packet onto the private host. */
    _nx_nat_ip_packet_send(nat_ptr, packet_ptr, record_entry, NX_NAT_INBOUND_PACKET, next_hop_address); 

    /* Return packet send completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_outbound_packet                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function processes a packet from a local host bound for the     */
/*   external network.  If a matching entry for the private host is not   */
/*   found in the NAT translation table, one is created.  Packets are then*/
/*   directed to protocol specific handlers for detailed processing of the*/
/*   packet before sending out on the external network.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to the NAT server         */ 
/*    packet_ptr                        Pointer to the packet to process  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_NAT_INVALID_PROTOCOL          Unknown/unsupported protocol status*/
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_nat_process_outbound_TCP_packet                                  */
/*                                     Handler for outbound TCP packets   */
/*   _nx_nat_process_outbound_UDP_packet                                  */
/*                                     Handler for outbound UDP packets   */
/*   _nx_nat_process_outbound_ICMP_packet                                 */
/*                                     Handler for outbound ICMP packets  */
/*   nx_packet_release                 Release packet back to packet pool */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_nat_packet_process            Process packets forwarded to NAT by*/ 
/*                                         Netx                           */
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
static UINT  _nx_nat_process_outbound_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr)
{

UCHAR                    protocol;  
NX_IPV4_HEADER           *ip_header_ptr;
NX_NAT_TRANSLATION_ENTRY translation_entry;


    /* Set up an actual IP header pointer. */
    ip_header_ptr =  (NX_IPV4_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);   
                                    
    /* Determine what protocol the current IP datagram is.  */
    protocol =  (ip_header_ptr -> nx_ip_header_word_2 >> 16) & 0xFF; 

    /* Set up the search criteria in the NAT translation table. */
    memset(&translation_entry, 0, sizeof(NX_NAT_TRANSLATION_ENTRY));
    translation_entry.local_ip_address = ip_header_ptr -> nx_ip_header_source_ip;
    translation_entry.peer_ip_address = ip_header_ptr -> nx_ip_header_destination_ip;  
    translation_entry.protocol = protocol;    

    /* Direct the packet to a protocol specific handler. */
    switch (protocol)        
    {

        case  NX_PROTOCOL_TCP:
            /* Process the packet for TCP protocol. */
            _nx_nat_process_outbound_TCP_packet(nat_ptr, packet_ptr, &translation_entry);
            break;

        case  NX_PROTOCOL_UDP:
            /* Process the packet for UDP protocol. */
            _nx_nat_process_outbound_UDP_packet(nat_ptr, packet_ptr, &translation_entry);
            break;

        case  NX_PROTOCOL_ICMP:
            /* Process the packet for ICMP (including error message packets) protocol. */
            _nx_nat_process_outbound_ICMP_packet(nat_ptr, packet_ptr, &translation_entry);
            break;

        default:
        {

#ifndef NX_DISABLE_NAT_INFO
            /* Update the count.  */
            nat_ptr -> forwarded_packets_dropped++;
#endif

            /* Toss the IP packet since we don't know what to do with it! */
            nx_packet_release(packet_ptr);  

            return NX_NAT_INVALID_PROTOCOL;
        }
    }

    /* Return completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_outbound_TCP_packet                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function processes a TCP packet from a local host on the private*/
/*   network destined for the external network with a global IP address   */
/*   and port.  The source IP address and port are replaced by NAT with a */
/*   global IP address:port. NAT then updates the TCP header checksum.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                         Pointer to the NAT server           */ 
/*    packet_ptr                      Pointer to the packet to process    */ 
/*    entry_ptr                       Pointer to the entry                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_nat_outbound_entry_find       Find outbound entry in entry list */ 
/*    _nx_nat_ip_packet_send            Send packet to private interface  */
/*    nx_packet_release                 Release the packet                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
static UINT  _nx_nat_process_outbound_TCP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{                      

UINT                        status;  
USHORT                      old_port;
USHORT                      new_port;
ULONG                       old_address;
ULONG                       new_address;
USHORT                      checksum;
ULONG                       compute_checksum; 
ULONG                       next_hop_address;
NX_TCP_HEADER               *tcp_header_ptr;
NX_NAT_TRANSLATION_ENTRY    *record_entry; 

                                   
    /* Initialize local variables. */
    compute_checksum = 1;    

    /* Pickup the pointer to the head of the UDP packet.  */           
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length -= sizeof(NX_IPV4_HEADER);
    tcp_header_ptr =  (NX_TCP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);

    /* Adjust byte order for endianness.  */      
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_sequence_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_acknowledgment_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_3);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);
             
    /* Find the outbound entry, set the packet global interface and next hop address.  */
    status = _nx_nat_outbound_entry_find(nat_ptr, packet_ptr, entry_ptr, &record_entry, &next_hop_address); 

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
              
#ifndef NX_DISABLE_NAT_INFO
        /* Increase the session count of dropped forwarded packets. */
        nat_ptr -> forwarded_packets_dropped++;
#endif

        /* Release the packet back to the packet pool. */
        nx_packet_release(packet_ptr);            

        /* Return the error status. */
        return status;
    }

    /* Update the source port if the NAT device mapped it to another port. */
    if (record_entry -> external_port != record_entry -> local_port)
    {

        /* Yes, write to the upper bits of the TCP word. */
        tcp_header_ptr -> nx_tcp_header_word_0 = (tcp_header_ptr -> nx_tcp_header_word_0 & NX_LOWER_16_MASK) |
                                                 (((ULONG) (record_entry -> external_port)) << NX_SHIFT_BY_16);         
    }                      

#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
    if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM)
        compute_checksum = 0;
    else
        compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    if(compute_checksum)
    {

        /* Get the old checksum.  */
        checksum = (USHORT)(tcp_header_ptr -> nx_tcp_header_word_4 >> NX_SHIFT_BY_16);

        /* Check whether the port is updated.   */
        if (record_entry -> external_port != record_entry -> local_port)
        {

            /* Set the old port and new port.  */
            old_port = record_entry -> local_port;
            new_port = record_entry -> external_port;

            /* Adjust the checksum for port.  */
            _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_port, sizeof(USHORT), (UCHAR *)&new_port, sizeof(USHORT));
        }
                                    
        /* Set the old address and new address.  */
        old_address = record_entry -> local_ip_address;
        new_address = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;

        /* Adjust the checksum for address.  */
        _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_address, sizeof(LONG), (UCHAR *)&new_address, sizeof(LONG)); 

        /* OK to clear the TCP checksum field to zero before the checksum update. */
        tcp_header_ptr -> nx_tcp_header_word_4 = tcp_header_ptr -> nx_tcp_header_word_4 & NX_LOWER_16_MASK;  

        /* Place the checksum into the first header word.  */
        tcp_header_ptr -> nx_tcp_header_word_4 = tcp_header_ptr -> nx_tcp_header_word_4 | (ULONG)(checksum << NX_SHIFT_BY_16); 
    }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {

        /* OK to clear the TCP checksum field to zero before the checksum update. */
        tcp_header_ptr -> nx_tcp_header_word_4 = tcp_header_ptr -> nx_tcp_header_word_4 & NX_LOWER_16_MASK;  

        /* Set the flag.  */
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Swap byte order back to big endian before sending if little endian is specified. */
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_sequence_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_acknowledgment_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_3);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);       
                                          
    /* Send the TCP packet onto the global host. */
    _nx_nat_ip_packet_send(nat_ptr,packet_ptr, record_entry, NX_NAT_OUTBOUND_PACKET, next_hop_address);   

    return NX_SUCCESS;
}
                     
                                          
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_outbound_UDP_packet                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function processes a UDP packet from a local host destined for  */
/*   the external network; NAT replaces the private IP address:port with a*/
/*   global IP address:port. NAT updates the UDP header checksum field for*/
/*   all packets having a non zero UDP packet checksum.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                         Pointer to the NAT server           */ 
/*    packet_ptr                      Pointer to the packet to process    */  
/*    entry_ptr                       Pointer to the entry                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_NAT_ZERO_UDP_CHECKSUM         Illegal zero UDP header checksum   */
/*    NX_NAT_BAD_UDP_CHECKSUM          UDP header checksum is invalid     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */    
/*    _nx_nat_outbound_entry_find       Find outbound entry in entry list */ 
/*    _nx_nat_ip_packet_send            Send packet to private interface  */
/*    nx_packet_release                 Release the packet                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
static UINT  _nx_nat_process_outbound_UDP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{

UINT                        status;
USHORT                      old_port;
USHORT                      new_port;
ULONG                       old_address;
ULONG                       new_address;
USHORT                      checksum;
ULONG                       compute_checksum; 
ULONG                       next_hop_address;      
NX_UDP_HEADER               *udp_header_ptr;   
NX_NAT_TRANSLATION_ENTRY    *record_entry;   

    /* Initialize local variables. */
    compute_checksum = 1;   

    /* Pickup the pointer to the head of the UDP packet.  */           
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length -= sizeof(NX_IPV4_HEADER);
    udp_header_ptr =  (NX_UDP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);

    /* For little endian processors, swap byte order to little endian. */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);   
                 
    /* Find the outbound entry, set the packet global interface and next hop address.  */
    status = _nx_nat_outbound_entry_find(nat_ptr, packet_ptr, entry_ptr, &record_entry, &next_hop_address);  

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
             
#ifndef NX_DISABLE_NAT_INFO
        /* Increase the session count of dropped forwarded packets. */
        nat_ptr -> forwarded_packets_dropped++;
#endif

        /* Release the packet back to the packet pool. */
        nx_packet_release(packet_ptr);            

        /* Return the error status. */
        return status;
    }

    /* OK to zero out the checksum field so we can update the header with a new checksum later. */
    udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 & ~NX_LOWER_16_MASK;    
                               
    /* Did NAT assign a global UDP source port? */
    if (record_entry -> external_port != record_entry -> local_port)
    {

        /* Yes, so replace local host UDP port with NAT global inside UDP port in the header.  */   
        udp_header_ptr -> nx_udp_header_word_0 = (udp_header_ptr -> nx_udp_header_word_0 & NX_LOWER_16_MASK) |
                                                 (((ULONG) (record_entry -> external_port)) << NX_SHIFT_BY_16);   
    }           
                    
#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
    if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM)
        compute_checksum = 0;
    else
        compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Get the old checksum.  */
    checksum = udp_header_ptr -> nx_udp_header_word_1 & NX_LOWER_16_MASK;

    /*  UDP headers with 0 checksum should not be modified, RFC3022, Section4.1, Page8.  */
    if (checksum == 0)
        compute_checksum = 0;   

    /* Compute the checksum.  */
    if(compute_checksum)
    {                                                             

        /* Check whether the port is updated.   */
        if (record_entry -> external_port != record_entry -> local_port)
        {

            /* Set the old port and new port.  */
            old_port = record_entry -> local_port;
            new_port = record_entry -> external_port;

            /* Adjust the checksum for port.  */
            _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_port, sizeof(USHORT), (UCHAR *)&new_port, sizeof(USHORT));
        }
                                    
        /* Set the old address and new address.  */
        old_address = record_entry -> local_ip_address;
        new_address = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;

        /* Adjust the checksum for address.  */
        _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_address, sizeof(LONG), (UCHAR *)&new_address, sizeof(LONG)); 

        /* OK to clear the UDP checksum field to zero before the checksum update. */
        udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 & ~NX_LOWER_16_MASK;  

        /* Place the checksum into the first header word.  */
        udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 | checksum; 
    }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
                                          
        /* OK to clear the UDP checksum field to zero before the checksum update. */
        udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 & ~NX_LOWER_16_MASK; 

        /* Set the flag.  */
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* If NX_LITTLE_ENDIAN is defined, the headers need to be swapped to match 
       that of the data area.  */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);   
                      
    /* Send the UDP packet onto the global host. */
    _nx_nat_ip_packet_send(nat_ptr, packet_ptr, record_entry, NX_NAT_OUTBOUND_PACKET, next_hop_address); 
       
    /* Return packet send completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_process_outbound_ICMP_packet                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function processes a ICMP packet from a local network host for  */
/*   sending out onto the external network. NAT replaces the private      */
/*   source IP address and local query ID with a global source IP address */
/*   and query ID.  NAT then recomputes the ICMP header checksum.         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                         Pointer to the NAT server           */ 
/*    packet_ptr                      Pointer to the packet to process    */ 
/*    entry_ptr                       Pointer to the entry                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */ 
/*    NX_SUCCESS                       Successful completion status       */
/*    NX_NAT_BAD_ICMP_CHECKSUM         Packet failed ICMP checksum check  */
/*    NX_NAT_INVALID_IP_HEADER         Packet has an invalid IP header    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_nat_outbound_entry_find       Find outbound entry in entry list */ 
/*    _nx_nat_ip_packet_send            Send packet to private interface  */
/*    nx_packet_release                 Release the packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
static UINT  _nx_nat_process_outbound_ICMP_packet(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{

UINT               status;
ULONG              sequence;
UINT               is_icmp_error_msg;
NX_ICMP_HEADER     *icmp_header_ptr;
UINT               type;    
USHORT             old_port;
USHORT             new_port;
USHORT             checksum;
ULONG              compute_checksum;  
ULONG              next_hop_address;
NX_NAT_TRANSLATION_ENTRY    *record_entry; 
                                              

    /* Initialize local variables. */
    compute_checksum = 1;

    /* Pickup the pointer to the head of the ICMP packet.  */
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length -= sizeof(NX_IPV4_HEADER);
    icmp_header_ptr =  (NX_ICMP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

    /* Swap ENDian-ness for our ICMP header. We've only swapped the IP header data so far. */
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_1);
                                                                        
    /* Extract the ICMP type and code. */
    type = icmp_header_ptr -> nx_icmp_header_word_0 >> 24;
            
    /* Determine if this outbound packet an ICMP error message packet. */
    _nx_nat_packet_is_icmp_error_message(packet_ptr, &is_icmp_error_msg);

    /* Is the this an error message packet? */
    if (is_icmp_error_msg)
    {

#ifndef NX_DISABLE_NAT_INFO
        /* Drop the packet and bail! */
        nat_ptr -> forwarded_packets_dropped++;
#endif

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

         /* Return completion status. */
         return NX_SUCCESS;
    }
                             
    /* Find the outbound entry, set the packet global interface and next hop address.  */
    status = _nx_nat_outbound_entry_find(nat_ptr, packet_ptr, entry_ptr, &record_entry, &next_hop_address); 

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
              
#ifndef NX_DISABLE_NAT_INFO
        /* Increase the session count of dropped forwarded packets. */
        nat_ptr -> forwarded_packets_dropped++;
#endif

        /* Release the packet back to the packet pool. */
        nx_packet_release(packet_ptr);            

        /* Return the error status. */
        return status;
    }                 

    /* Update the ICMP Query ID ("port") if the NAT device mapped it to another Query ID 
       but not if this is a local host responding to a REQUEST ICMP packet, in which case 
       it must keep the same Query ID. */
    if ((type != NX_ICMP_ECHO_REPLY_TYPE) && (record_entry -> external_port != record_entry -> local_port))
    {

        /* Pick up the ICMP sequence number. */
        sequence =  icmp_header_ptr -> nx_icmp_header_word_1 & NX_LOWER_16_MASK;

        /* Set the ICMP Query ID in the ICMP header.  */
        icmp_header_ptr -> nx_icmp_header_word_1 = (ULONG) (record_entry -> external_port << NX_SHIFT_BY_16) | sequence;     
                                                                                                                  
#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
        if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM)
            compute_checksum = 0;
        else
            compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
        if(compute_checksum)
        {

            /* Set the old checksum.  */
            checksum = icmp_header_ptr -> nx_icmp_header_word_0 & NX_LOWER_16_MASK;

            /* Set the old port and new port.  */
            old_port = record_entry -> local_port;
            new_port = record_entry -> external_port;

            /* Adjust the checksum.  */
            _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_port, sizeof(USHORT), (UCHAR *)&new_port, sizeof(USHORT)); 

            /* Ok to zero out the checksum because we'll replace it with an updated checksum. */
            icmp_header_ptr -> nx_icmp_header_word_0 = icmp_header_ptr -> nx_icmp_header_word_0 & ~NX_LOWER_16_MASK;

            /* Place the checksum into the first header word.  */
            icmp_header_ptr -> nx_icmp_header_word_0 = icmp_header_ptr -> nx_icmp_header_word_0 | checksum; 
        }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {
            
            /* Set the checksum to zero. */
            icmp_header_ptr -> nx_icmp_header_word_0 = icmp_header_ptr -> nx_icmp_header_word_0 & ~NX_LOWER_16_MASK;

            /* Set the flag.  */
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    }
                   
    /* If NX_LITTLE_ENDIAN is defined, the headers need to be swapped to match 
       that of the data area.  */
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(icmp_header_ptr -> nx_icmp_header_word_1);                             
 
    /* Send the ICMP packet onto the global host. */
    _nx_nat_ip_packet_send(nat_ptr, packet_ptr, record_entry, NX_NAT_OUTBOUND_PACKET, next_hop_address); 

    /* Return completion status. */
    return NX_SUCCESS;      
}

                 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_ip_packet_send                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function is the NAT equivalent of nx_ip_packet_send. It handles */ 
/*   all packets inbound or outbound of any protocol and forwards them    */
/*   directly to the driver.                                              */
/*                                                                        */ 
/*   This function also handles fragmented datagrams.                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    nat_ptr                             Pointer to NAT server           */  
/*    packet_ptr                          Pointer to packet               */
/*    entry_ptr                           Pointer to NAT entry            */ 
/*    packet_type                         Packet type(inbound/outbound)   */
/*    next_hop_address                    Next hop address to target      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status.       */
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_NAT_FRAGMENT_QUEUE_NOT_FOUND     Fragment queue for datagram     */
/*                                            not found                   */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   nx_packet_release                    Release the packet              */ 
/*   _nx_packet_data_append               Append the overflow data        */
/*   _nx_nat_checksum_adjust              Adjust checksum for NAT changes */
/*                                            to IP header                */
/*   _nx_ip_driver_packet_send            Forward packet to driver to send*/  
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_nat_process_outbound_TCP_packet  Process outbound TCP packet     */ 
/*   _nx_nat_process_outbound_UDP_packet  Process outbound UDP packet     */ 
/*   _nx_nat_process_outbound_ICMP_packet Process outbound ICMP packet    */ 
/*   _nx_nat_process_inbound_TCP_packet   Process outbound TCP packet     */ 
/*   _nx_nat_process_inbound_UDP_packet   Process outbound UDP packet     */ 
/*   _nx_nat_process_inbound_ICMP_packet  Process outbound ICMP packet    */ 
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
static VOID  _nx_nat_ip_packet_send(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, UCHAR packet_type, ULONG next_hop_address) 
{

ULONG               fragment_bits;  
ULONG               old_address;
ULONG               new_address;
ULONG               old_fragment;
ULONG               new_fragment;
USHORT              checksum;
ULONG               compute_checksum = 1;
ULONG               destination_ip; 
UINT                status;
NX_IPV4_HEADER      *ip_header_ptr; 


    /* Pickup the pointer to the head of the ICMP packet.  */ 
    packet_ptr -> nx_packet_prepend_ptr -= sizeof(NX_IPV4_HEADER);
    packet_ptr -> nx_packet_length += sizeof(NX_IPV4_HEADER);
    ip_header_ptr =  (NX_IPV4_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);

    /* Check the packet type.  */
    if (packet_type == NX_NAT_OUTBOUND_PACKET)
    {
        
        /* Update the source ip address.  */  
        ip_header_ptr -> nx_ip_header_source_ip = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;   
        
        /* Set the local address and external address.  */
        old_address = entry_ptr -> local_ip_address;
        new_address = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;
    }
    else
    {
        /* Update the destination ip address.  */   
        ip_header_ptr -> nx_ip_header_destination_ip = entry_ptr -> local_ip_address;

        /* Set the old address and new address.  */
        old_address = nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index].nx_interface_ip_address;
        new_address = entry_ptr -> local_ip_address;    
    }                           
          
    /* Record the destination IP address.  */
    destination_ip = ip_header_ptr -> nx_ip_header_destination_ip;

    /* Record the fragment bit.  */
    fragment_bits = (ip_header_ptr -> nx_ip_header_word_1 & NX_IP_DONT_FRAGMENT);
    old_fragment = ip_header_ptr -> nx_ip_header_word_1 & NX_LOWER_16_MASK;
    new_fragment = old_fragment;

    /* Check if the packet is fragmented and if the fragment field is not zero.  */
    if ((fragment_bits != NX_IP_DONT_FRAGMENT) && (old_fragment != 0))
    {

        /* Clear the fragment field.  */ 
        ip_header_ptr -> nx_ip_header_word_1 = ip_header_ptr -> nx_ip_header_word_1 & ~NX_LOWER_16_MASK;
        new_fragment = 0;
    }

#ifdef NX_ENABLE_INTERFACE_CAPABILITY    
    if(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM)
        compute_checksum = 0;
    else
        compute_checksum = 1;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */ 
                                                      
    /* Compute the checksum.  */
    if(compute_checksum)
    {                     

        /* Get the checksum.  */
        checksum = ip_header_ptr -> nx_ip_header_word_2 & NX_LOWER_16_MASK;

        /* Adjust the checksum for address.  */
        _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_address, sizeof(LONG), (UCHAR *)&new_address, sizeof(LONG));

        /* Check if the fragment field is updated.  */
        if (old_fragment != new_fragment)
        {

            /* Adjust the checksum for fragment field.  */
            _nx_nat_checksum_adjust((UCHAR *)&checksum, (UCHAR *)&old_fragment, sizeof(LONG), (UCHAR *)&new_fragment, sizeof(LONG));
        }

        /* Clear the checksum value.  */
        ip_header_ptr -> nx_ip_header_word_2 = ip_header_ptr -> nx_ip_header_word_2 & ~NX_LOWER_16_MASK;     

        /* Place the checksum into the header word.  */
        ip_header_ptr -> nx_ip_header_word_2 = ip_header_ptr -> nx_ip_header_word_2 | checksum; 
    }   
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
                                        
        /* Clear the checksum value.  */
        ip_header_ptr -> nx_ip_header_word_2 = ip_header_ptr -> nx_ip_header_word_2 & ~NX_LOWER_16_MASK;     

        /* Set the flag.  */
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
    swap the endian of the IP header.  */
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);

    /* Check if the packet can fill physical header.  */
    status = _nx_packet_data_adjust(packet_ptr, NX_PHYSICAL_HEADER);

    /* Check status.  */
    if (status)
    {

#ifndef NX_DISABLE_NAT_INFO
        /* Update the count.  */
        nat_ptr -> forwarded_packets_dropped ++;
#endif

        /* Release the packet. */
        _nx_packet_release(packet_ptr);
        return;
    }

#ifndef NX_DISABLE_NAT_INFO
    /* Update the packet sent count.  */
    nat_ptr -> forwarded_packets_sent++;
#endif

    /* Call the function to directly forward the packet.  */ 
    _nx_ip_driver_packet_send(nat_ptr -> nx_nat_ip_ptr, packet_ptr, destination_ip, fragment_bits, next_hop_address);   
    return;
}     
    

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function finds the inbound entry in NAT translation entry list, */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                             Pointer to NAT server           */  
/*    packet_ptr                          Pointer to packet               */
/*    entry_ptr                           Pointer to NAT entry            */  
/*    entry_ptr                           Pointer to the matched entry    */
/*    next_hop_address                    The next hop address for route  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */   
/*    _nx_nat_utility_get_source_port   Extract source port from packet   */
/*    _nx_nat_utility_get_destination_port                                */ 
/*                                      Extract destination port from     */
/*                                           packet                       */
/*    _nx_nat_entry_create              Create entry for packet in NAT    */ 
/*                                          translation table             */
/*    _nx_nat_entry_find                Find the entry                    */
/*    _nx_ip_route_find                 Find the suitable interface       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */  
/*   _nx_nat_process_inbound_TCP_packet   Process outbound TCP packet     */ 
/*   _nx_nat_process_inbound_UDP_packet   Process outbound UDP packet     */ 
/*   _nx_nat_process_inbound_ICMP_packet  Process outbound ICMP packet    */ 
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
static UINT  _nx_nat_inbound_entry_find(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, 
                                        NX_NAT_TRANSLATION_ENTRY **matched_entry_ptr, ULONG *next_hop_address)
{

UINT                            status;
ULONG                           timeout;
NX_NAT_TRANSLATION_ENTRY       *record_entry;
                   

    /* Check the protocol.  */
    if ((entry_ptr -> protocol == NX_PROTOCOL_TCP) || (entry_ptr -> protocol == NX_PROTOCOL_UDP))
    {

        /* Get sender's source port. */
        _nx_nat_utility_get_source_port(packet_ptr, entry_ptr -> protocol, &entry_ptr -> peer_port);
    }

    /* Get the destination port from header data. */
    _nx_nat_utility_get_destination_port(packet_ptr, entry_ptr -> protocol, &entry_ptr -> external_port);

    /* Check the timeout for all dynamic entries.  */
    _nx_nat_entry_timeout_check(nat_ptr);

    /* Now search the table for a matching NAT translation entry. */
    _nx_nat_entry_find(nat_ptr, entry_ptr, &record_entry, NX_NAT_INBOUND_PACKET, NX_TRUE); 
              
    /* Was a matching entry found in our translation table? */
    if (!record_entry)
    {        

        /* Next see if the destination is a local host server.  */
    
        /* Find a 'server' for packet destination IP address and port */
        _nx_nat_entry_find(nat_ptr, entry_ptr, &record_entry, NX_NAT_INBOUND_PACKET, NX_FALSE); 
                        
        /* Check the record entry.  */
        if (record_entry)
        {

            /* We found a matching server entry in the table. Now create an 
               entry specifically for this packet. */
    
            /* Get the private IP address from the entry we just found, and apply to the new entry we're creating. */
            entry_ptr -> local_ip_address = record_entry -> local_ip_address;
    
            /* Check the private inside port.  */
            if (record_entry -> local_port == 0)
            {

                /* Set the private inside port same as global inside port.  */
                entry_ptr -> local_port = entry_ptr -> external_port; 
            }
            else
            {

                /* Set private inside port. */
                entry_ptr -> local_port = record_entry -> local_port;
            }

            /* Set the entry expiration timeout. */
            if (entry_ptr -> protocol == NX_PROTOCOL_TCP) 
            {

                /* TCP session.  */
                timeout = NX_NAT_TCP_SESSION_TIMEOUT;
            }
            else
            {

                /* Non-TCP session.  */
                timeout = NX_NAT_NON_TCP_SESSION_TIMEOUT;
            }

            /* Now create the entry. */
            status = _nx_nat_entry_create(nat_ptr, entry_ptr -> protocol,  
                                          entry_ptr -> local_ip_address, 
                                          entry_ptr -> peer_ip_address,
                                          entry_ptr -> local_port, 
                                          entry_ptr -> external_port, 
                                          entry_ptr -> peer_port, 
                                          timeout, 
                                          &record_entry);
    
            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                         
                /* Return status.*/
                return status;
            }
        }
        else
        {

            /* Return status.*/
            return (NX_NAT_ENTRY_NOT_FOUND);           
        }
    }
                                                                   
    /* Clear the packet interface as private interface.  */
    packet_ptr -> nx_packet_address.nx_packet_interface_ptr = NX_NULL;   
                         
    /* Set the inbound interface and next hop address.  */
    if(_nx_ip_route_find(nat_ptr -> nx_nat_ip_ptr, record_entry -> local_ip_address, 
                         &packet_ptr -> nx_packet_address.nx_packet_interface_ptr, next_hop_address) != NX_SUCCESS)
    {

        /* No suitable private interface configured. */   
                                                 
        /* Return the error status. */
        return (NX_NAT_ROUTE_FIND_ERROR);
    }

    /* Set the matched entry pointer.  */
    *matched_entry_ptr = record_entry;

    /* Return success.  */ 
    return (NX_SUCCESS);
}

       
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_outbound_entry_find                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*   This function finds the outbound entry in NAT translation entry list.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                             Pointer to NAT server           */  
/*    packet_ptr                          Pointer to packet               */
/*    entry_ptr                           Pointer to NAT entry            */  
/*    entry_ptr                           Pointer to the matched entry    */
/*    next_hop_address                    The next hop address for route  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_nat_utility_get_source_port   Extract source port from packet   */
/*    _nx_nat_utility_get_destination_port                                */ 
/*                                      Extract destination port from     */
/*                                           packet                       */
/*    _nx_nat_entry_create              Create entry for packet in NAT    */ 
/*                                          translation table             */
/*    _nx_nat_entry_find                Find the entry                    */
/*    _nx_ip_route_find                 Find the suitable interface       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */   
/*   _nx_nat_process_outbound_TCP_packet  Process outbound TCP packet     */ 
/*   _nx_nat_process_outbound_UDP_packet  Process outbound UDP packet     */ 
/*   _nx_nat_process_outbound_ICMP_packet Process outbound ICMP packet    */ 
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
static UINT  _nx_nat_outbound_entry_find(NX_NAT_DEVICE *nat_ptr, NX_PACKET *packet_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, 
                                         NX_NAT_TRANSLATION_ENTRY **matched_entry_ptr, ULONG *next_hop_address)
{
                                
UINT                        status;
ULONG                       timeout;
NX_NAT_TRANSLATION_ENTRY   *record_entry; 

         
    /* Get source ports from the header. */
    _nx_nat_utility_get_source_port(packet_ptr, entry_ptr -> protocol, &(entry_ptr -> local_port));                                  
                                                    
    /* Check the protocol.  */
    if ((entry_ptr -> protocol == NX_PROTOCOL_TCP) || (entry_ptr -> protocol == NX_PROTOCOL_UDP))
    {

        /* Get destination port from the header. */
        _nx_nat_utility_get_destination_port(packet_ptr, entry_ptr -> protocol, &(entry_ptr -> peer_port));
    }
                         
    /* Check the timeout for all dynamic entries.  */
    _nx_nat_entry_timeout_check(nat_ptr);

    /* Search the table for a match. */
    _nx_nat_entry_find(nat_ptr, entry_ptr, &record_entry, NX_NAT_OUTBOUND_PACKET, NX_TRUE); 

    /* Was a matching table entry found? */
    if (!record_entry)
    {

        /* Find the available port. */
        status = _nx_nat_find_available_port(nat_ptr, entry_ptr -> protocol, &entry_ptr -> external_port);

        /* Check for error. */
        if (status != NX_SUCCESS)
        {

            return status;
        }

        /* Set the entry expiration timeout. */
        if (entry_ptr -> protocol == NX_PROTOCOL_TCP) 
        {

            /* TCP session.  */
            timeout = NX_NAT_TCP_SESSION_TIMEOUT;
        }
        else
        {

            /* Non-TCP session.  */
            timeout = NX_NAT_NON_TCP_SESSION_TIMEOUT;
        }

        /* Create an entry with NAT translation for IP address/port. */
        status = _nx_nat_entry_create(nat_ptr, entry_ptr -> protocol,  
                                      entry_ptr -> local_ip_address, 
                                      entry_ptr -> peer_ip_address, 
                                      entry_ptr -> local_port, 
                                      entry_ptr -> external_port, 
                                      entry_ptr -> peer_port,
                                      timeout, 
                                      &record_entry);

        /* Check for error. */
        if (status != NX_SUCCESS)
        {

            /* Return error status. */
            return status;
        }
    }           

    /* Set the packet interface as global interface.  */
    packet_ptr -> nx_packet_address.nx_packet_interface_ptr = &(nat_ptr -> nx_nat_ip_ptr -> nx_ip_interface[nat_ptr -> nx_nat_global_interface_index]); 
    
    /* Set the next hop address.  */
    if(_nx_ip_route_find(nat_ptr -> nx_nat_ip_ptr, record_entry -> peer_ip_address, &packet_ptr -> nx_packet_address.nx_packet_interface_ptr, next_hop_address) != NX_SUCCESS)
    {

        /* No suitable private interface configured. */   
                                                 
        /* Return the error status. */
        return (NX_NAT_ROUTE_FIND_ERROR);
    }

    /* Set the matched entry pointer.  */
    *matched_entry_ptr = record_entry;

    /* Return success.  */ 
    return (NX_SUCCESS);
}
            

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_entry_create                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function dynamically creates a NAT translation table entry      */
/*   and appends the entry to the  translation list.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */    
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    protocol                          Protocol of packets matching entry*/
/*    local_ip_address                  Entry's private IP address        */
/*    peer_ip_address                   Entry's external host IP          */
/*    local_port                        Entry's private port              */
/*    external_port                     Entry's global port               */
/*    peer_port                         Entry's external port (optional)  */
/*    response_timeout                  Entry expiration timeout          */ 
/*    match_entry_ptr                   Pointer to entry created          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_SUCCESS                        Successful completion status      */
/*    NX_NAT_TRANSLATION_TABLE_FULL     Table is full (max capacity)      */
/*    NX_NAT_INVALID_TABLE_ENTRY        Invalid criteria for table entry  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_entry_add                Add entry to linked list of entries*/
/*    memset                           Clear specified area of memory     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find        Find inbound entry in entry list  */ 
/*    _nx_nat_outbound_entry_find       Find outbound entry in entry list */ 
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
static UINT  _nx_nat_entry_create(NX_NAT_DEVICE *nat_ptr, UCHAR protocol, 
                                  ULONG local_ip_address, ULONG peer_ip_address, 
                                  USHORT local_port, USHORT  external_port, USHORT  peer_port,
                                  ULONG response_timeout, NX_NAT_TRANSLATION_ENTRY **match_entry_ptr)
{

NX_NAT_TRANSLATION_ENTRY *insert_entry_ptr = NX_NULL;

#ifdef NX_NAT_ENABLE_REPLACEMENT
NX_NAT_TRANSLATION_ENTRY *insert_previous_ptr = NX_NULL;
NX_NAT_TRANSLATION_ENTRY *entry_ptr = NX_NULL;
NX_NAT_TRANSLATION_ENTRY *previous_ptr = NX_NULL;
#endif /* NX_NAT_ENABLE_REPLACEMENT  */


    /* Initialize result to not found. */
    *match_entry_ptr = NX_NULL;     

    /* Perform some simple sanity checks on this entry. */ 
    
    /* Was an invalid IP address submitted? */
    if ((peer_ip_address == 0x0) ||
        (local_ip_address == 0x0))
    {

        return (NX_NAT_INVALID_ENTRY);
    }

    /* Check available entries. */
    if (nat_ptr -> nx_nat_dynamic_available_entries)
    {

        /* Get one available entry.  */
        insert_entry_ptr = nat_ptr -> nx_nat_dynamic_available_entry_head;

        /* Update the entry head.  */
        nat_ptr -> nx_nat_dynamic_available_entry_head = insert_entry_ptr -> next_entry_ptr;
    }
    else
    {
#ifdef NX_NAT_ENABLE_REPLACEMENT

        /* Get a pointer to the start of the entries in the translation table. */
        entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;

        /* Search the whole entry list to find the oldest non-TCP entry.  */
        while (entry_ptr)
        {

            /* Check the entry type and protocol.  */
            if ((entry_ptr -> translation_type == NX_NAT_STATIC_ENTRY) || (entry_ptr -> protocol == NX_PROTOCOL_TCP))
            {

                /* Ingore this entry, Get the next entry in the table. */
                previous_ptr = entry_ptr;
                entry_ptr = entry_ptr -> next_entry_ptr;
                continue;
            }
            else
            {

                /* Check if set the insert_entry_ptr(oldest entry).  */
                if (insert_entry_ptr == NX_NULL)
                {

                    /* Assume the first entry is the oldest entry.  */
                    insert_entry_ptr = entry_ptr;
                    insert_previous_ptr = previous_ptr;
                }
                else
                {

                    /* Compare the timestamp.  */
                    if (((INT)insert_entry_ptr -> response_timestamp - (INT)entry_ptr -> response_timestamp) > 0)
                    {

                        /* entry_ptr is an older entry, so update the insert_entry_ptr.  */
                        insert_entry_ptr = entry_ptr;
                        insert_previous_ptr = previous_ptr;
                    }
                }

                /* Get the next entry in the table. */
                previous_ptr = entry_ptr;
                entry_ptr = entry_ptr -> next_entry_ptr;
            }
        }

        /* Check if found the oldest non-TCP entry.  */
        if (insert_entry_ptr)
        {

            /* Yes, found it. Check if this is the first entry in the list. */
            if (insert_previous_ptr)
            {

                /* It is not, so link the previous entry around the entry we are deleting. */
                insert_previous_ptr -> next_entry_ptr = insert_entry_ptr -> next_entry_ptr;
            }
            else 
            {

                /* It is the first entry, so set the next pointer as the starting translation table entry. */
                nat_ptr -> nx_nat_dynamic_active_entry_head = insert_entry_ptr -> next_entry_ptr;
            }

            /* Update the entry count.  */
            nat_ptr -> nx_nat_dynamic_active_entries --;
            nat_ptr -> nx_nat_dynamic_available_entries ++;
        }
        else
        {
#endif /* NX_NAT_ENABLE_REPLACEMENT  */

            /* This service cache does not have room for the entry. */
            /* Invoke user-installed cache full notify function .*/
            if(nat_ptr -> nx_nat_cache_full_notify)
            {

                /* Call the callback function.  */
                (nat_ptr -> nx_nat_cache_full_notify)(nat_ptr);
            }

            /* Return error status.  */
            return (NX_NAT_CACHE_FULL);

#ifdef NX_NAT_ENABLE_REPLACEMENT
        }
#endif /* NX_NAT_ENABLE_REPLACEMENT  */
    }

    /* Initialize the allocated memory to NULL. */
    memset(insert_entry_ptr, 0, sizeof(NX_NAT_TRANSLATION_ENTRY));

    /* Assign the entry attributes. */ 
    insert_entry_ptr -> protocol = protocol;
    insert_entry_ptr -> local_ip_address = local_ip_address;
    insert_entry_ptr -> peer_ip_address = peer_ip_address;
    insert_entry_ptr -> local_port = local_port;
    insert_entry_ptr -> external_port = external_port;
    insert_entry_ptr -> peer_port = peer_port;
    insert_entry_ptr -> response_timeout = response_timeout; 

    /* Set the entry timestamp.  */
    insert_entry_ptr -> response_timestamp = tx_time_get();

    /* Set entry type to dynamically created. */
    insert_entry_ptr -> translation_type = NX_NAT_DYNAMIC_ENTRY;

    /* Append to the table. */
    _nx_nat_entry_add(nat_ptr, insert_entry_ptr);     

    /* Set a pointer to the newly created entry. */
    *match_entry_ptr = insert_entry_ptr;

    /* Return successful completion status. */
    return (NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_entry_add                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function adds a NAT translation entry to the NAT entry list     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */       
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    entry_ptr                         Pointer to NAT translation entry  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_create     Create inbound entry to NAT table  */ 
/*                                       before starting the NAT server   */
/*    _nx_nat_entry_create             Create and add entry to NAT table  */ 
/*                                         after starting the NAT server  */
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
static UINT  _nx_nat_entry_add(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr)
{


    /* Add this entry onto the table entry list.  */
    entry_ptr -> next_entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;
    nat_ptr -> nx_nat_dynamic_active_entry_head = entry_ptr;                                                                  

    /* Update the entry count.  */
    if (entry_ptr -> translation_type == NX_NAT_DYNAMIC_ENTRY)
    {

        /* Update the entry count.  */
        nat_ptr -> nx_nat_dynamic_active_entries ++;
        nat_ptr -> nx_nat_dynamic_available_entries --;
    }
    else                                                    
        nat_ptr -> nx_nat_static_active_entries ++;
   
    /* Return success status. */
    return(NX_SUCCESS);
}                           
                   

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_entry_find                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to find an entry in the NAT translation list */ 
/*    that matches the entry submitted by the caller.  If none is found it*/
/*    returns a null pointer. There is an option to skip entries          */
/*    designated for local hosts accepting packets from external hosts    */
/*    (e.g. servers).                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                     Pointer to NAT server                   */ 
/*    entry_to_match              Pointer to entry to match in the list   */ 
/*    match_entry_ptr             Pointer to matching entry in the list   */ 
/*    direction                   Forward direction(inbound/outbound)     */
/*    skip_inbound_init_entries   Skip entries for local hosts allowing   */ 
/*                                   initial packet from external source  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_time_get                      Get the system time                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find       Find inbound entry in entry list   */ 
/*    _nx_nat_outbound_entry_find      Find outbound entry in entry list  */ 
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
static VOID  _nx_nat_entry_find(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_to_match, NX_NAT_TRANSLATION_ENTRY **match_entry_ptr, 
                                UCHAR direction, UINT skip_static_entries)
{
                                    
NX_NAT_TRANSLATION_ENTRY    *entry_ptr;  
NX_NAT_TRANSLATION_ENTRY    *previous_ptr;   
                                                                                     

    /* Initialize the search result to null (no match found). */
    *match_entry_ptr = 0x0;
                    
    /* Check the inbound entry.  */
    if ((direction == NX_NAT_INBOUND_PACKET) &&
        (!entry_to_match -> peer_ip_address)) 
    {

        /* Return. */  
        return;
    }  

    /* Check the outbound entry.  */
    if ((direction == NX_NAT_OUTBOUND_PACKET) &&
        ((!entry_to_match -> local_ip_address) || (!entry_to_match -> peer_ip_address))) 
    {

        /* Return. */
        return ;
    }                                   
                            
    /* Initialize the previous pointer.  */
    previous_ptr = NX_NULL;

    /* Get a pointer to the start of the entries in the translation table. */
    entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;     
                                                                 
    /* Search the whole table until a match is found. */
    while (entry_ptr) 
    {          

        /* Check the entry type, (e.g. accepts packets from external hosts) ?*/
        if ((entry_ptr -> translation_type == NX_NAT_STATIC_ENTRY) && (skip_static_entries == NX_TRUE))
        {
                        
            /* Set the previous entry pointer.  */
            previous_ptr = entry_ptr;

            /* Ingore the static entry, get the next entry in the table. */
            entry_ptr = entry_ptr -> next_entry_ptr;

            continue;
        }                

        /* Do sender and entry protocols match? */
        if (entry_ptr -> protocol != entry_to_match -> protocol)
        {
                        
            /* Set the previous entry pointer.  */
            previous_ptr = entry_ptr;

            /* Get the next entry in the table. */
            entry_ptr = entry_ptr -> next_entry_ptr;

            continue;
        }
         
        /* Continue matching IP address and port criteria. */

        /* Do external IP addresses, if specified, match? */
        if ((entry_ptr -> peer_ip_address) &&
            (entry_ptr -> peer_ip_address != entry_to_match -> peer_ip_address))
        {
                       
            /* Set the previous entry pointer.  */
            previous_ptr = entry_ptr;

            /* No, get the next entry. */
            entry_ptr = entry_ptr -> next_entry_ptr;

            continue;
        }
               
        /* Do external ports, if specified match? */ 
        if ((entry_ptr -> peer_port) &&
            (entry_ptr -> peer_port != entry_to_match -> peer_port))
        {
                        
            /* Set the previous entry pointer.  */
            previous_ptr = entry_ptr;

            /* No, get the next entry in the table. */
            entry_ptr = entry_ptr -> next_entry_ptr;

            continue;
        }                                             

        /* Check the inbound entry.  */
        if (direction == NX_NAT_INBOUND_PACKET)
        {                        

            /* Does the inside host global port, if specified, match? */
            if ((entry_ptr -> external_port) &&   
                (entry_ptr -> external_port != entry_to_match -> external_port))
            {
                        
                /* Set the previous entry pointer.  */
                previous_ptr = entry_ptr;

                /* No, get the next entry in the table. */
                entry_ptr = entry_ptr -> next_entry_ptr;

                continue;
            }
        }
        else
        {

            /* Do private inside IP addresses, if specified, match? */
            if ((entry_ptr -> local_ip_address) &&
                (entry_ptr -> local_ip_address != entry_to_match -> local_ip_address))
            {
                        
                /* Set the previous entry pointer.  */
                previous_ptr = entry_ptr;

                /* No, get the next entry in the table. */
                entry_ptr = entry_ptr -> next_entry_ptr;

                continue;
            }                                 

            /* Does the inside host private port, if specified, match? */
            if ((entry_ptr -> local_port) &&
                (entry_ptr -> local_port != entry_to_match -> local_port))
            {
                           
                /* Set the previous entry pointer.  */
                previous_ptr = entry_ptr;

                /* No, get the next entry in the table. */
                entry_ptr = entry_ptr -> next_entry_ptr;

                continue;
            }
        }      
                     
        /* If we got this far, all criteria matched up. Set a pointer to 
           this entry in the table. */
        *match_entry_ptr = entry_ptr;

        /* The entry is active, reset the timeout to the present.  */
        entry_ptr -> response_timestamp = tx_time_get(); 

        /* Yes; check if this is the first entry in the list. */
        if (previous_ptr)
        {

            /* It is not. Put this entry at the head of the list to improve searching effectiveness. */
            previous_ptr -> next_entry_ptr = entry_ptr -> next_entry_ptr;
            entry_ptr -> next_entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;
            nat_ptr -> nx_nat_dynamic_active_entry_head = entry_ptr;     
        }       
        break;
    }         
    return;
}                     
          

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_entry_timeout_check                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to check the entry's timeout, and remove the */ 
/*    expiration entries from dynamic active translation list.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                          Pointer to NAT server              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_time_get                      Get the system time                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find       Find inbound entry in entry list   */ 
/*    _nx_nat_outbound_entry_find      Find outbound entry in entry list  */ 
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
static VOID  _nx_nat_entry_timeout_check(NX_NAT_DEVICE *nat_ptr)
{
                                    
ULONG                       current_time; 
ULONG                       elapsed_time;
NX_NAT_TRANSLATION_ENTRY    *entry_ptr;  
NX_NAT_TRANSLATION_ENTRY    *previous_ptr;   
NX_NAT_TRANSLATION_ENTRY    *next_entry_ptr;

                                                                          
    /* Get the current time.  */
    current_time = tx_time_get();

    /* Get a pointer to the start of the entries in the translation table. */
    entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;     

    /* Initialize the previous pointer.  */
    previous_ptr = NX_NULL;

    /* Search thru the whole entry list. */
    while (entry_ptr) 
    {
                                 
        /* Set a pointer to the next entry in the table. */
        next_entry_ptr = entry_ptr -> next_entry_ptr;

        /* Check the entry type and update the time.  */
        if (entry_ptr -> translation_type == NX_NAT_DYNAMIC_ENTRY)
        {        

            /* Calculate the elapsed time.  */
            elapsed_time = current_time - entry_ptr -> response_timestamp;

            /* Update the time remaining.  */
            if (elapsed_time >= entry_ptr -> response_timeout)
            {

                /* Delete this entry from active entry list.  */

                /* Check if this is the first entry in the list). */
                if (previous_ptr)
                {

                    /* It is not, so link the previous entry around the entry we are deleting. */
                    previous_ptr -> next_entry_ptr = next_entry_ptr;
                }
                else 
                {

                    /* It is the first entry, so set the next pointer as the starting translation table entry. */
                    nat_ptr -> nx_nat_dynamic_active_entry_head = next_entry_ptr;
                } 
                       
                /* Add the entry onto available entry list.  */ 
                entry_ptr -> next_entry_ptr = nat_ptr -> nx_nat_dynamic_available_entry_head;
                nat_ptr -> nx_nat_dynamic_available_entry_head = entry_ptr;        

                /* Update the entry count.  */
                nat_ptr -> nx_nat_dynamic_active_entries --;
                nat_ptr -> nx_nat_dynamic_available_entries ++;      
            }
            else
            {    

                /* Set the previous entry. */
                previous_ptr = entry_ptr;  
            }
        }         
        else
        {    

            /* Set the previous entry. */
            previous_ptr = entry_ptr;  
        }            
            
        /* Get the next entry in the table. */
        entry_ptr = next_entry_ptr;              
    } 

    return;
}      


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_utility_get_destination_port                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function extracts the destination port from the packet's        */
/*   protocol header. In the case of ICMP packets, it extracts the ICMP   */
/*   header identifier (ID) instead.                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                  Pointer to packet with destination port */ 
/*    protocol                    Packet protocol (TCP, UDP etc)          */ 
/*    peer_port                   Pointer to external (destination) port  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                  Successful completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_packet_is_icmp_error_message                                */
/*                                Indicate if ICMP packet is query or     */ 
/*                                     error message packet               */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */  
/*    _nx_nat_inbound_entry_find       Find inbound entry in entry list   */ 
/*    _nx_nat_outbound_entry_find      Find outbound entry in entry list  */ 
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
static UINT  _nx_nat_utility_get_destination_port(NX_PACKET *packet_ptr, UCHAR protocol, USHORT *peer_port)
{

NX_TCP_HEADER           *tcp_header_ptr;
NX_UDP_HEADER           *udp_header_ptr;
NX_ICMP_HEADER          *icmp_header_ptr;
UINT                    is_icmp_error_msg;
                                                                       

    /* Is this a TCP packet? */
    if (protocol == NX_PROTOCOL_TCP)
    {
        
        /* Pickup the pointer to the head of the TCP packet.  */
        tcp_header_ptr =  (NX_TCP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

        /* Pickup the destination TCP port.  */
        *peer_port =  (USHORT) (tcp_header_ptr -> nx_tcp_header_word_0 & NX_LOWER_16_MASK);
    }
    /* Is this a UDP packet? */
    else if (protocol == NX_PROTOCOL_UDP)
    {

        /* Pickup the pointer to the head of the UDP packet.  */
        udp_header_ptr =  (NX_UDP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

        /* Pickup the destination UDP port.  */
        *peer_port =  (USHORT) (udp_header_ptr -> nx_udp_header_word_0 & NX_LOWER_16_MASK);
    }
    /* Is this an ICMP packet? */
    else if (protocol == NX_PROTOCOL_ICMP) 
    {

        /* Determin type of ICMP message. */
        _nx_nat_packet_is_icmp_error_message(packet_ptr, &is_icmp_error_msg);

        /* Is this an ICMP error message? */
        if (is_icmp_error_msg )
        {

            /* Yes, these don't have query ID fields. */
            *peer_port = 0;
        }
        else
        {
        
            /* Setup the pointer to the ICMP header located in the data area after the IP header.  */
            icmp_header_ptr =  (NX_ICMP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);
    
            /* Pickup the ICMP identifier which we will call the 'source port'.  */
            *peer_port =  (USHORT) (icmp_header_ptr -> nx_icmp_header_word_1 >> NX_SHIFT_BY_16);
        }
    }
    else
    {

        /* Unknown or unsupported packet protocol. */
        *peer_port = 0;
    }                           

    return NX_SUCCESS;
}             


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_utility_get_source_port                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function extracts the source port from the packet's protocol    */
/*   header. In the case of ICMP packets, it extracts the query identifier*/
/*   (ID) instead.                                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                    Pointer to packet to get source port  */ 
/*    protocol                      Packet protocol (TCP, UDP etc)        */ 
/*    source_port                   Pointer to packet source port         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                     Successful completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_nat_packet_is_icmp_error_message                                */
/*                                   Indicate if ICMP packet is query or  */ 
/*                                     error message ICMP packet          */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_inbound_entry_find       Find inbound entry in entry list   */ 
/*    _nx_nat_outbound_entry_find      Find outbound entry in entry list  */ 
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
static UINT  _nx_nat_utility_get_source_port(NX_PACKET *packet_ptr, UCHAR protocol, USHORT *source_port)
{

NX_TCP_HEADER           *tcp_header_ptr;
NX_UDP_HEADER           *udp_header_ptr;
NX_ICMP_HEADER          *icmp_header_ptr;
UINT                    is_icmp_error_msg;

    if (protocol == NX_PROTOCOL_TCP)
    {
        
        /* Pickup the pointer to the head of the TCP packet.  */
        tcp_header_ptr =  (NX_TCP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

        /* Pickup the source TCP port.  */
        *source_port =  (USHORT) (tcp_header_ptr -> nx_tcp_header_word_0 >> NX_SHIFT_BY_16);
    }
    else if (protocol == NX_PROTOCOL_UDP)
    {

        /* Pickup the pointer to the head of the UDP packet.  */
        udp_header_ptr =  (NX_UDP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

        /* Pickup the source UDP port.  */
        *source_port =  (USHORT) (udp_header_ptr -> nx_udp_header_word_0 >> NX_SHIFT_BY_16);
    }
    else if (protocol == NX_PROTOCOL_ICMP) 
    {

        /* Determine type of ICMP message. */
        _nx_nat_packet_is_icmp_error_message(packet_ptr, &is_icmp_error_msg);

        /* Is this an ICMP error message? */
        if (is_icmp_error_msg )
        {

            /* Yes, return a zero query ID (error messages have no Query ID). */
            *source_port = 0;
        }
        else
        {
        
            /* Setup the pointer to the ICMP header located in the data area after the IP header.  */
            icmp_header_ptr =  (NX_ICMP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);

            /* Pickup the ICMP Query ID which is used in place of a source port.  */
            *source_port =  (USHORT)(icmp_header_ptr -> nx_icmp_header_word_1 >> NX_SHIFT_BY_16);        
        }
    }
    else
        /* Unknown or unsupported protocol. */
        *source_port = 0;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_find_available_port                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function finds an available a ICMP query identifier (ID) or     */
/*   or TCP/UDP port when NAT is configured to share a global IP address  */
/*   among its local hosts and consequently must use the query ID/port    */
/*   field to uniquely identify.                                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nat_ptr                           Pointer to NAT instance           */ 
/*    protocol                          Network protocol(TCP, UDP, ICMP)  */
/*    port                              Pointer to an unused port         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_NAT_NO_FREE_PORT_AVAILABLE    No free port found status          */ 
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_nat_outbound_entry_find      Find outbound entry in entry list  */ 
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
static UINT  _nx_nat_find_available_port(NX_NAT_DEVICE *nat_ptr, UCHAR protocol, USHORT *port)
{

UINT                     bound;
USHORT                   start_port;
USHORT                   end_port;
NX_NAT_TRANSLATION_ENTRY *entry_ptr;  


    /* Start at the lowest of number for translated query ID/port for NAT. */
    if (protocol == NX_PROTOCOL_TCP)
    {
        start_port = NX_NAT_START_TCP_PORT;
        end_port = NX_NAT_END_TCP_PORT;
    }
    else if(protocol == NX_PROTOCOL_UDP)
    {      
        start_port = NX_NAT_START_UDP_PORT;
        end_port = NX_NAT_END_UDP_PORT;
    }
    else
    {           
        start_port = NX_NAT_START_ICMP_QUERY_ID;
        end_port = NX_NAT_END_ICMP_QUERY_ID;
    }

    /* Set the start port.  */
    *port = start_port;

    /* Search for a Query ID not found in the translation table. */
    while(*port < end_port)
    {             

        /* Initialize the bound flag. */
        bound = NX_FALSE;

        /* Serach dynamic entries.  */   
        entry_ptr = nat_ptr -> nx_nat_dynamic_active_entry_head;

        /* Loop through the whole translation table. */
        while(entry_ptr)
        {

            /* Check the protocol.  */
            if (entry_ptr -> protocol == protocol)
            {

                /* Does this entry have a matching port ID. */
                if (entry_ptr -> external_port == *port)
                {

                    /* Set the flag so we can abort the current search. */
                    bound = NX_TRUE;
                    break;
                }
            }

            /* Get the next entry in the table. */
            entry_ptr = entry_ptr -> next_entry_ptr;
        }
                          
        /* Check if we got through the entire table with no bound port. */
        if ((bound == NX_FALSE) && 
            ((protocol == NX_PROTOCOL_TCP) || (protocol == NX_PROTOCOL_UDP)))
        {       

            /* Yes, check the NetXDuo TCP/UDP socket.   */
            bound = _nx_nat_socket_port_verify(nat_ptr -> nx_nat_ip_ptr, protocol, *port);     
        }

        /* Check if we found a socket with a matching port. */
        if (bound == NX_FALSE)
        {

            /* We did not. OK to use this port. */
            return NX_SUCCESS;
        }

        /* Found a match. This port is not available.  */

        /* Bump the port up one. */
        (*port)++;
    }

    /* If we got here we could not find a free port. */
    return (NX_NAT_NO_FREE_PORT_AVAILABLE);
}                                                             
             

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_entry_port_verify                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function verifies whether the supplied port is bound. lookup   */ 
/*    all NAT translation entries, If same ports are found, return        */ 
/*    NX_TRUE, else return NX_FALSE.                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                                IP instance pointer           */  
/*    protocol                              Protocol                      */
/*    port                                  Port                          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    bound                                 Completion status             */ 
/*                                            NX_TRUE: port is bound      */ 
/*                                            NX_FALSE: port is not bound */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    none                                                                */ 
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
static UINT  _nx_nat_entry_port_verify(NX_IP *ip_ptr, UINT protocol, UINT port)
{

UINT                bound;  
NX_NAT_TRANSLATION_ENTRY *entry_ptr;

    NX_PARAMETER_NOT_USED(ip_ptr);

    /* Initialize the return value.  */
    bound = NX_FALSE;

    /* Serach dynamic entries.  */   
    entry_ptr = nat_server_ptr -> nx_nat_dynamic_active_entry_head;

    /* Loop through the whole translation table. */
    while(entry_ptr)
    {

        /* Check the protocol.  */
        if (entry_ptr -> protocol == protocol)
        {

            /* Does this entry have a matching port ID. */
            if (entry_ptr -> external_port == port)
            {

                /* Set the flag so we can abort the current search. */
                bound = NX_TRUE;
                break;
            }
        }

        /* Get the next entry in the table. */
        entry_ptr = entry_ptr -> next_entry_ptr;
    }         

    /* Return status to the caller.  */
    return(bound);
}
     

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_socket_port_verify                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function verifies whether the supplied port is bound. Search   */ 
/*    all TCP/UDP sockets, and if the port is found, return NX_TRUE,      */ 
/*    else return NX_TRUE.                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                                IP instance pointer           */  
/*    protocol                              Protocol                      */
/*    port                                  Port                          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    bound                                 Completion status             */
/*                                            NX_TRUE: port is bound      */
/*                                            NX_FALSE: port is not bound */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    none                                                                */ 
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
static UINT  _nx_nat_socket_port_verify(NX_IP *ip_ptr, UINT protocol, UINT port)
{

UINT                index;
UINT                bound;
NX_TCP_SOCKET       *tcp_search_ptr;
NX_TCP_SOCKET       *tcp_end_ptr;
NX_UDP_SOCKET       *udp_search_ptr;
NX_UDP_SOCKET       *udp_end_ptr;
                                            

    /* Initialize the return value.  */
    bound = NX_FALSE;

    /* Check the protocol.  */
    if (protocol == NX_PROTOCOL_TCP)
    {       

        /* Calculate the hash index in the TCP port array of the associated IP instance.  */
        index =  (UINT) ((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK); 

        /* Pickup the head of the TCP ports bound list.  */
        tcp_search_ptr =  ip_ptr -> nx_ip_tcp_port_table[index];

        /* Determine if we need to perform a list search.  */
        if (tcp_search_ptr)
        {

            /* Walk through the circular list of UDP sockets that are already bound.  */
            tcp_end_ptr = tcp_search_ptr;
            do
            {

                /* Determine if this entry is the same as the requested port.  */
                if (tcp_search_ptr -> nx_tcp_socket_port == port)
                {

                    /* Yes, the port has already been bound.  */
                    bound = NX_TRUE;
                    break;
                }

                /* Move to the next entry in the list.  */
                tcp_search_ptr =  tcp_search_ptr -> nx_tcp_socket_bound_next;

            } while (tcp_search_ptr != tcp_end_ptr);
        }
    }
    else
    {

        /* Calculate the hash index in the UDP port array of the associated IP instance.  */
        index =  (UINT) ((port + (port >> 8)) & NX_UDP_PORT_TABLE_MASK); 

        /* Pickup the head of the UDP ports bound list.  */
        udp_search_ptr =  ip_ptr -> nx_ip_udp_port_table[index];

        /* Determine if we need to perform a list search.  */
        if (udp_search_ptr)
        {

            /* Walk through the circular list of UDP sockets that are already  bound.  */
            udp_end_ptr = udp_search_ptr;
            do
            {

                /* Determine if this entry is the same as the requested port.  */
                if (udp_search_ptr -> nx_udp_socket_port == port)
                {

                    /* Yes, the port has already been bound.  */
                    bound = NX_TRUE;
                    break;
                }

                /* Move to the next entry in the list.  */
                udp_search_ptr =  udp_search_ptr -> nx_udp_socket_bound_next;

            } while (udp_search_ptr != udp_end_ptr);
        }
    }

    /* Return success to the caller.  */
    return(bound);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_packet_is_icmp_error_message                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function determines if an ICMP packet is a query message or an  */
/*   error message.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                        Pointer to ICMP packet to parse   */ 
/*    is_icmp_error_msg                 Indicates if packet is an error   */
/*                                         message packet                 */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   None                                                                 */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */  
/*   _nx_nat_process_inbound_ICMP_packet   Forward inbound ICMP packets   */ 
/*   _nx_nat_process_outbound_ICMP_packet  Forward outbound ICMP packets  */ 
/*   _nx_nat_utility_get_source_port       Extract source port from header*/ 
/*   _nx_nat_utility_get_destination_port  Extract destination port from  */
/*                                             header                     */ 
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
static UINT  _nx_nat_packet_is_icmp_error_message(NX_PACKET *packet_ptr, UINT *is_icmp_error_msg)
{

UINT            type;
UINT            protocol;
NX_IPV4_HEADER  *ip_header_ptr;
NX_ICMP_HEADER  *icmp_header_ptr;


    /* Initialize the result to non error message type. */
    *is_icmp_error_msg = NX_FALSE;

    /* Set up an IP header pointer to the packet. */
    ip_header_ptr =  (NX_IPV4_HEADER *) (packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER));

    /* Set up an ICMP header pointer to the packet. */
    icmp_header_ptr = (NX_ICMP_HEADER *) (packet_ptr -> nx_packet_prepend_ptr);
                               
    /* Determine what protocol the current IP datagram is.  */
    protocol =  (ip_header_ptr -> nx_ip_header_word_2 >> 16) & 0xFF;     

    /* Is this an ICMP packet? */
    if (protocol == NX_PROTOCOL_ICMP)
    {                        

        /* Extract the ICMP type and code. */
        type = icmp_header_ptr -> nx_icmp_header_word_0 >> 24;

        /* Determine if this is an error message. */
        if (type != NX_ICMP_ECHO_REPLY_TYPE     &&
            type != NX_ICMP_ECHO_REQUEST_TYPE   &&
            type < NX_ICMP_TIMESTAMP_REQ_TYPE)
        {

            /* It is. Set the flag to true. */
            *is_icmp_error_msg = NX_TRUE;
        }
    }

    /* Return successful completion status. */
    return NX_SUCCESS;
}          

           
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_nat_checksum_adjust                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function is an optimization for recomputing a checksum by just  */
/*   computing the difference where a small amount of data has changed.   */
/*   NAT uses this optimization when, for example, it is only changing an */
/*   IP address or port.                                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*    old_checksum                       Pointer to the old checksum      */
/*    old_data                           Pointer to the data being changed*/
/*    new_data                           Pointer to new data replacing the*/
/*                                              old data                  */
/*    data_adjustment_length             Size of data being changed       */ 
/*    adjusted_checksum                  Pointer to the updated checksum  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*  _nx_nat_process_inbound_TCP_packet   Send TCP packet to local host    */ 
/*  _nx_nat_process_inbound_UDP_packet   Send UDP packet to local host    */ 
/*  _nx_nat_process_inbound_ICMP_packet  Send ICMP packet to local host   */ 
/*  _nx_nat_process_outbound_TCP_packet  Send TCP packet to external host */ 
/*  _nx_nat_process_outbound_UDP_packet  Send UDP packet to external host */ 
/*  _nx_nat_process_outbound_ICMP_packet Send ICMP packet to external host*/
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
static VOID  _nx_nat_checksum_adjust(UCHAR *checksum, UCHAR *old_data, INT old_data_length, UCHAR *new_data, INT new_data_length)
{

LONG    x, old_value, new_value;
INT     i,j;     

#ifdef NX_LITTLE_ENDIAN
    i = 1;
    j = 0;
#else
    i = 0;
    j = 1;
#endif

    /* Checksum Adjustment, RFC 3022, Section 4.2, Page 9.  */

    /* Get the old checksum.  */
    x = checksum[i] * 256 + checksum[j];
    x = ~x & 0xFFFF;

    /* Update the checksum for old data.  */
    while (old_data_length)
    {

        /* Get the old data.  */
        old_value = old_data[i] * 256 + old_data[j];

        /* Update the old data pointer.  */
        old_data += 2;

        /* Update the checksum.  */
        x -= old_value & 0xFFFF;

        /* Check the value.  */
        if (x <= 0)
        {

            /* Update the value.  */
            x --;
            x &= 0xFFFF;
        }

        /* Update the old data length.  */
        old_data_length -= 2;
    }

    /* Update the checksum for new data.  */
    while (new_data_length)
    {

        /* Get the new data.  */
        new_value = new_data[i] * 256 + new_data[j];

        /* Update the new data pointer.  */
        new_data += 2;

        /* Update the checksum.  */
        x += new_value & 0xFFFF;

        /* Check the value.  */
        if (x & 0x10000)
        {

            /* Update the value.  */
            x ++;
            x &= 0xFFFF;
        }

        /* Update the new data length.  */
        new_data_length -= 2;
    }

    /* Update the value.  */
    x = ~x & 0xFFFF;

    /* Update the checksum.  */
    checksum[i] = (UCHAR)(x /256);
    checksum[j] = (UCHAR)(x & 0xFF);       

    /* Return.  */
    return;
}
#endif
