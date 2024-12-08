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


#ifndef TX_MISRA_ENABLE
#define TX_THREAD_INIT
#endif


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"


/* Define the pointer that contains the system stack pointer.  This is
   utilized when control returns from a thread to the system to reset the
   current stack.  This is setup in the low-level initialization function. */

VOID *                      _tx_thread_system_stack_ptr[TX_THREAD_SMP_MAX_CORES];


/* Define the current thread pointer.  This variable points to the currently
   executing thread.  If this variable is NULL, no thread is executing.  */

TX_THREAD *                 _tx_thread_current_ptr[TX_THREAD_SMP_MAX_CORES];


/* Define the variable that holds the next thread to execute.  It is important
   to remember that this is not necessarily equal to the current thread
   pointer.  */

TX_THREAD *                 _tx_thread_execute_ptr[TX_THREAD_SMP_MAX_CORES];


/* Define the ThreadX SMP scheduling and mapping data structures.  */

TX_THREAD *                 _tx_thread_smp_schedule_list[TX_THREAD_SMP_MAX_CORES];
ULONG                       _tx_thread_smp_reschedule_pending;
TX_THREAD_SMP_PROTECT       _tx_thread_smp_protection;
volatile ULONG              _tx_thread_smp_release_cores_flag;
ULONG                       _tx_thread_smp_system_error;
ULONG                       _tx_thread_smp_inter_core_interrupts[TX_THREAD_SMP_MAX_CORES];

ULONG                       _tx_thread_smp_protect_wait_list_size;
ULONG                       _tx_thread_smp_protect_wait_list[TX_THREAD_SMP_PROTECT_WAIT_LIST_SIZE];
ULONG                       _tx_thread_smp_protect_wait_counts[TX_THREAD_SMP_MAX_CORES];
ULONG                       _tx_thread_smp_protect_wait_list_lock_protect_in_force;
ULONG                       _tx_thread_smp_protect_wait_list_tail;
ULONG                       _tx_thread_smp_protect_wait_list_head;


/* Define logic for conditional dynamic maximum number of cores.  */

#ifdef TX_THREAD_SMP_DYNAMIC_CORE_MAX

ULONG                       _tx_thread_smp_max_cores;
ULONG                       _tx_thread_smp_detected_cores;

#endif



/* Define the head pointer of the created thread list.  */

TX_THREAD *                 _tx_thread_created_ptr;


/* Define the variable that holds the number of created threads. */

ULONG                       _tx_thread_created_count;


/* Define the current state variable.  When this value is 0, a thread
   is executing or the system is idle.  Other values indicate that
   interrupt or initialization processing is active.  This variable is
   initialized to TX_INITIALIZE_IN_PROGRESS to indicate initialization is
   active.  */

volatile ULONG              _tx_thread_system_state[TX_THREAD_SMP_MAX_CORES];


/* Define the 32-bit priority bit-maps. There is one priority bit map for each
   32 priority levels supported. If only 32 priorities are supported there is
   only one bit map. Each bit within a priority bit map represents that one
   or more threads at the associated thread priority are ready.  */

ULONG                       _tx_thread_priority_maps[TX_MAX_PRIORITIES/32];


/* Define the priority map active bit map that specifies which of the previously
   defined priority maps have something set. This is only necessary if more than
   32 priorities are supported.  */

#if TX_MAX_PRIORITIES > 32
ULONG                       _tx_thread_priority_map_active;
#endif


#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

/* Define the 32-bit preempt priority bit maps.  There is one preempt bit map
   for each 32 priority levels supported. If only 32 priorities are supported
   there is only one bit map. Each set set bit corresponds to a preempted priority
   level that had preemption-threshold active to protect against preemption of a
   range of relatively higher priority threads.  */

ULONG                       _tx_thread_preempted_maps[TX_MAX_PRIORITIES/32];


/* Define the preempt map active bit map that specifies which of the previously
   defined preempt maps have something set. This is only necessary if more than
   32 priorities are supported.  */

#if TX_MAX_PRIORITIES > 32
ULONG                       _tx_thread_preempted_map_active;
#endif


/* Define the array that contains the thread at each priority level that was scheduled with
   preemption-threshold enabled. This will be useful when returning from a nested
   preemption-threshold condition.  */

TX_THREAD                   *_tx_thread_preemption_threshold_list[TX_MAX_PRIORITIES];


#endif


/* Define the last thread scheduled with preemption-threshold. When preemption-threshold is
   disabled, a thread with preemption-threshold set disables all other threads from running.
   Effectively, its preemption-threshold is 0.  */

TX_THREAD                   *_tx_thread_preemption__threshold_scheduled;


/* Define the array of thread pointers.  Each entry represents the threads that
   are ready at that priority group.  For example, index 10 in this array
   represents the first thread ready at priority 10.  If this entry is NULL,
   no threads are ready at that priority.  */

