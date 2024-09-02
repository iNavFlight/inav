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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_object_pointer_get_extended     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the object pointer of a particular type     */
/*    with a particular name. If the object is not found, an error is     */
/*    returned. Otherwise, if the object is found, the address of that    */
/*    object is placed in object_ptr.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_type                       Type of object, as follows:       */
/*                                                                        */
/*                                          TXM_BLOCK_POOL_OBJECT         */
/*                                          TXM_BYTE_POOL_OBJECT          */
/*                                          TXM_EVENT_FLAGS_OBJECT        */
/*                                          TXM_MUTEX_OBJECT              */
/*                                          TXM_QUEUE_OBJECT              */
/*                                          TXM_SEMAPHORE_OBJECT          */
/*                                          TXM_THREAD_OBJECT             */
/*                                          TXM_TIMER_OBJECT              */
/*    search_name                       Name to search for                */
/*    search_name_length                Length of the name excluding      */
/*                                        null-terminator                 */
/*    object_ptr                        Pointer to the object             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion             */
/*    TX_PTR_ERROR                      Invalid name or object ptr        */
/*    TX_OPTION_ERROR                   Invalid option type               */
/*    TX_NO_INSTANCE                    Object not found                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_module_manager_object_name_compare                             */
/*                                      String compare routine            */
/*    [_txm_module_manager_*_object_pointer_get]                          */
/*                                      Optional external component       */
/*                                        object pointer get              */
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
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_object_pointer_get_extended(UINT object_type, CHAR *search_name, UINT search_name_length, VOID **object_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD               *thread_ptr;
TX_TIMER                *timer_ptr;
TX_QUEUE                *queue_ptr;
TX_EVENT_FLAGS_GROUP    *events_ptr;
TX_SEMAPHORE            *semaphore_ptr;
TX_MUTEX                *mutex_ptr;
TX_BLOCK_POOL           *block_pool_ptr;
TX_BYTE_POOL            *byte_pool_ptr;
ULONG                   i;
UINT                    status;
TXM_MODULE_INSTANCE     *module_instance;


    /* Determine if the name or object pointer are NULL.  */
    if ((search_name == TX_NULL) || (object_ptr == TX_NULL))
    {

        /* Return error!  */
        return(TX_PTR_ERROR);
    }

    /* Default status to not found.  */
    status =  TX_NO_INSTANCE;

    /* Set the return value to NULL.  */
    *object_ptr =  TX_NULL;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Temporarily disable preemption.  This will keep other threads from creating and deleting threads.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Process relative to the object type.  */
    switch(object_type)
    {

    /* Determine if a thread object is requested.  */
    case TXM_THREAD_OBJECT:
    {

        /* Loop to find the first matching thread.  */
        i = 0;
        thread_ptr =  _tx_thread_created_ptr;
        while (i < _tx_thread_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, thread_ptr -> tx_thread_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) thread_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next thread.  */
            thread_ptr =  thread_ptr -> tx_thread_created_next;
        }
        break;
    }

    /* Determine if a timer object is requested.  */
    case TXM_TIMER_OBJECT:
    {

        /* Loop to find the first matching timer.  */
        i = 0;
        timer_ptr =  _tx_timer_created_ptr;
        while (i < _tx_timer_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, timer_ptr -> tx_timer_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) timer_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next timer.  */
            timer_ptr =  timer_ptr -> tx_timer_created_next;
        }
        break;
    }

    /* Determine if a queue object is requested.  */
    case TXM_QUEUE_OBJECT:
    {

        /* Loop to find the first matching queue.  */
        i = 0;
        queue_ptr =  _tx_queue_created_ptr;
        while (i < _tx_queue_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, queue_ptr -> tx_queue_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) queue_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next queue.  */
            queue_ptr =  queue_ptr -> tx_queue_created_next;
        }
        break;
    }

    /* Determine if a event flags object is requested.  */
    case TXM_EVENT_FLAGS_OBJECT:
    {

        /* Loop to find the first matching event flags group.  */
        i = 0;
        events_ptr =  _tx_event_flags_created_ptr;
        while (i < _tx_event_flags_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, events_ptr -> tx_event_flags_group_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) events_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next event flags group.  */
            events_ptr =  events_ptr -> tx_event_flags_group_created_next;
        }
        break;
    }

    /* Determine if a semaphore object is requested.  */
    case TXM_SEMAPHORE_OBJECT:
    {

        /* Loop to find the first matching semaphore.  */
        i = 0;
        semaphore_ptr =  _tx_semaphore_created_ptr;
        while (i < _tx_semaphore_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, semaphore_ptr -> tx_semaphore_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) semaphore_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next semaphore.  */
            semaphore_ptr =  semaphore_ptr -> tx_semaphore_created_next;
        }
        break;
    }

    /* Determine if a mutex object is requested.  */
    case TXM_MUTEX_OBJECT:
    {

        /* Loop to find the first matching mutex.  */
        i = 0;
        mutex_ptr =  _tx_mutex_created_ptr;
        while (i < _tx_mutex_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, mutex_ptr -> tx_mutex_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) mutex_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next mutex.  */
            mutex_ptr =  mutex_ptr -> tx_mutex_created_next;
        }
        break;
    }

    /* Determine if a block pool object is requested.  */
    case TXM_BLOCK_POOL_OBJECT:
    {

        /* Get the module instance.  */
        module_instance =  _tx_thread_current_ptr -> tx_thread_module_instance_ptr;

        /* Is a module making this request?  */
        if (module_instance != TX_NULL)
        {

            /* Is memory protection enabled?  */
            if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
            {

                /* Modules with memory protection can only access block pools they created.  */
                status =  TXM_MODULE_INVALID;
                break;
            }
        }

        /* Loop to find the first matching block pool.  */
        i = 0;
        block_pool_ptr =  _tx_block_pool_created_ptr;
        while (i < _tx_block_pool_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, block_pool_ptr -> tx_block_pool_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) block_pool_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next block pool.  */
            block_pool_ptr =  block_pool_ptr -> tx_block_pool_created_next;
        }
        break;
    }

    /* Determine if a byte pool object is requested.  */
    case TXM_BYTE_POOL_OBJECT:
    {

        /* Get the module instance.  */
        module_instance =  _tx_thread_current_ptr -> tx_thread_module_instance_ptr;

        /* Is a module making this request?  */
        if (module_instance != TX_NULL)
        {

            /* Is memory protection enabled?  */
            if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
            {

                /* Modules with memory protection can only access block pools they created.  */
                status =  TXM_MODULE_INVALID;
                break;
            }
        }

        /* Loop to find the first matching byte pool.  */
        i = 0;
        byte_pool_ptr =  _tx_byte_pool_created_ptr;
        while (i < _tx_byte_pool_created_count)
        {

            /* Do we have a match?  */
            if (_txm_module_manager_object_name_compare(search_name, search_name_length, byte_pool_ptr -> tx_byte_pool_name))
            {

                /* Yes, we found it - return the necessary info!  */
                *object_ptr =  (VOID *) byte_pool_ptr;

                /* Set the the status to success!  */
                status =  TX_SUCCESS;
                break;
            }

            /* Increment the counter.  */
            i++;

            /* Move to next byte pool.  */
            byte_pool_ptr =  byte_pool_ptr -> tx_byte_pool_created_next;
        }
        break;
    }

    default:

        /* Invalid object ID.  */
        status =  TX_OPTION_ERROR;

        /* External Object pointer get.  */

