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
/**   AutoIP (AutoIP)                                                     */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_AUTO_IP_SOURCE_CODE


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
#include "nx_arp.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "nx_auto_ip.h" 

/* Keep the AutoIP instance for callback notify.  */
static NX_AUTO_IP    *_nx_auto_ip_ptr;

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_auto_ip_create                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the AutoIP create function       */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    name                                  Pointer to AutoIP name        */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_ptr                             Pointer to start of stack     */ 
/*    stack_size                            Size of stack in bytes        */ 
/*    priority                              Priority of AutoIP thread     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_auto_ip_create                    AutoIP create function        */ 
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
UINT  _nxe_auto_ip_create(NX_AUTO_IP *auto_ip_ptr, CHAR *name, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (auto_ip_ptr == NX_NULL) || (auto_ip_ptr -> nx_auto_ip_id == NX_AUTO_IP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING
    
    /* Call actual AutoIP create routine.  */
    status =  _nx_auto_ip_create(auto_ip_ptr, name, ip_ptr, stack_ptr, stack_size, priority);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_create                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates an AutoIP instance for the associated IP      */ 
/*    instance. There must be only one AutoIP instance per IP instance.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    name                                  Pointer to AutoIP name        */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_ptr                             Pointer to start of stack     */ 
/*    stack_size                            Size of stack in bytes        */ 
/*    priority                              Priority of AutoIP thread     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_create                      Create AutoIP thread          */ 
/*    tx_event_flags_create                 Create event flags group      */ 
/*    tx_event_flags_delete                 Delete event flags group      */ 
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
UINT  _nx_auto_ip_create(NX_AUTO_IP *auto_ip_ptr, CHAR *name, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority)
{

UINT  status;


    /* Initialize the AutoIP control block to zero.  */
    memset((void *) auto_ip_ptr, 0, sizeof(NX_AUTO_IP));

    /* Create the AutoIP event flags.  */
    status =  tx_event_flags_create(&(auto_ip_ptr -> nx_auto_ip_conflict_event), "NetX AutoIP Collision Event");

    /* Determine if successful.  */
    if (status)
    {

        /* No, return an error.  */
        return(NX_AUTO_IP_ERROR);
    }

    /* Create the AutoIP processing thread.  */
    status =  tx_thread_create(&(auto_ip_ptr -> nx_auto_ip_thread), "NetX AutoIP", _nx_auto_ip_thread_entry, (ULONG) auto_ip_ptr,
                        stack_ptr, stack_size, priority, priority, 1, TX_DONT_START);

    /* Determine if the thread creation was successful.  */
    if (status)
    {

        /* Delete the event flags.  */
        tx_event_flags_delete(&(auto_ip_ptr -> nx_auto_ip_conflict_event));

        /* No, return an error.  */
        return(NX_AUTO_IP_ERROR);
    }

    /* Save the IP pointer and interface fields.  */
    auto_ip_ptr -> nx_auto_ip_ip_ptr =  ip_ptr;

    /* Default the auto IP interface to the primary interface. */
    auto_ip_ptr -> nx_ip_interface_index = 0;

    /* Save the AutoIP name.  */
    auto_ip_ptr -> nx_auto_ip_name =  name;

    /* Update the AutoIP structure ID.  */
    auto_ip_ptr -> nx_auto_ip_id =  NX_AUTO_IP_ID;

    /* Keep the AutoIP instance for callback notify.  */
    _nx_auto_ip_ptr = auto_ip_ptr;

    /* Return a successful status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_auto_ip_set_interface                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking services for the set interface*/
/*    index service.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    interface_index                       Index into IP interface table */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
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
UINT  _nxe_auto_ip_set_interface(NX_AUTO_IP *auto_ip_ptr, UINT interface_index)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((auto_ip_ptr == NX_NULL) || (auto_ip_ptr -> nx_auto_ip_id != NX_AUTO_IP_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    status = _nx_auto_ip_set_interface(auto_ip_ptr, interface_index);

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_set_interface                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the index for the interface on the local host    */
/*    needing auto IP to set the address. The default value is zero       */
/*    (primary interface).                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    interface_index                       Index into IP interface table */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
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
UINT  _nx_auto_ip_set_interface(NX_AUTO_IP *auto_ip_ptr, UINT interface_index)
{

    /* Determine if a local IP address was successfully setup.  */
    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {

        /* Bad interface index.  */
        return(NX_AUTO_IP_BAD_INTERFACE_INDEX);
    }
    

    /* Set the index to the specified value.  */
    auto_ip_ptr -> nx_ip_interface_index = interface_index;

    /* Return successful outcome.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_auto_ip_get_address                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the AutoIP get address function  */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    local_ip_address                      Destination for local IP addr */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_auto_ip_get_address               AutoIP get address function   */ 
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
UINT  _nxe_auto_ip_get_address(NX_AUTO_IP *auto_ip_ptr, ULONG *local_ip_address)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((auto_ip_ptr == NX_NULL) || (auto_ip_ptr -> nx_auto_ip_id != NX_AUTO_IP_ID) || (local_ip_address == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual AutoIP get address routine.  */
    status =  _nx_auto_ip_get_address(auto_ip_ptr, local_ip_address);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_get_address                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the local IP address resolved by the AutoIP */ 
/*    protocol. If there is no valid local IP address this routine        */ 
/*    returns an error and an IP address of all zeros.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    local_ip_address                      Destination for local IP addr */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
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
UINT  _nx_auto_ip_get_address(NX_AUTO_IP *auto_ip_ptr, ULONG *local_ip_address)
{

ULONG   host_ip_address;
NX_IP   *ip_ptr;


    /* Set local variables for convenience. */
    ip_ptr = auto_ip_ptr -> nx_auto_ip_ip_ptr;
    host_ip_address = ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_address;

    /* Determine if a local IP address was successfully setup.  */
    if (auto_ip_ptr -> nx_auto_ip_current_local_address == host_ip_address)
    {

        /* Yes, a local IP address is setup.  */
        *local_ip_address =  auto_ip_ptr -> nx_auto_ip_current_local_address;

        /* Return success!  */
        return(NX_SUCCESS);
    }
    else
    {

        /* No, a local IP address has not been setup.  Clear the return value.  */
        *local_ip_address =  0;

        /* Return an error.  */
        return(NX_AUTO_IP_NO_LOCAL);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_auto_ip_start                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the AutoIP start function        */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_auto_ip_start                     AutoIP start function         */ 
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
UINT  _nxe_auto_ip_start(NX_AUTO_IP *auto_ip_ptr, ULONG starting_local_address)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((auto_ip_ptr == NX_NULL) || (auto_ip_ptr -> nx_auto_ip_id != NX_AUTO_IP_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual AutoIP start routine.  */
    status =  _nx_auto_ip_start(auto_ip_ptr, starting_local_address);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_start                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts the AutoIP thread of a previously created      */ 
/*    AutoIP instance.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*    starting_local_address                Starting local IP address,    */ 
/*                                              only IP addresses from    */ 
/*                                              169.254.1.0 through       */ 
/*                                              169.254.254.255 are valid,*/ 
/*                                              where a value of 0 implies*/ 
/*                                              a random generated value  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_resume                      Resume AutoIP thread          */ 
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
UINT  _nx_auto_ip_start(NX_AUTO_IP *auto_ip_ptr, ULONG starting_local_address)
{


    /* First, start the local IP address to that supplied.  */
    auto_ip_ptr ->  nx_auto_ip_current_local_address =  starting_local_address;

    /* Set the restart flag.   */
    auto_ip_ptr -> nx_auto_ip_restart_flag =  NX_TRUE;

    /* Set the event flags to ensure AutoIP thread wakes up.  */
    tx_event_flags_set(&(auto_ip_ptr -> nx_auto_ip_conflict_event), 0x1, TX_OR);

    /* Resume the AutoIP thread.  */
    tx_thread_resume(&(auto_ip_ptr -> nx_auto_ip_thread));

    /* Return status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_auto_ip_stop                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the AutoIP stop function         */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_auto_ip_stop                      AutoIP stop function          */ 
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
UINT  _nxe_auto_ip_stop(NX_AUTO_IP *auto_ip_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((auto_ip_ptr == NX_NULL) || (auto_ip_ptr -> nx_auto_ip_id != NX_AUTO_IP_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual AutoIP stop routine.  */
    status =  _nx_auto_ip_stop(auto_ip_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_stop                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stops the AutoIP thread of a previously created and   */ 
/*    started AutoIP instance.                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_suspend                     Suspend AutoIP thread         */ 
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
UINT  _nx_auto_ip_stop(NX_AUTO_IP *auto_ip_ptr)
{

UINT        status;


    /* Suspend the AutoIP thread.  */
    status =  tx_thread_suspend(&(auto_ip_ptr -> nx_auto_ip_thread));

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_auto_ip_delete                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the AutoIP delete function       */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_auto_ip_delete                    AutoIP delete function        */ 
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
UINT  _nxe_auto_ip_delete(NX_AUTO_IP *auto_ip_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((auto_ip_ptr == NX_NULL) || (auto_ip_ptr -> nx_auto_ip_id != NX_AUTO_IP_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual AutoIP delete routine.  */
    status =  _nx_auto_ip_delete(auto_ip_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_delete                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the previously created and stopped AutoIP     */ 
/*    AutoIP instance.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_delete                 Delete event flags            */ 
/*    tx_thread_delete                      Delete AutoIP thread          */ 
/*    tx_thread_terminate                   Terminate AutoIP thread       */ 
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
UINT  _nx_auto_ip_delete(NX_AUTO_IP *auto_ip_ptr)
{

NX_IP       *ip_ptr;


    /* First, set the ID to show it is invalid.  */
    auto_ip_ptr -> nx_auto_ip_id =  0;

    /* Pickup the IP pointer.  */
    ip_ptr =  auto_ip_ptr -> nx_auto_ip_ip_ptr;

    /* Get IP mutex so IP conflict notification can be setup.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Clear the handler to avoid conflict notification callbacks.  */
    ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_conflict_notify_handler =  NX_NULL;
    ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_probe_address = 0;

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Terminate the AutoIP thread.  */
    tx_thread_terminate(&(auto_ip_ptr -> nx_auto_ip_thread));

    /* Delete the AutoIP thread.  */
    tx_thread_delete(&(auto_ip_ptr -> nx_auto_ip_thread));

    /* Delete the AutoIP event flags.  */
    tx_event_flags_delete(&(auto_ip_ptr -> nx_auto_ip_conflict_event));

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_thread_entry                            PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the entry point of the AutoIP thread. All AutoIP   */ 
/*    actions are coordinated by this thread.                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    auto_ip_ptr                           Pointer to AutoIP instance    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_auto_ip_arp_packet_send           Send AutoIP probe and         */ 
/*                                            announce ARP messages       */ 
/*    nx_ip_interface_address_set           Set interface address         */ 
/*    nx_ip_interface_status_check          Get interface status          */ 
/*    tx_event_flags_get                    Get event flags               */ 
/*    tx_mutex_get                          Get IP protection             */ 
/*    tx_mutex_put                          Put IP protection             */ 
/*    tx_thread_sleep                       Thread sleep                  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_auto_ip_thread_entry(ULONG auto_ip_ptr_info)  
{

NX_AUTO_IP      *auto_ip_ptr;
NX_IP           *ip_ptr;
UINT            i, status;
ULONG           addresses;
ULONG           conflict;
ULONG           temp;
ULONG           delay;
ULONG           hw_address_lsw;
ULONG           host_ip_address;


    /* Pickup the AutoIP pointer.  */
    auto_ip_ptr =  (NX_AUTO_IP *) auto_ip_ptr_info;

    /* Pickup the associated IP pointer.  */
    ip_ptr =  auto_ip_ptr -> nx_auto_ip_ip_ptr;   

    /* Wait for the IP instance to be initialized before proceeding. This will ensure the 
       MAC address is valid before a random local IP address is generated.  */
    nx_ip_interface_status_check(ip_ptr, auto_ip_ptr -> nx_ip_interface_index, NX_IP_LINK_ENABLED, &temp, NX_WAIT_FOREVER);

    /* Set LSW of hardware address */
    hw_address_lsw = ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_physical_address_lsw;

    /* Setup the conflict flag to false.  */
    conflict =  NX_FALSE;

    /* Loop forever inside the AutoIP thread!  */
    while(1)
    {

        /* Probe for local IP address use!  */

        /* Update the local variable in case anything has changed. */
        host_ip_address =  ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_address;

        /* Wait until the IP address is non-zero.  The only way this can be non-zero is 
           if the application has changed it or it was resolved by the DHCP protocol.  */
        while (host_ip_address)
        {

            /* Sleep for the maximum probe period.  */
            tx_thread_sleep(NX_IP_PERIODIC_RATE * NX_AUTO_IP_PROBE_MAX);

            /* Get the IP address from the IP task and see if it has changed from non zero. */ 
            host_ip_address =  ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_address;
        }

        /* Determine if the restart flag is set.  */
        if (auto_ip_ptr -> nx_auto_ip_restart_flag)
        {

            /* Clear the restart flag.  */
            auto_ip_ptr -> nx_auto_ip_restart_flag =  NX_FALSE;

            /* Reset the conflict count.  */
            auto_ip_ptr -> nx_auto_ip_conflict_count =  0;

            /* Clear the conflict flag, since we might have an new starting local IP address.  */
            conflict =  NX_FALSE;
        }

        /* Determine if a conflict condition is present.  This flag is set at various places
           below.  */
        if (conflict)
        {

            /* Yes, a conflict was detected.  */

            /* Increment the conflict count.  */
            auto_ip_ptr -> nx_auto_ip_conflict_count++;

            /* Determine if we have reached the maximum number of conflicts.  */
            if (auto_ip_ptr -> nx_auto_ip_conflict_count > NX_AUTO_IP_MAX_CONFLICTS)
            {

                /* Yes, we have had excessive conflicts... we must now add extra delay.  */
                tx_thread_sleep(NX_IP_PERIODIC_RATE * NX_AUTO_IP_RATE_LIMIT_INTERVAL);
            }

            /* Clear the the local IP address in order to generate another random one.  */
            auto_ip_ptr -> nx_auto_ip_current_local_address =  0;

            /* Clear the conflict flag.  */
            conflict =  NX_FALSE;
        }

        /* Determine if a starting local IP address needs to be derived.  */
        if (auto_ip_ptr -> nx_auto_ip_current_local_address == 0)
        {

            /* Yes, the starting local IP address must be derived.  */

            /* Get pseudo random value with LSW of MAC address.  */
            temp =  ((ULONG) NX_RAND()) + hw_address_lsw;

            /* Determine the address range of local IP addresses.  */
            addresses =  NX_AUTO_IP_END_ADDRESS - NX_AUTO_IP_START_ADDRESS;

            /* Make sure that the random number fits within this range.  */
            temp =  temp % addresses;

            /* Now create a starting local IP address.  */
            auto_ip_ptr -> nx_auto_ip_current_local_address =  NX_AUTO_IP_START_ADDRESS + temp;
        }

        /* Register with NetX for ARP conflict detection for this local IP address.  */

        /* Get IP mutex so ARP notification can be setup.  */
        tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

        /* Finally, setup the handler to indicate the we want collision notification.  */
        ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_conflict_notify_handler = _nx_auto_ip_conflict;

        /* Clear any outstanding events.  */
        tx_event_flags_get(&(auto_ip_ptr -> nx_auto_ip_conflict_event), 0x1, TX_OR_CLEAR, &temp, TX_NO_WAIT);

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Calculate the delay time.  */
        delay = (ULONG)NX_RAND() % (NX_IP_PERIODIC_RATE * NX_AUTO_IP_PROBE_WAIT);

        /* Sleep for a small period of time.  */
        tx_thread_sleep(delay);

        /* Loop to probe for the specified local IP address.  */
        for (i = 0; i < NX_AUTO_IP_PROBE_NUM; i++)
        {

            /* Increment the total probe count.  */
            auto_ip_ptr -> nx_auto_ip_probe_count++;

            /* Send the ARP probe.  */
            _nx_arp_probe_send(ip_ptr, auto_ip_ptr -> nx_ip_interface_index, auto_ip_ptr -> nx_auto_ip_current_local_address);

            /* Calculate the delay time.  */
            delay =  ((ULONG) NX_RAND()) % (NX_IP_PERIODIC_RATE * NX_AUTO_IP_PROBE_MAX);

            /* Determine if this is less than the minimum.  */
            if (delay < (NX_IP_PERIODIC_RATE * NX_AUTO_IP_PROBE_MIN))
            {

                /* Set the delay to the minimum.  */
                delay =  (NX_IP_PERIODIC_RATE * NX_AUTO_IP_PROBE_MIN);
            }

            /* Delay by waiting for conflict events before sending the next ARP probe. This event is 
               set by the conflict callback function when an ARP entry is received that matches 
               the registered address.  */
            status =  tx_event_flags_get(&(auto_ip_ptr -> nx_auto_ip_conflict_event), 0x1, TX_OR_CLEAR, &temp, delay);

            /* Determine if conflict was detected.  */
            if (status == NX_SUCCESS)
            {

                /* Yes, a conflict is present.  */ 

                /* Set the conflict flag and get out of the loop.  */
                conflict =  NX_TRUE;
                break;
            }

            /* Update the local variable in case anything has changed. */
            host_ip_address =  ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_address;

            /* Check for a restart or an IP address change.  */
            if (host_ip_address || (auto_ip_ptr -> nx_auto_ip_restart_flag))
                break;
        }

        /* Determine if there was a conflict. If so, continue at the top of the loop.  */
        if (conflict)
        {

#ifdef NX_AUTO_IP_DEBUG
            printf("AutoIP %s, CONFLICT for: %lu,%lu,%lu,%lu\n", auto_ip_ptr -> nx_auto_ip_name, 
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 24),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 16 & 0xFF),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 8 & 0xFF),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address & 0xFF));
#endif
            continue;
        }

        /* Check for a restart or an IP address change.  */
        if (host_ip_address || (auto_ip_ptr -> nx_auto_ip_restart_flag))
            continue;

        /* At this point, the local IP address has been successfully probed via ARP messages without
           any collisions.  It is now time to go into an announce phase to indicate local IP address
           is ours!  */
        
#ifdef NX_AUTO_IP_DEBUG
        printf("AutoIP %s, RESOLVED for: %lu,%lu,%lu,%lu\n", auto_ip_ptr -> nx_auto_ip_name, 
                                (auto_ip_ptr -> nx_auto_ip_current_local_address >> 24),
                                (auto_ip_ptr -> nx_auto_ip_current_local_address >> 16 & 0xFF),
                                (auto_ip_ptr -> nx_auto_ip_current_local_address >> 8 & 0xFF),
                                (auto_ip_ptr -> nx_auto_ip_current_local_address & 0xFF));
#endif

        /* Set the NetX IP address.  */
        nx_ip_interface_address_set(ip_ptr, auto_ip_ptr -> nx_ip_interface_index, auto_ip_ptr -> nx_auto_ip_current_local_address, 0xFFFF0000);

        /* Delay before announcing: NX_AUTO_IP_ANNOUNCE_WAIT.  */
        tx_thread_sleep(NX_IP_PERIODIC_RATE * NX_AUTO_IP_ANNOUNCE_WAIT);

       /* It is now time to go into an announce phase to indicate local IP address is ours!  */
        for (i = 0; i < NX_AUTO_IP_ANNOUNCE_NUM; i++)
        {

            /* Increment the total announcement count.  */
            auto_ip_ptr -> nx_auto_ip_announce_count++;
                
            /* Send the ARP announcement.  */
            _nx_arp_announce_send(ip_ptr, auto_ip_ptr -> nx_ip_interface_index);

            /* Calculate the delay time.  */
            delay =  (NX_IP_PERIODIC_RATE * NX_AUTO_IP_ANNOUNCE_INTERVAL);

            /* Delay by waiting for conflict events before sending the next ARP announcement. This event is 
               set by the conflict callback function when an ARP entry is received that matches 
               the registered address.  */
            status =  tx_event_flags_get(&(auto_ip_ptr -> nx_auto_ip_conflict_event), 0x1, TX_OR_CLEAR, &temp, delay);

            /* Determine if conflict was detected.  */
            if (status == NX_SUCCESS)
            {

                /* Yes, a conflict is present.  */ 

                /* Set the conflict flag and get out of the loop.  */
                conflict =  NX_TRUE;
                break;
            }
        }

        /* Determine if there was a conflict. If so, continue at the top of the loop.  */
        if (conflict)
        {

#ifdef NX_AUTO_IP_DEBUG
            printf("AutoIP %s, CONFLICT for: %lu,%lu,%lu,%lu\n", auto_ip_ptr -> nx_auto_ip_name, 
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 24),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 16 & 0xFF),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 8 & 0xFF),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address & 0xFF));
#endif

            /* Conflict, clear the IP address.  */
            nx_ip_interface_address_set(ip_ptr, auto_ip_ptr -> nx_ip_interface_index, 0, 0);

            continue;
        }

        /* Now, wait infinitely for another conflict situation.  */
        tx_event_flags_get(&(auto_ip_ptr -> nx_auto_ip_conflict_event), 0x1, TX_OR_CLEAR, &temp, TX_WAIT_FOREVER);

        /* Update the local variable in case anything has changed. */
        host_ip_address =  ip_ptr -> nx_ip_interface[auto_ip_ptr -> nx_ip_interface_index].nx_interface_ip_address;

        /* Determine if a conflict is present.  */

        /* If we get here a late conflict is present.  */ 
        if (auto_ip_ptr -> nx_auto_ip_current_local_address == host_ip_address)
        {

            /* Increment the total defend count.  */
            auto_ip_ptr -> nx_auto_ip_defend_count++;

#ifdef NX_AUTO_IP_DEBUG
            printf("AutoIP %s, DEFEND for: %lu,%lu,%lu,%lu\n", auto_ip_ptr -> nx_auto_ip_name, 
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 24),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 16 & 0xFF),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address >> 8 & 0xFF),
                                    (auto_ip_ptr -> nx_auto_ip_current_local_address & 0xFF));
#endif

            /* No defense currently, just clear the IP address once a late collision is detected
               and start over.  */
            nx_ip_interface_address_set(ip_ptr, auto_ip_ptr -> nx_ip_interface_index, 0, 0);

            /* Set the conflict flag.  */
            conflict =  NX_TRUE;
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_auto_ip_conflict                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function notifies the AutoIP instance that a conflict was      */ 
/*    detected by the NetX ARP receive packet handling routine.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                                IP instance pointer           */ 
/*    interface_index                       IP Interface Index            */
/*    ip_address                            IP Address to bind to         */ 
/*    physical_msw                          Physical address MSW          */ 
/*    physical_lsw                          Physical address LSW          */ 
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
/*    NetX                                                                */ 
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
VOID  _nx_auto_ip_conflict(NX_IP *ip_ptr, UINT interface_index, ULONG ip_address, ULONG physical_msw, ULONG physical_lsw)
{

    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(interface_index);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);

    /* Set the event flags to indicate that a collision was detected by NetX ARP processing.  */
    tx_event_flags_set(&(_nx_auto_ip_ptr -> nx_auto_ip_conflict_event), 0x1, TX_OR);
}
#endif /* NX_DISABLE_IPV4 */
