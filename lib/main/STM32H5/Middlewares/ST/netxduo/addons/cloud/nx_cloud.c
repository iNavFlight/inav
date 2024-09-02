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
/**   Cloud Helper                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_CLOUD_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include "nx_cloud.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/* Define the DHCP Internal Function.  */
static VOID _nx_cloud_thread_entry(ULONG cloud_ptr_value);
static VOID _nx_cloud_periodic_timer_entry(ULONG cloud_ptr_value);


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_cloud_create                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the cloud create function call.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    cloud_name                            Name of this cloud instnace   */
/*    memory_ptr                            Pointer memory area for cloud */
/*    memory_size                           Size of cloud memory area     */
/*    priority                              Priority of helper thread     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_cloud_create                      Actual cloud create function  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_cloud_create(NX_CLOUD *cloud_ptr, const CHAR *cloud_name, VOID *memory_ptr, ULONG memory_size, UINT priority)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((cloud_ptr == NX_NULL) || (memory_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for a memory size error.  */
    if (memory_size < TX_MINIMUM_STACK)
    {
        return(NX_SIZE_ERROR);
    }

    /* Check the priority specified.  */
    if (priority >= TX_MAX_PRIORITIES)
    {
        return(NX_OPTION_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual Cloud instance create function.  */
    status = _nx_cloud_create(cloud_ptr, cloud_name, memory_ptr, memory_size, priority);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_create                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an Internet Protocol instance, and setting up */
/*    all appropriate data structures.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    cloud_name                            Name of this cloud instnace   */
/*    memory_ptr                            Pointer memory area for cloud */
/*    memory_size                           Size of cloud memory area     */
/*    priority                              Priority of helper thread     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_create                 Create cloud event flags      */
/*    tx_mutex_create                       Create cloud protection mutex */
/*    tx_thread_create                      Create cloud helper thread    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_cloud_create(NX_CLOUD *cloud_ptr, const CHAR* cloud_name, VOID* memory_ptr, ULONG memory_size, UINT priority)
{

UINT status;
UINT old_threshold = 0;


    /* Initialize the cloud control block to zero.  */
    memset(cloud_ptr, 0, sizeof(NX_CLOUD));

    /* Save the supplied name.  */
    cloud_ptr -> nx_cloud_name = cloud_name;

    /* Create the mutex.  */
    status = tx_mutex_create(&(cloud_ptr -> nx_cloud_mutex), (CHAR*)cloud_name, TX_NO_INHERIT);

    /* Check status.  */
    if (status)
    {

        /* Return status.  */
        return(status);
    }

    /* Create the event flag.  */
    status = tx_event_flags_create(&cloud_ptr -> nx_cloud_events, (CHAR*)cloud_name);
    
    /* Check status.  */
    if (status)
    {
        
        /* Release resource.  */
        tx_mutex_delete(&(cloud_ptr -> nx_cloud_mutex));

        /* Return status.  */
        return(status);
    }

    /* Disable preemption. */
    tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

    /* Create cloud helper thread. */
    status = tx_thread_create(&(cloud_ptr -> nx_cloud_thread), (CHAR*)cloud_name,
                              _nx_cloud_thread_entry, (ULONG)cloud_ptr,
                              memory_ptr, memory_size, priority, priority, 1, TX_AUTO_START);
        
    /* Check status.  */
    if (status)
    {
        
        /* Release resources.  */
        tx_event_flags_delete(&(cloud_ptr -> nx_cloud_events));
        tx_mutex_delete(&(cloud_ptr -> nx_cloud_mutex));

        /* Restore preemption . */
        tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);

        /* Return status.  */
        return(status);
    }
    
    /* Create the periodic timer for cloud modules.  */
    status = tx_timer_create(&(cloud_ptr -> nx_cloud_periodic_timer), (CHAR*)cloud_name,
                             _nx_cloud_periodic_timer_entry, (ULONG)cloud_ptr,
                             NX_IP_PERIODIC_RATE, NX_IP_PERIODIC_RATE, TX_AUTO_ACTIVATE);

    /* Check status.  */
    if (status)
    {
        
        /* Release resources.  */
        tx_thread_delete(&(cloud_ptr -> nx_cloud_thread));
        tx_event_flags_delete(&(cloud_ptr -> nx_cloud_events));
        tx_mutex_delete(&(cloud_ptr -> nx_cloud_mutex));

        /* Restore preemption . */
        tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);

        /* Return status.  */
        return(status);
    }

    /* Load the ID field.  */
    cloud_ptr -> nx_cloud_id = NX_CLOUD_ID;

    /* Restore preemption . */
    tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_cloud_delete                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the cloud delete function call.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_cloud_delete                      Actual cloud delete function  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_cloud_delete(NX_CLOUD *cloud_ptr)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((cloud_ptr == NX_NULL) || (cloud_ptr -> nx_cloud_id != NX_CLOUD_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual Cloud instance delete function.  */
    status = _nx_cloud_delete(cloud_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_delete                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an Internet Protocol instance, and setting up */
/*    all appropriate data structures.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    cloud_name                            Name of this cloud instnace   */
/*    memory_ptr                            Pointer memory area for cloud */
/*    memory_size                           Size of cloud memory area     */
/*    priority                              Priority of helper thread     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain cloud protection mutex */
/*    tx_mutex_put                          Release cloud protection mutex*/
/*    tx_mutex_delete                       Delete cloud protection mutex */
/*    tx_timer_deactivate                   Deactivate cloud timer        */
/*    tx_timer_delete                       Delete cloud timer            */
/*    tx_event_flags_delete                 Delete cloud event flags      */
/*    tx_thread_terminate                   Terminate cloud helper thread */
/*    tx_thread_delete                      Delete cloud helper thread    */
/*    tx_thread_preemption_change           Change thread preemption      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_cloud_delete(NX_CLOUD *cloud_ptr)
{

UINT old_threshold = 0;


    /* Get mutex protection.  */
    tx_mutex_get(&(cloud_ptr -> nx_cloud_mutex), TX_WAIT_FOREVER);

    /* Check if all modules are deregistered.  */
    if (cloud_ptr -> nx_cloud_modules_count)
    {
        
        /* Still modules bound to this Cloud instance. They must all be deleted prior
           to deleting the Cloud instance.  Release the mutex and return
           an error code.  */
        tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));

        return(NX_CLOUD_MODULE_BOUND);
    }

    /* Disable preemption. */
    tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

    /* Release mutex protection.  */
    tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));
    
    /* Deactivate and delete timer. */
    tx_timer_deactivate(&(cloud_ptr -> nx_cloud_periodic_timer));
    tx_timer_delete(&(cloud_ptr -> nx_cloud_periodic_timer));

    /* Terminate the internal cloud thread.  */
    tx_thread_terminate(&(cloud_ptr -> nx_cloud_thread));
    
    /* Delete the internal cloud protection mutex.  */
    tx_mutex_delete(&(cloud_ptr -> nx_cloud_mutex));

    /* Delete the internal cloud event flag object.  */
    tx_event_flags_delete(&(cloud_ptr -> nx_cloud_events));

    /* Delete the internal cloud helper thread for handling more processing intensive
       duties.  */
    tx_thread_delete(&(cloud_ptr -> nx_cloud_thread));
    
    /* Clear the ID to make it invalid.  */
    cloud_ptr -> nx_cloud_id =  0;

    /* Restore preemption . */
    tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_cloud_module_register                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the cloud module register        */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*    module_name                           Name of Cloud module          */