#ifdef TXM_MODULE_ENABLE_NETX

        /* Determine if there is a NetX object get request.  */
        if ((object_type >= TXM_NETX_OBJECTS_START) && (object_type < TXM_NETX_OBJECTS_END))
        {

            /* Call the NetX module object get function.  */
            status =  _txm_module_manager_netx_object_pointer_get(object_type, search_name, search_name_length, object_ptr);
        }
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO

        /* Determine if there is a NetX Duo object get request.  */
        if ((object_type >= TXM_NETXDUO_OBJECTS_START) && (object_type < TXM_NETXDUO_OBJECTS_END))
        {

            /* Call the NetX Duo module object get function.  */
            status =  _txm_module_manager_netxduo_object_pointer_get(object_type, search_name, search_name_length, object_ptr);
        }
#endif

#ifdef TXM_MODULE_ENABLE_FILEX

        /* Determine if there is a FileX object get request.  */
        if ((object_type >= TXM_FILEX_OBJECTS_START) && (object_type < TXM_FILEX_OBJECTS_END))
        {

            /* Call the FileX module object get function.  */
            status =  _txm_module_manager_filex_object_pointer_get(object_type, search_name, search_name_length, object_ptr);
        }
#endif


#ifdef TXM_MODULE_ENABLE_GUIX

        /* Determine if there is a GUIX object get request.  */
        if ((object_type >= TXM_GUIX_OBJECTS_START) && (object_type < TXM_GUIX_OBJECTS_END))
        {

            /* Call the GUIX module object get function.  */
            status =  _txm_module_manager_guix_object_pointer_get(object_type, search_name, search_name_length, object_ptr);
        }
#endif

#ifdef TXM_MODULE_ENABLE_USBX

        /* Determine if there is a USBX object get request.  */
        if ((object_type >= TXM_USBX_OBJECTS_START) && (object_type < TXM_USBX_OBJECTS_END))
        {

            /* Call the USBX object get function.  */
            status =  _txm_module_manager_usbx_object_pointer_get(object_type, search_name, search_name_length, object_ptr);
        }
#endif

        break;
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Enable preemption again.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return success.  */
    return(status);
}
