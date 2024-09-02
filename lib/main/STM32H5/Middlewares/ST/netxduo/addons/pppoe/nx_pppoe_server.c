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
/**   PPP Over Ethernet (PPPoE)                                           */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_PPPOE_SERVER_SOURCE_CODE


/* Force error checking to be disabled in this module */
#include "tx_port.h"

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

#ifndef TX_SAFETY_CRITICAL
#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif
#endif


/* Include necessary system files.  */

#include "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include "nx_ip.h"
#include "nx_pppoe_server.h"
#include "nx_packet.h"

/* Define the PPPoE created list head pointer and count.  */ 
NX_PPPOE_SERVER  *_nx_pppoe_server_created_ptr = NX_NULL;


/* Define internal PPPoE services. */
                                                  
static VOID    _nx_pppoe_server_thread_entry(ULONG pppoe_server_ptr_value);  
static VOID    _nx_pppoe_server_packet_receive(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PACKET *packet_ptr);
static VOID    _nx_pppoe_server_discovery_packet_process(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PACKET *packet_ptr, ULONG client_mac_msw, ULONG client_mac_lsw, UINT is_broadcast);
static VOID    _nx_pppoe_server_session_packet_process(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PACKET *packet_ptr, ULONG client_mac_msw, ULONG client_mac_lsw);
static UINT    _nx_pppoe_server_discovery_send(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PPPOE_CLIENT_SESSION *client_session_ptr, UINT code);   
static VOID    _nx_pppoe_server_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PPPOE_CLIENT_SESSION *client_session_ptr, NX_PACKET *packet_ptr, UINT command); 
static UINT    _nx_pppoe_server_tag_process(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PPPOE_CLIENT_SESSION *client_session_ptr, UINT code, UCHAR *tag_ptr, ULONG length);
static ULONG   _nx_pppoe_server_data_get(UCHAR *data, UINT size);
static VOID    _nx_pppoe_server_data_add(UCHAR *data, UINT size, ULONG value);
static VOID    _nx_pppoe_server_string_add(UCHAR *dest, UCHAR *source, UINT size);
static UINT    _nx_pppoe_server_tag_string_add(UCHAR *data_ptr, UINT tag_type, UINT tag_length,  UCHAR *tag_value_string, UINT *index);
static UINT    _nx_pppoe_server_session_find(NX_PPPOE_SERVER *pppoe_server_ptr, ULONG client_mac_msw, ULONG client_mac_lsw, 
                                             ULONG session_id, UINT *session_index, NX_PPPOE_CLIENT_SESSION **client_session_ptr);   
static UINT    _nx_pppoe_server_session_cleanup(NX_PPPOE_CLIENT_SESSION *client_session_ptr);  

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE    
static UCHAR   *nx_pppoe_service_name[1];
#endif
 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Server instance create */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    name                                  Name of this PPPoE instance   */
/*    ip_ptr                                Pointer to IP control block   */
/*    interface_index                       IP Interface Index            */
/*    pppoe_link_driver                     User supplied link driver     */ 
/*    pool_ptr                              packet pool                   */
/*    stack_ptr                             Pointer stack area for PPPoE  */
/*    stack_size                            Size of PPPoE stack area      */
/*    priority                              Priority of PPPoE  thread     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_create               Actual PPPoE instance create  */
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_create(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                               VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *), NX_PACKET_POOL *pool_ptr,
                               VOID *stack_ptr, ULONG stack_size, UINT priority)
{   

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id == NX_PPPOE_SERVER_ID) || 
        (ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (pppoe_link_driver == NX_NULL) || (pool_ptr == NX_NULL) ||
        (pool_ptr -> nx_packet_pool_id != NX_PACKET_POOL_ID) ||
        (stack_ptr == NX_NULL))
        return(NX_PPPOE_SERVER_PTR_ERROR);
                                         
    /* Check for invalid interface ID */
    if(interface_index >= NX_MAX_PHYSICAL_INTERFACES)
        return(NX_PPPOE_SERVER_INVALID_INTERFACE);
                                                   
    /* Check for interface being valid. */
    if(!ip_ptr -> nx_ip_interface[interface_index].nx_interface_valid)
        return(NX_PPPOE_SERVER_INVALID_INTERFACE);

    /* Check for payload size of packet pool.  */
    if (pool_ptr -> nx_packet_pool_payload_size < NX_PPPOE_SERVER_MIN_PACKET_PAYLOAD_SIZE)
        return(NX_PPPOE_SERVER_PACKET_PAYLOAD_ERROR);

    /* Check for a memory size error.  */
    if (stack_size < TX_MINIMUM_STACK)
        return(NX_PPPOE_SERVER_MEMORY_SIZE_ERROR);
                          
    /* Check the priority specified.  */
    if (priority >= TX_MAX_PRIORITIES)
        return(NX_PPPOE_SERVER_PRIORITY_ERROR);

    /* Call actual PPPoE server instance create function.  */
    status =  _nx_pppoe_server_create(pppoe_server_ptr, name, ip_ptr, interface_index, 
                                      pppoe_link_driver, pool_ptr, stack_ptr, stack_size, priority); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function creates an PPPoE Server instance, including setting   */
/*    up all appropriate data structures, creating PPPoE event flag       */
/*    object and PPPoE thread.                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    name                                  Name of this PPPoE instance   */
/*    ip_ptr                                Pointer to IP control block   */ 
/*    interface_index                       Interface Index               */
/*    pppoe_link_driver                     User supplied link driver     */
/*    pool_ptr                              packet pool                   */
/*    stack_ptr                             Pointer stack area for PPPoE  */
/*    stack_size                            Size of PPPoE stack area      */
/*    priority                              Priority of PPPoE  thread     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_event_flags_create                 Create IP event flags         */
/*    tx_thread_create                      Create IP helper thread       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_create(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                              VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *), NX_PACKET_POOL *pool_ptr,
                              VOID *stack_ptr, ULONG stack_size, UINT priority)
{
    
TX_INTERRUPT_SAVE_AREA 
   

    /* Initialize the PPPoE Server control block to zero.  */
    memset((void *) pppoe_server_ptr, 0, sizeof(NX_PPPOE_SERVER));

    /* Save the PPPoE name.  */
    pppoe_server_ptr -> nx_pppoe_name = name;

    /* Save the length of PPPoE name to avoid compute strlen repeatedly in _nx_pppoe_server_discovery_send.  */
    if (_nx_utility_string_length_check((char *)pppoe_server_ptr -> nx_pppoe_name, 
                                        &(pppoe_server_ptr -> nx_pppoe_name_length),
                                        NX_MAX_STRING_LENGTH))
    {
        return(NX_PPPOE_SERVER_PTR_ERROR);
    }

    /* Save the IP pointer.  */
    pppoe_server_ptr -> nx_pppoe_ip_ptr = ip_ptr; 

    /* Setup the interface index and interface pointer.  */
    pppoe_server_ptr -> nx_pppoe_interface_ptr = &(ip_ptr -> nx_ip_interface[interface_index]);  

    /* Save the packet pool pointer.  */
    pppoe_server_ptr -> nx_pppoe_packet_pool_ptr = pool_ptr;

    /* Setup the enabled flag.  */
    pppoe_server_ptr -> nx_pppoe_enabled = NX_FALSE;

    /* Save the starting Session ID.  */
    pppoe_server_ptr -> nx_pppoe_session_id = NX_PPPOE_SERVER_START_SESSION_ID;

    /* Initialize PPPoE notify function pointer */
    pppoe_server_ptr -> nx_pppoe_discover_initiation_notify = NX_NULL; 
    pppoe_server_ptr -> nx_pppoe_discover_request_notify = NX_NULL; 
    pppoe_server_ptr -> nx_pppoe_discover_terminate_notify = NX_NULL; 
    pppoe_server_ptr -> nx_pppoe_discover_terminate_confirm = NX_NULL; 
    pppoe_server_ptr -> nx_pppoe_data_receive_notify = NX_NULL; 
    pppoe_server_ptr -> nx_pppoe_data_send_notify = NX_NULL;
               
    /* Setup the link driver address.  */
    pppoe_server_ptr -> nx_pppoe_link_driver_entry = pppoe_link_driver;

    /* Create event flag group to control the PPPoE processing thread.  */
    tx_event_flags_create(&(pppoe_server_ptr -> nx_pppoe_events), "PPPoE Server EVENTS") ;

    /* Create the PPPoE processing thread.  */
    tx_thread_create(&(pppoe_server_ptr -> nx_pppoe_thread), "PPPoE Server THREAD", _nx_pppoe_server_thread_entry, (ULONG) pppoe_server_ptr,  
                     stack_ptr, stack_size, priority, priority, NX_PPPOE_SERVER_THREAD_TIME_SLICE, TX_DONT_START);

    /* Otherwise, the PPPoE initialization was successful.  Place the
       PPPoE control block on created PPPoE instance.  */
    TX_DISABLE

    /* Load the PPPoE Server ID field in the PPPoE Server control block.  */
    pppoe_server_ptr -> nx_pppoe_id =  NX_PPPOE_SERVER_ID;   

    /* Set the pointer of global variable PPPoE.  */
    _nx_pppoe_server_created_ptr = pppoe_server_ptr;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_ac_name_set                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Server set Access      */
/*    Concentrator name function call.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    ac_name                               Access Concentrator name      */
/*    ac_name_length                        Length of Name                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_ac_name_set          Set Access Concentrator       */
/*                                            name function               */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_ac_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *ac_name, UINT ac_name_length)
{   

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);
  
    /* Call Access Concentrator name set function.  */
    status =  _nx_pppoe_server_ac_name_set(pppoe_server_ptr, ac_name, ac_name_length); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_ac_name_set                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets Access Concentrator name function call.          */
