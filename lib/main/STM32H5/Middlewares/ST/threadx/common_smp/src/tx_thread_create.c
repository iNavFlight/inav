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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_initialize.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_create                                  PORTABLE SMP     */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a thread and places it on the list of created */
/*    threads.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*    name_ptr                              Pointer to thread name string */
/*    entry_function                        Entry function of the thread  */
/*    entry_input                           32-bit input value to thread  */
/*    stack_start                           Pointer to start of stack     */
/*    stack_size                            Stack size in bytes           */
/*    priority                              Priority of thread (0-31)     */
/*    preempt_threshold                     Preemption threshold          */
/*    time_slice                            Thread time-slice value       */
/*    auto_start                            Automatic start selection     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                         Thread create return status   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_rebalance_execute_list Rebalance execution list      */
/*    _tx_thread_stack_build                Build initial thread stack    */
/*    _tx_thread_system_resume              Resume automatic start thread */
/*    _tx_thread_system_ni_resume           Noninterruptable resume thread*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _tx_timer_initialize                  Create system timer thread    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      William E. Lamie        Initial Version 6.1           */
/*  12-31-2020      Andres Mlinar           Modified comment(s),          */
/*                                            resulting in version 6.1.3  */
/*  08-02-2021      Scott Larson            Removed unneeded cast,        */
/*                                            resulting in version 6.1.8  */
/*  10-31-2022      Scott Larson            Removed ifdef block to always */
/*                                            restore interrupts at end   */
/*                                            of if block,                */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr,
                            VOID (*entry_function)(ULONG id), ULONG entry_input,
                            VOID *stack_start, ULONG stack_size, UINT priority, UINT preempt_threshold,
                            ULONG time_slice, UINT auto_start)
{

TX_INTERRUPT_SAVE_AREA

UINT                    core_index;
TX_THREAD               *next_thread;
TX_THREAD               *previous_thread;
UCHAR                   *temp_ptr;
#ifdef TX_ENABLE_STACK_CHECKING
ALIGN_TYPE              new_stack_start;
ALIGN_TYPE              updated_stack_start;
#endif


#ifndef TX_DISABLE_STACK_FILLING

    /* Set the thread stack to a pattern prior to creating the initial
       stack frame.  This pattern is used by the stack checking routines
       to see how much has been used.  */
    TX_MEMSET(stack_start, ((UCHAR) TX_STACK_FILL), stack_size);
#endif

#ifdef TX_ENABLE_STACK_CHECKING

    /* Ensure that there are two ULONG of 0xEF patterns at the top and
       bottom of the thread's stack. This will be used to check for stack
       overflow conditions during run-time.  */
    stack_size =  ((stack_size/(sizeof(ULONG))) * (sizeof(ULONG))) - (sizeof(ULONG));

    /* Ensure the starting stack address is evenly aligned.  */
    new_stack_start =  TX_POINTER_TO_ALIGN_TYPE_CONVERT(stack_start);
    updated_stack_start =  (((new_stack_start) + ((sizeof(ULONG)) - ((ULONG) 1)) ) & (~((sizeof(ULONG)) - ((ULONG) 1))));

    /* Determine if the starting stack address is different.  */
    if (new_stack_start != updated_stack_start)
    {

        /* Yes, subtract another ULONG from the size to avoid going past the stack area.  */
        stack_size =  stack_size - (sizeof(ULONG));
    }

    /* Update the starting stack pointer.  */
    stack_start =  TX_ALIGN_TYPE_TO_POINTER_CONVERT(updated_stack_start);
#endif

    /* Prepare the thread control block prior to placing it on the created
       list.  */

    /* Initialize thread control block to all zeros.  */
    TX_MEMSET(thread_ptr, 0, sizeof(TX_THREAD));

    /* Place the supplied parameters into the thread's control block.  */
    thread_ptr -> tx_thread_name =                name_ptr;
    thread_ptr -> tx_thread_entry =               entry_function;
    thread_ptr -> tx_thread_entry_parameter =     entry_input;
    thread_ptr -> tx_thread_stack_start =         stack_start;
    thread_ptr -> tx_thread_stack_size =          stack_size;
    thread_ptr -> tx_thread_priority =            priority;
    thread_ptr -> tx_thread_user_priority =       priority;
    thread_ptr -> tx_thread_time_slice =          time_slice;
    thread_ptr -> tx_thread_new_time_slice =      time_slice;
    thread_ptr -> tx_thread_inherit_priority =    ((UINT) TX_MAX_PRIORITIES);
    thread_ptr -> tx_thread_smp_core_executing =  ((UINT) TX_THREAD_SMP_MAX_CORES);
    thread_ptr -> tx_thread_smp_cores_excluded =  ((ULONG) 0);
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    thread_ptr -> tx_thread_smp_cores_allowed =   ((ULONG) TX_THREAD_SMP_CORE_MASK);
#else
    thread_ptr -> tx_thread_smp_cores_allowed =   (((ULONG) 1) << _tx_thread_smp_max_cores) - 1;
#endif

#ifdef TX_THREAD_SMP_ONLY_CORE_0_DEFAULT

    /* Default thread creation such that core0 is the only allowed core for execution, i.e., bit 1 is set to exclude core1.  */
    thread_ptr -> tx_thread_smp_cores_excluded =  (TX_THREAD_SMP_CORE_MASK & 0xFFFFFFFE);
    thread_ptr -> tx_thread_smp_cores_allowed  =  1;

    /* Default the timers to run on core 0 as well.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_smp_cores_excluded =  (TX_THREAD_SMP_CORE_MASK & 0xFFFFFFFE);

    /* Default the mapped to 0 too.  */
    thread_ptr -> tx_thread_smp_core_mapped =  0;
#endif

    /* Calculate the end of the thread's stack area.  */
    temp_ptr =  TX_VOID_TO_UCHAR_POINTER_CONVERT(stack_start);
    temp_ptr =  (TX_UCHAR_POINTER_ADD(temp_ptr, (stack_size - ((ULONG) 1))));
    thread_ptr -> tx_thread_stack_end =         TX_UCHAR_TO_VOID_POINTER_CONVERT(temp_ptr);


#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

    /* Preemption-threshold is enabled, setup accordingly.  */
    thread_ptr -> tx_thread_preempt_threshold =       preempt_threshold;
    thread_ptr -> tx_thread_user_preempt_threshold =  preempt_threshold;
#else

    /* Preemption-threshold is disabled, determine if preemption-threshold was required.  */
    if (priority != preempt_threshold)
    {

        /* Preemption-threshold specified. Since specific preemption-threshold is not supported,
           disable all preemption.  */
        thread_ptr -> tx_thread_preempt_threshold =       ((UINT) 0);
        thread_ptr -> tx_thread_user_preempt_threshold =  ((UINT) 0);
    }
    else
    {

        /* Preemption-threshold is not specified, just setup with the priority.  */
        thread_ptr -> tx_thread_preempt_threshold =       priority;
        thread_ptr -> tx_thread_user_preempt_threshold =  priority;
    }
#endif

    /* Now fill in the values that are required for thread initialization.  */
    thread_ptr -> tx_thread_state =  TX_SUSPENDED;

    /* Setup the necessary fields in the thread timer block.  */
    TX_THREAD_CREATE_TIMEOUT_SETUP(thread_ptr)

    /* Perform any additional thread setup activities for tool or user purpose.  */
    TX_THREAD_CREATE_INTERNAL_EXTENSION(thread_ptr)

    /* Call the target specific stack frame building routine to build the
       thread's initial stack and to setup the actual stack pointer in the
       control block.  */
    _tx_thread_stack_build(thread_ptr, _tx_thread_shell_entry);

#ifdef TX_ENABLE_STACK_CHECKING

    /* Setup the highest usage stack pointer.  */
    thread_ptr -> tx_thread_stack_highest_ptr =  thread_ptr -> tx_thread_stack_ptr;
#endif

    /* Prepare to make this thread a member of the created thread list.  */
    TX_DISABLE

    /* Load the thread ID field in the thread control block.  */
    thread_ptr -> tx_thread_id =  TX_THREAD_ID;

    /* Place the thread on the list of created threads.  First,
       check for an empty list.  */
    if (_tx_thread_created_count == TX_EMPTY)
    {

        /* The created thread list is empty.  Add thread to empty list.  */
        _tx_thread_created_ptr =                    thread_ptr;
        thread_ptr -> tx_thread_created_next =      thread_ptr;
        thread_ptr -> tx_thread_created_previous =  thread_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_thread =  _tx_thread_created_ptr;
        previous_thread =  next_thread -> tx_thread_created_previous;

        /* Place the new thread in the list.  */
        next_thread -> tx_thread_created_previous =  thread_ptr;
        previous_thread -> tx_thread_created_next =  thread_ptr;

        /* Setup this thread's created links.  */
        thread_ptr -> tx_thread_created_previous =  previous_thread;
        thread_ptr -> tx_thread_created_next =      next_thread;
    }

    /* Increment the thread created count.  */
    _tx_thread_created_count++;

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_THREAD, thread_ptr, name_ptr, TX_POINTER_TO_ULONG_CONVERT(stack_start), stack_size)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_CREATE, thread_ptr, priority, TX_POINTER_TO_ULONG_CONVERT(stack_start), stack_size, TX_TRACE_THREAD_EVENTS)

    /* Register thread in the thread array structure.  */
    TX_EL_THREAD_REGISTER(thread_ptr)

    /* Log this kernel call.  */
    TX_EL_THREAD_CREATE_INSERT