/*    module_registered_event               Module registered event       */
/*    module_process                        Module processing routine     */
/*    module_context                        Context                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_cloud_module_register             Actual cloud module register  */
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
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_cloud_module_register(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE *module_ptr, const CHAR *module_name, ULONG module_registered_event,
                               VOID (*module_process)(VOID* module_context, ULONG common_events, ULONG module_own_events), VOID *module_context)
{

 UINT status;


    /* Check for invalid input pointers.  */
    if ((cloud_ptr == NX_NULL) || (cloud_ptr -> nx_cloud_id != NX_CLOUD_ID) || 
        (module_ptr == NX_NULL) || (module_process == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid module.  */
    if (module_registered_event == 0)
    {
        return(NX_CLOUD_MODULE_EVENT_INVALID);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual Cloud instance create function.  */
    status = _nx_cloud_module_register(cloud_ptr, module_ptr, module_name, module_registered_event, module_process, module_context);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_module_register                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers the cloud module which is running on cloud  */
/*    helper thread.                                                      */
/*                                                                        */
/*    Note: Registered event should be module own event | common events,  */
/*    common events can be null or all.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*    module_name                           Name of Cloud module          */
/*    module_registered_event               Module registered event       */
/*    module_process                        Module processing routine     */
/*    module_context                        Context                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain cloud protection mutex */
/*    tx_mutex_put                          Release cloud protection mutex*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_cloud_module_register(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE *module_ptr, const CHAR *module_name, ULONG module_registered_event,
                               VOID(*module_process)(VOID *module_context, ULONG common_events, ULONG module_own_events), VOID *module_context)
{
    
NX_CLOUD_MODULE *current_module;


    /* Get mutex. */
    tx_mutex_get(&(cloud_ptr -> nx_cloud_mutex), NX_WAIT_FOREVER);
            
    /* Perform duplicate module detection.  */
    for (current_module = cloud_ptr -> nx_cloud_modules_list_header; current_module; current_module = current_module -> nx_cloud_module_next)
    {
        
        /* Check if the module is already registered.  */
        if (current_module == module_ptr)
        {

            /* Release mutex. */
            tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));

            return(NX_CLOUD_MODULE_ALREADY_REGISTERED);
        }
    }

    /* Set module info.  */
    module_ptr -> nx_cloud_module_name = module_name;
    module_ptr -> nx_cloud_module_registered_events = module_registered_event;
    module_ptr -> nx_cloud_module_process = module_process;
    module_ptr -> nx_cloud_module_context = module_context;
    module_ptr -> nx_cloud_ptr = cloud_ptr;

    /* Update the module list and count.  */
    module_ptr -> nx_cloud_module_next = cloud_ptr -> nx_cloud_modules_list_header;
    cloud_ptr -> nx_cloud_modules_list_header = module_ptr;
    cloud_ptr -> nx_cloud_modules_count ++;

    /* Release mutex. */
    tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_cloud_module_deregister                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the cloud module deregister      */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_cloud_module_deregister           Actual cloud module deregister*/
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
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_cloud_module_deregister(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr)
{

 UINT status;


    /* Check for invalid input pointers.  */
    if ((cloud_ptr == NX_NULL) || (cloud_ptr -> nx_cloud_id != NX_CLOUD_ID) ||
        (module_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual Cloud instance create function.  */
    status = _nx_cloud_module_deregister(cloud_ptr, module_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_module_deregister                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deregisters the cloud module which is running on      */
/*    cloud helper thread.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr                             Pointer to cloud control block*/
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain cloud protection mutex */
/*    tx_mutex_put                          Release cloud protection mutex*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_cloud_module_deregister(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr)
{

NX_CLOUD_MODULE *previous_module;
NX_CLOUD_MODULE *current_module;
UINT            found = NX_FALSE;


    /* Get mutex. */
    tx_mutex_get(&(cloud_ptr -> nx_cloud_mutex), NX_WAIT_FOREVER);

    /* Loop to find the module.  */
    for (previous_module = NX_NULL, current_module = cloud_ptr -> nx_cloud_modules_list_header;
         current_module; previous_module = current_module, current_module = current_module -> nx_cloud_module_next)
    {

        /* Check the module.  */
        if (current_module == module_ptr)
        {
            found = NX_TRUE;
            break;
        }
    }

    /* Check if found the module.  */
    if (found == NX_FALSE)
    {

        /* Release mutex. */
        tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));

        return(NX_CLOUD_MODULE_NOT_REGISTERED);
    }

    /* Yes, found.  */
    if (previous_module == NX_NULL)
    {

        /* This is the header of the list. */
        cloud_ptr -> nx_cloud_modules_list_header = current_module -> nx_cloud_module_next;
    }
    else
    {
        previous_module -> nx_cloud_module_next = current_module -> nx_cloud_module_next;
    }
    
    /* Decrease the created modules count.  */
    cloud_ptr -> nx_cloud_modules_count--;

    /* Release mutex. */
    tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_cloud_module_event_set                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the cloud module event set       */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*    module_own_event                      Module event                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_cloud_module_event_set            Actual cloud module event set */
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
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_cloud_module_event_set(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event)
{
  
UINT status;

    /* Check for invalid input pointers.  */
    if (cloud_module == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid module event.  */
    if (module_own_event == 0)
    {
        return(NX_CLOUD_MODULE_EVENT_INVALID);
    }

    /* Call actual cloud module event set function.  */
    status = _nx_cloud_module_event_set(cloud_module, module_own_event);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_module_event_set                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the cloud module events that are processed in    */
/*    module processing routine.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*    module_own_event                      Module event                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    tx_event_flags_set                    Set event flags to wakeup     */
/*                                            cloud helper thread         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_cloud_module_event_set(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event)
{

TX_INTERRUPT_SAVE_AREA

NX_CLOUD        *cloud_ptr = cloud_module -> nx_cloud_ptr;
ULONG           registered_event;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Define the actual module event in this module that are processed in module processing routine.  */
    cloud_module -> nx_cloud_module_own_events |= module_own_event;

    /* Set module event that are used to stimulate the cloud helper thread and call the module processing routine.  */
    registered_event = (cloud_module -> nx_cloud_module_registered_events)&(~NX_CLOUD_COMMON_PERIODIC_EVENT);

    /* Restore interrupts.  */
    TX_RESTORE

    tx_event_flags_set(&(cloud_ptr -> nx_cloud_events), registered_event, TX_OR);
    
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_cloud_module_event_clear                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the cloud module event clear     */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*    module_own_event                      Module event                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_cloud_module_event_set            Actual cloud module event     */
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
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nxe_cloud_module_event_clear(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event)
{
  
UINT status;

    /* Check for invalid input pointers.  */
    if (cloud_module == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid module event.  */
    if (module_own_event == 0)
    {
        return(NX_CLOUD_MODULE_EVENT_INVALID);
    }

    /* Call actual cloud module event set function.  */
    status = _nx_cloud_module_event_set(cloud_module, module_own_event);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_module_event_clear                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function clears the cloud module events that are processed in  */
/*    module processing routine.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_ptr                            Pointer to cloud module       */
/*                                            control block               */
/*    module_event                          Module event                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    tx_event_flags_set                    Set event flags to wakeup     */
/*                                            cloud helper thread         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _nx_cloud_module_event_clear(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event)
{

TX_INTERRUPT_SAVE_AREA

NX_CLOUD        *cloud_ptr = cloud_module -> nx_cloud_ptr;
ULONG           registered_event;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Define the actual module event in this module that are processed in module processing routine.  */
    cloud_module -> nx_cloud_module_own_events &= ~module_own_event;

    /* Check if have the own events of module.  */
    if (cloud_module -> nx_cloud_module_own_events == 0)
    {

        /* Clear the module event that are used to stimulate the cloud helper thread and call the module processing routine.  */
        registered_event = (cloud_module -> nx_cloud_module_registered_events)&(~NX_CLOUD_COMMON_PERIODIC_EVENT);

        /* Restore interrupts.  */
        TX_RESTORE

        tx_event_flags_set(&(cloud_ptr -> nx_cloud_events), ~registered_event, TX_AND);
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }
    
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_thread_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the entry point for cloud helper thread. The cloud */
/*    helper thread is responsible for periodic all modules events.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr_value                       Pointer to Cloud block        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get the DHCP mutex            */
/*    tx_mutex_put                          Release the DHCP mutex        */
/*    (nx_cloud_module_process)             Module processing             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX Scheduler                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
static VOID _nx_cloud_thread_entry(ULONG cloud_ptr_value)
{

TX_INTERRUPT_SAVE_AREA

NX_CLOUD        *cloud_ptr;
NX_CLOUD_MODULE *cloud_module;
ULONG           cloud_events;
ULONG           module_own_events;


    /* Setup the Cloud pointer.  */
    cloud_ptr = (NX_CLOUD*)cloud_ptr_value;

    for (;;)
    {

        /* Wait for event.  */
        tx_event_flags_get(&cloud_ptr -> nx_cloud_events, NX_CLOUD_ALL_EVENTS,
                           TX_OR_CLEAR, &cloud_events, TX_WAIT_FOREVER);

        /* Get mutex. */
        tx_mutex_get(&cloud_ptr -> nx_cloud_mutex, NX_WAIT_FOREVER);

        /* Wake up the module.  */
        for (cloud_module = cloud_ptr -> nx_cloud_modules_list_header; cloud_module; cloud_module = cloud_module -> nx_cloud_module_next)
        {

            /* Check the cloud events.  */
            if (cloud_events & cloud_module -> nx_cloud_module_registered_events)
            {

                /* Disable interrupts.  */
                TX_DISABLE

                /* Get the module own events.  */
                module_own_events = cloud_module -> nx_cloud_module_own_events;

                /* Clear the module own events.  */
                cloud_module -> nx_cloud_module_own_events = 0;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Release the mutex.  */
                tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));

                /* Call the module processing routine.  */
                cloud_module -> nx_cloud_module_process(cloud_module -> nx_cloud_module_context, cloud_events & cloud_module -> nx_cloud_module_registered_events, module_own_events);

                /* Get mutex. */
                tx_mutex_get(&cloud_ptr -> nx_cloud_mutex, NX_WAIT_FOREVER);
            }
        }

        /* Release the mutex.  */
        tx_mutex_put(&(cloud_ptr -> nx_cloud_mutex));
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_cloud_periodic_timer_entry                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles waking up the Cloud helper thread on a        */
/*    periodic timer event.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cloud_ptr_value                       Cloud address in a ULONG      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set event flags to wakeup     */
/*                                            cloud helper thread         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX system timer thread                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
static VOID _nx_cloud_periodic_timer_entry(ULONG cloud_ptr_value)
{

NX_CLOUD *cloud_ptr = (NX_CLOUD *)cloud_ptr_value;


    /* Wakeup this cloud's helper thread.  */
    tx_event_flags_set(&(cloud_ptr -> nx_cloud_events), NX_CLOUD_COMMON_PERIODIC_EVENT, TX_OR);
}