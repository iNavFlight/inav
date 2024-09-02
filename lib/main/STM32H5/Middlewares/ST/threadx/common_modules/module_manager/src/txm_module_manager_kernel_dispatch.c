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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Module Manager                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE

#include "tx_api.h"
#include "tx_block_pool.h"
#include "tx_byte_pool.h"
#include "tx_event_flags.h"
#include "tx_queue.h"
#include "tx_mutex.h"
#include "tx_semaphore.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_trace.h"
#include "txm_module.h"
#include "txm_module_manager_util.h"
#include "txm_module_manager_dispatch.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_kernel_dispatch                 PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function dispatches the module's kernel request based upon the */
/*    ID and parameters specified in the request.                         */
/*    To disallow modules to use specific ThreadX services, define        */
/*    TXM_***_CALL_NOT_USED (see #ifndefs surrounding each case).         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    kernel_request                    Module's kernel request           */
/*    param_1                           First parameter                   */
/*    param_2                           Second parameter                  */
/*    param_3                           Third parameter                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_module_manager_application_request   Application-specific req  */
/*    _txm_module_manager_object_pointer_get    Find object pointer       */
/*    _txm_module_manager_thread_create         Module thread create      */
/*    [_txm_module_manager_*_dispatch]          Optional external         */
/*                                                component dispatch      */
/*    ThreadX API Calls                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  12-31-2020      Scott Larson            Modified comment(s), added    */
/*                                            port-specific dispatch,     */
/*                                            resulting in version 6.1.3  */
/*  04-02-2021      Scott Larson            Modified comment(s),          */
/*                                            added optional defines to   */
/*                                            remove unneeded functions,  */
/*                                            resulting in version 6.1.6  */
/*  01-31-2022      Scott Larson            Modified comments and added   */
/*                                            CALL_NOT_USED option,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
ALIGN_TYPE _txm_module_manager_kernel_dispatch(ULONG kernel_request, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value = TX_NOT_AVAILABLE;
TXM_MODULE_INSTANCE *module_instance;


    /* Get the module instance.  */
    module_instance = _tx_thread_current_ptr -> tx_thread_module_instance_ptr;

    /* Sanity-check for a valid module instance.  */
    if (module_instance == TX_NULL)
    {
        /* Just return!  */
        return(TXM_MODULE_INVALID);
    }

    switch (kernel_request)
    {
    #ifndef TXM_BLOCK_ALLOCATE_CALL_NOT_USED
    case TXM_BLOCK_ALLOCATE_CALL:
    {
        return_value = _txm_module_manager_tx_block_allocate_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_POOL_CREATE_CALL_NOT_USED
    case TXM_BLOCK_POOL_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_block_pool_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_POOL_DELETE_CALL_NOT_USED
    case TXM_BLOCK_POOL_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_block_pool_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_POOL_INFO_GET_CALL_NOT_USED
    case TXM_BLOCK_POOL_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_block_pool_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_POOL_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_BLOCK_POOL_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_block_pool_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_block_pool_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_POOL_PRIORITIZE_CALL_NOT_USED
    case TXM_BLOCK_POOL_PRIORITIZE_CALL:
    {
        return_value = _txm_module_manager_tx_block_pool_prioritize_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_BLOCK_RELEASE_CALL_NOT_USED
    case TXM_BLOCK_RELEASE_CALL:
    {
        return_value = _txm_module_manager_tx_block_release_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_BYTE_ALLOCATE_CALL_NOT_USED
    case TXM_BYTE_ALLOCATE_CALL:
    {
        return_value = _txm_module_manager_tx_byte_allocate_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BYTE_POOL_CREATE_CALL_NOT_USED
    case TXM_BYTE_POOL_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_byte_pool_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BYTE_POOL_DELETE_CALL_NOT_USED
    case TXM_BYTE_POOL_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_byte_pool_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_BYTE_POOL_INFO_GET_CALL_NOT_USED
    case TXM_BYTE_POOL_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_byte_pool_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BYTE_POOL_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_BYTE_POOL_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_byte_pool_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_byte_pool_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_BYTE_POOL_PRIORITIZE_CALL_NOT_USED
    case TXM_BYTE_POOL_PRIORITIZE_CALL:
    {
        return_value = _txm_module_manager_tx_byte_pool_prioritize_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_BYTE_RELEASE_CALL_NOT_USED
    case TXM_BYTE_RELEASE_CALL:
    {
        return_value = _txm_module_manager_tx_byte_release_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_CREATE_CALL_NOT_USED
    case TXM_EVENT_FLAGS_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_create_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_DELETE_CALL_NOT_USED
    case TXM_EVENT_FLAGS_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_GET_CALL_NOT_USED
    case TXM_EVENT_FLAGS_GET_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_INFO_GET_CALL_NOT_USED
    case TXM_EVENT_FLAGS_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_EVENT_FLAGS_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_EVENT_FLAGS_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_SET_CALL_NOT_USED
    case TXM_EVENT_FLAGS_SET_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_set_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_EVENT_FLAGS_SET_NOTIFY_CALL_NOT_USED
    case TXM_EVENT_FLAGS_SET_NOTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_event_flags_set_notify_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_CREATE_CALL_NOT_USED
    case TXM_MUTEX_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_DELETE_CALL_NOT_USED
    case TXM_MUTEX_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_GET_CALL_NOT_USED
    case TXM_MUTEX_GET_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_get_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_INFO_GET_CALL_NOT_USED
    case TXM_MUTEX_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_MUTEX_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_PRIORITIZE_CALL_NOT_USED
    case TXM_MUTEX_PRIORITIZE_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_prioritize_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_MUTEX_PUT_CALL_NOT_USED
    case TXM_MUTEX_PUT_CALL:
    {
        return_value = _txm_module_manager_tx_mutex_put_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_CREATE_CALL_NOT_USED
    case TXM_QUEUE_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_queue_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_DELETE_CALL_NOT_USED
    case TXM_QUEUE_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_queue_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_FLUSH_CALL_NOT_USED
    case TXM_QUEUE_FLUSH_CALL:
    {
        return_value = _txm_module_manager_tx_queue_flush_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_FRONT_SEND_CALL_NOT_USED
    case TXM_QUEUE_FRONT_SEND_CALL:
    {
        return_value = _txm_module_manager_tx_queue_front_send_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_INFO_GET_CALL_NOT_USED
    case TXM_QUEUE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_queue_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_QUEUE_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_queue_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_QUEUE_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_queue_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_PRIORITIZE_CALL_NOT_USED
    case TXM_QUEUE_PRIORITIZE_CALL:
    {
        return_value = _txm_module_manager_tx_queue_prioritize_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_RECEIVE_CALL_NOT_USED
    case TXM_QUEUE_RECEIVE_CALL:
    {
        return_value = _txm_module_manager_tx_queue_receive_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif
    
    #ifndef TXM_QUEUE_SEND_CALL_NOT_USED
    case TXM_QUEUE_SEND_CALL:
    {
        return_value = _txm_module_manager_tx_queue_send_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_QUEUE_SEND_NOTIFY_CALL_NOT_USED
    case TXM_QUEUE_SEND_NOTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_queue_send_notify_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_CEILING_PUT_CALL_NOT_USED
    case TXM_SEMAPHORE_CEILING_PUT_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_ceiling_put_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_CREATE_CALL_NOT_USED
    case TXM_SEMAPHORE_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_DELETE_CALL_NOT_USED
    case TXM_SEMAPHORE_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_GET_CALL_NOT_USED
    case TXM_SEMAPHORE_GET_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_get_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_INFO_GET_CALL_NOT_USED
    case TXM_SEMAPHORE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_SEMAPHORE_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_PRIORITIZE_CALL_NOT_USED
    case TXM_SEMAPHORE_PRIORITIZE_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_prioritize_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_PUT_CALL_NOT_USED
    case TXM_SEMAPHORE_PUT_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_put_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_SEMAPHORE_PUT_NOTIFY_CALL_NOT_USED
    case TXM_SEMAPHORE_PUT_NOTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_semaphore_put_notify_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_THREAD_CREATE_CALL_NOT_USED
    case TXM_THREAD_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_thread_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_DELETE_CALL_NOT_USED
    case TXM_THREAD_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_thread_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_ENTRY_EXIT_NOTIFY_CALL_NOT_USED
    case TXM_THREAD_ENTRY_EXIT_NOTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_thread_entry_exit_notify_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_THREAD_IDENTIFY_CALL_NOT_USED
    case TXM_THREAD_IDENTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_thread_identify_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_INFO_GET_CALL_NOT_USED
    case TXM_THREAD_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_thread_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_INTERRUPT_CONTROL_CALL_NOT_USED
    case TXM_THREAD_INTERRUPT_CONTROL_CALL:
    {
        return_value = _txm_module_manager_tx_thread_interrupt_control_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_THREAD_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_thread_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_thread_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_PREEMPTION_CHANGE_CALL_NOT_USED
    case TXM_THREAD_PREEMPTION_CHANGE_CALL:
    {
        return_value = _txm_module_manager_tx_thread_preemption_change_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_PRIORITY_CHANGE_CALL_NOT_USED
    case TXM_THREAD_PRIORITY_CHANGE_CALL:
    {
        return_value = _txm_module_manager_tx_thread_priority_change_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_RELINQUISH_CALL_NOT_USED
    case TXM_THREAD_RELINQUISH_CALL:
    {
        return_value = _txm_module_manager_tx_thread_relinquish_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_RESET_CALL_NOT_USED
    case TXM_THREAD_RESET_CALL:
    {
        return_value = _txm_module_manager_tx_thread_reset_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_RESUME_CALL_NOT_USED
    case TXM_THREAD_RESUME_CALL:
    {
        return_value = _txm_module_manager_tx_thread_resume_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_SLEEP_CALL_NOT_USED
    case TXM_THREAD_SLEEP_CALL:
    {
        return_value = _txm_module_manager_tx_thread_sleep_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_STACK_ERROR_NOTIFY_CALL_NOT_USED
    case TXM_THREAD_STACK_ERROR_NOTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_thread_stack_error_notify_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_SUSPEND_CALL_NOT_USED
    case TXM_THREAD_SUSPEND_CALL:
    {
        return_value = _txm_module_manager_tx_thread_suspend_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_SYSTEM_SUSPEND_CALL_NOT_USED
    case TXM_THREAD_SYSTEM_SUSPEND_CALL:
    {
        return_value = _txm_module_manager_tx_thread_system_suspend_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_TERMINATE_CALL_NOT_USED
    case TXM_THREAD_TERMINATE_CALL:
    {
        return_value = _txm_module_manager_tx_thread_terminate_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_THREAD_TIME_SLICE_CHANGE_CALL_NOT_USED
    case TXM_THREAD_TIME_SLICE_CHANGE_CALL:
    {
        return_value = _txm_module_manager_tx_thread_time_slice_change_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_THREAD_WAIT_ABORT_CALL_NOT_USED
    case TXM_THREAD_WAIT_ABORT_CALL:
    {
        return_value = _txm_module_manager_tx_thread_wait_abort_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TIME_GET_CALL_NOT_USED
    case TXM_TIME_GET_CALL:
    {
        return_value = _txm_module_manager_tx_time_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_TIME_SET_CALL_NOT_USED
    case TXM_TIME_SET_CALL:
    {
        return_value = _txm_module_manager_tx_time_set_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TIMER_ACTIVATE_CALL_NOT_USED
    case TXM_TIMER_ACTIVATE_CALL:
    {
        return_value = _txm_module_manager_tx_timer_activate_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TIMER_CHANGE_CALL_NOT_USED
    case TXM_TIMER_CHANGE_CALL:
    {
        return_value = _txm_module_manager_tx_timer_change_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_TIMER_CREATE_CALL_NOT_USED
    case TXM_TIMER_CREATE_CALL:
    {
        return_value = _txm_module_manager_tx_timer_create_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_TIMER_DEACTIVATE_CALL_NOT_USED
    case TXM_TIMER_DEACTIVATE_CALL:
    {
        return_value = _txm_module_manager_tx_timer_deactivate_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TIMER_DELETE_CALL_NOT_USED
    case TXM_TIMER_DELETE_CALL:
    {
        return_value = _txm_module_manager_tx_timer_delete_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TIMER_INFO_GET_CALL_NOT_USED
    case TXM_TIMER_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_timer_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_TIMER_PERFORMANCE_INFO_GET_CALL_NOT_USED
    case TXM_TIMER_PERFORMANCE_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_timer_performance_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_TIMER_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
    case TXM_TIMER_PERFORMANCE_SYSTEM_INFO_GET_CALL:
    {
        return_value = _txm_module_manager_tx_timer_performance_system_info_get_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_TRACE_BUFFER_FULL_NOTIFY_CALL_NOT_USED
    case TXM_TRACE_BUFFER_FULL_NOTIFY_CALL:
    {
        return_value = _txm_module_manager_tx_trace_buffer_full_notify_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TRACE_DISABLE_CALL_NOT_USED
    case TXM_TRACE_DISABLE_CALL:
    {
        return_value = _txm_module_manager_tx_trace_disable_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_TRACE_ENABLE_CALL_NOT_USED
    case TXM_TRACE_ENABLE_CALL:
    {
        return_value = _txm_module_manager_tx_trace_enable_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_TRACE_EVENT_FILTER_CALL_NOT_USED
    case TXM_TRACE_EVENT_FILTER_CALL:
    {
        return_value = _txm_module_manager_tx_trace_event_filter_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TRACE_EVENT_UNFILTER_CALL_NOT_USED
    case TXM_TRACE_EVENT_UNFILTER_CALL:
    {
        return_value = _txm_module_manager_tx_trace_event_unfilter_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TRACE_INTERRUPT_CONTROL_CALL_NOT_USED
    case TXM_TRACE_INTERRUPT_CONTROL_CALL:
    {
        return_value = _txm_module_manager_tx_trace_interrupt_control_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TRACE_ISR_ENTER_INSERT_CALL_NOT_USED
    case TXM_TRACE_ISR_ENTER_INSERT_CALL:
    {
        return_value = _txm_module_manager_tx_trace_isr_enter_insert_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TRACE_ISR_EXIT_INSERT_CALL_NOT_USED
    case TXM_TRACE_ISR_EXIT_INSERT_CALL:
    {
        return_value = _txm_module_manager_tx_trace_isr_exit_insert_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_TRACE_USER_EVENT_INSERT_CALL_NOT_USED
    case TXM_TRACE_USER_EVENT_INSERT_CALL:
    {
        return_value = _txm_module_manager_tx_trace_user_event_insert_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    #ifndef TXM_MODULE_OBJECT_ALLOCATE_CALL_NOT_USED
    case TXM_MODULE_OBJECT_ALLOCATE_CALL:
    {
        return_value = _txm_module_manager_txm_module_object_allocate_dispatch(module_instance, param_0, param_1);
        break;
    }
    #endif

    #ifndef TXM_MODULE_OBJECT_DEALLOCATE_CALL_NOT_USED
    case TXM_MODULE_OBJECT_DEALLOCATE_CALL:
    {
        return_value = _txm_module_manager_txm_module_object_deallocate_dispatch(module_instance, param_0);
        break;
    }
    #endif

    #ifndef TXM_MODULE_OBJECT_POINTER_GET_CALL_NOT_USED
    case TXM_MODULE_OBJECT_POINTER_GET_CALL:
    {
        return_value = _txm_module_manager_txm_module_object_pointer_get_dispatch(module_instance, param_0, param_1, param_2);
        break;
    }
    #endif

    #ifndef TXM_MODULE_OBJECT_POINTER_GET_EXTENDED_CALL_NOT_USED
    case TXM_MODULE_OBJECT_POINTER_GET_EXTENDED_CALL:
    {
        return_value = _txm_module_manager_txm_module_object_pointer_get_extended_dispatch(module_instance, param_0, param_1, (ALIGN_TYPE *) param_2);
        break;
    }
    #endif

    default:
    {
#ifdef TXM_MODULE_PORT_DISPATCH
        /* Is this a port-specific request? */
        if ((kernel_request >= TXM_MODULE_PORT_EXTENSION_API_ID_START) && (kernel_request <= TXM_MODULE_PORT_EXTENSION_API_ID_END))
        {
            /* Yes, call the port-specific dispatcher. */
            return_value = (ALIGN_TYPE) _txm_module_manager_port_dispatch(module_instance, kernel_request, param_0, param_1, param_2);
        }
#endif

        #ifndef TXM_MODULE_APPLICATION_REQUEST_CALL_NOT_USED
        /* Determine if an application request is present.  */
        if (kernel_request >= TXM_APPLICATION_REQUEST_ID_BASE)
        {
            /* Yes, call the module manager function that the application defines in order to
               support application-specific requests.  */
            return_value =  (ALIGN_TYPE)  _txm_module_manager_application_request(kernel_request-TXM_APPLICATION_REQUEST_ID_BASE, param_0, param_1, param_2);
        }
        #endif

#ifdef TXM_MODULE_ENABLE_NETX
        /* Determine if there is a NetX request.  */
        else if ((kernel_request >= TXM_NETX_API_ID_START) && (kernel_request < TXM_NETX_API_ID_END))
        {
            /* Call the NetX module dispatch function.  */
            return_value =  _txm_module_manager_netx_dispatch(module_instance, kernel_request, param_0, param_1, param_2);
        }
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO
        /* Determine if there is a NetX Duo request.  */
        else if ((kernel_request >= TXM_NETXDUO_API_ID_START) && (kernel_request < TXM_NETXDUO_API_ID_END))
        {
            /* Call the NetX Duo module dispatch function.  */
            return_value =  _txm_module_manager_netxduo_dispatch(module_instance, kernel_request, param_0, param_1, param_2);
        }
#endif

#ifdef TXM_MODULE_ENABLE_FILEX
        /* Determine if there is a FileX request.  */
        else if ((kernel_request >= TXM_FILEX_API_ID_START) && (kernel_request < TXM_FILEX_API_ID_END))
        {
            /* Call the FileX module dispatch function.  */
            return_value =  _txm_module_manager_filex_dispatch(module_instance, kernel_request, param_0, param_1, param_2);
        }
#endif

#ifdef TXM_MODULE_ENABLE_GUIX
        /* Determine if there is a GUIX request.  */
        else if ((kernel_request >= TXM_GUIX_API_ID_START) && (kernel_request < TXM_GUIX_API_ID_END))
        {
            /* Call the GUIX module dispatch function.  */
            return_value =  _txm_module_manager_guix_dispatch(module_instance, kernel_request, param_0, param_1, param_2);
        }
#endif

#ifdef TXM_MODULE_ENABLE_USBX
        /* Determine if there is a USBX request.  */
        else if ((kernel_request >= TXM_USBX_API_ID_START) && (kernel_request < TXM_USBX_API_ID_END))
        {
            /* Call the USBX dispatch function.  */
            return_value =  _txm_module_manager_usbx_dispatch(module_instance, kernel_request, param_0, param_1, param_2);
        }
#endif

        /* Unhandled kernel request, return an error!  */
        break;
    }
    }

    return(return_value);
}
