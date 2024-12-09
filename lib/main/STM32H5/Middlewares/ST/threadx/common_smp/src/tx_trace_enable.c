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
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#ifdef TX_ENABLE_EVENT_TRACE
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_event_flags.h"
#include "tx_queue.h"
#include "tx_semaphore.h"
#include "tx_mutex.h"
#include "tx_block_pool.h"
#include "tx_byte_pool.h"
#endif
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_trace_enable                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the ThreadX trace buffer and the          */
/*    associated control variables, enabling it for operation.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    trace_buffer_start                    Start of trace buffer         */
/*    trace_buffer_size                     Size (bytes) of trace buffer  */
/*    registry_entries                      Number of object registry     */
/*                                            entries.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    _tx_trace_object_register             Register existing objects     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_trace_enable(VOID *trace_buffer_start, ULONG trace_buffer_size, ULONG registry_entries)
{

#ifdef TX_ENABLE_EVENT_TRACE

TX_INTERRUPT_SAVE_AREA

TX_THREAD                       *thread_ptr;
TX_TIMER                        *timer_ptr;
TX_EVENT_FLAGS_GROUP            *event_flags_ptr;
TX_QUEUE                        *queue_ptr;
TX_SEMAPHORE                    *semaphore_ptr;
TX_MUTEX                        *mutex_ptr;
TX_BLOCK_POOL                   *block_pool_ptr;
TX_BYTE_POOL                    *byte_pool_ptr;
UCHAR                           *work_ptr;
UCHAR                           *event_start_ptr;
TX_TRACE_OBJECT_ENTRY           *entry_ptr;
TX_TRACE_BUFFER_ENTRY           *event_ptr;
ULONG                           i;
UINT                            status;


    /* First, see if there is enough room for the control header, the registry entries, and at least one event in
       memory supplied to this call.  */
    if (trace_buffer_size < ((sizeof(TX_TRACE_HEADER)) + ((sizeof(TX_TRACE_OBJECT_ENTRY)) * registry_entries) + (sizeof(TX_TRACE_BUFFER_ENTRY))))
    {

        /* No, the memory isn't big enough to hold one trace buffer entry.  Return an error.  */
        status =  TX_SIZE_ERROR;
    }

    /* Determine if trace is already enabled.  */
    else if (_tx_trace_buffer_current_ptr != TX_NULL)
    {

        /* Yes, trace is already enabled.  */
        status =  TX_NOT_DONE;
    }
    else
    {

        /* Set the enable bits for all events enabled.  */
        _tx_trace_event_enable_bits =  0xFFFFFFFFUL;

        /* Setup working pointer to the supplied memory.  */
        work_ptr =  TX_VOID_TO_UCHAR_POINTER_CONVERT(trace_buffer_start);

        /* Setup pointer to the trace control area.  */
        _tx_trace_header_ptr =  TX_UCHAR_TO_HEADER_POINTER_CONVERT(work_ptr);

        /* Move the working pointer past the control area.  */
        work_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(TX_TRACE_HEADER)));

        /* Save the start of the trace object registry.  */
        _tx_trace_registry_start_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

        /* Setup the end of the trace object registry.  */
        work_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(TX_TRACE_OBJECT_ENTRY))*registry_entries);
        _tx_trace_registry_end_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

        /* Loop to make all trace object registry entries empty and valid.  */
        for (i = ((ULONG) 0); i < registry_entries; i++)
        {

            /* Setup the work pointer.  */
            work_ptr =  TX_OBJECT_TO_UCHAR_POINTER_CONVERT(_tx_trace_registry_start_ptr);
            work_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(TX_TRACE_OBJECT_ENTRY))*i);

            /* Convert to a registry entry pointer.  */
            entry_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

            /* Initialize object registry entry.  */
            entry_ptr -> tx_trace_object_entry_available =         (UCHAR) TX_TRUE;
            entry_ptr -> tx_trace_object_entry_type =              (UCHAR) TX_TRACE_OBJECT_TYPE_NOT_VALID;
            entry_ptr -> tx_trace_object_entry_reserved1 =         (UCHAR) 0;
            entry_ptr -> tx_trace_object_entry_reserved2 =         (UCHAR) 0;
            entry_ptr -> tx_trace_object_entry_thread_pointer =    (ULONG) 0;
        }

        /* Setup the total number of registry entries.  */
        _tx_trace_total_registry_entries =  registry_entries;

        /* Setup the object registry available count to the total number of registry entries.  */
        _tx_trace_available_registry_entries =  registry_entries;

        /* Setup the search starting index to the first entry.  */
        _tx_trace_registry_search_start =  ((ULONG) 0);

        /* Setup the work pointer to after the trace object registry.  */
        work_ptr =  TX_OBJECT_TO_UCHAR_POINTER_CONVERT(_tx_trace_registry_end_ptr);

        /* Adjust the remaining trace buffer size.  */
        trace_buffer_size =  trace_buffer_size - ((sizeof(TX_TRACE_OBJECT_ENTRY)) * registry_entries) - (sizeof(TX_TRACE_HEADER));

        /* Setup pointer to the start of the actual event trace log.  */
        _tx_trace_buffer_start_ptr =      TX_UCHAR_TO_ENTRY_POINTER_CONVERT(work_ptr);

        /* Save the event trace log start address.  */
        event_start_ptr =  work_ptr;

        /* Calculate the end of the trace buffer.  */
        work_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, ((trace_buffer_size/(sizeof(TX_TRACE_BUFFER_ENTRY)))*(sizeof(TX_TRACE_BUFFER_ENTRY))));
        _tx_trace_buffer_end_ptr =        TX_UCHAR_TO_ENTRY_POINTER_CONVERT(work_ptr);

        /* Loop to mark all entries in the trace buffer as invalid.  */
        for (i = ((ULONG) 0); i < (trace_buffer_size/(sizeof(TX_TRACE_BUFFER_ENTRY))); i++)
        {

            /* Setup the work pointer.  */
            work_ptr =  TX_UCHAR_POINTER_ADD(event_start_ptr, (sizeof(TX_TRACE_BUFFER_ENTRY))*i);

            /* Convert to a trace event pointer.  */
            event_ptr =  TX_UCHAR_TO_ENTRY_POINTER_CONVERT(work_ptr);

            /* Mark this trace event as invalid.  */
            event_ptr -> tx_trace_buffer_entry_thread_pointer =  ((ULONG) 0);
        }

        /* Now, fill in the event trace control header.  */
        _tx_trace_header_ptr -> tx_trace_header_id =                             TX_TRACE_VALID;
        _tx_trace_header_ptr -> tx_trace_header_timer_valid_mask =               TX_TRACE_TIME_MASK;
        _tx_trace_header_ptr -> tx_trace_header_trace_base_address =             TX_POINTER_TO_ULONG_CONVERT(trace_buffer_start);
        _tx_trace_header_ptr -> tx_trace_header_registry_start_pointer =         TX_POINTER_TO_ULONG_CONVERT(_tx_trace_registry_start_ptr);
        _tx_trace_header_ptr -> tx_trace_header_reserved1 =                      ((USHORT) 0);
        _tx_trace_header_ptr -> tx_trace_header_object_name_size =               ((USHORT) TX_TRACE_OBJECT_REGISTRY_NAME);
        _tx_trace_header_ptr -> tx_trace_header_registry_end_pointer =           TX_POINTER_TO_ULONG_CONVERT(_tx_trace_registry_end_ptr);
        _tx_trace_header_ptr -> tx_trace_header_buffer_start_pointer =           TX_POINTER_TO_ULONG_CONVERT(_tx_trace_buffer_start_ptr);
        _tx_trace_header_ptr -> tx_trace_header_buffer_end_pointer =             TX_POINTER_TO_ULONG_CONVERT(_tx_trace_buffer_end_ptr);
        _tx_trace_header_ptr -> tx_trace_header_buffer_current_pointer =         TX_POINTER_TO_ULONG_CONVERT(_tx_trace_buffer_start_ptr);
        _tx_trace_header_ptr -> tx_trace_header_reserved2 =                      0xAAAAAAAAUL;
        _tx_trace_header_ptr -> tx_trace_header_reserved3 =                      0xBBBBBBBBUL;
        _tx_trace_header_ptr -> tx_trace_header_reserved4 =                      0xCCCCCCCCUL;

        /* Now, loop through all existing ThreadX objects and register them in the newly setup trace buffer.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* First, disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Pickup the first thread and the number of created threads.  */
        thread_ptr =  _tx_thread_created_ptr;
        i =           _tx_thread_created_count;

        /* Loop to register all threads.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this thread.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_THREAD, thread_ptr, thread_ptr -> tx_thread_name,
                                        TX_POINTER_TO_ULONG_CONVERT(thread_ptr -> tx_thread_stack_start), (ULONG) thread_ptr -> tx_thread_stack_size);

            /* Move to the next thread.  */
            thread_ptr =  thread_ptr -> tx_thread_created_next;
        }

        /* Pickup the first timer and the number of created timers.  */
        timer_ptr =  _tx_timer_created_ptr;
        i =          _tx_timer_created_count;

        /* Loop to register all timers.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this timer.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_TIMER, timer_ptr, timer_ptr -> tx_timer_name,
                                                       ((ULONG) 0), timer_ptr -> tx_timer_internal.tx_timer_internal_re_initialize_ticks);

            /* Move to the next timer.  */
            timer_ptr =  timer_ptr -> tx_timer_created_next;
        }


        /* Pickup the first event flag group and the number of created groups.  */
        event_flags_ptr =  _tx_event_flags_created_ptr;
        i =                _tx_event_flags_created_count;

        /* Loop to register all event flags groups.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this event flags group.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_EVENT_FLAGS, event_flags_ptr, event_flags_ptr -> tx_event_flags_group_name, ((ULONG) 0), ((ULONG) 0));

            /* Move to the next event flags group.  */
            event_flags_ptr =  event_flags_ptr -> tx_event_flags_group_created_next;
        }

        /* Pickup the first queue and the number of created queues.  */
        queue_ptr =  _tx_queue_created_ptr;
        i =          _tx_queue_created_count;

        /* Loop to register all queues.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this queue.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_QUEUE, queue_ptr, queue_ptr -> tx_queue_name,
                                                                    (queue_ptr -> tx_queue_capacity * (sizeof(ULONG))), ((ULONG) 0));

            /* Move to the next queue.  */
            queue_ptr =  queue_ptr -> tx_queue_created_next;
        }

        /* Pickup the first semaphore and the number of created semaphores.  */
        semaphore_ptr =  _tx_semaphore_created_ptr;
        i =              _tx_semaphore_created_count;

        /* Loop to register all semaphores.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this semaphore.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_SEMAPHORE, semaphore_ptr, semaphore_ptr -> tx_semaphore_name, ((ULONG) 0), ((ULONG) 0));

            /* Move to the next semaphore.  */
            semaphore_ptr =  semaphore_ptr -> tx_semaphore_created_next;
        }

        /* Pickup the first mutex and the number of created mutexes.  */
        mutex_ptr =  _tx_mutex_created_ptr;
        i =          _tx_mutex_created_count;

        /* Loop to register all mutexes.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this mutex.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_MUTEX, mutex_ptr, mutex_ptr -> tx_mutex_name,
                                                                        (ULONG) mutex_ptr -> tx_mutex_inherit, ((ULONG) 0));

            /* Move to the next mutex.  */
            mutex_ptr =  mutex_ptr -> tx_mutex_created_next;
        }

        /* Pickup the first block pool and the number of created block pools.  */
        block_pool_ptr =  _tx_block_pool_created_ptr;
        i =               _tx_block_pool_created_count;

        /* Loop to register all block pools.  */
        while (i != ((ULONG) 0))
        {

             /* Decrement the counter.  */
            i--;

            /* Register this block pool.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_BLOCK_POOL, block_pool_ptr, block_pool_ptr -> tx_block_pool_name,
                                                                            block_pool_ptr -> tx_block_pool_size, ((ULONG) 0));

            /* Move to the next block pool.  */
            block_pool_ptr =  block_pool_ptr -> tx_block_pool_created_next;
        }

        /* Pickup the first byte pool and the number of created byte pools.  */
        byte_pool_ptr =  _tx_byte_pool_created_ptr;
        i =              _tx_byte_pool_created_count;

        /* Loop to register all byte pools.  */
        while (i != ((ULONG) 0))
        {

            /* Decrement the counter.  */
            i--;

            /* Register this byte pool.  */
            _tx_trace_object_register(TX_TRACE_OBJECT_TYPE_BYTE_POOL, byte_pool_ptr, byte_pool_ptr -> tx_byte_pool_name,
                                                                            byte_pool_ptr -> tx_byte_pool_size, ((ULONG) 0));

            /* Move to the next byte pool.  */
            byte_pool_ptr =  byte_pool_ptr -> tx_byte_pool_created_next;
        }

        /* Disable interrupts.  */
        TX_DISABLE

        /* Release the preeemption.  */
        _tx_thread_preempt_disable--;

        /* Finally, setup the current buffer pointer, which effectively enables the trace!  */
        _tx_trace_buffer_current_ptr =    (TX_TRACE_BUFFER_ENTRY *) _tx_trace_buffer_start_ptr;

        /* Insert two RUNNING events so the buffer is not empty.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_RUNNING, 0, 0, 0, 0, TX_TRACE_INTERNAL_EVENTS)
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_RUNNING, 0, 0, 0, 0, TX_TRACE_INTERNAL_EVENTS)

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();

        /* Return successful completion.  */
        status =  TX_SUCCESS;
    }

    /* Return completion status.  */
    return(status);
#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (trace_buffer_start != TX_NULL)
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (trace_buffer_size == ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (registry_entries == ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
#endif
}



