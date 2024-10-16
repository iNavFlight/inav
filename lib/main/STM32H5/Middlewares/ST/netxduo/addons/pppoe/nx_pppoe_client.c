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

#define NX_PPPOE_CLIENT_SOURCE_CODE


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
#include "nx_packet.h"
#include "nx_pppoe_client.h"
#include "tx_thread.h"
#include "tx_timer.h"

/* Define the PPPoE created list head pointer and count.  */ 
NX_PPPOE_CLIENT  *_nx_pppoe_client_created_ptr = NX_NULL;


/* Define internal PPPoE services. */

static VOID    _nx_pppoe_client_thread_entry(ULONG pppoe_client_ptr_value);
static VOID    _nx_pppoe_client_timer_entry(ULONG pppoe_client_address);
static VOID    _nx_pppoe_client_packet_receive(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr);
static VOID    _nx_pppoe_client_discovery_packet_process(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr, ULONG server_mac_msw, ULONG server_mac_lsw);
static VOID    _nx_pppoe_client_session_packet_process(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr, ULONG server_mac_msw, ULONG server_mac_lsw);
static UINT    _nx_pppoe_client_discovery_send(NX_PPPOE_CLIENT *pppoe_client_ptr, UINT code);
static VOID    _nx_pppoe_client_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr, UINT command); 
static ULONG   _nx_pppoe_client_data_get(UCHAR *data, UINT size);
static VOID    _nx_pppoe_client_data_add(UCHAR *data, UINT size, ULONG value);
static VOID    _nx_pppoe_client_string_add(UCHAR *dest, UCHAR *source, UINT size);
static UINT    _nx_pppoe_client_tag_string_add(UCHAR *data_ptr, UINT tag_type, UINT tag_length,  UCHAR *tag_value_string, UINT *index);
static UINT    _nx_pppoe_client_session_cleanup(NX_PPPOE_CLIENT *pppoe_client_ptr);
static VOID    _nx_pppoe_client_session_connect_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
static VOID    _nx_pppoe_client_session_thread_resume(TX_THREAD **suspension_list_head, UINT status);
static VOID    _nx_pppoe_client_session_thread_suspend(TX_THREAD **suspension_list_head, VOID (*suspend_cleanup)(TX_THREAD * NX_CLEANUP_PARAMETER), 
                                                     NX_PPPOE_CLIENT *pppoe_client_ptr, TX_MUTEX *mutex_ptr, ULONG wait_option);


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Client instance create */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
/*    name                                  Name of this PPPoE instance   */
/*    ip_ptr                                Pointer to IP control block   */
/*    interface_index                       IP Interface Index            */
/*    pool_ptr                              packet pool                   */
/*    stack_ptr                             Pointer stack area for PPPoE  */
/*    stack_size                            Size of PPPoE stack area      */
/*    priority                              Priority of PPPoE  thread     */
/*    pppoe_link_driver                     User supplied link driver     */
/*    pppoe_packet_receive                  User supplied function to     */
/*                                            receive PPPoE packet        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_create               Actual PPPoE instance create  */
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
UINT  _nxe_pppoe_client_create(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                               NX_PACKET_POOL *pool_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority,
                               VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                               VOID (*pppoe_packet_receive)(NX_PACKET *packet_ptr))
{   

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id == NX_PPPOE_CLIENT_ID) || 
        (ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (pool_ptr == NX_NULL) || (pool_ptr -> nx_packet_pool_id != NX_PACKET_POOL_ID) ||
        (stack_ptr == NX_NULL) || (pppoe_link_driver == NX_NULL) || (pppoe_packet_receive == NX_NULL)) 
        return(NX_PPPOE_CLIENT_PTR_ERROR);
                                         
    /* Check for invalid interface ID */
    if(interface_index >= NX_MAX_PHYSICAL_INTERFACES)
        return(NX_PPPOE_CLIENT_INVALID_INTERFACE);
                                                   
    /* Check for interface being valid. */
    if(!ip_ptr -> nx_ip_interface[interface_index].nx_interface_valid)
        return(NX_PPPOE_CLIENT_INVALID_INTERFACE);

    /* Check for payload size of packet pool.  */
    if (pool_ptr -> nx_packet_pool_payload_size < NX_PPPOE_CLIENT_MIN_PACKET_PAYLOAD_SIZE)
        return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);

    /* Check for a memory size error.  */
    if (stack_size < TX_MINIMUM_STACK)
        return(NX_PPPOE_CLIENT_MEMORY_SIZE_ERROR);
                          
    /* Check the priority specified.  */
    if (priority >= TX_MAX_PRIORITIES)
        return(NX_PPPOE_CLIENT_PRIORITY_ERROR);

    /* Call actual PPPoE client instance create function.  */
    status =  _nx_pppoe_client_create(pppoe_client_ptr, name, ip_ptr, interface_index, 
                                      pool_ptr, stack_ptr, stack_size, priority, 
                                      pppoe_link_driver, pppoe_packet_receive); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function creates an PPPoE Client instance, including setting   */