/*                                                                        */
/*    Note: The string of ac_name must be NULL-terminated and length      */
/*    of ac_name matches the length specified in the argument list.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    ac_name                               Access Concentrator name      */
/*    ac_name_length                        Length of Name                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_utility_string_length_check       Check string length           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_ac_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *ac_name, UINT ac_name_length)
{
UINT temp_name_length = 0;

    /* Get the length of ac_name string.  */
    if (_nx_utility_string_length_check((CHAR*)ac_name, &temp_name_length, ac_name_length))
        return(NX_SIZE_ERROR);

    /* Check the name length.  */
    if (ac_name_length != temp_name_length)
        return(NX_SIZE_ERROR);

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
    
    /* Save the nx_pppoe_ac_name.  */
    pppoe_server_ptr -> nx_pppoe_ac_name = ac_name;
    
    /* Save the nx_pppoe_ac_name length. */
    pppoe_server_ptr -> nx_pppoe_ac_name_length = ac_name_length;
       
    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Server instance delete */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_delete               Actual PPPoE instance delete  */
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_delete(NX_PPPOE_SERVER *pppoe_server_ptr)              
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);
    
    /* Call actual PPPoE server instance delete function.  */
    status =  _nx_pppoe_server_delete(pppoe_server_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function deletes an PPPoE Server instance. including deleting  */
/*    PPPoE event flag object and PPPoE thread.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_thread_terminate                   Terminate PPPoE helper thread */
/*    tx_thread_delete                      Delete PPPoE helper thread    */ 
/*    tx_event_flags_delete                 Delete PPPoE event flags      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_delete(NX_PPPOE_SERVER *pppoe_server_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Determine if the caller is the PPPoE thread itself. This is not allowed since
       a thread cannot delete itself in ThreadX.  */
    if (&pppoe_server_ptr -> nx_pppoe_thread == tx_thread_identify())
    {

        /* Invalid caller of this routine, return an error!  */
        return(NX_CALLER_ERROR);
    }
              
    /* Disable interrupts.  */
    TX_DISABLE

    /* Clear the PPPOE ID to show that it is no longer valid.  */
    pppoe_server_ptr -> nx_pppoe_id =  0;      
    pppoe_server_ptr -> nx_pppoe_enabled = NX_FALSE;

    /* Clear the created pointer.  */
    _nx_pppoe_server_created_ptr = NX_NULL;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Terminate the thread.  */
    tx_thread_terminate(&(pppoe_server_ptr -> nx_pppoe_thread));

    /* Delete the PPPoE thread.  */
    tx_thread_delete(&(pppoe_server_ptr -> nx_pppoe_thread));   

    /* Delete the event flag group.  */
    tx_event_flags_delete(&(pppoe_server_ptr -> nx_pppoe_events));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_enable                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Server enable function */
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_enable               Actual PPPoE enable function  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_enable(NX_PPPOE_SERVER *pppoe_server_ptr)              
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);

    /* Make sure the data receive callback function is set before enable.  
       Setting this function by nx_pppoe_server_callback_notify_set() API.  */
    if (pppoe_server_ptr -> nx_pppoe_data_receive_notify == NX_NULL)
        return(NX_PPPOE_SERVER_PTR_ERROR);
    
    /* Call actual PPPoE server instance enable function.  */
    status =  _nx_pppoe_server_enable(pppoe_server_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_enable                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function enables the PPPoE Server feature.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*    tx_thread_resume                      Resume PPPoE helper thread    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_enable               Actual PPPoE enable function  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_enable(NX_PPPOE_SERVER *pppoe_server_ptr)
{

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Set the enabled flag.  */
    pppoe_server_ptr -> nx_pppoe_enabled = NX_TRUE; 

    /* Resume the PPPoE server thread.  */
    tx_thread_resume(&(pppoe_server_ptr -> nx_pppoe_thread));  

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_disable                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Server disable function*/
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_disable              Actual PPPoE disable function */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_disable(NX_PPPOE_SERVER *pppoe_server_ptr)              
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);
    
    /* Call actual PPPoE server instance disable function.  */
    status =  _nx_pppoe_server_disable(pppoe_server_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_disable                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function disables the PPPoE Server feature.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*    tx_thread_suspend                     Suspend PPPoE helper thread   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_enable               Actual PPPoE enable function  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_disable(NX_PPPOE_SERVER *pppoe_server_ptr)
{

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Set the enabled flag.  */
    pppoe_server_ptr -> nx_pppoe_enabled = NX_FALSE;    

    /* Suspend the PPPoE server thread.  */
    tx_thread_suspend(&(pppoe_server_ptr -> nx_pppoe_thread));

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_callback_notify_set               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Server disable function*/
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    pppoe_discover_initiation_notify      Routine to call when discovery*/ 
/*                                            initiation data is received */ 
/*    pppoe_discover_request_notify         Routine to call when discovery*/ 
/*                                            reques data is received     */ 
/*    pppoe_discover_terminate_notify       Routine to call when discovery*/ 
/*                                            terminate data is received  */ 
/*    pppoe_discover_terminate_confirm      Routine to call when discovery*/ 
/*                                            terminate data is sent      */  
/*    pppoe_data_receive_notify             Routine to call when session  */ 
/*                                            data is received            */ 
/*    pppoe_data_send_notify                Routine to call when session  */ 
/*                                            data is sent                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_callback_notify_set  Actual PPPoE callback set     */
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_callback_notify_set(NX_PPPOE_SERVER *pppoe_server_ptr, 
                                            VOID (* pppoe_discover_initiation_notify)(UINT session_index), 
                                            VOID (* pppoe_discover_request_notify)(UINT session_index, ULONG length, UCHAR *data),
                                            VOID (* pppoe_discover_terminate_notify)(UINT session_index),
                                            VOID (* pppoe_discover_terminate_confirm)(UINT session_index),
                                            VOID (* pppoe_data_receive_notify)(UINT session_index, ULONG length, UCHAR *data, UINT packet_id),
                                            VOID (* pppoe_data_send_notify)(UINT session_index, UCHAR *data))
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);

    /* Check to see if pppoe_data_receive_notify is set.  */
    if (pppoe_data_receive_notify == NX_NULL) 
        return(NX_PPPOE_SERVER_PTR_ERROR);

    /* Call actual PPPoE service callback notify set function.  */
    status =  _nx_pppoe_server_callback_notify_set(pppoe_server_ptr, pppoe_discover_initiation_notify, pppoe_discover_request_notify,
                                                   pppoe_discover_terminate_notify, pppoe_discover_terminate_confirm,
                                                   pppoe_data_receive_notify, pppoe_data_send_notify); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_callback_notify_set                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets the callback notify functions.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    pppoe_discover_initiation_notify      Routine to call when discovery*/ 
/*                                            initiation data is received */ 
/*    pppoe_discover_request_notify         Routine to call when discovery*/ 
/*                                            reques data is received     */ 
/*    pppoe_discover_terminate_notify       Routine to call when discovery*/ 
/*                                            terminate data is received  */ 
/*    pppoe_discover_terminate_confirm      Routine to call when discovery*/ 
/*                                            terminate data is sent      */  
/*    pppoe_data_receive_notify             Routine to call when session  */ 
/*                                            data is received            */ 
/*    pppoe_data_send_notify                Routine to call when session  */ 
/*                                            data is sent                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_callback_notify_set(NX_PPPOE_SERVER *pppoe_server_ptr, 
                                           VOID (* pppoe_discover_initiation_notify)(UINT session_index), 
                                           VOID (* pppoe_discover_request_notify)(UINT session_index, ULONG length, UCHAR *data),
                                           VOID (* pppoe_discover_terminate_notify)(UINT session_index),
                                           VOID (* pppoe_discover_terminate_confirm)(UINT session_index),
                                           VOID (* pppoe_data_receive_notify)(UINT session_index, ULONG length, UCHAR *data, UINT packet_id),
                                           VOID (* pppoe_data_send_notify)(UINT session_index, UCHAR *data))
{
                  
    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Install PPPoE notify function pointer */
    pppoe_server_ptr -> nx_pppoe_discover_initiation_notify = pppoe_discover_initiation_notify; 
    pppoe_server_ptr -> nx_pppoe_discover_request_notify = pppoe_discover_request_notify; 
    pppoe_server_ptr -> nx_pppoe_discover_terminate_notify = pppoe_discover_terminate_notify; 
    pppoe_server_ptr -> nx_pppoe_discover_terminate_confirm = pppoe_discover_terminate_confirm; 
    pppoe_server_ptr -> nx_pppoe_data_receive_notify = pppoe_data_receive_notify; 
    pppoe_server_ptr -> nx_pppoe_data_send_notify = pppoe_data_send_notify;     

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_service_name_set                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE service name set       */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    service_name                          Pointer to an array service   */
/*                                            names.  Each service name   */
/*                                            must be Null-terminated     */
/*                                            string.                     */ 
/*    service_name_count                    The counter of service names  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_service_name_set     Actual PPPoE service name set */
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_service_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR **service_name, UINT service_name_count)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);

    /* Call actual PPPoE service name set function.  */
    status =  _nx_pppoe_server_service_name_set(pppoe_server_ptr, service_name, service_name_count); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_service_name_set                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets the the PPPoE service name.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    service_name                          Pointer to an array service   */