#ifndef TX_NOT_INTERRUPTABLE

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;
#endif

    /* Determine if an automatic start was requested.  If so, call the resume
       thread function and then check for a preemption condition.  */
    if (auto_start == TX_AUTO_START)
    {

#ifdef TX_NOT_INTERRUPTABLE

        /* Perform any additional activities for tool or user purpose.  */
        TX_THREAD_CREATE_EXTENSION(thread_ptr)

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

#else

        /* Restore previous interrupt posture.  */
        TX_RESTORE

        /* Perform any additional activities for tool or user purpose.  */
        TX_THREAD_CREATE_EXTENSION(thread_ptr)

        /* Call the resume thread function to make this thread ready.  */
        _tx_thread_system_resume(thread_ptr);

        /* Disable interrupts again.  */
        TX_DISABLE
#endif

        /* Determine if the execution list needs to be re-evaluated.  */
        if (_tx_thread_smp_current_state_get() >= TX_INITIALIZE_IN_PROGRESS)
        {

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

            /* Clear the preemption bit maps, since nothing has yet run during initialization.  */
            TX_MEMSET(_tx_thread_preempted_maps, 0, sizeof(_tx_thread_preempted_maps));
#if TX_MAX_PRIORITIES > 32
            _tx_thread_preempted_map_active =  ((ULONG) 0);
#endif

            /* Clear the entry in the preempted thread list.  */
            _tx_thread_preemption_threshold_list[priority] =  TX_NULL;
#endif

            /* Set the pointer to the thread currently with preemption-threshold set to NULL.  */
            _tx_thread_preemption__threshold_scheduled =  TX_NULL;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(12, 0, thread_ptr);
#endif

            /* Get the core index.  */
            core_index =  TX_SMP_CORE_ID;

            /* Call the rebalance routine. This routine maps cores and ready threads.  */
            _tx_thread_smp_rebalance_execute_list(core_index);

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(13, 0, thread_ptr);
#endif
        }

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

#ifdef TX_NOT_INTERRUPTABLE

        /* Perform any additional activities for tool or user purpose.  */
        TX_THREAD_CREATE_EXTENSION(thread_ptr)

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Restore interrupts.  */
        TX_RESTORE

        /* Perform any additional activities for tool or user purpose.  */
        TX_THREAD_CREATE_EXTENSION(thread_ptr)

        /* Disable interrupts.  */
        TX_DISABLE

        /* Re-enable preemption.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();
#endif
    }

    /* Return success.  */
    return(TX_SUCCESS);
}

