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
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_queue.h"
#include "tx_event_flags.h"
#include "tx_semaphore.h"
#include "tx_mutex.h"
#include "tx_block_pool.h"
#include "tx_byte_pool.h"
#include "txm_module.h"
#include "txm_module_manager_util.h"

#ifdef TXM_MODULE_ENABLE_FILEX
extern UINT  _txm_module_manager_filex_stop(TXM_MODULE_INSTANCE *module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_NETX
extern UINT  _txm_module_manager_netx_stop(TXM_MODULE_INSTANCE *module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO
extern UINT  _txm_module_manager_netxduo_stop(TXM_MODULE_INSTANCE *module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_GUIX
extern UINT  _txm_module_manager_guix_stop(TXM_MODULE_INSTANCE *module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_USBX
extern UINT  _txm_module_manager_usbx_stop(TXM_MODULE_INSTANCE *module_instance);
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_stop                            PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function stops execution of the specified module.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_block_pool_delete                 Block pool delete             */
/*    _tx_byte_pool_delete                  Byte pool delete              */
/*    _tx_event_flags_delete                Event flags delete            */
/*    _tx_mutex_delete                      Mutex delete                  */
/*    _tx_mutex_get                         Get protection mutex          */
/*    _tx_mutex_put                         Release protection mutex      */
/*    _tx_queue_delete                      Queue delete                  */
/*    _tx_semaphore_delete                  Semaphore delete              */
/*    _tx_thread_delete                     Thread delete                 */
/*    _tx_thread_sleep                      Thread sleep                  */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    _tx_thread_terminate                  Thread terminate              */
/*    _tx_timer_delete                      Timer delete                  */
/*    _txm_module_manager_callback_deactivate                             */
/*                                          Deactivate callback           */
/*    _txm_module_manager_object_search     Search for object in module's */
/*                                            allocated object list       */
/*    _txm_module_manager_thread_create     Module thread create          */
/*    [_txm_module_manager_*_stop]          Optional external component   */
/*                                            stop functions              */
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
/*  03-02-2021      Scott Larson            Modified comments, fix        */
/*                                            object delete underflow,    */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_stop(TXM_MODULE_INSTANCE *module_instance)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD                       *thread_ptr, *next_thread_ptr;
TX_TIMER                        *timer_ptr, *next_timer_ptr;
TX_QUEUE                        *queue_ptr, *next_queue_ptr;
TX_EVENT_FLAGS_GROUP            *events_ptr, *next_events_ptr;
TX_SEMAPHORE                    *semaphore_ptr, *next_semaphore_ptr;
TX_MUTEX                        *mutex_ptr, *next_mutex_ptr;
TX_BLOCK_POOL                   *block_pool_ptr, *next_block_pool_ptr;
TX_BYTE_POOL                    *byte_pool_ptr, *next_byte_pool_ptr;
UCHAR                           created_by_module;
ULONG                           i;
TXM_MODULE_ALLOCATED_OBJECT     *object_ptr;


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

    /* Determine if this is a legal request.  */

    /* Is there a current thread?  */
    if (thread_ptr == TX_NULL)
    {

        /* Illegal caller of this service.  */
        return(TX_CALLER_ERROR);
    }

    /* Is the caller an ISR or Initialization?  */
    if (TX_THREAD_GET_SYSTEM_STATE() != 0)
    {

        /* Illegal caller of this service.  */
        return(TX_CALLER_ERROR);
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Check for invalid caller of this function.  First check for a calling thread.  */
    if (thread_ptr == &_tx_timer_thread)
    {

        /* Invalid caller of this function, return appropriate error code.  */
        return(TX_CALLER_ERROR);
    }
#endif

    /* Determine if the module manager has not been initialized yet.  */
    if (_txm_module_manager_ready != TX_TRUE)
    {

        /* Module manager has not been initialized.  */
        return(TX_NOT_AVAILABLE);
    }

    /* Determine if the module is valid.  */
    if (module_instance == TX_NULL)
    {

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Get module manager protection mutex.  */
    _tx_mutex_get(&_txm_module_manager_mutex, TX_WAIT_FOREVER);

    /* Determine if the module instance is valid.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Determine if the module instance is in the loaded state.  */
    if (module_instance -> txm_module_instance_state != TXM_MODULE_STARTED)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_START_ERROR);
    }

    /* Set the module state to indicate the module is stopping.  */
    module_instance -> txm_module_instance_state =  TXM_MODULE_STOPPING;

    /* This thread was previously used as the start thread. So first, make sure it is terminated and deleted before doing anything else.  */
    _tx_thread_terminate(&(module_instance -> txm_module_instance_start_stop_thread));
    _tx_thread_delete(&(module_instance -> txm_module_instance_start_stop_thread));

    /* Determine if there is a module stop function.  */
    if (module_instance -> txm_module_instance_stop_thread_entry)
    {

        /* Yes, there is a stop function.  Build a thread for executing the module stop function.  */

        /* Create the module stop thread.  */
        _txm_module_manager_thread_create(&(module_instance -> txm_module_instance_start_stop_thread),
                                          "Module Stop Thread",
                                          module_instance -> txm_module_instance_shell_entry_function,
                                          module_instance -> txm_module_instance_stop_thread_entry,
                                          module_instance -> txm_module_instance_application_module_id,
                                          module_instance -> txm_module_instance_start_stop_stack_start_address,
                                          module_instance -> txm_module_instance_start_stop_stack_size,
                                          (UINT) module_instance -> txm_module_instance_start_stop_priority,
                                          (UINT) module_instance -> txm_module_instance_start_stop_priority,
                                          TXM_MODULE_TIME_SLICE,
                                          TX_AUTO_START,
                                          sizeof(TX_THREAD),
                                          module_instance);

        /* Wait for the stop thread to complete.  */
        i =  0;
        while ((i < TXM_MODULE_TIMEOUT) && (module_instance -> txm_module_instance_start_stop_thread.tx_thread_state != TX_COMPLETED))
        {

            /* Sleep to let the module stop thread run.  */
            _tx_thread_sleep(1);

            /* Increment the counter.  */
            i++;
        }

        /* At this point, we need to terminate and delete the stop thread.  */
        _tx_thread_terminate(&(module_instance -> txm_module_instance_start_stop_thread));
        _tx_thread_delete(&(module_instance -> txm_module_instance_start_stop_thread));
    }

    /* Delete the module's callback thread and queue for the callback thread.  */
    _tx_thread_terminate(&(module_instance -> txm_module_instance_callback_request_thread));
    _tx_thread_delete(&(module_instance -> txm_module_instance_callback_request_thread));
    _tx_queue_delete(&(module_instance -> txm_module_instance_callback_request_queue));

    /* Disable interrupts.  */
    TX_DISABLE

    /* Temporarily disable preemption.  This will keep other threads from creating and deleting threads.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Determine if any external component stop functions have been registered and
       if so, call them to cleanup any module objects in that component.  */

#ifdef TXM_MODULE_ENABLE_FILEX

    /* Call the FileX stop function.  */
    _txm_module_manager_filex_stop(module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_NETX

    /* Call the NetX stop function.  */
    _txm_module_manager_netx_stop(module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO

    /* Call the NetX Duo stop function.  */
    _txm_module_manager_netxduo_stop(module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_GUIX

    /* Call the GUIX stop function.  */
    _txm_module_manager_guix_stop(module_instance);
#endif

#ifdef TXM_MODULE_ENABLE_USBX

    /* Call the USBX stop function.  */
    _txm_module_manager_usbx_stop(module_instance);
#endif

    /* Loop to delete any and all threads created by the module.  */
    i = _tx_thread_created_count;
    thread_ptr =  _tx_thread_created_ptr;
    while (i--)
    {

        /* Pickup the next thread pointer.  */
        next_thread_ptr =  thread_ptr -> tx_thread_created_next;

        /* Determine if the thread control block is inside the module.  */
        if ( (((CHAR *) thread_ptr) >= ((CHAR *) module_instance -> txm_module_instance_data_start)) &&
             (((CHAR *) thread_ptr) < ((CHAR *) module_instance -> txm_module_instance_data_end)))
        {

            /* Terminate and delete this thread, since it is part of this module.  */
            _tx_thread_terminate(thread_ptr);
            _tx_thread_delete(thread_ptr);
        }

        /* Is this thread part of the module?  */
        else if (thread_ptr -> tx_thread_module_instance_ptr == module_instance)
        {

            /* Terminate and delete this thread, since it is part of this module.  */
            _tx_thread_terminate(thread_ptr);
            _tx_thread_delete(thread_ptr);
        }

        /* Move to next thread.  */
        thread_ptr =  next_thread_ptr;
    }

    /* Loop to delete any and all timers created by the module.  */
    i = _tx_timer_created_count;
    timer_ptr =  _tx_timer_created_ptr;
    while (i--)
    {

        /* Pickup the next timer pointer.  */
        next_timer_ptr =  timer_ptr -> tx_timer_created_next;

        /* Check if this module created this timer.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) timer_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this timer, since it is part of this module.  */
            _tx_timer_delete(timer_ptr);
        }

        /* Move to next timer.  */
        timer_ptr =  next_timer_ptr;
    }

    /* Loop to delete any and all queues created by the module.  */
    i = _tx_queue_created_count;
    queue_ptr =  _tx_queue_created_ptr;
    while (i--)
    {

        /* Pickup the next queue pointer.  */
        next_queue_ptr =   queue_ptr -> tx_queue_created_next;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if the queue callback function is associated with this module.  */
        if ((queue_ptr -> tx_queue_module_instance == module_instance) &&
            (queue_ptr -> tx_queue_send_notify == _txm_module_manager_queue_notify_trampoline))
        {

            /* Clear the callback notification for this queue since it is no longer valid.  */
            queue_ptr -> tx_queue_send_notify =  TX_NULL;
        }
#endif

        /* Check if this module created this queue.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) queue_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this queue, since it is part of this module.  */
            _tx_queue_delete(queue_ptr);
        }

        /* Move to next queue.  */
        queue_ptr =  next_queue_ptr;
    }

    /* Loop to delete any and all event flag groups created by the module.  */
    i = _tx_event_flags_created_count;
    events_ptr =  _tx_event_flags_created_ptr;
    while (i--)
    {

        /* Pickup the next event flags group pointer.  */
        next_events_ptr =   events_ptr -> tx_event_flags_group_created_next;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if the event flags callback function is associated with this module.  */
        if ((events_ptr -> tx_event_flags_group_module_instance == module_instance) &&
            (events_ptr -> tx_event_flags_group_set_notify == _txm_module_manager_event_flags_notify_trampoline))
        {

            /* Clear the callback notification for this event flag group since it is no longer valid.  */
            events_ptr -> tx_event_flags_group_set_notify =  TX_NULL;
        }
#endif

        /* Check if this module created this event flags.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) events_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this event flags group, since it is part of this module.  */
            _tx_event_flags_delete(events_ptr);
        }

        /* Move to next event flags group.  */
        events_ptr =  next_events_ptr;
    }

    /* Loop to delete any and all semaphores created by the module.  */
    i = _tx_semaphore_created_count;
    semaphore_ptr =  _tx_semaphore_created_ptr;
    while (i--)
    {

        /* Pickup the next semaphore pointer.  */
        next_semaphore_ptr =   semaphore_ptr -> tx_semaphore_created_next;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if the semaphore callback function is associated with this module.  */
        if ((semaphore_ptr -> tx_semaphore_module_instance == module_instance) &&
            (semaphore_ptr -> tx_semaphore_put_notify == _txm_module_manager_semaphore_notify_trampoline))
        {

            /* Clear the callback notification for this semaphore since it is no longer valid.  */
            semaphore_ptr -> tx_semaphore_put_notify =  TX_NULL;
        }
#endif

        /* Check if this module created this semaphore.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) semaphore_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this semaphore, since it is part of this module.  */
            _tx_semaphore_delete(semaphore_ptr);
        }

        /* Move to next semaphore.  */
        semaphore_ptr =  next_semaphore_ptr;
    }

    /* Loop to delete any and all mutexes created by the module.  */
    i = _tx_mutex_created_count;
    mutex_ptr =  _tx_mutex_created_ptr;
    while (i--)
    {

        /* Pickup the next mutex pointer.  */
        next_mutex_ptr =   mutex_ptr -> tx_mutex_created_next;

        /* Check if this module created this mutex.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) mutex_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this mutex, since it is part of this module.  */
            _tx_mutex_delete(mutex_ptr);
        }

        /* Move to next mutex.  */
        mutex_ptr =  next_mutex_ptr;
    }

    /* Loop to delete any and all block pools created by the module.  */
    i = _tx_block_pool_created_count;
    block_pool_ptr =  _tx_block_pool_created_ptr;
    while (i--)
    {

        /* Pickup the next block pool pointer.  */
        next_block_pool_ptr =   block_pool_ptr -> tx_block_pool_created_next;

        /* Check if this module created this block pool.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) block_pool_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this block pool, since it is part of this module.  */
            _tx_block_pool_delete(block_pool_ptr);
        }

        /* Move to next block pool.  */
        block_pool_ptr =  next_block_pool_ptr;
    }

    /* Loop to delete any and all byte pools created by the module.  */
    i = _tx_byte_pool_created_count;
    byte_pool_ptr =  _tx_byte_pool_created_ptr;
    while (i--)
    {

        /* Pickup the next byte pool pointer.  */
        next_byte_pool_ptr =   byte_pool_ptr -> tx_byte_pool_created_next;

        /* Check if this module created this byte pool.  */
        created_by_module =  _txm_module_manager_created_object_check(module_instance, (VOID *) byte_pool_ptr);
        if (created_by_module == TX_TRUE)
        {

            /* Delete this byte pool, since it is part of this module.  */
            _tx_byte_pool_delete(byte_pool_ptr);
        }

        /* Move to next byte pool.  */
        byte_pool_ptr =  next_byte_pool_ptr;
    }

#ifdef TX_ENABLE_EVENT_TRACE
    /* Has trace been enabled?  */
    if (_tx_trace_buffer_current_ptr != TX_NULL)
    {

        /* Is the trace buffer located inside the module?  */
        if ((ULONG) _tx_trace_header_ptr -> tx_trace_header_buffer_start_pointer >= (ULONG) module_instance -> txm_module_instance_data_start &&
            (ULONG) _tx_trace_header_ptr -> tx_trace_header_buffer_start_pointer < (ULONG) module_instance -> txm_module_instance_data_end)
        {
            _tx_trace_disable();
        }
    }
#endif

    /* Delete the allocated objects for this module.  */
    while (module_instance -> txm_module_instance_object_list_count != 0)
    {
        /* Pickup the current object pointer.  */
        object_ptr =  module_instance -> txm_module_instance_object_list_head;

        /* Move the head pointer forward.  */
        module_instance -> txm_module_instance_object_list_head =  object_ptr -> txm_module_allocated_object_next;

        /* Release the object.  */
        _tx_byte_release((VOID *) object_ptr);

        /* Decrement count.  */
        module_instance -> txm_module_instance_object_list_count--;
    }

    /* Set the allocated list head pointer to NULL.  */
    module_instance -> txm_module_instance_object_list_head =  TX_NULL;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Enable preemption again.  */
    _tx_thread_preempt_disable--;

    /* Set the module state to indicate the module is stopped.  */
    module_instance -> txm_module_instance_state =  TXM_MODULE_STOPPED;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return success.  */
    return(TX_SUCCESS);
}