TX_THREAD *                 _tx_thread_priority_list[TX_MAX_PRIORITIES];


/* Define the global preempt disable variable.  If this is non-zero, preemption is
   disabled.  It is used internally by ThreadX to prevent preemption of a thread in
   the middle of a service that is resuming or suspending another thread.  */

volatile UINT               _tx_thread_preempt_disable;


/* Define the global function pointer for mutex cleanup on thread completion or
   termination. This pointer is setup during mutex initialization.  */

VOID            (*_tx_thread_mutex_release)(TX_THREAD *thread_ptr);


/* Define the global build options variable.  This contains a bit map representing
   how the ThreadX library was built. The following are the bit field definitions:

                    Bit(s)                   Meaning

                    31                  Reserved
                    30                  TX_NOT_INTERRUPTABLE defined
                    29-24               Priority groups 1  -> 32 priorities
                                                        2  -> 64 priorities
                                                        3  -> 96 priorities

                                                        ...

                                                        32 -> 1024 priorities
                    23                  TX_TIMER_PROCESS_IN_ISR defined
                    22                  TX_REACTIVATE_INLINE defined
                    21                  TX_DISABLE_STACK_FILLING defined
                    20                  TX_ENABLE_STACK_CHECKING defined
                    19                  TX_DISABLE_PREEMPTION_THRESHOLD defined
                    18                  TX_DISABLE_REDUNDANT_CLEARING defined
                    17                  TX_DISABLE_NOTIFY_CALLBACKS defined
                    16                  TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO defined
                    15                  TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO defined
                    14                  TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO defined
                    13                  TX_MUTEX_ENABLE_PERFORMANCE_INFO defined
                    12                  TX_QUEUE_ENABLE_PERFORMANCE_INFO defined
                    11                  TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO defined
                    10                  TX_THREAD_ENABLE_PERFORMANCE_INFO defined
                    9                   TX_TIMER_ENABLE_PERFORMANCE_INFO defined
                    8                   TX_ENABLE_EVENT_TRACE | TX_ENABLE_EVENT_LOGGING defined
                    7                   Reserved
                    6                   Reserved
                    5                   Reserved
                    4                   Reserved
                    3                   Reserved
                    2                   Reserved
                    1                   64-bit FPU Enabled
                    0                   Reserved  */

ULONG                       _tx_build_options;


#ifdef TX_ENABLE_STACK_CHECKING

/* Define the global function pointer for stack error handling. If a stack error is
   detected and the application has registered a stack error handler, it will be
   called via this function pointer.  */

VOID                        (*_tx_thread_application_stack_error_handler)(TX_THREAD *thread_ptr);

#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

/* Define the total number of thread resumptions. Each time a thread enters the
   ready state this variable is incremented.  */

ULONG                       _tx_thread_performance_resume_count;


/* Define the total number of thread suspensions. Each time a thread enters a
   suspended state this variable is incremented.  */

ULONG                       _tx_thread_performance_suspend_count;


/* Define the total number of solicited thread preemptions. Each time a thread is
   preempted by directly calling a ThreadX service, this variable is incremented.  */

ULONG                       _tx_thread_performance_solicited_preemption_count;


/* Define the total number of interrupt thread preemptions. Each time a thread is
   preempted as a result of an ISR calling a ThreadX service, this variable is
   incremented.  */

ULONG                       _tx_thread_performance_interrupt_preemption_count;


/* Define the total number of priority inversions. Each time a thread is blocked by
   a mutex owned by a lower-priority thread, this variable is incremented.  */

ULONG                       _tx_thread_performance_priority_inversion_count;


/* Define the total number of time-slices.  Each time a time-slice operation is
   actually performed (another thread is setup for running) this variable is
   incremented.  */

ULONG                       _tx_thread_performance_time_slice_count;


/* Define the total number of thread relinquish operations.  Each time a thread
   relinquish operation is actually performed (another thread is setup for running)
   this variable is incremented.  */

ULONG                       _tx_thread_performance_relinquish_count;


/* Define the total number of thread timeouts. Each time a thread has a
   timeout this variable is incremented.  */

ULONG                       _tx_thread_performance_timeout_count;


/* Define the total number of thread wait aborts. Each time a thread's suspension
   is lifted by the tx_thread_wait_abort call this variable is incremented.  */

ULONG                       _tx_thread_performance_wait_abort_count;


/* Define the total number of idle system thread returns. Each time a thread returns to
   an idle system (no other thread is ready to run) this variable is incremented.  */

ULONG                       _tx_thread_performance_idle_return_count;


/* Define the total number of non-idle system thread returns. Each time a thread returns to
   a non-idle system (another thread is ready to run) this variable is incremented.  */

ULONG                       _tx_thread_performance_non_idle_return_count;

#endif


/* Define special string.  */