/*                                            names.  Each service name   */
/*                                            must be Null-terminated     */
/*                                            string.                     */ 
/*    service_name_count                    The counter of service names  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_service_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR **service_name, UINT service_name_count)
{

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup service name pointer.  */
    pppoe_server_ptr -> nx_pppoe_service_name = service_name;

    /* Setup service name count.  */
    pppoe_server_ptr -> nx_pppoe_service_name_count = service_name_count;

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_session_send                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE session data send      */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */ 
/*    data_ptr                              Session Data pointer          */ 
/*    data_length                           Length of Session Data        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_session_send         Actual PPPoE Session data send*/
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_session_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, UCHAR *data_ptr, UINT data_length)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID) || (data_ptr == NX_NULL))
        return(NX_PPPOE_SERVER_PTR_ERROR);

    /* Check the service_name_count.  */
    if (data_length == 0)  
        return(NX_PPPOE_SERVER_PTR_ERROR);  

    /* Check to see if PPPoE is enabled.  */
    if (pppoe_server_ptr -> nx_pppoe_enabled != NX_TRUE)
        return(NX_PPPOE_SERVER_NOT_ENABLED);

    /* Check for invalid session index.  */
    if(session_index >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return(NX_PPPOE_SERVER_INVALID_SESSION);

    /* Check to see if PPPoE session is established.  */
    if ((pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_valid != NX_TRUE) || 
        (pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_session_id == 0))
        return(NX_PPPOE_SERVER_SESSION_NOT_ESTABLISHED);

    /* Call actual PPPoE session send function.  */
    status =  _nx_pppoe_server_session_send(pppoe_server_ptr, session_index, data_ptr, data_length); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_send                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function builds an PPPoE Session packet and calls the          */
/*    associated driver to send it out on the network.                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */ 
/*    data_ptr                              Session Data pointer          */ 
/*    data_length                           Length of Session Data        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */ 
/*    nx_packet_allocate                    Allocate a packet for the     */ 
/*                                            PPPoE Session               */
/*    nx_packet_release                     Release packet to packet pool */ 
/*    nx_packet_data_append                 Copies the specified data     */ 
/*    _nx_pppoe_server_data_add             Add PPPoE data                */ 
/*    _nx_pppoe_server_packet_send          Send out PPPoE packet         */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_session_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, UCHAR *data_ptr, UINT data_length)
{               

NX_PPPOE_CLIENT_SESSION    *client_session_ptr;
NX_PACKET                  *packet_ptr;
UCHAR                      *work_ptr;
UINT                        status;


    /* Allocate a PPPoE packet.  */
    status =  nx_packet_allocate(pppoe_server_ptr -> nx_pppoe_packet_pool_ptr, &packet_ptr, NX_PHYSICAL_HEADER, NX_PPPOE_SERVER_PACKET_TIMEOUT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Return status.  */
        return(status);
    }

    /* Obtain the IP internal mutex.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Set the client session pointer.  */
    client_session_ptr = &(pppoe_server_ptr -> nx_pppoe_client_session[session_index]);

    /* Set the work pointer.  */
    work_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Added the PPPoE header.  */
    /*
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |  VER | TYPE  |      CODE       |         SESSION_ID           |
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |           LENGTH               |          payload             
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    /* Add version and type.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_VER_TYPE, 1, NX_PPPOE_SERVER_VERSION_TYPE);  

    /* Add code.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_CODE, 1, NX_PPPOE_SERVER_CODE_ZERO);

    /* Add session id.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_SESSION_ID, 2, client_session_ptr -> nx_pppoe_session_id); 

    /* Add length.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_LENGTH, 2, data_length);

    /* Update the pointer to add the payload of PPPoE.  */
    packet_ptr -> nx_packet_append_ptr += NX_PPPOE_SERVER_OFFSET_PAYLOAD;   
    packet_ptr -> nx_packet_length += NX_PPPOE_SERVER_OFFSET_PAYLOAD; 

    /* Release the mutex before a blocking call. */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Append the data into PPPoE packet.  */
    status = nx_packet_data_append(packet_ptr, data_ptr, data_length, pppoe_server_ptr -> nx_pppoe_packet_pool_ptr, NX_PPPOE_SERVER_PACKET_TIMEOUT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Relase the packet.  */
        nx_packet_release(packet_ptr);

        /* Return status.  */
        return(status);
    }

    /* Regain obtain the IP internal mutex. */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Send PPPoE session packet.  */
    _nx_pppoe_server_packet_send(pppoe_server_ptr, client_session_ptr, packet_ptr, NX_LINK_PPPOE_SESSION_SEND);

    /* Check the PPPoE send function.  */
    if (pppoe_server_ptr -> nx_pppoe_data_send_notify)
    {

        /* Call the function to send the data frame.  */
        pppoe_server_ptr -> nx_pppoe_data_send_notify(session_index, data_ptr);
    }

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_session_packet_send               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE session packet send    */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */ 
/*    packet_ptr                            Pointer to packet to send     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_session_packet_send  Actual PPPoE Session data send*/
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_session_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, NX_PACKET *packet_ptr)
{

UINT    status;

    /* Check for invalid packet.  */
    if (packet_ptr == NX_NULL)
    {
        return(NX_PPPOE_SERVER_PTR_ERROR);
    }

    /* Check for minimum packet length (PPP DATA Header).  */
    if (packet_ptr -> nx_packet_length < 2)
    {

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_SERVER_PACKET_PAYLOAD_ERROR);
    }

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_SERVER_PTR_ERROR);
    }

    /* Check to see if PPPoE is enabled.  */
    if (pppoe_server_ptr -> nx_pppoe_enabled != NX_TRUE)
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_SERVER_NOT_ENABLED);
    }

    /* Check for invalid session index.  */
    if(session_index >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_SERVER_INVALID_SESSION);
    }

    /* Check to see if PPPoE session is established.  */
    if ((pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_valid != NX_TRUE) || 
        (pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_session_id == 0))
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_SERVER_SESSION_NOT_ESTABLISHED);
    }

    /* Call actual PPPoE session packet_send function.  */
    status =  _nx_pppoe_server_session_packet_send(pppoe_server_ptr, session_index, packet_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_packet_send                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function builds an PPPoE Session packet and calls the          */
/*    associated driver to send it out on the network.                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */ 
/*    data_ptr                              Session Data pointer          */ 
/*    data_length                           Length of Session Data        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */ 
/*    nx_packet_release                     Release packet to packet pool */ 
/*    nx_packet_data_append                 Copies the specified data     */ 
/*    _nx_pppoe_server_data_add             Add PPPoE data                */ 
/*    _nx_pppoe_server_packet_send          Send out PPPoE packet         */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_session_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, NX_PACKET *packet_ptr)
{

NX_PPPOE_CLIENT_SESSION    *client_session_ptr;
UCHAR                      *work_ptr;


    /* Obtain the IP internal mutex.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check for an invalid packet prepend pointer.  */
    if ((packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < NX_PPPOE_SERVER_OFFSET_PAYLOAD)
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        /* Return error code.  */
        return(NX_PPPOE_SERVER_PACKET_PAYLOAD_ERROR);
    }

    /* Set the client session pointer.  */
    client_session_ptr = &(pppoe_server_ptr -> nx_pppoe_client_session[session_index]);

    /* Set the work pointer.  */
    packet_ptr -> nx_packet_prepend_ptr -= NX_PPPOE_SERVER_OFFSET_PAYLOAD;
    work_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Added the PPPoE header.  */
    /*
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |  VER | TYPE  |      CODE       |         SESSION_ID           |
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |           LENGTH               |          payload             
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    /* Add version and type.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_VER_TYPE, 1, NX_PPPOE_SERVER_VERSION_TYPE);  

    /* Add code.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_CODE, 1, NX_PPPOE_SERVER_CODE_ZERO);

    /* Add session id.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_SESSION_ID, 2, client_session_ptr -> nx_pppoe_session_id); 

    /* Add length.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_LENGTH, 2, packet_ptr -> nx_packet_length);

    /* Update the packet length.  */
    packet_ptr -> nx_packet_length += NX_PPPOE_SERVER_OFFSET_PAYLOAD;

    /* Send PPPoE session packet.  */
    _nx_pppoe_server_packet_send(pppoe_server_ptr, client_session_ptr, packet_ptr, NX_LINK_PPPOE_SESSION_SEND);

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_session_terminate                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE session terminate      */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_session_terminate    Actual PPPoE Session terminate*/
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_session_terminate(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);
      
    /* Check to see if PPPoE is enabled.  */
    if (pppoe_server_ptr -> nx_pppoe_enabled != NX_TRUE)
        return(NX_PPPOE_SERVER_NOT_ENABLED);

    /* Check for invalid session index.  */
    if(session_index >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return(NX_PPPOE_SERVER_INVALID_SESSION);

    /* Check to see if PPPoE session is established.  */
    if ((pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_valid != NX_TRUE) || 
        (pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_session_id == 0))
        return(NX_PPPOE_SERVER_SESSION_NOT_ESTABLISHED);

    /* Call actual PPPoE session terminate function.  */
    status =  _nx_pppoe_server_session_terminate(pppoe_server_ptr, session_index); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_terminate                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function builds an PPPoE packet to terminate the session.      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */ 
/*    _nx_pppoe_server_discovery_send       Send out PPPoE packet         */
/*    _nx_pppoe_server_session_cleanup      Cleanup PPPoE session         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_session_terminate(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index)
{

NX_PPPOE_CLIENT_SESSION    *client_session_ptr;
UINT                        status;

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
                 
    /* Set the client session pointer.  */
    client_session_ptr = &(pppoe_server_ptr -> nx_pppoe_client_session[session_index]);

    /* Terminate the PPPoE session.  */
    status = _nx_pppoe_server_discovery_send(pppoe_server_ptr, client_session_ptr, NX_PPPOE_SERVER_CODE_PADT);
           
    /* Check the status.  */
    if (status == NX_PPPOE_SERVER_SUCCESS)
    {                               

        /* Cleanup the session.  */
        _nx_pppoe_server_session_cleanup(client_session_ptr);

        /* Check the PPPoE terminate confirm function.  */
        if (pppoe_server_ptr -> nx_pppoe_discover_terminate_confirm)
        {

            /* Call terminate confirm function.  */
            pppoe_server_ptr -> nx_pppoe_discover_terminate_confirm(session_index);
        }
    }
          
    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_server_session_get                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE session get function   */
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         The index of Client Session   */ 
/*    client_mac_msw                        Client physical address MSW   */
/*    client_mac_lsw                        Client physical address LSW   */
/*    session_id                            Session ID                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_session_get          Actual PPPoE Session get      */
/*                                            function                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_pppoe_server_session_get(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, ULONG *client_mac_msw, ULONG *client_mac_lsw, ULONG *session_id)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_server_ptr == NX_NULL) || (pppoe_server_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return(NX_PPPOE_SERVER_PTR_ERROR);
                                   
    /* Check for invalid session index.  */
    if(session_index >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return(NX_PPPOE_SERVER_INVALID_SESSION);

    /* Call actual PPPoE session get function.  */
    status =  _nx_pppoe_server_session_get(pppoe_server_ptr, session_index, client_mac_msw, client_mac_lsw, session_id); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_get                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function builds an PPPoE packet to get the session physical    */
/*    address and session id.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         The index of Client Session   */ 
/*    client_mac_msw                        Client physical address MSW   */
/*    client_mac_lsw                        Client physical address LSW   */
/*    session_id                            Session ID                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_pppoe_server_session_get(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, ULONG *client_mac_msw, ULONG *client_mac_lsw, ULONG *session_id)
{  


    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check to see if PPPoE session is established.  */
    if ((pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_valid != NX_TRUE) ||
        (pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_session_id == 0))
    {
        
        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        return(NX_PPPOE_SERVER_SESSION_NOT_ESTABLISHED);
    }

    /* Yes, get the Client physical address MSW.  */
    if (client_mac_msw)
        *client_mac_msw = pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_physical_address_msw;

    /* Yes, get the Client physical address LSW.  */
    if (client_mac_lsw)
        *client_mac_lsw = pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_physical_address_lsw;

    /* Yes, get the Session ID.  */
    if (session_id)
        *session_id = pppoe_server_ptr -> nx_pppoe_client_session[session_index].nx_pppoe_session_id;

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return status.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


 /**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_packet_deferred_receive            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function receives a packet from the link driver (usually the   */
/*    link driver's input ISR) and places it in the deferred receive      */
/*    packet queue.  This moves the minimal receive packet processing     */
/*    from the ISR to the PPPoE helper thread.                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    packet_ptr                            Pointer to packet to receive  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_event_flags_set                    Set events for PPPoE thread   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
VOID  _nx_pppoe_server_packet_deferred_receive(NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Check to see if PPPoE instance is created.  */
    if (_nx_pppoe_server_created_ptr == NX_NULL)
    {   

        /* Release the packet.  */;
        nx_packet_release(packet_ptr);

        return;
    }

    /* Check to see if PPPoE is enabled.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_enabled != NX_TRUE)
    {
        
        /* Release the packet.  */;
        nx_packet_release(packet_ptr);

        return;
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Check to see if the deferred processing queue is empty.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_deferred_received_packet_head)
    {

        /* Not empty, just place the packet at the end of the queue.  */
        (_nx_pppoe_server_created_ptr -> nx_pppoe_deferred_received_packet_tail) -> nx_packet_queue_next = packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        _nx_pppoe_server_created_ptr -> nx_pppoe_deferred_received_packet_tail = packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Empty deferred receive processing queue.  Just setup the head pointers and
           set the event flags to ensure the PPPoE helper thread looks at the deferred processing
           queue.  */
        _nx_pppoe_server_created_ptr -> nx_pppoe_deferred_received_packet_head = packet_ptr;
        _nx_pppoe_server_created_ptr -> nx_pppoe_deferred_received_packet_tail = packet_ptr;
        packet_ptr -> nx_packet_queue_next = NX_NULL;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup PPPoE helper thread to process the PPPoE deferred receive.  */
        tx_event_flags_set(&(_nx_pppoe_server_created_ptr -> nx_pppoe_events), NX_PPPOE_SERVER_PACKET_RECEIVE_EVENT, TX_OR);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_thread_entry                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function is the entry point for each PPPoE's helper thread.    */
/*    The PPPoE helper thread is responsible for processing PPPoE packet. */
/*                                                                        */
/*    Note that the priority of this function is determined by the PPPoE  */
/*    create service.                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr_value                Pointer to PPPoE control block*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_event_flags_get                    Suspend on event flags that   */
/*                                            are used to signal this     */
/*                                            thread what to do           */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_pppoe_server_packet_receive       PPPoE receive packet          */  
/*                                            processing                  */
/*    nx_packet_release                     Release packet to packet pool */ 
/*    (pppoe_link_driver)                   User supplied link driver     */
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
static VOID _nx_pppoe_server_thread_entry(ULONG pppoe_server_ptr_value)
{

TX_INTERRUPT_SAVE_AREA

#ifdef NX_PPPOE_SERVER_INITIALIZE_DRIVER_ENABLE
NX_IP_DRIVER        driver_request;
#endif
NX_PPPOE_SERVER    *pppoe_server_ptr;
NX_IP              *ip_ptr;     
NX_PACKET          *packet_ptr;
ULONG               pppoe_events;
#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
UINT                i;
#endif


    /* Setup the PPPoE pointer.  */
    pppoe_server_ptr =  (NX_PPPOE_SERVER *) pppoe_server_ptr_value;

    /* Setup the IP pointer.  */
    ip_ptr = pppoe_server_ptr -> nx_pppoe_ip_ptr;
       
    /* Obtain the IP internal mutex before calling the driver.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifdef NX_PPPOE_SERVER_INITIALIZE_DRIVER_ENABLE

    /* Initialize and enable the hardware for physical interface.  */

    /* Is this a valid interface with a link driver associated with it?  */
    if((pppoe_server_ptr -> nx_pppoe_interface_ptr -> nx_interface_valid) && (pppoe_server_ptr -> nx_pppoe_link_driver_entry))
    {

        /* Yes; attach the interface to the device. */
        driver_request.nx_ip_driver_ptr        = ip_ptr;
        driver_request.nx_ip_driver_command    = NX_LINK_INTERFACE_ATTACH;
        driver_request.nx_ip_driver_interface  = pppoe_server_ptr -> nx_pppoe_interface_ptr;
        (pppoe_server_ptr -> nx_pppoe_link_driver_entry) (&driver_request);

        /* Call the link driver to initialize the hardware. Among other
           responsibilities, the driver is required to provide the
           Maximum Transfer Unit (MTU) for the physical layer. The MTU
           should represent the actual physical layer transfer size
           less the physical layer headers and trailers.  */
        driver_request.nx_ip_driver_ptr     = ip_ptr;
        driver_request.nx_ip_driver_command = NX_LINK_INITIALIZE;
        (pppoe_server_ptr -> nx_pppoe_link_driver_entry) (&driver_request);

        /* Call the link driver again to enable the interface.  */
        driver_request.nx_ip_driver_ptr     = ip_ptr;
        driver_request.nx_ip_driver_command = NX_LINK_ENABLE;
        (pppoe_server_ptr -> nx_pppoe_link_driver_entry) (&driver_request);
        
        /* Indicate to the IP software that IP to physical mapping
           is not required.  */
        pppoe_server_ptr -> nx_pppoe_interface_ptr -> nx_interface_address_mapping_needed = NX_FALSE; 
    }
#endif

    /* Loop to continue processing incoming bytes.  */
    while(NX_FOREVER)
    {


        /* Release the IP internal mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Pickup IP event flags.  */
        tx_event_flags_get(&(pppoe_server_ptr -> nx_pppoe_events), NX_PPPOE_SERVER_ALL_EVENTS, TX_OR_CLEAR, &pppoe_events, NX_WAIT_FOREVER);
              
        /* Obtain the IP internal mutex before processing the IP event.  */
        tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

        /* Check for PPPoE event.  */
        if (pppoe_events & NX_PPPOE_SERVER_PACKET_RECEIVE_EVENT)
        {

            /* Loop to process all deferred packet requests.  */
            while (pppoe_server_ptr -> nx_pppoe_deferred_received_packet_head)
            {

                /* Remove the first packet and process it!  */

                /* Disable interrupts.  */
                TX_DISABLE

                /* Pickup the first packet.  */
                packet_ptr =  pppoe_server_ptr -> nx_pppoe_deferred_received_packet_head;

                /* Move the head pointer to the next packet.  */
                pppoe_server_ptr -> nx_pppoe_deferred_received_packet_head =  packet_ptr -> nx_packet_queue_next;
                                                      
                /* Check for end of deferred processing queue.  */
                if (pppoe_server_ptr -> nx_pppoe_deferred_received_packet_head == NX_NULL)
                {

                    /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                    pppoe_server_ptr -> nx_pppoe_deferred_received_packet_tail =  NX_NULL;
                }

                /* Restore interrupts.  */
                TX_RESTORE
                             
#ifndef NX_DISABLE_PACKET_CHAIN

                /* Discard the chained packets.  */
                if (packet_ptr -> nx_packet_next)
                {
                    nx_packet_release(packet_ptr);
                    continue;
                }
#endif

                /* Check for valid packet length.  */
                if (packet_ptr -> nx_packet_length < NX_PPPOE_SERVER_OFFSET_PAYLOAD)
                {

                    /* Release the packet.  */
                    nx_packet_release(packet_ptr);
                    return;
                }

                /* Check the packet interface.  */
                if ((packet_ptr -> nx_packet_ip_interface != NX_NULL) &&
                    (packet_ptr -> nx_packet_ip_interface != pppoe_server_ptr -> nx_pppoe_interface_ptr))
                {
                    nx_packet_release(packet_ptr);
                    continue;
                }

                /* Call the actual PPPoE Server packet receive function.  */
                _nx_pppoe_server_packet_receive(pppoe_server_ptr, packet_ptr);
            }
        }

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
        /* Check for PPPoE Session Receive event.  */
        if (pppoe_events & NX_PPPOE_SERVER_SESSION_RECEIVE_EVENT)
        {

            for(i = 0; i < NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER; i ++) 
            {

                /* Check if this session is valid.  */
                if (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_valid != NX_TRUE)
                    continue;

                /* Check if this session is ready to receive the packet.  */
                if (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_packet_receive_stopped == NX_TRUE)
                    continue;

                /* Check if this session queued the packets.  */
                if (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_deferred_received_packet_head)
                {

                    /* Remove the first packet and process it!  */

                    /* Disable interrupts.  */
                    TX_DISABLE

                    /* Pickup the first packet.  */
                    packet_ptr = pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_deferred_received_packet_head;

                    /* Move the head pointer to the next packet.  */
                    pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_deferred_received_packet_head = packet_ptr -> nx_packet_queue_next;

                    /* Check for end of deferred processing queue.  */
                    if (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_deferred_received_packet_head == NX_NULL)
                    {

                        /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                        pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_deferred_received_packet_tail = NX_NULL;
                    }

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Set the flag to stop receive the next packet.  */
                    pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_packet_receive_stopped = NX_TRUE;

                    /* Check the PPPoE receive function.  */
                    if (_nx_pppoe_server_created_ptr -> nx_pppoe_data_receive_notify)
                    {

                        /* Call the function to receive the data frame.  */
                        _nx_pppoe_server_created_ptr -> nx_pppoe_data_receive_notify(i, packet_ptr -> nx_packet_length, packet_ptr -> nx_packet_prepend_ptr, (UINT)(packet_ptr));
                    }
                }
            }
        }
#endif
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_packet_receive                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function receives a PPPoE packet from the PPPoE deferred       */
/*    processing queue.                                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    packet_ptr                            Pointer to packet to receive  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_release                     Release packet to packet pool */
/*    _nx_pppoe_server_data_get             Get the PPPoE data            */
/*    _nx_pppoe_server_discovery_packet_process                           */  
/*                                          Process discovery packet      */
/*    _nx_pppoe_server_session_packet_process                             */  
/*                                          Process session packet        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_thread_entry                                       */
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
VOID  _nx_pppoe_server_packet_receive(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PACKET *packet_ptr)
{

UCHAR                      *ethernet_header_ptr; 
ULONG                       server_mac_msw;
ULONG                       server_mac_lsw;
ULONG                       client_mac_msw;
ULONG                       client_mac_lsw;
UINT                        ethernet_type; 
UINT                        is_broadcast = NX_FALSE;

    /* Set up UCHAR pointer to ethernet header to extract client hardware address
       which we will use as the client's unique identifier.  */
    ethernet_header_ptr = packet_ptr -> nx_packet_prepend_ptr - NX_PPPOE_SERVER_ETHER_HEADER_SIZE;

    /* Pickup the MSW and LSW of the destination MAC address.  */
    server_mac_msw = (((ULONG) ethernet_header_ptr[0]) << 8)  | ((ULONG) ethernet_header_ptr[1]);
    server_mac_lsw = (((ULONG) ethernet_header_ptr[2]) << 24) | (((ULONG) ethernet_header_ptr[3]) << 16) |
                     (((ULONG) ethernet_header_ptr[4]) << 8)  | ((ULONG) ethernet_header_ptr[5]);

    /* Check the server hardware (mac address) field is filled in. */
    if ((server_mac_msw == 0) && (server_mac_lsw == 0))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Check if it is a broadcast message.  */
    if ((server_mac_msw == 0xFFFF) && (server_mac_lsw == 0xFFFFFFFF))
        is_broadcast = NX_TRUE;

    /* Pickup the MSW and LSW of the source MAC address.  */
    client_mac_msw = (((ULONG) ethernet_header_ptr[6]) << 8)  | ((ULONG) ethernet_header_ptr[7]);
    client_mac_lsw = (((ULONG) ethernet_header_ptr[8]) << 24) | (((ULONG) ethernet_header_ptr[9]) << 16) |
                     (((ULONG) ethernet_header_ptr[10]) << 8)  | ((ULONG) ethernet_header_ptr[11]);

    /* Check the client hardware (mac address) field is filled in. */
    if ((client_mac_msw == 0) && (client_mac_lsw == 0))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Get the ethernet type.  */
    ethernet_type = _nx_pppoe_server_data_get(ethernet_header_ptr + 12, 2);     

    /* Process the packet according to packet type. */
    if(ethernet_type == NX_PPPOE_SERVER_ETHER_TYPE_DISCOVERY)
    {

        /* Process the discovery packet.  */
        _nx_pppoe_server_discovery_packet_process(pppoe_server_ptr, packet_ptr, client_mac_msw, client_mac_lsw, is_broadcast);
    }
    else if(ethernet_type == NX_PPPOE_SERVER_ETHER_TYPE_SESSION)
    {

        /* Session Stage, all Ethernet packets are unicast.  */
        if (is_broadcast == NX_TRUE)
        {

            /* Release the packet.  */   
            nx_packet_release(packet_ptr);
            return;
        }

        /* Process the session packet.  */
        _nx_pppoe_server_session_packet_process(pppoe_server_ptr, packet_ptr, client_mac_msw, client_mac_lsw);
    }
    else
    {

        /* Relase the packet.  */
        nx_packet_release(packet_ptr);
    }

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_discovery_packet_process           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function processes an incoming discovery packet.               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    packet_ptr                            Pointer to packet to receive  */ 
/*    client_mac_msw                        Client physical address MSW   */
/*    client_mac_lsw                        Client physical address LSW   */
/*    is_broadcast                          Broadcast flag                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_release                     Release packet to packet pool */
/*    _nx_pppoe_server_data_get             Get the PPPoE data            */ 
/*    _nx_pppoe_server_tag_process          Process PPPoE tags            */ 
/*    _nx_pppoe_server_discovery_send       Send discovery packet         */
/*    _nx_pppoe_server_session_find         Find the PPPoE session        */
/*    _nx_pppoe_server_session_cleanup      Cleanup the PPPoE session     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_packet_receive       Receive the PPPoE packet      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_pppoe_server_discovery_packet_process(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PACKET *packet_ptr, ULONG client_mac_msw, ULONG client_mac_lsw, UINT is_broadcast)
{

UCHAR                      *pppoe_header_ptr;
ULONG                       ver_type;
ULONG                       code;
ULONG                       session_id;
ULONG                       length;
UINT                        status;
UCHAR                      *tag_ptr;
UINT                        session_index = 0;
NX_PPPOE_CLIENT_SESSION    *client_session_ptr = NX_NULL;


    /* Setup the PPPoE header.  */
    pppoe_header_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the version and type.  */
    ver_type = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_VER_TYPE, 1);
                 
    /* Check the version and type.  */
    if (ver_type != NX_PPPOE_SERVER_VERSION_TYPE)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the code.  */
    code = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_CODE, 1);
           
    /* Check the code.  */
    if ((code != NX_PPPOE_SERVER_CODE_PADI) && 
        (code != NX_PPPOE_SERVER_CODE_PADR) &&
        (code != NX_PPPOE_SERVER_CODE_PADT))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* For PADI, the destination address should be broadcast. 
       For PADR and PART, the destination address should be unicast.  */
    if (((code == NX_PPPOE_SERVER_CODE_PADI) && (is_broadcast != NX_TRUE)) ||
        ((code != NX_PPPOE_SERVER_CODE_PADI) && (is_broadcast == NX_TRUE)))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the session id.  */   
    session_id = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_SESSION_ID, 2);

    /* Check the session id. 
       Session ID must be zero for PADI and PADR,
       Session ID must be not zero for PADT.  */
    if (((code != NX_PPPOE_SERVER_CODE_PADT) && (session_id != 0)) ||
        ((code == NX_PPPOE_SERVER_CODE_PADT) && (session_id == 0)))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Find the PPPoE Session.  */
    status = _nx_pppoe_server_session_find(pppoe_server_ptr, client_mac_msw, client_mac_lsw, session_id, &session_index, &client_session_ptr);

    /* Check the status.  */
    if (status)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the length of tags.  */
    length = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_LENGTH, 2);      

    /* Check for valid payload.  */
    if (length + NX_PPPOE_SERVER_OFFSET_PAYLOAD > packet_ptr -> nx_packet_length)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Set the tag pointer.  */
    tag_ptr = pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_PAYLOAD;

    /* Process the tag.  */
    status = _nx_pppoe_server_tag_process(pppoe_server_ptr, client_session_ptr, code, tag_ptr, length);

    /* Now we can release the packet. */
    nx_packet_release(packet_ptr);

    /* Check the status.  */
    if (status)
    {

        /* If the Access Concentrator does not like the Service-Name in the PADR, 
           then it MUST reply with a PADS containing a TAG of TAG_TYPE Service-Name-Error (and any number of other TAG types).
           In this case the SESSION_ID MUST be set to 0x0000. RFC2516, Section5.4, Page6.  */
        if ((status == NX_PPPOE_SERVER_SERVICE_NAME_ERROR) && (code == NX_PPPOE_SERVER_CODE_PADR))
        {

            /* Clear the session id.  */
            client_session_ptr -> nx_pppoe_session_id = 0;

            /* Send PADS.  */ 
            _nx_pppoe_server_discovery_send(pppoe_server_ptr, client_session_ptr, NX_PPPOE_SERVER_CODE_PADS);
        }

        /* Cleanup the PPPoE session.  */
        _nx_pppoe_server_session_cleanup(client_session_ptr);

        return;
    }

    /* Check the code value.  */
    if (code == NX_PPPOE_SERVER_CODE_PADI)
    {

        /* It is PPPoE Active Discovery Initiation packet.  */
        if (pppoe_server_ptr -> nx_pppoe_discover_initiation_notify)
        {

            /* Call discover initiation notify function.  */
            pppoe_server_ptr -> nx_pppoe_discover_initiation_notify(session_index);

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
            /* Send PPPoE Active Discover Offer packet in PppDiscoverCnf().  */
            return;
#endif
        }

        /* Send PPPoE Active Discovery Offer packet.  */
        _nx_pppoe_server_discovery_send(pppoe_server_ptr, client_session_ptr, NX_PPPOE_SERVER_CODE_PADO);
    }
    else if (code == NX_PPPOE_SERVER_CODE_PADR)
    {

        /* Generate the unique session id.  */
        if (client_session_ptr -> nx_pppoe_session_id == 0)
        {

            /* The Session ID should not be zero and 0xFFFF.  
               RFC2516, Section4, Page4.  */
            if ((pppoe_server_ptr -> nx_pppoe_session_id == 0) ||
                (pppoe_server_ptr -> nx_pppoe_session_id == 0xFFFF))
                pppoe_server_ptr -> nx_pppoe_session_id = 1;

            /* Setup the session id.  */
            client_session_ptr -> nx_pppoe_session_id = pppoe_server_ptr -> nx_pppoe_session_id;

            /* Update the session id for next client session.  */
            pppoe_server_ptr -> nx_pppoe_session_id++;
        }

        /* It is PPPoE Active Discovery Request packet.  */
        if (pppoe_server_ptr -> nx_pppoe_discover_request_notify)
        {

            /* Call discover reqest notify function.  */
            if (client_session_ptr -> nx_pppoe_service_name == NX_NULL)
            {     
                pppoe_server_ptr -> nx_pppoe_discover_request_notify(session_index, 0, NX_NULL);
            }
            else
            {
                /* Check service name length.  */
                _nx_utility_string_length_check((char*)client_session_ptr -> nx_pppoe_service_name, (UINT *)&length, NX_MAX_STRING_LENGTH);

                pppoe_server_ptr -> nx_pppoe_discover_request_notify(session_index, length, client_session_ptr -> nx_pppoe_service_name);
            }
            
#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
            /* Send PPPoE Active Discover Session-confirmation packet in PppOpenCnf().  */
            return;
#endif
        }

        /* Send PPPoE Active Discovery Session-confirmation packet.  */
        _nx_pppoe_server_discovery_send(pppoe_server_ptr, client_session_ptr, NX_PPPOE_SERVER_CODE_PADS);
    }
    else if (code == NX_PPPOE_SERVER_CODE_PADT)
    {

#ifndef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
        /* Cleanup the PPPoE session.  */
        _nx_pppoe_server_session_cleanup(client_session_ptr);
#endif

        /* It is PPPoE Active Discovery Terminate packet.  */
        if (pppoe_server_ptr -> nx_pppoe_discover_terminate_notify)
        {

            /* Call discover terminate notify function.  */
            pppoe_server_ptr -> nx_pppoe_discover_terminate_notify(session_index);
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_packet_process             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function processes an incoming session packet.                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/
/*    packet_ptr                            Pointer to packet to receive  */ 
/*    client_mac_msw                        Client physical address MSW   */
/*    client_mac_lsw                        Client physical address LSW   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_release                     Release packet to packet pool */
/*    _nx_pppoe_server_data_get             Get the PPPoE data            */
/*    _nx_pppoe_server_session_find         Find the PPPoE session        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_packet_receive       Receive the PPPoE packet      */
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
static VOID  _nx_pppoe_server_session_packet_process(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PACKET *packet_ptr, ULONG client_mac_msw, ULONG client_mac_lsw)
{

UCHAR                      *pppoe_header_ptr;
ULONG                       ver_type;
ULONG                       code;
ULONG                       session_id;
ULONG                       length;        
UINT                        status;
UINT                        session_index = 0;
NX_PPPOE_CLIENT_SESSION    *client_session_ptr = NX_NULL;


    /* Setup the PPPoE header.  */
    pppoe_header_ptr = packet_ptr -> nx_packet_prepend_ptr;
                  
    /* Pickup the version and type.  */   
    ver_type = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_VER_TYPE, 1);

    /* Check the version and type.  */
    if (ver_type != NX_PPPOE_SERVER_VERSION_TYPE)
    {
         
        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the code.  */
    code = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_CODE, 1);
         
    /* Check the code.  */
    if (code != NX_PPPOE_SERVER_CODE_ZERO)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the session id.  */   
    session_id = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_SESSION_ID, 2);   
       
    /* Check the session id.  */
    if (session_id == 0)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Setup the prepend pointer to point the payload of PPPoE.  */
    packet_ptr -> nx_packet_prepend_ptr += NX_PPPOE_SERVER_OFFSET_PAYLOAD;
    packet_ptr -> nx_packet_length -= NX_PPPOE_SERVER_OFFSET_PAYLOAD;

    /* Look up the PPPoE session by physical address and interface index in Client Records table. */
    status = _nx_pppoe_server_session_find(pppoe_server_ptr, client_mac_msw, client_mac_lsw, session_id, &session_index, &client_session_ptr);
     
    /* Check the status.  */
    if (status)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the length of payload.  */
    length = _nx_pppoe_server_data_get(pppoe_header_ptr + NX_PPPOE_SERVER_OFFSET_LENGTH, 2);

    /* Check for valid payload.  */
    if (length > packet_ptr -> nx_packet_length)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Remove the Ethernet padding.  */
    if (length < packet_ptr -> nx_packet_length)
    {
        packet_ptr -> nx_packet_append_ptr -= (packet_ptr -> nx_packet_length - length);
        packet_ptr -> nx_packet_length -= (packet_ptr -> nx_packet_length - length);
    }

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE

    /* Check to see if the deferred processing queue is empty.  */
    if (client_session_ptr -> nx_pppoe_deferred_received_packet_head)
    {

        /* Not empty, just place the packet at the end of the queue.  */
        (client_session_ptr -> nx_pppoe_deferred_received_packet_tail) -> nx_packet_queue_next = packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        client_session_ptr -> nx_pppoe_deferred_received_packet_tail = packet_ptr;

        /* Return.  */
        return;
    }
    else
    {

        /* Check the packet receive flag.  */
        if (client_session_ptr -> nx_pppoe_packet_receive_stopped == NX_TRUE)
        {

            /* Empty deferred receive processing queue.  Just setup the head pointers and
               set the event flags to ensure the PPPoE helper thread looks at the deferred processing queue.  */
            client_session_ptr -> nx_pppoe_deferred_received_packet_head = packet_ptr;
            client_session_ptr -> nx_pppoe_deferred_received_packet_tail = packet_ptr;
            packet_ptr -> nx_packet_queue_next = NX_NULL; 

            /* Return.  */
            return;
        }
    } 

    /* Set the flag to stop receive the next packet.  */
    client_session_ptr -> nx_pppoe_packet_receive_stopped = NX_TRUE;
#endif

    /* Check the PPPoE receive function.  */
    if (pppoe_server_ptr -> nx_pppoe_data_receive_notify)
    {

        /* Call the function to receive the data frame.
           Notice: the receive function must release this packet.  */
        pppoe_server_ptr -> nx_pppoe_data_receive_notify(session_index, length, packet_ptr -> nx_packet_prepend_ptr, (UINT)(packet_ptr));
    }
    else
    {

        /* Release the packet.  */  
        nx_packet_release(packet_ptr);
    }

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_discovery_send                     PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sends a PPPoE discovery packet.                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    client_session_ptr                    Pointer to Client Session     */
/*    code                                  PPPoE code                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */ 
/*    nx_packet_allocate                    Allocate a packet for the     */ 
/*                                            PPPoE Discovery             */
/*    nx_packet_release                     Release packet to packet pool */ 
/*    _nx_pppoe_server_data_add             Add PPPoE data                */
/*    _nx_pppoe_server_tag_string_add       Add PPPoE tag                 */ 
/*    _nx_pppoe_server_packet_send          Send out PPPoE packet         */
/*    _nx_pppoe_server_session_find         Find the PPPoE session        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_terminate    Terminate the PPPoE session   */ 
/*    _nx_pppoe_server_discovery_packet_process                           */ 
/*                                          Process PPPoE Discovery packet*/ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT    _nx_pppoe_server_discovery_send(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PPPOE_CLIENT_SESSION *client_session_ptr, UINT code)
{               

NX_PACKET       *packet_ptr;
UCHAR           *work_ptr;
UINT            status;
UINT            index = 0;
UINT            tag_length;
UINT            service_name_index = 0;
#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
UCHAR          *service_name_ptr;
#endif


    /* Release the mutex before a blocking call. */
    tx_mutex_put(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Allocate a PPPoE packet.  */
    status =  nx_packet_allocate(pppoe_server_ptr -> nx_pppoe_packet_pool_ptr, &packet_ptr, NX_PHYSICAL_HEADER, NX_PPPOE_SERVER_PACKET_TIMEOUT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Return status.  */
        return(status);
    }

    /* Obtain the IP internal mutex.  */
    tx_mutex_get(&(pppoe_server_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Set the work pointer.  */
    work_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* First skip the PPPoE header.  */
    index += NX_PPPOE_SERVER_OFFSET_PAYLOAD;

    /* The PPPoE payload contains zero or more TAGs.  A TAG is a TLV (type-length-value) construct and is defined as follows.  */

    /*                      1                   2                   3
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |          TAG_TYPE              |         TAG_LENGTH           |
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |          TAG_VALUE  ...                                        
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    /* Add the PPPoE tags.  */ 
    if (code == NX_PPPOE_SERVER_CODE_PADO)
    {

        /* The PADO packet MUST contain one AC-Name TAG containing the Access Concentrator's name. RFC2516, Section 5.2, Page6. */

        /* Added the AC-Name tag.  */
        if (pppoe_server_ptr -> nx_pppoe_ac_name)
        {
            
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_AC_NAME, pppoe_server_ptr -> nx_pppoe_ac_name_length, (UCHAR *)(pppoe_server_ptr -> nx_pppoe_ac_name), &index);
        }
        else
        {
            
            /* If user does not separately set Access Concentrator name, will use PPPoE server name as Access Concentrator name.*/
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_AC_NAME, pppoe_server_ptr -> nx_pppoe_name_length, (UCHAR *)(pppoe_server_ptr -> nx_pppoe_name), &index);
        }            
                                                                                                                                             
        /* The PADO packet MUST contain a Service-Name TAG identical to the one in the PADI, 
           and any number of other Service-Name TAGs indicating other services. RFC2516, Section 5.2, Page6.  */

        /* Added a Service-Name TAG identical to the one in the PADI.  */
        if  (client_session_ptr -> nx_pppoe_service_name)
        {

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE    
            /* Set the service name pointer.  */
            service_name_ptr = client_session_ptr -> nx_pppoe_service_name;

            /* Loop to add service name that user configured.  */
            while(service_name_index < client_session_ptr -> nx_pppoe_service_name_length)
            {               

                /* Get the service name.  */
                if (_nx_utility_string_length_check((char *)(service_name_ptr), &tag_length, NX_MAX_STRING_LENGTH))
                {
                    nx_packet_release(packet_ptr);
                    return(NX_PPPOE_SERVER_SERVICE_NAME_ERROR);
                }

                /* Added the Service-Name tag.  */
                _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME, tag_length, service_name_ptr, &index);

                /* Update the service name pointer, length + null-terminated.  */
                service_name_ptr += (tag_length + 1);
                service_name_index += (tag_length + 1);
            }
#else
            /* Calculate the name length.  */
            if (_nx_utility_string_length_check((char *)(client_session_ptr -> nx_pppoe_service_name), &tag_length, NX_MAX_STRING_LENGTH))
            {
                nx_packet_release(packet_ptr);
                return(NX_PPPOE_SERVER_SERVICE_NAME_ERROR);
            }

            /* Added the Service-Name tag.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME, tag_length, client_session_ptr -> nx_pppoe_service_name, &index); 
#endif
        }
        else
        {

            /* Added the Service-Name tag.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME, 0, NX_NULL, &index); 
        }

#ifndef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
        /* Add any number of other Service-Name TAGs indicating other services.   */
        if  (pppoe_server_ptr -> nx_pppoe_service_name_count)
        {

            /* The PADO packet can contain any number of Service-Name TAGs.  */
            for (service_name_index = 0; service_name_index < pppoe_server_ptr -> nx_pppoe_service_name_count; service_name_index ++)
            {

                /* Check if this Service-Name has been added.  */
                if (pppoe_server_ptr -> nx_pppoe_service_name[service_name_index] == client_session_ptr -> nx_pppoe_service_name)
                    continue;

                /* Calculate the name length.  */
                if (_nx_utility_string_length_check((char *)(pppoe_server_ptr -> nx_pppoe_service_name[service_name_index]), &tag_length, NX_MAX_STRING_LENGTH))
                {
                    nx_packet_release(packet_ptr);
                    return(NX_PPPOE_SERVER_SERVICE_NAME_ERROR);
                }

                /* Added the Service-Name tag.  */
                _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME, tag_length, (UCHAR *)(pppoe_server_ptr -> nx_pppoe_service_name[service_name_index]), &index);
            }
        }
#endif

        /* If the Access Concentrator receives this Host-Uniq TAG, it MUST include the TAG unmodified in associated PADO or PADS response. 
           RFC2516, Appendix A, Host-Uniq.  */         
        if  (client_session_ptr -> nx_pppoe_host_uniq_size)
        {

            /* Added the Host-Uniq.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_HOST_UNIQ, client_session_ptr -> nx_pppoe_host_uniq_size, client_session_ptr -> nx_pppoe_host_uniq, &index);
        } 
                                                                                                                                                                                        
        /* If either the Host or Access Concentrator receives this Relay-Session-Id TAG, they MUST include it unmodified in any discovery packet they send as a response. 
           RFC2516, Appendix A, Relay-Session-Id.  */
        if  (client_session_ptr -> nx_pppoe_relay_session_id_size)
        {
                                               
            /* Added the Host-Uniq.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_RELAY_SESSION_ID, client_session_ptr -> nx_pppoe_relay_session_id_size, client_session_ptr -> nx_pppoe_relay_session_id, &index);
        }
    }
    else if (code == NX_PPPOE_SERVER_CODE_PADS)
    {

        /* Check the error.  */
        if (client_session_ptr -> nx_pppoe_error_flag & NX_PPPOE_SERVER_ERROR_SERVICE_NAME)
        {

            /* Added Service-Name-Error tag.  */   
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME_ERROR, 0, NX_NULL, &index); 
        }

        /* The PADS packet MUST contain one exactly one TAG of TAG_TYPE Service-Name. RFC2516, Section 5.4, Page6 */

        /* Check the service name.  */
        if  (client_session_ptr -> nx_pppoe_service_name)
        {

            /* Calculate the name length.  */
            if (_nx_utility_string_length_check((char *)(client_session_ptr -> nx_pppoe_service_name), &tag_length, NX_MAX_STRING_LENGTH))
            {
                nx_packet_release(packet_ptr);
                return(NX_PPPOE_SERVER_SERVICE_NAME_ERROR);
            }

            /* Added the Service-Name tag.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME, tag_length, client_session_ptr -> nx_pppoe_service_name, &index); 
        }
        else
        {

            /* Added the Service-Name tag.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME, 0, NX_NULL, &index); 
        }

        /* If the Access Concentrator receives this Host-Uniq TAG, it MUST include the TAG unmodified in associated PADO or PADS response. 
           RFC2516, Appendix A, Host-Uniq.  */         
        if  (client_session_ptr -> nx_pppoe_host_uniq_size)
        {

            /* Added the Host-Uniq.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_HOST_UNIQ, client_session_ptr -> nx_pppoe_host_uniq_size, client_session_ptr -> nx_pppoe_host_uniq, &index);
        }

        /* If either the Host or Access Concentrator receives this Relay-Session-Id TAG, they MUST include it unmodified in any discovery packet they send as a response. 
           RFC2516, Appendix A, Relay-Session-Id.  */
        if  (client_session_ptr -> nx_pppoe_relay_session_id_size)
        {

            /* Added the Host-Uniq.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_RELAY_SESSION_ID, client_session_ptr -> nx_pppoe_relay_session_id_size, client_session_ptr -> nx_pppoe_relay_session_id, &index);
        }
    }

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
    else if (code == NX_PPPOE_SERVER_CODE_PADT)
    {

        /* Check the Generic-Error.  */
        if (client_session_ptr -> nx_pppoe_generic_error)
        {

            /* Calculate the Generic-Error string length.  */
            if (_nx_utility_string_length_check((char *)(client_session_ptr -> nx_pppoe_generic_error), &tag_length, NX_MAX_STRING_LENGTH))
            {
                nx_packet_release(packet_ptr);
                return(NX_SIZE_ERROR);
            }

            /* Added the Generic-Error.  */
            _nx_pppoe_server_tag_string_add(work_ptr, NX_PPPOE_SERVER_TAG_TYPE_GENERIC_ERROR, tag_length, client_session_ptr -> nx_pppoe_generic_error, &index);
        }
    }
#endif

    /* Add the PPPoE header.  */     
    /*
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |  VER | TYPE  |      CODE       |         SESSION_ID           |
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * |           LENGTH               |          payload             
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    /* Add version and type.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_VER_TYPE, 1, NX_PPPOE_SERVER_VERSION_TYPE);  

    /* Add code.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_CODE, 1, code);

    /* Add the Session id.  */
    if (code == NX_PPPOE_SERVER_CODE_PADO)
    {

        /* Add session id.  */
        _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_SESSION_ID, 2, 0); 
    }
    else
    {

        /* Add session id.  */
        _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_SESSION_ID, 2, client_session_ptr -> nx_pppoe_session_id); 
    }

    /* Add length.  */
    _nx_pppoe_server_data_add(work_ptr + NX_PPPOE_SERVER_OFFSET_LENGTH, 2, (index - NX_PPPOE_SERVER_OFFSET_PAYLOAD));   

    /* Update the append pointer and length.  */
    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + index;
    packet_ptr -> nx_packet_length = index;

    /* Send PPPoE session packet.  */
    _nx_pppoe_server_packet_send(pppoe_server_ptr, client_session_ptr, packet_ptr, NX_LINK_PPPOE_DISCOVERY_SEND);

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_packet_send                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sends a PPPoE packet to the appropriate link driver.  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    client_session_ptr                    Pointer to Client Session     */
/*    packet_ptr                            Pointer to packet to send     */
/*    command                               Driver command                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (ip_link_driver)                      User supplied link driver     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_discovery_send       Send PPPoE Discovery packet   */ 
/*    _nx_pppoe_server_session_send         Send PPPoE Session packet     */ 
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
static VOID    _nx_pppoe_server_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PPPOE_CLIENT_SESSION *client_session_ptr, NX_PACKET *packet_ptr, UINT command)
{

NX_IP_DRIVER                driver_request;


    /* Initialize the driver request. */
    driver_request.nx_ip_driver_ptr =                   pppoe_server_ptr -> nx_pppoe_ip_ptr;
    driver_request.nx_ip_driver_packet =                packet_ptr;
    driver_request.nx_ip_driver_interface =             pppoe_server_ptr -> nx_pppoe_interface_ptr;
    driver_request.nx_ip_driver_physical_address_msw =  client_session_ptr -> nx_pppoe_physical_address_msw;
    driver_request.nx_ip_driver_physical_address_lsw =  client_session_ptr -> nx_pppoe_physical_address_lsw;   
    driver_request.nx_ip_driver_command =               command;   

    /* Sendout the PPPoE packet.  */
    (pppoe_server_ptr -> nx_pppoe_link_driver_entry) (&driver_request);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_tag_process                        PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function processes tags of PPPoE packet.                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    client_session_ptr                    Pointer to Client Session     */
/*    code                                  PPPoE code                    */
/*    tag_ptr                               Pointer to PPPoE tag          */
/*    length                                Length of PPPoe tags          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_data_get             Get the PPPoE data            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_discovery_packet_process                           */ 
/*                                          Process PPPoE Discovery packet*/  
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            string length verification, */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_pppoe_server_tag_process(NX_PPPOE_SERVER *pppoe_server_ptr, NX_PPPOE_CLIENT_SESSION *client_session_ptr, UINT code, UCHAR *tag_ptr, ULONG length)
{

ULONG           tag_type;
ULONG           tag_length;
UINT            tag_index = 0;
UINT            tag_service_name_count = 0; 
UINT            tag_service_name_valid = NX_FALSE;
UINT            service_name_index = 0;
UINT            service_name_length;
#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
UCHAR          *service_name_ptr;
#endif

    /* Initialize the value.  */
    client_session_ptr -> nx_pppoe_host_uniq_size = 0;  
    client_session_ptr -> nx_pppoe_relay_session_id_size = 0;
    client_session_ptr -> nx_pppoe_error_flag = 0;       
#ifndef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
    client_session_ptr -> nx_pppoe_service_name = NX_NULL;
#endif


    /* Loop to process the tag.  */
    while (tag_index + 4 <= length)
    {

        /* Pickup the tag type.  */   
        tag_type = _nx_pppoe_server_data_get(tag_ptr + tag_index, 2); 

        /* Update the index.  */
        tag_index += 2;

        /* Pickup the tag length.  */
        tag_length = _nx_pppoe_server_data_get(tag_ptr + tag_index, 2);
        
        /* Update the index.  */
        tag_index += 2;

        /* Check for valid tag length.  */
        if ((tag_index + tag_length) > length)
        {
            return(NX_PPPOE_SERVER_PACKET_PAYLOAD_ERROR);
        }

        /* Process the option type. */
        switch (tag_type)
        {
                                     
            case NX_PPPOE_SERVER_TAG_TYPE_END_OF_LIST:
            {

                /* End tag.  */
                break;
            }
            case NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME:
            {

                /* Service name tag.  */
                tag_service_name_count ++;

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
                if (code == NX_PPPOE_SERVER_CODE_PADI)
                {

                    /* The service name of incoming PADI must match the default service name of PPPoE.  */ 
                    /* Check the tag length.  */
                    if (tag_length == 0)
                    {

                        /* Check the default service name.  */
                        if (pppoe_server_ptr -> nx_pppoe_service_name_count == 0)
                            tag_service_name_valid = NX_TRUE;
                    }
                    else
                    {

                        /* Loop to find the service name.  */
                        while (service_name_index < pppoe_server_ptr -> nx_pppoe_service_name_count)
                        {

                            /* Get the length of service name.  */
                            if (_nx_utility_string_length_check((char *)(pppoe_server_ptr -> nx_pppoe_service_name[service_name_index]), 
                                                                &service_name_length, tag_length))
                            {
                                service_name_index++;
                                continue;
                            }

                            /* Compare the same service name with PPPoE Service Name.  */
                            if ((service_name_length == tag_length) &&
                                (!memcmp(tag_ptr + tag_index, (pppoe_server_ptr -> nx_pppoe_service_name[service_name_index]), tag_length)))
                            {
                                tag_service_name_valid = NX_TRUE;
                                client_session_ptr -> nx_pppoe_service_name = pppoe_server_ptr -> nx_pppoe_service_name[service_name_index];
                                client_session_ptr -> nx_pppoe_service_name_length = service_name_length;
                                break;
                            }
                            service_name_index ++;
                        }
                    }
                }
                else if (code == NX_PPPOE_SERVER_CODE_PADR)
                {

                    /* Compare the service name with session service name.  */
                    /* Check the tag length.  */
                    if ((tag_length == 0) && (client_session_ptr -> nx_pppoe_service_name_length == 0))
                    {          

                        /* Update the information.  */
                        tag_service_name_valid = NX_TRUE;
                        client_session_ptr -> nx_pppoe_service_name = NX_NULL;
                        client_session_ptr -> nx_pppoe_service_name_length = 0;
                        break;
                    }

                    /* Set the service name pointer.  */
                    service_name_ptr = client_session_ptr -> nx_pppoe_service_name;
                                                                                        
                    /* Loop to compare the service name with session service name.  */
                    while(service_name_index < client_session_ptr -> nx_pppoe_service_name_length)
                    {

                        /* Get the service name.  */
                        if (_nx_utility_string_length_check((char *)(service_name_ptr), &service_name_length, NX_MAX_STRING_LENGTH))
                        {
                            return(NX_PPPOE_SERVER_SERVICE_NAME_ERROR);
                        }

                        /* Check the tag length.  */
                        if ((tag_length == 0) && (service_name_length == 0))
                        {
                            tag_service_name_valid = NX_TRUE;
                        }
                        else if ((tag_length != 0) && (tag_length == service_name_length))
                        {

                            /* Compare the service name.  */
                            if (!memcmp(tag_ptr + tag_index, service_name_ptr, tag_length))
                                tag_service_name_valid = NX_TRUE;
                        }

                        /* Update the service name information for PADS.  */
                        if (tag_service_name_valid == NX_TRUE)
                        {
                            client_session_ptr -> nx_pppoe_service_name = service_name_ptr;
                            client_session_ptr -> nx_pppoe_service_name_length = service_name_length;
                            break;
                        }

                        /* Update the service name pointer, length + null-terminated.  */
                        service_name_ptr += (service_name_length + 1);
                        service_name_index += (service_name_length + 1);
                    }
                }
#else

                /* Check the tag length.  */
                if (tag_length == 0)
                {

                    /* When the tag length is zero this tag is used to indicate that any service is acceptable.  */
                    tag_service_name_valid = NX_TRUE;
                    break;
                }

                /* Compare the service name with PPPoE Service name.  */
                while (service_name_index < pppoe_server_ptr -> nx_pppoe_service_name_count)
                {

                    /* Get the length of service name.  */
                    if (_nx_utility_string_length_check((char *)(pppoe_server_ptr -> nx_pppoe_service_name[service_name_index]), 
                                                        &service_name_length, tag_length))
                    {
                        service_name_index++;
                        continue;
                    }

                    /* Find the same service name.  */
                    if ((service_name_length == tag_length) &&
                        (!memcmp(tag_ptr + tag_index, (pppoe_server_ptr -> nx_pppoe_service_name[service_name_index]), tag_length)))
                    {

                        /* Update the information.  */
                        tag_service_name_valid = NX_TRUE;
                        client_session_ptr -> nx_pppoe_service_name = pppoe_server_ptr -> nx_pppoe_service_name[service_name_index];
                        break;
                    }
                    service_name_index ++;
                }
#endif
                break;
            } 
            case NX_PPPOE_SERVER_TAG_TYPE_AC_NAME:
            {

                if (pppoe_server_ptr -> nx_pppoe_ac_name)
                {
                    /* Check the access concentrator name.  */
                    if ((pppoe_server_ptr -> nx_pppoe_ac_name_length != tag_length) ||
                        (memcmp(tag_ptr + tag_index, (pppoe_server_ptr -> nx_pppoe_ac_name), tag_length)))
                    {                

                        return(NX_PPPOE_SERVER_AC_NAME_ERROR);
                    }
                }
                else
                {
                    /* If user does not separately set Access Concentrator name, will use PPPoE server name as Access Concentrator name.*/
                    if ((pppoe_server_ptr -> nx_pppoe_name_length != tag_length) ||
                        (memcmp(tag_ptr + tag_index, (pppoe_server_ptr -> nx_pppoe_name), tag_length)))
                    {                

                        return(NX_PPPOE_SERVER_AC_NAME_ERROR);
                    }   
                }
                break;
            }
            case NX_PPPOE_SERVER_TAG_TYPE_HOST_UNIQ:
            {

                /* Check the cache for Host-Uniq.  */
                if (tag_length> NX_PPPOE_SERVER_MAX_HOST_UNIQ_SIZE)
                    return (NX_PPPOE_SERVER_HOST_UNIQ_CACHE_ERROR);

                /* Save the Host-Uniq.  */
                memcpy(client_session_ptr -> nx_pppoe_host_uniq, tag_ptr + tag_index, tag_length); /* Use case of memcpy is verified. */

                /* Set the Host-Uniq size.  */
                client_session_ptr -> nx_pppoe_host_uniq_size = tag_length;
                break;
            }
            case NX_PPPOE_SERVER_TAG_TYPE_RELAY_SESSION_ID:
            {

                /* Check the cache for Relay-Session_Id.  */
                if (tag_length> NX_PPPOE_SERVER_MAX_RELAY_SESSION_ID_SIZE)
                    return (NX_PPPOE_SERVER_RELAY_SESSION_ID_CACHE_ERROR);

                /* Save the Relay-Session_Id.  */
                memcpy(client_session_ptr -> nx_pppoe_relay_session_id, tag_ptr + tag_index, tag_length); /* Use case of memcpy is verified. */
                
                /* Set the Relay-Session_Id size.  */
                client_session_ptr -> nx_pppoe_relay_session_id_size = tag_length;
                break;
            }
            default:
                break;
        }

        /* Move to the next tag. */
        tag_index += tag_length; 
    }

    /* Check the code.  */
    if ((code == NX_PPPOE_SERVER_CODE_PADI) || (code == NX_PPPOE_SERVER_CODE_PADR))
    {

        /* The PADI and PADR packet MUST contains exactly one TAG of TAG_TYPE Service- Name,  RFC2516  */
        if ((tag_service_name_count != 1) || (tag_service_name_valid != NX_TRUE))
        {

            /* Set the service name error flag.  */
            client_session_ptr -> nx_pppoe_error_flag |= NX_PPPOE_SERVER_ERROR_SERVICE_NAME;

            /* Service-Name tag error.  */
            return(NX_PPPOE_SERVER_SERVICE_NAME_ERROR);
        }
    } 

    /* TAGs processed.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_data_get                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function gets the datas of PPPoE packet.                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    data                                  Pointer to buffer data        */ 
/*    size                                  Size of data value            */ 
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
/*    _nx_pppoe_server_packet_receive       Receive the PPPoE packet      */ 
/*    _nx_pppoe_server_discovery_packet_process                           */ 
/*                                          Process PPPoE Discovery packet*/ 
/*    _nx_pppoe_server_session_packet_process                             */ 
/*                                          Process PPPoE Session packet  */  
/*    _nx_pppoe_server_tag_process          Process PPPoE TAGs            */  
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
static ULONG  _nx_pppoe_server_data_get(UCHAR *data, UINT size)
{

ULONG   value = 0;


    /* Process the data retrieval request.  */
    while (size-- > 0)
    {

        /* Build return value.  */
        value = (value << 8) | *data++;
    }

    /* Return value.  */
    return(value);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_data_add                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function adds the datas into PPPoE packet.                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    data                                  Pointer to buffer data        */ 
/*    size                                  Size of data value            */
/*    value                                 Value to add                  */  
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
/*    _nx_pppoe_server_discovery_send       Send PPPoE Discovery packet   */ 
/*    _nx_pppoe_server_session_send         Send PPPoE Session packet     */  
/*    _nx_pppoe_server_tag_string_add       Add PPPoE string TAG          */  
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
static VOID  _nx_pppoe_server_data_add(UCHAR *data, UINT size, ULONG value)
{

    /* Make sure that data is left justified.  */
    switch (size)
    {

        case 1:

            value <<= 24;
            break;

        case 2:

            value <<= 16;
            break;

        case 3:

            value <<= 8;
            break;

        default:
            break;
    }

    /* Store the value.  */
    while (size-- > 0)
    {

        *data = (UCHAR) ((value >> 24) & 0xff);
        data++;
        value <<= 8;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_string_add                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function adds the string into PPPoE packet.                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dest                                  Pointer to destination buffer */ 
/*    source                                Pointer to source buffer      */ 
/*    size                                  Number of bytes to add        */
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
/*    _nx_pppoe_server_tag_string_add       Add PPPoE string TAG          */  
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
static VOID  _nx_pppoe_server_string_add(UCHAR *dest, UCHAR *source, UINT size)
{

    /* Loop to copy all bytes.  */
    while (size-- > 0)
    {
        
        /* Copy a byte.  */
        *dest++ = *source++;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_tag_string_add                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function adds the TAG with string into PPPoE packet.           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data_ptr                              Pointer to data buffer        */ 
/*    tag_type                              Type of TAG                   */ 
/*    tag_length                            Length of TAG                 */ 
/*    tag_value_string                      String value of TAG to add    */
/*    index                                 Location into data buffer     */ 
/*                                             to write data              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_pppoe_server_data_add             Add PPPoE data                */  
/*    _nx_pppoe_server_string_add           Add PPPoE string data         */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_server_discovery_send       Send PPPoE Discovery packet   */
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
static UINT  _nx_pppoe_server_tag_string_add(UCHAR *data_ptr, UINT tag_type, UINT tag_length, UCHAR *tag_value_string, UINT *index)
{

    /* Add the tag type.  */
    _nx_pppoe_server_data_add(data_ptr + (*index), 2, tag_type);
    (*index) += 2;

    /* Add the tag length.  */
    _nx_pppoe_server_data_add(data_ptr + (*index), 2, tag_length);
    (*index) += 2;

    /* Add the tag value string.  */
    _nx_pppoe_server_string_add(data_ptr + (*index), tag_value_string, tag_length);
    (*index) += tag_length;

    /* Return a successful completion.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_find                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function finds a PPPoE session by the client hardware mac      */ 
/*    address and session id. In Discovery Stage, match the client harware*/ 
/*    address, if not match, mark one available session. In Session Stage,*/ 
/*    match the client harware address and session id, if not match,      */ 
/*    return NX_PPPOE_CLIENT_SESSION_NOT_FOUND.                           */  
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_server_ptr                      Pointer to PPPoE control block*/ 
/*    client_mac_msw                        Client physical address MSW   */
/*    client_mac_lsw                        Client physical address LSW   */
/*    session_id                            Session ID                    */ 
/*    session_index                         Session Index                 */
/*    client_session_ptr                    Pointer to Client Session     */
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
/*    _nx_pppoe_server_discovery_packet_process                           */ 
/*                                          Process PPPoE Discovery packet*/ 
/*    _nx_pppoe_server_session_packet_process                             */ 
/*                                          Process PPPoE Session packet  */ 
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
static UINT  _nx_pppoe_server_session_find(NX_PPPOE_SERVER *pppoe_server_ptr, ULONG client_mac_msw, ULONG client_mac_lsw, 
                                           ULONG session_id, UINT *session_index, NX_PPPOE_CLIENT_SESSION **client_session_ptr)
{

UINT                i;
UINT                available_index;


    /* Initialize the value.   */
    i = 0;
    available_index = NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER;

    for(i = 0; i < NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER; i ++) 
    {

        /* Check if this session is valid.  */
        if (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_valid == NX_TRUE)
        {

            /* Compare the physical address.  */
            if ((pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_physical_address_msw != client_mac_msw) ||
                (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_physical_address_lsw != client_mac_lsw))
                continue;

            /* If the session id is not zero, means the session has been established.  */
            if (session_id != 0)
            {

                /* Compare the session id.  */
                if (pppoe_server_ptr -> nx_pppoe_client_session[i].nx_pppoe_session_id != session_id)
                    continue;
            }

            /* Yes, find the matched session.  */
            *session_index = i;
            *client_session_ptr = &(pppoe_server_ptr -> nx_pppoe_client_session[i]);

            return(NX_PPPOE_SERVER_SUCCESS);
        }
        else
        {

            /* Set the first available index.  */
            if (i < available_index)
                available_index = i;
        }
    }

    /* If the session id is not zero, means the session has been established.  */
    if (session_id != 0)
    {
        return(NX_PPPOE_SERVER_CLIENT_SESSION_NOT_FOUND);
    }              

    /* Check if there is available room in the table for a new client session. */
    if (available_index >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
    {

        /* No, we cannot add this client session into the server's table. */
        return(NX_PPPOE_SERVER_CLIENT_SESSION_FULL);
    }
    
    /* Set the session.  */
    pppoe_server_ptr -> nx_pppoe_client_session[available_index].nx_pppoe_physical_address_msw = client_mac_msw; 
    pppoe_server_ptr -> nx_pppoe_client_session[available_index].nx_pppoe_physical_address_lsw = client_mac_lsw;

    /* Mark this session is valid.  */
    pppoe_server_ptr -> nx_pppoe_client_session[available_index].nx_pppoe_valid = NX_TRUE;

    /* Set local pointer to an available slot. */
    *session_index = available_index;
    *client_session_ptr = &(pppoe_server_ptr -> nx_pppoe_client_session[available_index]);

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_cleanup                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function cleans up the PPPoE session.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_session_ptr                    Pointer to Client Session     */
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
/*    _nx_pppoe_server_session_terminate    Terminate the PPPoE session   */ 
/*    _nx_pppoe_server_session_packet_process                             */ 
/*                                          Process PPPoE Session packet  */ 
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
static UINT  _nx_pppoe_server_session_cleanup(NX_PPPOE_CLIENT_SESSION *client_session_ptr)
{

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
NX_PACKET   *next_packet;
NX_PACKET   *current_packet;
#endif

    /* Cleanup the client session.  */
    client_session_ptr -> nx_pppoe_valid = NX_FALSE;
    client_session_ptr -> nx_pppoe_session_id = NX_NULL;
    client_session_ptr -> nx_pppoe_physical_address_msw = NX_NULL;
    client_session_ptr -> nx_pppoe_physical_address_lsw = NX_NULL;
    client_session_ptr -> nx_pppoe_service_name = NX_NULL;
    
#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
    /* Loop to release the queued packet.  */
    next_packet = client_session_ptr -> nx_pppoe_deferred_received_packet_head;

    /* Release any packets queued up.  */
    while (next_packet)
    {

        /* Setup the current packet pointer.  */
        current_packet = next_packet;

        /* Move to the next packet.  */
        next_packet =  next_packet -> nx_packet_queue_next;

        /* Release the current packet.  */
        nx_packet_release(current_packet);
    }

    /* Cleanup the parameters.  */
    client_session_ptr -> nx_pppoe_deferred_received_packet_head = NX_NULL;
    client_session_ptr -> nx_pppoe_deferred_received_packet_tail = NX_NULL;
    client_session_ptr -> nx_pppoe_packet_receive_stopped = NX_FALSE;
#endif

    /* Return success.  */
    return(NX_PPPOE_SERVER_SUCCESS);
}


#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppInitInd                                          PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function configures the default Service Namet that the PPPoE   */ 
/*    should use to filter incoming PADI requests.                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    length                                The number of bytes in aData  */ 
/*    aData                                 Contains PPPoE Service Name   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_service_name_set     Set the service name          */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
VOID  PppInitInd(UINT length, UCHAR *aData)
{

    /* Check to see if PPPoE instance is created.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) ||
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;

    /* Check the length.  */
    if (length == 0)
    {

        /* Clean the default service name.  */  
        _nx_pppoe_server_service_name_set(_nx_pppoe_server_created_ptr, NX_NULL, 0); 
    }
    else
    {

        /* Transmit the Service.  */
        nx_pppoe_service_name[0] = aData;

        /* Set the default service name.  */
        _nx_pppoe_server_service_name_set(_nx_pppoe_server_created_ptr, nx_pppoe_service_name, 1);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppDiscoverCnf                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function defines the Service Name field of the PADO packet.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    interfaceHandle                       The handle of session         */ 
/*    length                                The number of bytes in aData  */ 
/*    aData                                 Contains PPPoE Service Name   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_pppoe_server_discovery_send       Send PPPoE Discovery          */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
VOID PppDiscoverCnf(UINT length, UCHAR *aData, UINT interfaceHandle)
{

NX_PPPOE_CLIENT_SESSION *client_session_ptr;

    /* Check to see if PPPoE instance is created.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) ||
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;
                     
    /* Check to see if PPPoE is enabled.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_enabled != NX_TRUE)
        return;

    /* Check for invalid session index.  */
    if(interfaceHandle >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return;
             
    /* Check to see if PPPoE session is valid.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_valid != NX_TRUE)
        return;

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Get the Session pointer.  */
    client_session_ptr = &(_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle]);

    /* Check if this session is valid.  */
    if (client_session_ptr -> nx_pppoe_valid != NX_TRUE)
    {

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        return;
    }

    /* Set the Service Name field.  */
    client_session_ptr -> nx_pppoe_service_name = aData;
    client_session_ptr -> nx_pppoe_service_name_length = length;

    /* Send PADO packet.  */  
    _nx_pppoe_server_discovery_send(_nx_pppoe_server_created_ptr, client_session_ptr, NX_PPPOE_SERVER_CODE_PADO);

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppOpenCnf                                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function allows PPPoE to accept or reject the PPPoE Session.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    accept                                The flag to accept or reject  */ 
/*    interfaceHandle                       The handle of session         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_pppoe_server_discovery_send       Send PPPoE Discovery          */ 
/*    _nx_pppoe_server_session_cleanup      Cleanup the PPPoE Session     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
VOID PppOpenCnf(UCHAR accept, UINT interfaceHandle)
{

NX_PPPOE_CLIENT_SESSION *client_session_ptr;

    /* Check to see if PPPoE instance is created.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) ||
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;
                     
    /* Check to see if PPPoE is enabled.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_enabled != NX_TRUE)
        return;

    /* Check for invalid session index.  */
    if(interfaceHandle >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return;
             
    /* Check to see if PPPoE session is valid.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_valid != NX_TRUE)
        return;

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
     
    /* Get the Session pointer.  */
    client_session_ptr = &(_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle]);

    /* Check the accept flag.  */
    if (accept == NX_TRUE)
    {

        /* Send PADS to accept the PPPoE Session.  */
        _nx_pppoe_server_discovery_send(_nx_pppoe_server_created_ptr, client_session_ptr, NX_PPPOE_SERVER_CODE_PADS);
    }
    else
    {

        /* Reject the PPPoE Session.  */
        _nx_pppoe_server_session_cleanup(client_session_ptr);
    }    

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppCloseInd                                         PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function allows PPPoE to terminate PPPoE Session.              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    interfaceHandle                       The handle of session         */ 
/*    causeCode                             The reason for terminating    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pppoe_server_session_terminate    Terminate the PPPoE Session   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
VOID PppCloseInd(UINT interfaceHandle, UCHAR *causeCode)
{

    /* Check to see if PPPoE instance is created.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) ||
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;
                     
    /* Check to see if PPPoE is enabled.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_enabled != NX_TRUE)
        return;

    /* Check for invalid session index.  */
    if(interfaceHandle >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return;

    /* Check to see if PPPoE session is established.  */
    if ((_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_valid != NX_TRUE) || 
        (_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_session_id == 0))
        return;

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Set the reason for terminating the session.  */
    _nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_generic_error = causeCode; 

    /* Send PADT to terminate the session.  */
    _nx_pppoe_server_session_terminate(_nx_pppoe_server_created_ptr, interfaceHandle);

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppCloseCnf                                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function confirm that the session has been freed.              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    interfaceHandle                       The handle of session         */ 
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
/*    Application                                                         */ 
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
VOID PppCloseCnf(UINT interfaceHandle)
{

    /* Check to see if PPPoE instance is created.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) ||
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;

    /* Check for invalid session index.  */
    if (interfaceHandle >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return;

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Cleanup the PPPoE session.  */
    _nx_pppoe_server_session_cleanup(&(_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle]));
    
    /* Release the IP internal mutex.  */
    tx_mutex_put(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppTransmitDataCnf                                  PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function allows TTP's software to receive new data frame.      */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    interfaceHandle                       The handle of session         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*    tx_event_flags_set                    Set events for PPPoE thread   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
VOID PppTransmitDataCnf(UINT interfaceHandle, UCHAR *data_ptr, UINT packet_id)
{

NX_PACKET   *packet_ptr;

    NX_PARAMETER_NOT_USED(data_ptr);

    /* Check to see if PPPoE instance is created.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) ||
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;

    /* Check for invalid session index.  */
    if(interfaceHandle >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return;
                
    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
    
    /* Release the packet.  */
    if (packet_id)
    {
        packet_ptr = (NX_PACKET *)(packet_id);
        nx_packet_release(packet_ptr);  
    }

    /* Clean the flag to receive the next packet.  */
    _nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_packet_receive_stopped = NX_FALSE;

    /* Check if this session queued the packets.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_deferred_received_packet_head)
    {

        /* Wakeup PPPoE helper thread to process the PPPoE Session receive.  */
        tx_event_flags_set(&(_nx_pppoe_server_created_ptr -> nx_pppoe_events), NX_PPPOE_SERVER_SESSION_RECEIVE_EVENT, TX_OR);
    }

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(_nx_pppoe_server_created_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    PppReceiveDataInd                                   PORTABLE C      */ 
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sends data fram over Ethernet.                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    interfaceHandle                       The handle of session         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_server_session_send         Send PPPoE session data       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
VOID PppReceiveDataInd(UINT interfaceHandle, UINT data_length, UCHAR *data_ptr)
{                                                                                          

    /* Check for invalid input pointers.  */
    if ((_nx_pppoe_server_created_ptr == NX_NULL) || 
        (_nx_pppoe_server_created_ptr -> nx_pppoe_id != NX_PPPOE_SERVER_ID))
        return;
      
    /* Check to see if PPPoE is enabled.  */
    if (_nx_pppoe_server_created_ptr -> nx_pppoe_enabled != NX_TRUE)
        return;

    /* Check for invalid session index.  */
    if(interfaceHandle >= NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER)
        return;

    /* Check to see if PPPoE session is established.  */
    if ((_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_valid != NX_TRUE) || 
        (_nx_pppoe_server_created_ptr -> nx_pppoe_client_session[interfaceHandle].nx_pppoe_session_id == 0))
        return;

    /* Send data.  */
    _nx_pppoe_server_session_send(_nx_pppoe_server_created_ptr, interfaceHandle, data_ptr, data_length);
}
#endif
#endif /* NX_DISABLE_IPV4 */