/*    up all appropriate data structures, creating PPPoE event flag       */
/*    object and PPPoE thread.                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
/*    name                                  Name of this PPPoE instance   */
/*    ip_ptr                                Pointer to IP control block   */ 
/*    interface_index                       Interface Index               */
/*    pool_ptr                              packet pool                   */
/*    stack_ptr                             Pointer stack area for PPPoE  */
/*    stack_size                            Size of PPPoE stack area      */
/*    priority                              Priority of PPPoE  thread     */
/*    pppoe_link_driver                     User supplied link driver     */ 
/*    pppoe_packet_receive                  User supplied function to     */
/*                                            receive PPPoE packet        */
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
UINT  _nx_pppoe_client_create(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                              NX_PACKET_POOL *pool_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority,
                              VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                              VOID (*pppoe_packet_receive)(NX_PACKET *packet_ptr))
{

TX_INTERRUPT_SAVE_AREA    


    /* Initialize the PPPoE Server control block to zero.  */
    memset((void *) pppoe_client_ptr, 0, sizeof(NX_PPPOE_CLIENT));

    /* Save the PPPoE name.  */
    pppoe_client_ptr -> nx_pppoe_name = name;

    /* Save the IP pointer.  */
    pppoe_client_ptr -> nx_pppoe_ip_ptr = ip_ptr; 

    /* Setup the interface index and interface pointer.  */
    pppoe_client_ptr -> nx_pppoe_interface_ptr = &(ip_ptr -> nx_ip_interface[interface_index]);  

    /* Save the packet pool pointer.  */
    pppoe_client_ptr -> nx_pppoe_packet_pool_ptr = pool_ptr;

    /* Setup the link driver address.  */
    pppoe_client_ptr -> nx_pppoe_link_driver_entry = pppoe_link_driver;

    /* Setup the function to receive PPPoE packet.  */
    pppoe_client_ptr ->nx_pppoe_packet_receive = pppoe_packet_receive;

    /* Create event flag group to control the PPPoE processing thread.  */
    tx_event_flags_create(&(pppoe_client_ptr -> nx_pppoe_events), "PPPoE Client EVENTS") ;

    /* Create the PPPoE processing thread.  */
    tx_thread_create(&(pppoe_client_ptr -> nx_pppoe_thread), "PPPoE Client THREAD", _nx_pppoe_client_thread_entry, (ULONG) pppoe_client_ptr,  
                     stack_ptr, stack_size, priority, priority, NX_PPPOE_CLIENT_THREAD_TIME_SLICE, TX_AUTO_START);

    /* Create the PPPoE timer.  */
    tx_timer_create(&(pppoe_client_ptr -> nx_pppoe_timer), "PPPoE Client timer",
                    (VOID (*)(ULONG))_nx_pppoe_client_timer_entry, (ULONG)pppoe_client_ptr,
                    NX_IP_PERIODIC_RATE, NX_IP_PERIODIC_RATE, TX_NO_ACTIVATE);

    /* Otherwise, the PPPoE initialization was successful.  Place the
       PPPoE control block on created PPPoE instance.  */
    TX_DISABLE

    /* Load the PPPoE Client ID field in the PPPoE Client control block.  */
    pppoe_client_ptr -> nx_pppoe_id =  NX_PPPOE_CLIENT_ID;   

    /* Set the pointer of global variable PPPoE.  */
    _nx_pppoe_client_created_ptr = pppoe_client_ptr;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Client instance delete */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_delete               Actual PPPoE instance delete  */
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
UINT  _nxe_pppoe_client_delete(NX_PPPOE_CLIENT *pppoe_client_ptr)              
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE client instance delete function.  */
    status =  _nx_pppoe_client_delete(pppoe_client_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function deletes an PPPoE Client instance. including deleting  */
/*    PPPoE event flag object and PPPoE thread.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
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
UINT  _nx_pppoe_client_delete(NX_PPPOE_CLIENT *pppoe_client_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Determine if the caller is the PPPoE thread itself. This is not allowed since
       a thread cannot delete itself in ThreadX.  */
    if (&pppoe_client_ptr -> nx_pppoe_thread == tx_thread_identify())
    {

        /* Invalid caller of this routine, return an error!  */
        return(NX_CALLER_ERROR);
    }
              
    /* Disable interrupts.  */
    TX_DISABLE

    /* Clear the PPPOE ID to show that it is no longer valid.  */
    pppoe_client_ptr -> nx_pppoe_id = 0;

    /* Clear the created pointer.  */
    _nx_pppoe_client_created_ptr = NX_NULL;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Terminate the thread.  */
    tx_thread_terminate(&(pppoe_client_ptr -> nx_pppoe_thread));

    /* Delete the PPPoE thread.  */
    tx_thread_delete(&(pppoe_client_ptr -> nx_pppoe_thread));   

    /* Delete the event flag group.  */
    tx_event_flags_delete(&(pppoe_client_ptr -> nx_pppoe_events));

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_service_name_set                  PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    service_name                          Pointer to an service name.   */
/*                                            Service name must be        */
/*                                            Null-terminated string.     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_service_name_set     Actual PPPoE service name set */
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
UINT  _nxe_pppoe_client_service_name_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE service name set function.  */
    status =  _nx_pppoe_client_service_name_set(pppoe_client_ptr, service_name); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_service_name_set                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets the service name for PPPoE Client.               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    service_name                          Pointer to an service name.   */
/*                                            Service name must be        */
/*                                            Null-terminated string.     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_utility_string_length_check       Check string length           */  
/*    _nx_pppoe_client_service_name_set     Actual PPPoE service name set */
/*                                          function                      */
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
UINT  _nx_pppoe_client_service_name_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name)
{
UINT status;
UINT service_name_length = 0;


    /* Calculate the service_name length.  */
    if (_nx_utility_string_length_check((CHAR *)service_name, &service_name_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_SIZE_ERROR);
    }
     
    /* Call actual PPPoE service name set function.  */
    status =  _nx_pppoe_client_service_name_set_extended(pppoe_client_ptr, service_name, service_name_length); 

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_service_name_set_extended         PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    service_name                          Pointer to an service name.   */
/*                                            Service name must be        */
/*                                            Null-terminated string.     */
/*    service_name_length                   Length of service_name        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_service_name_set_extended                          */
/*                                          Actual PPPoE service name set */
/*                                          function                      */
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
UINT  _nxe_pppoe_client_service_name_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name, UINT service_name_length)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE service name set function.  */
    status =  _nx_pppoe_client_service_name_set_extended(pppoe_client_ptr, service_name, service_name_length); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_service_name_set_extended          PORTABLE C      */ 
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets the service name for PPPoE Client.               */
/*                                                                        */ 
/*    Note: The string of service_name must be NULL-terminated and length */
/*    of service_name matches the length specified in the argument list.  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    service_name                          Pointer to an service name.   */
/*                                            Service name must be        */
/*                                            Null-terminated string.     */
/*    service_name_length                   Length of service_name        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_utility_string_length_check       Check string length           */ 
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
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            string length verification, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_pppoe_client_service_name_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name, UINT service_name_length)
{ 
 
UINT temp_service_name_length = 0;
    
    /* Get the length of service_name string.  */
    if (_nx_utility_string_length_check((CHAR*)service_name, &temp_service_name_length, service_name_length))
        return(NX_SIZE_ERROR);

    /* Check the service_name length.  */
    if (service_name_length != temp_service_name_length)
        return(NX_SIZE_ERROR);    
     
    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup service name pointer.  */
    pppoe_client_ptr -> nx_pppoe_service_name = service_name;

    /* Setup service name length.  */
    pppoe_client_ptr -> nx_pppoe_service_name_length = service_name_length;

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_host_uniq_set                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE host uniq set          */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    host_uniq                             Pointer to an host unique.    */
/*                                            Host unique must be         */
/*                                            Null-terminated string.     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_host_uniq_set        Actual PPPoE host uniq set    */
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
UINT  _nxe_pppoe_client_host_uniq_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE host uniq set function.  */
    status = _nx_pppoe_client_host_uniq_set(pppoe_client_ptr, host_uniq);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_host_uniq_set                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets the host unique for PPPoE Client.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    host_uniq                             Pointer to an host unique.    */
/*                                            Host unique must be         */
/*                                            Null-terminated string.     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_utility_string_length_check       Check string length           */ 
/*    _nx_pppoe_client_host_uniq_set_extended                             */
/*                                          Actual PPPoE host uniq set    */
/*                                          function                      */ 
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
UINT  _nx_pppoe_client_host_uniq_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq)
{
UINT status;
UINT host_uniq_length = 0;

    /* Calculate the host_uniq length.  */
    if (_nx_utility_string_length_check((CHAR *)host_uniq, &host_uniq_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_SIZE_ERROR);
    }

    status = _nx_pppoe_client_host_uniq_set_extended(pppoe_client_ptr, host_uniq, host_uniq_length);
    
    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_host_uniq_set_extended            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE host uniq set          */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    host_uniq                             Pointer to an host unique.    */
/*                                            Host unique must be         */
/*                                            Null-terminated string.     */
/*    host_uniq_length                      Length of host_uniq           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_host_uniq_set_extended                             */
/*                                          Actual PPPoE host uniq set    */
/*                                          function                      */ 
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
UINT  _nxe_pppoe_client_host_uniq_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq, UINT host_uniq_length)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE host uniq set function.  */
    status = _nx_pppoe_client_host_uniq_set_extended(pppoe_client_ptr, host_uniq, host_uniq_length);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_host_uniq_set_extended             PORTABLE C      */ 
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function sets the host unique for PPPoE Client.                */
/*                                                                        */
/*    Note: The string of host_uniq must be NULL-terminated and length    */
/*    of host_uniq matches the length specified in the argument list.     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    host_uniq                             Pointer to an host unique.    */
/*                                            Host unique must be         */
/*                                            Null-terminated string.     */
/*    host_uniq_length                      Length of host_uniq           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_utility_string_length_check       Check string length           */ 
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
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            string length verification, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_pppoe_client_host_uniq_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq, UINT host_uniq_length)
{
UINT temp_host_uniq_length = 0;
   
    /* Get the length of host_uniq string.  */
    if (_nx_utility_string_length_check((CHAR*)host_uniq, &temp_host_uniq_length, host_uniq_length))
        return(NX_SIZE_ERROR);

    /* Check the host_uniq length.  */
    if (host_uniq_length != temp_host_uniq_length)
        return(NX_SIZE_ERROR);
    
    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup host unique pointer.  */
    pppoe_client_ptr -> nx_pppoe_host_uniq = host_uniq;
    
    /* Setup host unique length.  */
    pppoe_client_ptr -> nx_pppoe_host_uniq_length = host_uniq_length;

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_session_connect                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function checks for errors in the PPPoE Client session connect */
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
/*    wait_option                           Suspension option             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_session_connect      Actual PPPoE connect function */
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
UINT  _nxe_pppoe_client_session_connect(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE client instance enable function.  */
    status =  _nx_pppoe_client_session_connect(pppoe_client_ptr, wait_option); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_connect                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    This function connects the PPPoE session.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
/*    wait_option                           Suspension option             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_session_connect      Actual PPPoE connect function */
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
UINT  _nx_pppoe_client_session_connect(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG wait_option)
{

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check to PPPoE state.  */
    if (pppoe_client_ptr -> nx_pppoe_state != NX_PPPOE_CLIENT_STATE_INITIAL)
    {

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        return(NX_PPPOE_CLIENT_INVALID_SESSION);
    }

    /* Activate the PPPoE client timer.  */
    tx_timer_activate(&pppoe_client_ptr -> nx_pppoe_timer);

    /* Update the state.  */
    pppoe_client_ptr -> nx_pppoe_state = NX_PPPOE_CLIENT_STATE_PADI_SENT;

    /* Set the retransmit timeout and count for PADI.  */
    pppoe_client_ptr -> nx_pppoe_rtr_timeout = NX_PPPOE_CLIENT_PADI_INIT_TIMEOUT;
    pppoe_client_ptr -> nx_pppoe_rtr_count = NX_PPPOE_CLIENT_PADI_COUNT - 1;

    /* Start to establish the PPPoE session connection.  */
    _nx_pppoe_client_discovery_send(pppoe_client_ptr, NX_PPPOE_CLIENT_CODE_PADI);

    /* Optionally suspend the thread.  If timeout occurs, return a connection timeout status.  If
       immediate response is selected, return a connection in progress status.  Only on a real
       connection should success be returned.  */
    if ((wait_option) && (_tx_thread_current_ptr != &(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_thread)))
    {

        /* Suspend the thread on this socket's connection attempt.  */
        /* Note: the IP protection mutex is released inside _nx_pppoe_client_session_thread_suspend().  */

        _nx_pppoe_client_session_thread_suspend(&(pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread), _nx_pppoe_client_session_connect_cleanup, 
                                                pppoe_client_ptr, &(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), wait_option);

        /* Just return the completion code.  */
        return(_tx_thread_current_ptr -> tx_thread_suspend_status);
    }
    else
    {

        /* No suspension is request, just release protection and return to the caller.  */

        /* Release the IP protection.  */            
        tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        /* Return in-progress completion status.  */
        return(NX_IN_PROGRESS);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_session_packet_send               PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */ 
/*    packet_ptr                            Pointer to packet to send     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_session_packet_send  Actual PPPoE Session data send*/
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
UINT  _nxe_pppoe_client_session_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr)
{

UINT    status;

    /* Check for invalid packet.  */
    if (packet_ptr == NX_NULL)
    {
        return(NX_PPPOE_CLIENT_PTR_ERROR);
    }

    /* Check for minimum packet length (PPP DATA Header).  */
    if (packet_ptr -> nx_packet_length < 2)
    {

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);
    }

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        return(NX_PPPOE_CLIENT_PTR_ERROR);
    }

    /* Call actual PPPoE session packet_send function.  */
    status =  _nx_pppoe_client_session_packet_send(pppoe_client_ptr, packet_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_packet_send                PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
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
/*    _nx_pppoe_client_data_add             Add PPPoE data                */ 
/*    _nx_pppoe_client_packet_send          Send out PPPoE packet         */
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
UINT  _nx_pppoe_client_session_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr)
{               

NX_PPPOE_SERVER_SESSION    *server_session_ptr;
UCHAR                      *work_ptr;


    /* Obtain the IP internal mutex.  */
    tx_mutex_get(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check to see if PPPoE session is established.  */
    if (pppoe_client_ptr -> nx_pppoe_state != NX_PPPOE_CLIENT_STATE_ESTABLISHED)
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        return(NX_PPPOE_CLIENT_SESSION_NOT_ESTABLISHED);
    }

    /* Check for an invalid packet prepend pointer.  */
    if ((packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < NX_PPPOE_CLIENT_OFFSET_PAYLOAD)
    {

        /* Adjust the packet prepend to remove the PPP header.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

        /* Release the packet.  */
        nx_packet_transmit_release(packet_ptr);

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        /* Return error code.  */
        return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);
    }

    /* Set the server session pointer.  */
    server_session_ptr = &(pppoe_client_ptr -> nx_pppoe_server_session);

    /* Set the work pointer.  */
    packet_ptr -> nx_packet_prepend_ptr -= NX_PPPOE_CLIENT_OFFSET_PAYLOAD;
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
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_VER_TYPE, 1, NX_PPPOE_CLIENT_VERSION_TYPE);  

    /* Add code.  */
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_CODE, 1, NX_PPPOE_CLIENT_CODE_ZERO);

    /* Add session id.  */
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_SESSION_ID, 2, server_session_ptr -> nx_pppoe_session_id); 

    /* Add length.  */
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_LENGTH, 2, packet_ptr -> nx_packet_length);

    /* Update the packet length.  */
    packet_ptr -> nx_packet_length += NX_PPPOE_CLIENT_OFFSET_PAYLOAD;

    /* Send PPPoE session packet.  */
    _nx_pppoe_client_packet_send(pppoe_client_ptr, packet_ptr, NX_LINK_PPPOE_SESSION_SEND);

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_session_terminate                 PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         Session index                 */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_session_terminate    Actual PPPoE Session terminate*/
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
UINT  _nxe_pppoe_client_session_terminate(NX_PPPOE_CLIENT *pppoe_client_ptr)
{

UINT    status;
               
    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE session terminate function.  */
    status =  _nx_pppoe_client_session_terminate(pppoe_client_ptr); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_terminate                  PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
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
/*    _nx_pppoe_client_discovery_send       Send out PPPoE packet         */
/*    _nx_pppoe_client_session_cleanup      Cleanup PPPoE session         */ 
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
UINT  _nx_pppoe_client_session_terminate(NX_PPPOE_CLIENT *pppoe_client_ptr)
{  

UINT        status;

    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check to see if PPPoE is established.  */
    if (pppoe_client_ptr -> nx_pppoe_state != NX_PPPOE_CLIENT_STATE_ESTABLISHED)
    {

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        return(NX_PPPOE_CLIENT_SESSION_NOT_ESTABLISHED);
    }

    /* Terminate the PPPoE session.  */
    status = _nx_pppoe_client_discovery_send(pppoe_client_ptr, NX_PPPOE_CLIENT_CODE_PADT);
           
    /* Check the status.  */
    if (status == NX_PPPOE_CLIENT_SUCCESS)
    {

        /* Cleanup the session.  */
        _nx_pppoe_client_session_cleanup(pppoe_client_ptr);
    }
          
    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pppoe_client_session_get                       PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         The index of Client Session   */ 
/*    server_mac_msw                        Server physical address MSW   */
/*    server_mac_lsw                        Server physical address LSW   */
/*    session_id                            Session ID                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_pppoe_client_session_get          Actual PPPoE Session get      */
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
UINT  _nxe_pppoe_client_session_get(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG *client_mac_msw, ULONG *client_mac_lsw, ULONG *session_id)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((pppoe_client_ptr == NX_NULL) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
        return(NX_PPPOE_CLIENT_PTR_ERROR);

    /* Call actual PPPoE session get function.  */
    status =  _nx_pppoe_client_session_get(pppoe_client_ptr, client_mac_msw, client_mac_lsw, session_id); 

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_get                        PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    session_index                         The index of Client Session   */ 
/*    server_mac_msw                        Server physical address MSW   */
/*    server_mac_lsw                        Server physical address LSW   */
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
UINT  _nx_pppoe_client_session_get(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG *server_mac_msw, ULONG *server_mac_lsw, ULONG *session_id)
{  


    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check to see if PPPoE session is established.  */
    if (pppoe_client_ptr -> nx_pppoe_state != NX_PPPOE_CLIENT_STATE_ESTABLISHED)
    {
        
        /* Release the IP internal mutex.  */
        tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

        return(NX_PPPOE_CLIENT_SESSION_NOT_ESTABLISHED);
    }

    /* Yes, get the Server physical address MSW.  */
    if (server_mac_msw)
        *server_mac_msw = pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_msw;

    /* Yes, get the Server physical address LSW.  */
    if (server_mac_lsw)
        *server_mac_lsw = pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_lsw;

    /* Yes, get the Session ID.  */
    if (session_id)
        *session_id = pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_session_id;

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(pppoe_client_ptr -> nx_pppoe_ip_ptr -> nx_ip_protection));

    /* Return status.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_packet_deferred_receive            PORTABLE C      */ 
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
VOID  _nx_pppoe_client_packet_deferred_receive(NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Check to see if PPPoE instance is created.  */
    if (_nx_pppoe_client_created_ptr == NX_NULL)
    {   

        /* Release the packet.  */;
        nx_packet_release(packet_ptr);

        return;
    }

    /* Check to see if PPPoE is ready to receive packet.  */
    if (_nx_pppoe_client_created_ptr -> nx_pppoe_state == NX_PPPOE_CLIENT_STATE_INITIAL)
    {
        
        /* Release the packet.  */;
        nx_packet_release(packet_ptr);

        return;
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Check to see if the deferred processing queue is empty.  */
    if (_nx_pppoe_client_created_ptr -> nx_pppoe_deferred_received_packet_head)
    {

        /* Not empty, just place the packet at the end of the queue.  */
        (_nx_pppoe_client_created_ptr -> nx_pppoe_deferred_received_packet_tail) -> nx_packet_queue_next = packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        _nx_pppoe_client_created_ptr -> nx_pppoe_deferred_received_packet_tail = packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Empty deferred receive processing queue.  Just setup the head pointers and
           set the event flags to ensure the PPPoE helper thread looks at the deferred processing
           queue.  */
        _nx_pppoe_client_created_ptr -> nx_pppoe_deferred_received_packet_head = packet_ptr;
        _nx_pppoe_client_created_ptr -> nx_pppoe_deferred_received_packet_tail = packet_ptr;
        packet_ptr -> nx_packet_queue_next = NX_NULL;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup PPPoE helper thread to process the PPPoE deferred receive.  */
        tx_event_flags_set(&(_nx_pppoe_client_created_ptr -> nx_pppoe_events), NX_PPPOE_CLIENT_PACKET_RECEIVE_EVENT, TX_OR);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_thread_entry                       PORTABLE C      */ 
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
/*    pppoe_client_ptr_value                Pointer to PPPoE control block*/
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
/*    _nx_pppoe_client_packet_receive       PPPoE receive packet          */  
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
static VOID _nx_pppoe_client_thread_entry(ULONG pppoe_client_ptr_value)
{


TX_INTERRUPT_SAVE_AREA

#ifdef NX_PPPOE_CLIENT_INITIALIZE_DRIVER_ENABLE
NX_IP_DRIVER        driver_request;
#endif
NX_PPPOE_CLIENT    *pppoe_client_ptr;
NX_IP              *ip_ptr;
NX_PACKET          *packet_ptr;
ULONG               pppoe_events;
ULONG               timeout = 0;


    /* Setup the PPPoE pointer.  */
    pppoe_client_ptr =  (NX_PPPOE_CLIENT *) pppoe_client_ptr_value;

    /* Setup the IP pointer.  */
    ip_ptr = pppoe_client_ptr -> nx_pppoe_ip_ptr;
       
    /* Obtain the IP internal mutex before calling the driver.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifdef NX_PPPOE_CLIENT_INITIALIZE_DRIVER_ENABLE

    /* Initialize and enable the hardware for physical interface.  */

    /* Is this a valid interface with a link driver associated with it?  */
    if((pppoe_client_ptr -> nx_pppoe_interface_ptr -> nx_interface_valid) && (pppoe_client_ptr -> nx_pppoe_link_driver_entry))
    {

        /* Yes; attach the interface to the device. */
        driver_request.nx_ip_driver_ptr        = ip_ptr;
        driver_request.nx_ip_driver_command    = NX_LINK_INTERFACE_ATTACH;
        driver_request.nx_ip_driver_interface  = pppoe_client_ptr -> nx_pppoe_interface_ptr;
        (pppoe_client_ptr -> nx_pppoe_link_driver_entry) (&driver_request);

        /* Call the link driver to initialize the hardware. Among other
           responsibilities, the driver is required to provide the
           Maximum Transfer Unit (MTU) for the physical layer. The MTU
           should represent the actual physical layer transfer size
           less the physical layer headers and trailers.  */
        driver_request.nx_ip_driver_ptr     = ip_ptr;
        driver_request.nx_ip_driver_command = NX_LINK_INITIALIZE;
        (pppoe_client_ptr -> nx_pppoe_link_driver_entry) (&driver_request);

        /* Call the link driver again to enable the interface.  */
        driver_request.nx_ip_driver_ptr     = ip_ptr;
        driver_request.nx_ip_driver_command = NX_LINK_ENABLE;
        (pppoe_client_ptr -> nx_pppoe_link_driver_entry) (&driver_request);
        
        /* Indicate to the IP software that IP to physical mapping
           is not required.  */
        pppoe_client_ptr -> nx_pppoe_interface_ptr -> nx_interface_address_mapping_needed = NX_FALSE; 
    }
#endif

    /* Loop to continue processing incoming bytes.  */
    while(NX_FOREVER)    
    {

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Pickup IP event flags.  */
        tx_event_flags_get(&(pppoe_client_ptr -> nx_pppoe_events), NX_PPPOE_CLIENT_ALL_EVENTS, TX_OR_CLEAR, &pppoe_events, NX_WAIT_FOREVER);

        /* Obtain the IP internal mutex before processing the IP event.  */
        tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

        /* Check for PPPoE packet receive event.  */
        if (pppoe_events & NX_PPPOE_CLIENT_PACKET_RECEIVE_EVENT)
        {

            /* Loop to process all deferred packet requests.  */
            while (pppoe_client_ptr -> nx_pppoe_deferred_received_packet_head)
            {

                /* Remove the first packet and process it!  */

                /* Disable interrupts.  */
                TX_DISABLE

                /* Pickup the first packet.  */
                packet_ptr =  pppoe_client_ptr -> nx_pppoe_deferred_received_packet_head;

                /* Move the head pointer to the next packet.  */
                pppoe_client_ptr -> nx_pppoe_deferred_received_packet_head =  packet_ptr -> nx_packet_queue_next;

                /* Check for end of deferred processing queue.  */
                if (pppoe_client_ptr -> nx_pppoe_deferred_received_packet_head == NX_NULL)
                {

                    /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                    pppoe_client_ptr -> nx_pppoe_deferred_received_packet_tail =  NX_NULL;
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
                if (packet_ptr -> nx_packet_length < NX_PPPOE_CLIENT_OFFSET_PAYLOAD)
                {

                    /* Release the packet.  */
                    nx_packet_release(packet_ptr);
                    return;
                }

                /* Check the packet interface.  */
                if ((packet_ptr -> nx_packet_ip_interface != NX_NULL) &&
                    (packet_ptr -> nx_packet_ip_interface != pppoe_client_ptr -> nx_pppoe_interface_ptr))
                {
                    nx_packet_release(packet_ptr);
                    continue;
                }

                /* Call the actual PPPoE Client packet receive function.  */
                _nx_pppoe_client_packet_receive(pppoe_client_ptr, packet_ptr);
            }
        }

        /* Check for PPPoE periodic timer event.  */
        if (pppoe_events & NX_PPPOE_CLIENT_TIMER_PERIODIC_EVENT)
        {

            /* Check if max number of PADI/PADR messages have been sent. */
            if (pppoe_client_ptr -> nx_pppoe_rtr_timeout != 0)
            {

                /* Check on count down timer for sending out next PADI/PADR message. */
                pppoe_client_ptr -> nx_pppoe_rtr_timeout--;

                /* Check the timer.  */
                if(pppoe_client_ptr -> nx_pppoe_rtr_timeout == 0)
                {

                    /* It has expired. Send out a PADI/PADR message. */

                    /* Check if max number of PADI/PADR messages have been sent. */
                    if (pppoe_client_ptr -> nx_pppoe_rtr_count)
                    {

                        /* Retransmit PADI/PADR message. */

                        /* When a host does not receive a PADO/PADS packet with in a specified amount of time,
                           it should resend it's PADI/PADR packet and double the waiting period. RFC2516, Section8, Page8.  */

                        /* Check PPPoE state.  */
                        if (pppoe_client_ptr -> nx_pppoe_state == NX_PPPOE_CLIENT_STATE_PADI_SENT)
                        {

                            /* Calculate timeout.  */
                            timeout = ((ULONG)NX_PPPOE_CLIENT_PADI_INIT_TIMEOUT) << (ULONG)(NX_PPPOE_CLIENT_PADI_COUNT - pppoe_client_ptr -> nx_pppoe_rtr_count);

                            /* Send PADI.  */
                            _nx_pppoe_client_discovery_send(pppoe_client_ptr, NX_PPPOE_CLIENT_CODE_PADI);
                        }

                        /* Check PPPoE state.  */
                        if (pppoe_client_ptr -> nx_pppoe_state == NX_PPPOE_CLIENT_STATE_PADR_SENT)
                        {

                            /* Calculate timeout.  */
                            timeout = ((ULONG)NX_PPPOE_CLIENT_PADR_INIT_TIMEOUT) << (ULONG)(NX_PPPOE_CLIENT_PADR_COUNT - pppoe_client_ptr -> nx_pppoe_rtr_count);

                            /* Send PADR.  */
                            _nx_pppoe_client_discovery_send(pppoe_client_ptr, NX_PPPOE_CLIENT_CODE_PADR);
                        }

                        /* Update the retransmit count.  */
                        pppoe_client_ptr -> nx_pppoe_rtr_count --;

                        /* Set the retransmit timeout.  */
                        pppoe_client_ptr -> nx_pppoe_rtr_timeout = timeout;
                    }
                    else
                    {

                        /* Reach the max number of PADI/PADR have been sent.  */

                        /* Deactivate the PPPoE client timer.  */
                        tx_timer_deactivate(&(pppoe_client_ptr -> nx_pppoe_timer));

                        /* Cleanup the PPPoE session.  */
                        _nx_pppoe_client_session_cleanup(pppoe_client_ptr);

                        /* Determine if we need to wake a thread suspended on the connection.  */
                        if (pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread)
                        {

                            /* Resume the suspended thread.  */
                            _nx_pppoe_client_session_thread_resume(&(pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread), NX_PPPOE_CLIENT_SESSION_NOT_ESTABLISHED);
                        }
                    }
                }
            }
        }

        /* Check for PPPoE event.  */
        if (pppoe_events & NX_PPPOE_CLIENT_SESSION_CONNECT_CLEANUP_EVENT)
        {

            if (pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread)
            {
                _nx_pppoe_client_session_connect_cleanup(pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread NX_CLEANUP_ARGUMENT);
            }
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_timer_entry                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles waking up the PPPoE Client helper thread on a */
/*    periodic basis.                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pppoe_client_address                  PPPoE address in a ULONG      */ 
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
static VOID  _nx_pppoe_client_timer_entry(ULONG pppoe_client_address)
{

NX_PPPOE_CLIENT *pppoe_client_ptr;


    /* Convert input parameter to an PPPoE Client pointer.  */
    pppoe_client_ptr =  (NX_PPPOE_CLIENT *)pppoe_client_address;

    /* Wakeup this PPPoE Client's helper thread.  */
    tx_event_flags_set(&(pppoe_client_ptr -> nx_pppoe_events), NX_PPPOE_CLIENT_TIMER_PERIODIC_EVENT, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_packet_receive                     PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
/*    packet_ptr                            Pointer to packet to receive  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_packet_release                     Release packet to packet pool */
/*    _nx_pppoe_client_data_get             Get the PPPoE data            */
/*    _nx_pppoe_client_discovery_packet_process                           */  
/*                                          Process discovery packet      */
/*    _nx_pppoe_client_session_packet_process                             */  
/*                                          Process session packet        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_client_thread_entry                                       */
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
VOID  _nx_pppoe_client_packet_receive(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr)
{
                                     
UCHAR                      *ethernet_header_ptr; 
ULONG                       dest_mac_msw;
ULONG                       dest_mac_lsw;
ULONG                       src_mac_msw;
ULONG                       src_mac_lsw;
UINT                        ethernet_type;


    /* Set up UCHAR pointer to ethernet header to extract client hardware address
       which we will use as the client's unique identifier.  */
    ethernet_header_ptr = packet_ptr -> nx_packet_prepend_ptr - NX_PPPOE_CLIENT_ETHER_HEADER_SIZE;

    /* Pickup the MSW and LSW of the destination MAC address.  */
    dest_mac_msw = (((ULONG) ethernet_header_ptr[0]) << 8)  | ((ULONG) ethernet_header_ptr[1]);
    dest_mac_lsw = (((ULONG) ethernet_header_ptr[2]) << 24) | (((ULONG) ethernet_header_ptr[3]) << 16) |
                    (((ULONG) ethernet_header_ptr[4]) << 8)  | ((ULONG) ethernet_header_ptr[5]);

    /* Check the destination hardware (mac address) field is filled in. */
    if ((dest_mac_msw == 0) && (dest_mac_lsw == 0))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Check if it is a broadcast message. 
       Only PADI packet with the destination address set to the broadcast address, and PADI only can be sent from PPPoE Client.   */
    if ((dest_mac_msw == 0xFFFF) && (dest_mac_lsw == 0xFFFFFFFF))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the MSW and LSW of the source MAC address.  */
    src_mac_msw = (((ULONG) ethernet_header_ptr[6]) << 8)  | ((ULONG) ethernet_header_ptr[7]);
    src_mac_lsw = (((ULONG) ethernet_header_ptr[8]) << 24) | (((ULONG) ethernet_header_ptr[9]) << 16) |
                   (((ULONG) ethernet_header_ptr[10]) << 8)  | ((ULONG) ethernet_header_ptr[11]);

    /* Check the source hardware (mac address) field is filled in. */
    if ((src_mac_msw == 0) && (src_mac_lsw == 0))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Get the ethernet type.  */
    ethernet_type = _nx_pppoe_client_data_get(ethernet_header_ptr + 12, 2);

    /* Process the packet according to packet type. */
    if(ethernet_type == NX_PPPOE_CLIENT_ETHER_TYPE_DISCOVERY)
    {

        /* Process the discovery packet.  */
        _nx_pppoe_client_discovery_packet_process(pppoe_client_ptr, packet_ptr, src_mac_msw, src_mac_lsw);
    }
    else if(ethernet_type == NX_PPPOE_CLIENT_ETHER_TYPE_SESSION)
    {

        /* Process the session packet.  */
        _nx_pppoe_client_session_packet_process(pppoe_client_ptr, packet_ptr, src_mac_msw, src_mac_lsw);
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
/*    _nx_pppoe_client_discovery_packet_process           PORTABLE C      */ 
/*                                                           6.1.3        */
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
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
/*    _nx_pppoe_client_data_get             Get the PPPoE data            */ 
/*    _nx_pppoe_client_tag_process          Process PPPoE tags            */ 
/*    _nx_pppoe_client_discovery_send       Send discovery packet         */
/*    _nx_pppoe_client_session_find         Find the PPPoE session        */
/*    _nx_pppoe_client_session_cleanup      Cleanup the PPPoE session     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_client_packet_receive       Receive the PPPoE packet      */ 
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
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            string length verification, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_pppoe_client_discovery_packet_process(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr, ULONG server_mac_msw, ULONG server_mac_lsw)
{


UCHAR                      *pppoe_header_ptr;
UINT                        is_accepted = NX_FALSE;
ULONG                       ver_type;
ULONG                       code;
ULONG                       session_id;
ULONG                       length;
UCHAR                      *tag_ptr;
ULONG                       tag_type;
ULONG                       tag_length;
UINT                        tag_index = 0;
UINT                        tag_ac_name_count = 0;
UINT                        tag_service_name_count = 0;
UINT                        tag_service_name_valid = NX_FALSE;
UINT                        tag_host_uniq_count = 0;
UINT                        tag_host_uniq_valid = NX_FALSE;


    /* Setup the PPPoE header.  */
    pppoe_header_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the version and type.  */   
    ver_type = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_VER_TYPE, 1);
                 
    /* Check the version and type.  */
    if (ver_type != NX_PPPOE_CLIENT_VERSION_TYPE)
    {
         
        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the code.  */
    code = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_CODE, 1);

    /* Check the session state and the code of incoming packet.  */
    if (((pppoe_client_ptr -> nx_pppoe_state == NX_PPPOE_CLIENT_STATE_PADI_SENT) && (code == NX_PPPOE_CLIENT_CODE_PADO)) ||
        ((pppoe_client_ptr -> nx_pppoe_state == NX_PPPOE_CLIENT_STATE_PADR_SENT) && (code == NX_PPPOE_CLIENT_CODE_PADS)) ||
        ((pppoe_client_ptr -> nx_pppoe_state == NX_PPPOE_CLIENT_STATE_ESTABLISHED) && (code == NX_PPPOE_CLIENT_CODE_PADT)))
    {

        /* Packet can be accepted.  */
        is_accepted = NX_TRUE;
    }

    /* Ignore the packet that is not accepted.   */
    if (is_accepted != NX_TRUE)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the session id.  */   
    session_id = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_SESSION_ID, 2);

    /* Check the session id. 
       Session ID must be zero for PADO,
       Session ID must be not zero for PADS and PADT.
       Session ID must be same value for PADT.  */
    if (((code == NX_PPPOE_CLIENT_CODE_PADO) && (session_id != 0)) ||
        ((code != NX_PPPOE_CLIENT_CODE_PADO) && (session_id == 0)) ||
        ((code == NX_PPPOE_CLIENT_CODE_PADT) && (session_id != pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_session_id)))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Check the MAC address for PADS and PADT before process tags.  */
    if (code != NX_PPPOE_CLIENT_CODE_PADO)
    {

        if ((pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_msw != server_mac_msw) ||
            (pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_lsw != server_mac_lsw))
        {

            /* Release the packet.  */   
            nx_packet_release(packet_ptr);
            return;
        }
    }

    /* Initialize the value.  */
    pppoe_client_ptr -> nx_pppoe_ac_name_size = 0;
    pppoe_client_ptr -> nx_pppoe_ac_cookie_size = 0;
    pppoe_client_ptr -> nx_pppoe_relay_session_id_size = 0;
    pppoe_client_ptr -> nx_pppoe_error_flag = 0;

    /* Pickup the length of tags.  */
    length = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_LENGTH, 2);      

    /* Check for valid payload.  */
    if (length + NX_PPPOE_CLIENT_OFFSET_PAYLOAD > packet_ptr -> nx_packet_length)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Set the tag pointer.  */
    tag_ptr = pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_PAYLOAD;

    /* Loop to process the tag.  */
    while (tag_index + 4 <= length)
    {

        /* Pickup the tag type.  */   
        tag_type = _nx_pppoe_client_data_get(tag_ptr + tag_index, 2); 

        /* Update the index.  */
        tag_index += 2;

        /* Pickup the tag length.  */
        tag_length = _nx_pppoe_client_data_get(tag_ptr + tag_index, 2);

        /* Update the index.  */
        tag_index += 2;

        /* Check for valid tag length.  */
        if ((tag_index + tag_length) > length)
        {

            /* Release the packet.  */   
            nx_packet_release(packet_ptr);
            return;
        }

        /* Process the option type. */
        switch (tag_type)
        {
                                     
            case NX_PPPOE_CLIENT_TAG_TYPE_END_OF_LIST:
            {

                /* End tag.  */
                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME:
            {

                /* Update the Service-Name count.  */
                tag_service_name_count ++;

                /* Check the tag length.  */
                if (tag_length == 0)
                {

                    /* When the tag length is zero this tag is used to indicate that any service is acceptable.  */
                    tag_service_name_valid = NX_TRUE;
                }
                else
                {

                    /* Compare the service name with PPPoE Service name that Client is requesting.  */
                    if ((pppoe_client_ptr -> nx_pppoe_service_name_length == tag_length) &&
                        (!memcmp(tag_ptr + tag_index, pppoe_client_ptr -> nx_pppoe_service_name, tag_length)))
                    {

                        /* Update the information.  */
                        tag_service_name_valid = NX_TRUE;
                    }
                }

                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_AC_NAME:
            {

                /* Check the cache for AC-Name.  */
                if (tag_length > NX_PPPOE_CLIENT_MAX_AC_NAME_SIZE) 
                {

                    /* Release the packet.  */   
                    nx_packet_release(packet_ptr);
                    return;
                }

                /* Update the AC-Name count.  */
                tag_ac_name_count ++;

                /* Save the AC-Name.  */
                memcpy(pppoe_client_ptr -> nx_pppoe_ac_name, tag_ptr + tag_index, tag_length); /* Use case of memcpy is verified. */
                
                /* Set the AC-Name size.  */
                pppoe_client_ptr -> nx_pppoe_ac_name_size = tag_length;

                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_HOST_UNIQ:
            {

                /* Update the Host-Uniq count.  */
                tag_host_uniq_count ++;

                /* Check the Host-Uniq.  */
                if ((pppoe_client_ptr -> nx_pppoe_host_uniq_length == tag_length) &&
                    (!memcmp(tag_ptr + tag_index, pppoe_client_ptr -> nx_pppoe_host_uniq, tag_length)))
                {

                    /* Update the information.  */
                    tag_host_uniq_valid = NX_TRUE;
                }

                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_AC_COOKIE:
            {

                /* Check the cache for AC-Cookie.  */
                if (tag_length > NX_PPPOE_CLIENT_MAX_AC_COOKIE_SIZE)
                {

                    /* Release the packet.  */   
                    nx_packet_release(packet_ptr);
                    return;
                }

                /* Save the AC-Cookie.  */
                memcpy(pppoe_client_ptr -> nx_pppoe_ac_cookie, tag_ptr + tag_index, tag_length); /* Use case of memcpy is verified. */
                
                /* Set the AC-Cookie size.  */
                pppoe_client_ptr -> nx_pppoe_ac_cookie_size = tag_length;
                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_RELAY_SESSION_ID:
            {

                /* Check the cache for Relay-Session_Id.  */
                if (tag_length > NX_PPPOE_CLIENT_MAX_RELAY_SESSION_ID_SIZE)
                {

                    /* Release the packet.  */   
                    nx_packet_release(packet_ptr);
                    return;
                }

                /* Save the Relay-Session_Id.  */
                memcpy(pppoe_client_ptr -> nx_pppoe_relay_session_id, tag_ptr + tag_index, tag_length); /* Use case of memcpy is verified. */
                
                /* Set the Relay-Session_Id size.  */
                pppoe_client_ptr -> nx_pppoe_relay_session_id_size = tag_length;
                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME_ERROR:
            {

                /* Set the service name error flag.  */
                pppoe_client_ptr -> nx_pppoe_error_flag |= NX_PPPOE_CLIENT_ERROR_SERVICE_NAME;

                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_AC_SYSTEM_ERROR:
            {

                /* Set the AC system error flag.  */
                pppoe_client_ptr -> nx_pppoe_error_flag |= NX_PPPOE_CLIENT_ERROR_AC_SYSTEM;

                break;
            }
            case NX_PPPOE_CLIENT_TAG_TYPE_GENERIC_ERROR:
            {

                /* Set the generic error flag.  */
                pppoe_client_ptr -> nx_pppoe_error_flag |= NX_PPPOE_CLIENT_ERROR_GENERIC;

                break;
            }
            default:
                break;
        }

        /* Move to the next tag. */
        tag_index += tag_length; 
    }

    /* Now we can release the packet. */
    nx_packet_release(packet_ptr);

    /* Check the code value.  */
    if (code == NX_PPPOE_CLIENT_CODE_PADO)
    {

        /* PADO packet MUST contain one AC-Name TAG containing the Access Concentrator's name,
           a Service-Name TAG indentical to the one in the PADI.  RFC2516, Section5.2, Page6.  */

        /* Check AC-Name.  */
        if (tag_ac_name_count != 1)
            return;

        /* The Host MAY include a Host-Uniq TAG in a PADI or
           PADR. If the Access Concentrator receives this TAG, it MUST
           include the TAG unmodified in the associated PADO or PADS
           response. RFC2516, Appendix A, Page10.  */
        if (pppoe_client_ptr -> nx_pppoe_host_uniq)
        {

            /* Check if include valid Host-Uniq.  */
            if ((tag_host_uniq_count != 1) || (tag_host_uniq_valid != NX_TRUE))
                return;
        }
        else
        {

            /* Check if include the Host-Uniq.  */
            if (tag_host_uniq_count)
                return;
        }

        /* Check Service-Name.  */
        if (tag_service_name_valid != NX_TRUE)
            return;

        /* Check for error.  */
        if (pppoe_client_ptr -> nx_pppoe_error_flag)
            return;

        /* Choose one PPPoE Server.
           Since the PADI was broadcast, the Host may receive more than one
           PADO.  The Host looks through the PADO packets it receives and
           chooses one.  The choice can be based on the AC-Name or the Services
           offered.  The Host then sends one PADR packet to the Access
           Concentrator that it has chosen. RFC2516, Section5.3, Page6.  */

        /* Simplify the logic, we choose the first PPPoE Server sent out PADO.  */

        /* Record server information.  */
        pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_msw = server_mac_msw;
        pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_lsw = server_mac_lsw;

        /* Send PPPoE Active Discovery Offer packet.  */
        _nx_pppoe_client_discovery_send(pppoe_client_ptr, NX_PPPOE_CLIENT_CODE_PADR);

        /* Update the state.  */
        pppoe_client_ptr -> nx_pppoe_state = NX_PPPOE_CLIENT_STATE_PADR_SENT;

        /* Set the retransmit timeout and count for PADR.  */
        pppoe_client_ptr -> nx_pppoe_rtr_timeout = NX_PPPOE_CLIENT_PADR_INIT_TIMEOUT;
        pppoe_client_ptr -> nx_pppoe_rtr_count = NX_PPPOE_CLIENT_PADR_COUNT - 1;
    }
    else if (code == NX_PPPOE_CLIENT_CODE_PADS)
    {

        /* The PADS packet contains exactly one TAG of TAG_TYPE Service-Name. RFC2516, Section5.4, Page6.  */

        /* Check Service-Name.  */
        if ((tag_service_name_count != 1) || (tag_service_name_valid != NX_TRUE))
            return;

        /* Check for error.  */
        if (pppoe_client_ptr -> nx_pppoe_error_flag)
            return;

        /* Record the session id.  */
        pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_session_id = (USHORT)session_id;

        /* Update the state.  */
        pppoe_client_ptr -> nx_pppoe_state = NX_PPPOE_CLIENT_STATE_ESTABLISHED;

        /* Deactivate the PPPoE client timer.  */
        tx_timer_deactivate(&(pppoe_client_ptr -> nx_pppoe_timer));

        /* Determine if we need to wake a thread suspended on the connection.  */
        if (pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread)
        {

            /* Resume the suspended thread.  */
            _nx_pppoe_client_session_thread_resume(&(pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread), NX_PPPOE_CLIENT_SUCCESS);
        }
    }
    else if (code == NX_PPPOE_CLIENT_CODE_PADT)
    {

        /* Cleanup the PPPoE session.  */
        _nx_pppoe_client_session_cleanup(pppoe_client_ptr);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_packet_process             PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/
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
/*    _nx_pppoe_client_data_get             Get the PPPoE data            */
/*    _nx_pppoe_client_session_find         Find the PPPoE session        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_client_packet_receive       Receive the PPPoE packet      */
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
static VOID  _nx_pppoe_client_session_packet_process(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr, ULONG server_mac_msw, ULONG server_mac_lsw)
{


UCHAR                      *pppoe_header_ptr;
ULONG                       ver_type;
ULONG                       code;
ULONG                       session_id;
ULONG                       length;
NX_PPPOE_SERVER_SESSION    *server_session_ptr;


    /* Setup the PPPoE header.  */                        
    pppoe_header_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the version and type.  */   
    ver_type = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_VER_TYPE, 1);

    /* Check the version and type.  */
    if (ver_type != NX_PPPOE_CLIENT_VERSION_TYPE)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the code.  */
    code = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_CODE, 1);

    /* Check the code.  */
    if (code != NX_PPPOE_CLIENT_CODE_ZERO)
    {
         
        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the session id.  */   
    session_id = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_SESSION_ID, 2);

    /* Check the session id.  */
    if (session_id == 0)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Check the session state.  */
    if (pppoe_client_ptr -> nx_pppoe_state != NX_PPPOE_CLIENT_STATE_ESTABLISHED)
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Set the server session pointer.  */
    server_session_ptr = &(pppoe_client_ptr -> nx_pppoe_server_session);

    /* Check the session id and server MAC address.  */
    if ((session_id != server_session_ptr -> nx_pppoe_session_id) ||
        (server_mac_msw != server_session_ptr -> nx_pppoe_physical_address_msw) ||
        (server_mac_lsw != server_session_ptr -> nx_pppoe_physical_address_lsw))
    {

        /* Release the packet.  */   
        nx_packet_release(packet_ptr);
        return;
    }

    /* Setup the prepend pointer to point the payload of PPPoE.  */
    packet_ptr -> nx_packet_prepend_ptr += NX_PPPOE_CLIENT_OFFSET_PAYLOAD;
    packet_ptr -> nx_packet_length -= NX_PPPOE_CLIENT_OFFSET_PAYLOAD;

    /* Pickup the length of payload.  */
    length = _nx_pppoe_client_data_get(pppoe_header_ptr + NX_PPPOE_CLIENT_OFFSET_LENGTH, 2);

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

    /* Check the PPPoE receive function.  */
    if (pppoe_client_ptr -> nx_pppoe_packet_receive)
    {

        /* Call the function to receive the data frame.  
           Notice: the receive function must release this packet.  */
        pppoe_client_ptr -> nx_pppoe_packet_receive(packet_ptr);
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
/*    _nx_pppoe_client_discovery_send                     PORTABLE C      */ 
/*                                                           6.1.3        */
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
/*    client_session_ptr                    Pointer to Client Session     */
/*    code                                  PPPoE code                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for the     */ 
/*                                            PPPoE Discovery             */
/*    nx_packet_release                     Release packet to packet pool */ 
/*    _nx_pppoe_client_data_add             Add PPPoE data                */
/*    _nx_pppoe_client_tag_string_add       Add PPPoE tag                 */ 
/*    _nx_pppoe_client_packet_send          Send out PPPoE packet         */
/*    _nx_pppoe_client_session_find         Find the PPPoE session        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_terminate    Terminate the PPPoE session   */ 
/*    _nx_pppoe_client_discovery_packet_process                           */ 
/*                                          Process PPPoE Discovery packet*/ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            string length verification, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
static UINT    _nx_pppoe_client_discovery_send(NX_PPPOE_CLIENT *pppoe_client_ptr, UINT code)
{


NX_PACKET       *packet_ptr;
UCHAR           *work_ptr;      
UINT            status;  
UINT            index = 0;


    /* Allocate a PPPoE packet.  */
    status =  nx_packet_allocate(pppoe_client_ptr -> nx_pppoe_packet_pool_ptr, &packet_ptr, NX_PHYSICAL_HEADER, NX_NO_WAIT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Return status.  */
        return(status);
    }

    /* Set the work pointer.  */
    work_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* First skip the PPPoE header.  */
    index += NX_PPPOE_CLIENT_OFFSET_PAYLOAD;

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
    if (code == NX_PPPOE_CLIENT_CODE_PADI)
    {

        /* The PADI packet MUST contain exactly one TAG of TAG_TYPE Service-Name, indicating the service the host is requesting,
           and any number of other TAG types. RFC2516, Section5.1, Page5.  */
        if (pppoe_client_ptr -> nx_pppoe_service_name)
        {

            if (((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(&work_ptr[index])) >=
                (4 + pppoe_client_ptr -> nx_pppoe_service_name_length))
            {

                /* Added the Service-Name tag.  */
                _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME, pppoe_client_ptr -> nx_pppoe_service_name_length, pppoe_client_ptr -> nx_pppoe_service_name, &index);
            }
            else
            {

                /* Packet too small. */
                nx_packet_release(packet_ptr);
                return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);
            }
        }
        else
        {

            /* Added the Service-Name tag.  */
            _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME, 0, NX_NULL, &index); 
        }

        /* The Host MAY include a Host-Uniq TAG in a PADI or PADR.  */
        if (pppoe_client_ptr -> nx_pppoe_host_uniq)
        {

            if (((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(&work_ptr[index])) >=
                (4 + pppoe_client_ptr -> nx_pppoe_host_uniq_length))
            {

                /* Added the Host-Uniq tag.  */
                _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_HOST_UNIQ, pppoe_client_ptr -> nx_pppoe_host_uniq_length, pppoe_client_ptr -> nx_pppoe_host_uniq, &index);
            }
            else
            {

                /* Packet too small. */
                nx_packet_release(packet_ptr);
                return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);
            }
        }
    }
    else if (code == NX_PPPOE_CLIENT_CODE_PADR)
    {

        /* The PADR packet MUST contain exactly one TAG of TAG_TYPE Service-Name, indicating the service the host is requesting,
           and any number of other TAG types. RFC2516, Section5.3, Page6.  */
        if (pppoe_client_ptr -> nx_pppoe_service_name)
        {

            if (((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(&work_ptr[index])) >=
                (4 + pppoe_client_ptr -> nx_pppoe_service_name_length))
            {
                
                /* Added the Service-Name tag.  */
                _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME, pppoe_client_ptr -> nx_pppoe_service_name_length, pppoe_client_ptr -> nx_pppoe_service_name, &index);
            }
            else
            {

                /* Packet too small. */
                nx_packet_release(packet_ptr);
                return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);
            }
        }
        else
        {

            /* Added the Service-Name tag.  */
            _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME, 0, NX_NULL, &index); 
        }

        /* The Host MAY include a Host-Uniq TAG in a PADI or PADR.  */
        if (pppoe_client_ptr -> nx_pppoe_host_uniq)
        {

            if (((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(&work_ptr[index])) >=
                (4 + pppoe_client_ptr -> nx_pppoe_host_uniq_length))
            {

                /* Added the Host-Uniq tag.  */
                _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_HOST_UNIQ, pppoe_client_ptr -> nx_pppoe_host_uniq_length, pppoe_client_ptr -> nx_pppoe_host_uniq, &index);
            }
            else
            {

                /* Packet too small. */
                nx_packet_release(packet_ptr);
                return(NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR);
            }
        }

        /* If a Host receives AC-Cookie tag, it MUST return the TAG unmodified in the following PADR.  */
        if (pppoe_client_ptr -> nx_pppoe_ac_cookie_size)
        {

            /* Added the AC-Cookie tag.  */
            _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_AC_COOKIE, pppoe_client_ptr -> nx_pppoe_ac_cookie_size, pppoe_client_ptr -> nx_pppoe_ac_cookie, &index);
        }

        /* If either the Host or Access Concentrator receives Relay-Session-Id TAG they MUST include it unmodified in any discovery packet they send as a response.  */
        if (pppoe_client_ptr -> nx_pppoe_relay_session_id_size)
        {

            /* Added the Host-Uniq tag.  */
            _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_HOST_UNIQ, pppoe_client_ptr -> nx_pppoe_relay_session_id_size, pppoe_client_ptr -> nx_pppoe_relay_session_id, &index);
        }
    }
    else if (code == NX_PPPOE_CLIENT_CODE_PADT)
    {

        /* If either the Host or Access Concentrator receives Relay-Session-Id TAG they MUST include it unmodified in any discovery packet they send as a response.  */
        if (pppoe_client_ptr -> nx_pppoe_relay_session_id_size)
        {

            /* Added the Host-Uniq tag.  */
            _nx_pppoe_client_tag_string_add(work_ptr, NX_PPPOE_CLIENT_TAG_TYPE_HOST_UNIQ, pppoe_client_ptr -> nx_pppoe_relay_session_id_size, pppoe_client_ptr -> nx_pppoe_relay_session_id, &index);
        }
    }

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
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_VER_TYPE, 1, NX_PPPOE_CLIENT_VERSION_TYPE);  

    /* Add code.  */
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_CODE, 1, code);

    /* Add the Session id.  */
    if (code == NX_PPPOE_CLIENT_CODE_PADT)
    {

        /* Add session id.  */
        _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_SESSION_ID, 2, pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_session_id);
    }
    else
    {

        /* Add session id.  */
        _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_SESSION_ID, 2, 0);
    }

    /* Add length.  */
    _nx_pppoe_client_data_add(work_ptr + NX_PPPOE_CLIENT_OFFSET_LENGTH, 2, (index - NX_PPPOE_CLIENT_OFFSET_PAYLOAD));   

    /* Update the append pointer and length.  */
    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + index;
    packet_ptr -> nx_packet_length = index;

    /* Send PPPoE session packet.  */
    _nx_pppoe_client_packet_send(pppoe_client_ptr, packet_ptr, NX_LINK_PPPOE_DISCOVERY_SEND);

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_packet_send                        PORTABLE C      */ 
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
/*    pppoe_client_ptr                      Pointer to PPPoE control block*/ 
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
/*    _nx_pppoe_client_discovery_send       Send PPPoE Discovery packet   */ 
/*    _nx_pppoe_client_session_send         Send PPPoE Session packet     */ 
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
static VOID    _nx_pppoe_client_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr, UINT command)
{

NX_IP_DRIVER                driver_request;


    /* Initialize the driver request. */
    driver_request.nx_ip_driver_command =               command;
    driver_request.nx_ip_driver_ptr =                   pppoe_client_ptr -> nx_pppoe_ip_ptr;
    driver_request.nx_ip_driver_packet =                packet_ptr;
    driver_request.nx_ip_driver_interface =             pppoe_client_ptr -> nx_pppoe_interface_ptr;

    /* Check if have the server address.  */
    if ((pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_msw) ||
        (pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_lsw))
    {

        /* Set the destination address as server address.  */
        driver_request.nx_ip_driver_physical_address_msw = pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_msw;
        driver_request.nx_ip_driver_physical_address_lsw = pppoe_client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_lsw;
    }
    else
    {

        /* Set the destination address as boradcast address.  */
        driver_request.nx_ip_driver_physical_address_msw = 0x0000FFFF;
        driver_request.nx_ip_driver_physical_address_lsw = 0xFFFFFFFF;
    }

    /* Sendout the PPPoE packet.  */
    (pppoe_client_ptr -> nx_pppoe_link_driver_entry) (&driver_request);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_data_get                           PORTABLE C      */ 
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
/*    _nx_pppoe_client_packet_receive       Receive the PPPoE packet      */ 
/*    _nx_pppoe_client_discovery_packet_process                           */ 
/*                                          Process PPPoE Discovery packet*/ 
/*    _nx_pppoe_client_session_packet_process                             */ 
/*                                          Process PPPoE Session packet  */  
/*    _nx_pppoe_client_tag_process          Process PPPoE TAGs            */  
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
static ULONG  _nx_pppoe_client_data_get(UCHAR *data, UINT size)
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
/*    _nx_pppoe_client_data_add                           PORTABLE C      */ 
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
/*    _nx_pppoe_client_discovery_send       Send PPPoE Discovery packet   */ 
/*    _nx_pppoe_client_session_send         Send PPPoE Session packet     */  
/*    _nx_pppoe_client_tag_string_add       Add PPPoE string TAG          */  
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
static VOID  _nx_pppoe_client_data_add(UCHAR *data, UINT size, ULONG value)
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
/*    _nx_pppoe_client_string_add                         PORTABLE C      */ 
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
/*    _nx_pppoe_client_tag_string_add       Add PPPoE string TAG          */  
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
static VOID  _nx_pppoe_client_string_add(UCHAR *dest, UCHAR *source, UINT size)
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
/*    _nx_pppoe_client_tag_string_add                     PORTABLE C      */ 
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
/*    _nx_pppoe_client_data_add             Add PPPoE data                */  
/*    _nx_pppoe_client_string_add           Add PPPoE string data         */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_pppoe_client_discovery_send       Send PPPoE Discovery packet   */
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
static UINT  _nx_pppoe_client_tag_string_add(UCHAR *data_ptr, UINT tag_type, UINT tag_length, UCHAR *tag_value_string, UINT *index)
{

    /* Add the tag type.  */
    _nx_pppoe_client_data_add(data_ptr + (*index), 2, tag_type);
    (*index) += 2;

    /* Add the tag length.  */
    _nx_pppoe_client_data_add(data_ptr + (*index), 2, tag_length);
    (*index) += 2;

    /* Add the tag value string.  */
    _nx_pppoe_client_string_add(data_ptr + (*index), tag_value_string, tag_length);
    (*index) += tag_length;

    /* Return a successful completion.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pppoe_client_session_cleanup                    PORTABLE C      */ 
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
/*    _nx_pppoe_client_session_terminate    Terminate the PPPoE session   */ 
/*    _nx_pppoe_client_session_packet_process                             */ 
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
static UINT  _nx_pppoe_client_session_cleanup(NX_PPPOE_CLIENT *client_ptr)
{

    /* Cleanup the state.  */
    client_ptr -> nx_pppoe_state = NX_PPPOE_CLIENT_STATE_INITIAL;

    /* Ceanup the tags.  */
    client_ptr -> nx_pppoe_ac_name_size = 0;
    client_ptr -> nx_pppoe_ac_cookie_size = 0;
    client_ptr -> nx_pppoe_relay_session_id_size = 0;

    /* Cleanup the server information.  */
    client_ptr -> nx_pppoe_server_session.nx_pppoe_session_id = NX_NULL;
    client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_msw = NX_NULL;
    client_ptr -> nx_pppoe_server_session.nx_pppoe_physical_address_lsw = NX_NULL;

    /* Return success.  */
    return(NX_PPPOE_CLIENT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pppoe_client_session_thread_suspend             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function suspends a thread on a PPPoE Session connect service  */
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
static VOID  _nx_pppoe_client_session_thread_suspend(TX_THREAD **suspension_list_head, VOID (*suspend_cleanup)(TX_THREAD * NX_CLEANUP_PARAMETER), 
                                                     NX_PPPOE_CLIENT *pppoe_client_ptr, TX_MUTEX *mutex_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* Setup suspension list.  */
    if (*suspension_list_head)
    {

        /* This list is not NULL, add current thread to the end. */
        thread_ptr -> tx_thread_suspended_next =      *suspension_list_head;
        thread_ptr -> tx_thread_suspended_previous =  (*suspension_list_head) -> tx_thread_suspended_previous;
        ((*suspension_list_head) -> tx_thread_suspended_previous) -> tx_thread_suspended_next =  thread_ptr;
        (*suspension_list_head) -> tx_thread_suspended_previous =   thread_ptr;
    }
    else
    {

        /* No other threads are suspended.  Setup the head pointer and
           just setup this threads pointers to itself.  */
        *suspension_list_head =  thread_ptr;
        thread_ptr -> tx_thread_suspended_next =        thread_ptr;
        thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
    }

    /* Setup cleanup routine pointer.  */
    thread_ptr -> tx_thread_suspend_cleanup =  suspend_cleanup;

    /* Setup cleanup information, i.e. this pool control
       block.  */
    thread_ptr -> tx_thread_suspend_control_block =  (void *)pppoe_client_ptr;

    /* Set the state to suspended.  */
    thread_ptr -> tx_thread_state =  TX_TCP_IP;

    /* Set the suspending flag.  */
    thread_ptr -> tx_thread_suspending =  TX_TRUE;

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Save the timeout value.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release protection.  */
    tx_mutex_put(mutex_ptr);

    /* Call actual thread suspension routine.  */
    _tx_thread_system_suspend(thread_ptr);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pppoe_client_session_thread_resume              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resumes a thread suspended on a PPPoE Client session  */
/*    connect service.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*    status                                Return status                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume              Resume suspended thread       */
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
VOID  _nx_pppoe_client_session_thread_resume(TX_THREAD **suspension_list_head, UINT status)
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

        /* Determine if there are anymore threads on the suspension list.  */
        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
        {

            /* Only this thread is on the suspension list.  Simply set the
               list head to NULL to reflect an empty suspension list.  */
            *suspension_list_head =  TX_NULL;
        }
        else
        {

            /* More than one thread is on the suspension list, we need to
               adjust the link pointers and move the next entry to the
               front of the list.  */
            *suspension_list_head =  thread_ptr -> tx_thread_suspended_next;

            /* Update the links of the adjacent threads.  */
            (thread_ptr -> tx_thread_suspended_next) -> tx_thread_suspended_previous =
                thread_ptr -> tx_thread_suspended_previous;
            (thread_ptr -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                thread_ptr -> tx_thread_suspended_next;
        }

        /* Prepare for resumption of the thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Put return status into the thread control block.  */
        thread_ptr -> tx_thread_suspend_status =  status;

        /* Resume thread.  */
        _tx_thread_system_resume(thread_ptr);
    }
    else
    {

        /* Nothing was suspended.  Simply restore interrupts.  */
        TX_RESTORE
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pppoe_client_session_connect_cleanup            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes PPPoE connect timeout and thread terminate  */
/*    actions that require the PPPoE Client data structures to be cleaned */
/*    up.                                                                 */
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
/*    tx_event_flags_set                    Set event flag                */
/*    _tx_thread_system_resume              Resume thread service         */
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
static VOID  _nx_pppoe_client_session_connect_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER)
{

TX_INTERRUPT_SAVE_AREA

NX_PPPOE_CLIENT *pppoe_client_ptr;      /* Working PPPoE Client pointer  */

    NX_CLEANUP_EXTENSION

    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup pointer to PPPoE Client control block.  */
    pppoe_client_ptr =  (NX_PPPOE_CLIENT *)thread_ptr -> tx_thread_suspend_control_block;

    /* Determine if the PPPoE Client pointer is valid.  */
    if ((!pppoe_client_ptr) || (pppoe_client_ptr -> nx_pppoe_id != NX_PPPOE_CLIENT_ID))
    {

        /* Restore interrupts.  */
        TX_RESTORE

        return;
    }

    /* Determine if the cleanup is still required.  */
    if (!(thread_ptr -> tx_thread_suspend_cleanup))
    {

        /* Restore interrupts.  */
        TX_RESTORE

        return;
    }

    /* Determine if the caller is an ISR or the system timer thread.  */
#ifndef TX_TIMER_PROCESS_IN_ISR
    if ((TX_THREAD_GET_SYSTEM_STATE()) || (_tx_thread_current_ptr == &_tx_timer_thread))
#else
    if (TX_THREAD_GET_SYSTEM_STATE())
#endif
    {

        /* Yes, defer the processing to the PPPoE Client thread.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Set the deferred cleanup flag for the IP thread.  */
        tx_event_flags_set(&(pppoe_client_ptr -> nx_pppoe_events), NX_PPPOE_CLIENT_SESSION_CONNECT_CLEANUP_EVENT, TX_OR);

        /* Return to caller.  */
        return;
    }
    else
    {

        /* Yes, we still have thread suspension!  */

        /* Clear the suspension cleanup flag.  */
        thread_ptr -> tx_thread_suspend_cleanup = TX_NULL;

        /* Clear the suspension pointer.   */
        pppoe_client_ptr -> nx_pppoe_session_connect_suspended_thread = NX_NULL;

        /* Cleanup the PPPoE session.  */
        _nx_pppoe_client_session_cleanup(pppoe_client_ptr);

        /* Now we need to determine if this cleanup is from a terminate, timeout,
           or from a wait abort.  */
        if (thread_ptr -> tx_thread_state == TX_TCP_IP)
        {

            /* Thread still suspended on the PPPoE Client.  Setup return error status and
               resume the thread.  */

            /* Setup return status.  */
            thread_ptr -> tx_thread_suspend_status = NX_PPPOE_CLIENT_SESSION_NOT_ESTABLISHED;

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
#endif /* NX_DISABLE_IPV4 */