#ifndef TX_MISRA_ENABLE
const CHAR _tx_thread_special_string[] =
  "G-ML-EL-ML-BL-DL-BL-GB-GL-M-D-DL-GZ-KH-EL-CM-NH-HA-GF-DD-JC-YZ-CT-AT-DW-USA-CA-SD-SDSU";
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_initialize                              PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the thread control component.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
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
/*    _tx_initialize_high_level         High level initialization         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_initialize(VOID)
{

    /* Note: the system stack pointer and the system state variables are
       initialized by the low and high-level initialization functions,
       respectively.  */

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Set current thread pointer to NULL.  */
    TX_THREAD_SET_CURRENT(0)

    /* Clear the execute thread list.  */
    TX_MEMSET(&_tx_thread_execute_ptr[0], 0, (sizeof(_tx_thread_execute_ptr)));

    /* Initialize the priority information.  */
    TX_MEMSET(&_tx_thread_priority_maps[0], 0, (sizeof(_tx_thread_priority_maps)));

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
    TX_MEMSET(&_tx_thread_preempted_maps[0], 0, (sizeof(_tx_thread_preempted_maps)));
    TX_MEMSET(&_tx_thread_preemption_threshold_list[0], 0, (sizeof(_tx_thread_preemption_threshold_list)));
#endif
    _tx_thread_preemption__threshold_scheduled =  TX_NULL;


#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the array of priority head pointers.  */
    TX_MEMSET(&_tx_thread_priority_list[0], 0, (sizeof(_tx_thread_priority_list)));

    /* Initialize the head pointer of the created threads list and the
       number of threads created.  */
    _tx_thread_created_ptr =        TX_NULL;
    _tx_thread_created_count =      TX_EMPTY;

    /* Clear the global preempt disable variable.  */
    _tx_thread_preempt_disable =    ((UINT) 0);

    /* Initialize the thread mutex release function pointer.  */
    _tx_thread_mutex_release =      TX_NULL;

#ifdef TX_ENABLE_STACK_CHECKING

    /* Clear application registered stack error handler.  */
    _tx_thread_application_stack_error_handler =  TX_NULL;
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

    /* Clear performance counters.  */
    _tx_thread_performance_resume_count =                ((ULONG) 0);
    _tx_thread_performance_suspend_count =               ((ULONG) 0);
    _tx_thread_performance_solicited_preemption_count =  ((ULONG) 0);
    _tx_thread_performance_interrupt_preemption_count =  ((ULONG) 0);
    _tx_thread_performance_priority_inversion_count =    ((ULONG) 0);
    _tx_thread_performance_time_slice_count =            ((ULONG) 0);
    _tx_thread_performance_relinquish_count =            ((ULONG) 0);
    _tx_thread_performance_timeout_count =               ((ULONG) 0);
    _tx_thread_performance_wait_abort_count =            ((ULONG) 0);
    _tx_thread_performance_idle_return_count =           ((ULONG) 0);
    _tx_thread_performance_non_idle_return_count =       ((ULONG) 0);
#endif
#endif

    /* Setup the build options flag. This is used to identify how the ThreadX library was constructed.  */
    _tx_build_options =  _tx_build_options
                            | (((ULONG) (TX_MAX_PRIORITIES/32)) << 24)
#ifdef TX_NOT_INTERRUPTABLE
                            | (((ULONG) 1) << 31)
#endif
#ifdef TX_INLINE_THREAD_RESUME_SUSPEND
                            | (((ULONG) 1) << 30)
#endif
#ifdef TX_TIMER_PROCESS_IN_ISR
                            | (((ULONG) 1) << 23)
#endif
#ifdef TX_REACTIVATE_INLINE
                            | (((ULONG) 1) << 22)
#endif
#ifdef TX_DISABLE_STACK_FILLING
                            | (((ULONG) 1) << 21)
#endif
#ifdef TX_ENABLE_STACK_CHECKING
                            | (((ULONG) 1) << 20)
#endif
#ifdef TX_DISABLE_PREEMPTION_THRESHOLD
                            | (((ULONG) 1) << 19)
#endif
#ifdef TX_DISABLE_REDUNDANT_CLEARING
                            | (((ULONG) 1) << 18)
#endif
#ifdef TX_DISABLE_NOTIFY_CALLBACKS
                            | (((ULONG) 1) << 17)
#endif
#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 16)
#endif
#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 15)
#endif
#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 14)
#endif
#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 13)
#endif
#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 12)
#endif
#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 11)
#endif
#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 10)
#endif
#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO
                            | (((ULONG) 1) << 9)
#endif
#ifdef TX_ENABLE_EVENT_TRACE
                            | (((ULONG) 1) << 8)
#endif
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
                            | (((ULONG) 1) << 7)
#endif
#if TX_PORT_SPECIFIC_BUILD_OPTIONS != 0
                            | TX_PORT_SPECIFIC_BUILD_OPTIONS
#endif
                            ;
}

