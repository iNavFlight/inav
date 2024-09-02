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


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_thread.h                                        PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX thread control component, including   */
/*    data types and external references.  It is assumed that tx_api.h    */
/*    and tx_port.h have already been included.                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef TX_THREAD_H
#define TX_THREAD_H


/* Add include files needed for in-line macros.  */

#include "tx_initialize.h"


/* Define thread control specific data definitions.  */

#define TX_THREAD_ID                            ((ULONG) 0x54485244)
#define TX_THREAD_MAX_BYTE_VALUES               256
#define TX_THREAD_PRIORITY_GROUP_MASK           ((ULONG) 0xFF)
#define TX_THREAD_PRIORITY_GROUP_SIZE           8
#define TX_THREAD_EXECUTE_LOG_SIZE              ((UINT) 8)
#define TX_THREAD_SMP_PROTECT_WAIT_LIST_SIZE    (TX_THREAD_SMP_MAX_CORES + 1)


/* Define the default thread stack checking. This can be overridden by
   a particular port, which is necessary if the stack growth is from
   low address to high address (the default logic is for stacks that
   grow from high address to low address.  */

#ifndef TX_THREAD_STACK_CHECK
#define TX_THREAD_STACK_CHECK(thread_ptr)                                                                                       \
    {                                                                                                                           \
    TX_INTERRUPT_SAVE_AREA                                                                                                      \
        TX_DISABLE                                                                                                              \
        if (((thread_ptr)) && ((thread_ptr) -> tx_thread_id == TX_THREAD_ID))                                                   \
        {                                                                                                                       \
            if (((ULONG *) (thread_ptr) -> tx_thread_stack_ptr) < ((ULONG *) (thread_ptr) -> tx_thread_stack_highest_ptr))      \
            {                                                                                                                   \
                (thread_ptr) -> tx_thread_stack_highest_ptr =  (thread_ptr) -> tx_thread_stack_ptr;                             \
            }                                                                                                                   \
            if ((*((ULONG *) (thread_ptr) -> tx_thread_stack_start) != TX_STACK_FILL) ||                                        \
                (*((ULONG *) (((UCHAR *) (thread_ptr) -> tx_thread_stack_end) + 1)) != TX_STACK_FILL) ||                        \
                (((ULONG *) (thread_ptr) -> tx_thread_stack_highest_ptr) < ((ULONG *) (thread_ptr) -> tx_thread_stack_start)))  \
            {                                                                                                                   \
                TX_RESTORE                                                                                                      \
                _tx_thread_stack_error_handler((thread_ptr));                                                                   \
                TX_DISABLE                                                                                                      \
            }                                                                                                                   \
            if (*(((ULONG *) (thread_ptr) -> tx_thread_stack_highest_ptr) - 1) != TX_STACK_FILL)                                \
            {                                                                                                                   \
                TX_RESTORE                                                                                                      \
                _tx_thread_stack_analyze((thread_ptr));                                                                         \
                TX_DISABLE                                                                                                      \
            }                                                                                                                   \
        }                                                                                                                       \
        TX_RESTORE                                                                                                              \
    }
#endif


/* Define default post thread delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_THREAD_DELETE_PORT_COMPLETION
#define TX_THREAD_DELETE_PORT_COMPLETION(t)
#endif


/* Define default post thread reset macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_THREAD_RESET_PORT_COMPLETION
#define TX_THREAD_RESET_PORT_COMPLETION(t)
#endif


/* Define the thread create internal extension macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_THREAD_CREATE_INTERNAL_EXTENSION
#define TX_THREAD_CREATE_INTERNAL_EXTENSION(t)
#endif


/* Define internal thread control function prototypes.  */

VOID        _tx_thread_initialize(VOID);
VOID        _tx_thread_schedule(VOID);
VOID        _tx_thread_shell_entry(VOID);
VOID        _tx_thread_stack_analyze(TX_THREAD *thread_ptr);
VOID        _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID));
VOID        _tx_thread_stack_error(TX_THREAD *thread_ptr);
VOID        _tx_thread_stack_error_handler(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_preempt_check(VOID);
VOID        _tx_thread_system_resume(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_ni_resume(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_return(VOID);
VOID        _tx_thread_system_suspend(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_ni_suspend(TX_THREAD *thread_ptr, ULONG timeout);
VOID        _tx_thread_time_slice(VOID);
VOID        _tx_thread_timeout(ULONG timeout_input);


/* Define all internal SMP prototypes.  */

void        _tx_thread_smp_current_state_set(ULONG new_state);
UINT        _tx_thread_smp_find_next_priority(UINT priority);
void        _tx_thread_smp_high_level_initialize(void);
void        _tx_thread_smp_rebalance_execute_list(UINT core_index);


/* Define all internal ThreadX SMP low-level assembly routines.   */

VOID        _tx_thread_smp_core_wait(void);
void        _tx_thread_smp_initialize_wait(void);
void        _tx_thread_smp_low_level_initialize(UINT number_of_cores);
void        _tx_thread_smp_core_preempt(UINT core);


/* Thread control component external data declarations follow.  */

#define THREAD_DECLARE extern


/* Define the pointer that contains the system stack pointer.  This is
   utilized when control returns from a thread to the system to reset the
   current stack.  This is setup in the low-level initialization function. */

THREAD_DECLARE  VOID *                      _tx_thread_system_stack_ptr[TX_THREAD_SMP_MAX_CORES];


/* Define the current thread pointer.  This variable points to the currently
   executing thread.  If this variable is NULL, no thread is executing.  */

THREAD_DECLARE  TX_THREAD *                 _tx_thread_current_ptr[TX_THREAD_SMP_MAX_CORES];


/* Define the variable that holds the next thread to execute.  It is important
   to remember that this is not necessarily equal to the current thread
   pointer.  */

THREAD_DECLARE  TX_THREAD *                 _tx_thread_execute_ptr[TX_THREAD_SMP_MAX_CORES];


/* Define the ThreadX SMP scheduling and mapping data structures.  */

THREAD_DECLARE  TX_THREAD *                 _tx_thread_smp_schedule_list[TX_THREAD_SMP_MAX_CORES];
THREAD_DECLARE  ULONG                       _tx_thread_smp_reschedule_pending;
THREAD_DECLARE  TX_THREAD_SMP_PROTECT       _tx_thread_smp_protection;
THREAD_DECLARE  volatile ULONG              _tx_thread_smp_release_cores_flag;
THREAD_DECLARE  ULONG                       _tx_thread_smp_system_error;
THREAD_DECLARE  ULONG                       _tx_thread_smp_inter_core_interrupts[TX_THREAD_SMP_MAX_CORES];

THREAD_DECLARE  ULONG                       _tx_thread_smp_protect_wait_list_size;
THREAD_DECLARE  ULONG                       _tx_thread_smp_protect_wait_list[TX_THREAD_SMP_PROTECT_WAIT_LIST_SIZE];
THREAD_DECLARE  ULONG                       _tx_thread_smp_protect_wait_counts[TX_THREAD_SMP_MAX_CORES];
THREAD_DECLARE  ULONG                       _tx_thread_smp_protect_wait_list_lock_protect_in_force;
THREAD_DECLARE  ULONG                       _tx_thread_smp_protect_wait_list_tail;
THREAD_DECLARE  ULONG                       _tx_thread_smp_protect_wait_list_head;


/* Define logic for conditional dynamic maximum number of cores.  */

#ifdef TX_THREAD_SMP_DYNAMIC_CORE_MAX

THREAD_DECLARE  ULONG                       _tx_thread_smp_max_cores;
THREAD_DECLARE  ULONG                       _tx_thread_smp_detected_cores;

#endif



/* Define the head pointer of the created thread list.  */

THREAD_DECLARE  TX_THREAD *                 _tx_thread_created_ptr;


/* Define the variable that holds the number of created threads. */

THREAD_DECLARE  ULONG                       _tx_thread_created_count;


/* Define the current state variable.  When this value is 0, a thread
   is executing or the system is idle.  Other values indicate that
   interrupt or initialization processing is active.  This variable is
   initialized to TX_INITIALIZE_IN_PROGRESS to indicate initialization is
   active.  */

THREAD_DECLARE  volatile ULONG              _tx_thread_system_state[TX_THREAD_SMP_MAX_CORES];


/* Determine if we need to remap system state to a function call.  */

#ifndef TX_THREAD_SMP_SOURCE_CODE


/* Yes, remap system state to a function call so we can get the system state for the current core.  */

#define _tx_thread_system_state             _tx_thread_smp_current_state_get()


/* Yes, remap get current thread to a function call so we can get the current thread for the current core.  */

#define _tx_thread_current_ptr              _tx_thread_smp_current_thread_get()

#endif


/* Define the 32-bit priority bit-maps. There is one priority bit map for each
   32 priority levels supported. If only 32 priorities are supported there is
   only one bit map. Each bit within a priority bit map represents that one
   or more threads at the associated thread priority are ready.  */

THREAD_DECLARE  ULONG                       _tx_thread_priority_maps[TX_MAX_PRIORITIES/32];


/* Define the priority map active bit map that specifies which of the previously
   defined priority maps have something set. This is only necessary if more than
   32 priorities are supported.  */

#if TX_MAX_PRIORITIES > 32
THREAD_DECLARE  ULONG                       _tx_thread_priority_map_active;
#endif


#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

/* Define the 32-bit preempt priority bit maps.  There is one preempt bit map
   for each 32 priority levels supported. If only 32 priorities are supported
   there is only one bit map. Each set set bit corresponds to a preempted priority
   level that had preemption-threshold active to protect against preemption of a
   range of relatively higher priority threads.  */

THREAD_DECLARE  ULONG                       _tx_thread_preempted_maps[TX_MAX_PRIORITIES/32];


/* Define the preempt map active bit map that specifies which of the previously
   defined preempt maps have something set. This is only necessary if more than
   32 priorities are supported.  */

#if TX_MAX_PRIORITIES > 32
THREAD_DECLARE  ULONG                       _tx_thread_preempted_map_active;
#endif


/* Define the array that contains the thread at each priority level that was scheduled with
   preemption-threshold enabled. This will be useful when returning from a nested
   preemption-threshold condition.  */

THREAD_DECLARE  TX_THREAD                   *_tx_thread_preemption_threshold_list[TX_MAX_PRIORITIES];


#endif


/* Define the last thread scheduled with preemption-threshold. When preemption-threshold is
   disabled, a thread with preemption-threshold set disables all other threads from running.
   Effectively, its preemption-threshold is 0.  */

THREAD_DECLARE  TX_THREAD                   *_tx_thread_preemption__threshold_scheduled;


/* Define the array of thread pointers.  Each entry represents the threads that
   are ready at that priority group.  For example, index 10 in this array
   represents the first thread ready at priority 10.  If this entry is NULL,
   no threads are ready at that priority.  */

THREAD_DECLARE  TX_THREAD *                 _tx_thread_priority_list[TX_MAX_PRIORITIES];


/* Define the global preempt disable variable.  If this is non-zero, preemption is
   disabled.  It is used internally by ThreadX to prevent preemption of a thread in
   the middle of a service that is resuming or suspending another thread.  */

THREAD_DECLARE  volatile UINT               _tx_thread_preempt_disable;


/* Define the global function pointer for mutex cleanup on thread completion or
   termination. This pointer is setup during mutex initialization.  */

THREAD_DECLARE  VOID            (*_tx_thread_mutex_release)(TX_THREAD *thread_ptr);


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

THREAD_DECLARE  ULONG                       _tx_build_options;


#ifdef TX_ENABLE_STACK_CHECKING

/* Define the global function pointer for stack error handling. If a stack error is
   detected and the application has registered a stack error handler, it will be
   called via this function pointer.  */

THREAD_DECLARE  VOID                        (*_tx_thread_application_stack_error_handler)(TX_THREAD *thread_ptr);

#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

/* Define the total number of thread resumptions. Each time a thread enters the
   ready state this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_resume_count;


/* Define the total number of thread suspensions. Each time a thread enters a
   suspended state this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_suspend_count;


/* Define the total number of solicited thread preemptions. Each time a thread is
   preempted by directly calling a ThreadX service, this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_solicited_preemption_count;


/* Define the total number of interrupt thread preemptions. Each time a thread is
   preempted as a result of an ISR calling a ThreadX service, this variable is
   incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_interrupt_preemption_count;


/* Define the total number of priority inversions. Each time a thread is blocked by
   a mutex owned by a lower-priority thread, this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_priority_inversion_count;


/* Define the total number of time-slices.  Each time a time-slice operation is
   actually performed (another thread is setup for running) this variable is
   incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_time_slice_count;


/* Define the total number of thread relinquish operations.  Each time a thread
   relinquish operation is actually performed (another thread is setup for running)
   this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_relinquish_count;


/* Define the total number of thread timeouts. Each time a thread has a
   timeout this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_timeout_count;


/* Define the total number of thread wait aborts. Each time a thread's suspension
   is lifted by the tx_thread_wait_abort call this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_wait_abort_count;


/* Define the total number of idle system thread returns. Each time a thread returns to
   an idle system (no other thread is ready to run) this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_idle_return_count;


/* Define the total number of non-idle system thread returns. Each time a thread returns to
   a non-idle system (another thread is ready to run) this variable is incremented.  */

THREAD_DECLARE  ULONG                       _tx_thread_performance_non_idle_return_count;

#endif


/* Define macros and helper functions.  */

/* Define the MOD32 bit set macro that is used to set/clear a priority bit within a specific
   priority group.  */

#if TX_MAX_PRIORITIES > 32
#define MAP_INDEX                               (map_index)
#ifndef TX_MOD32_BIT_SET
#define TX_MOD32_BIT_SET(a,b)                   (b) = (((ULONG) 1) << ((a)%((UINT) 32)));
#endif
#else
#define MAP_INDEX                               (0)
#ifndef TX_MOD32_BIT_SET
#define TX_MOD32_BIT_SET(a,b)                   (b) = (((ULONG) 1) << ((a)));
#endif
#endif


/* Define the DIV32 bit set macro that is used to set/clear a priority group bit and is
   only necessary when using priorities greater than 32.  */

#if TX_MAX_PRIORITIES > 32
#ifndef TX_DIV32_BIT_SET
#define TX_DIV32_BIT_SET(a,b)                   (b) = (((ULONG) 1) << ((a)/((UINT) 32)));
#endif
#endif


/* Define state change macro that can be used by run-mode debug agents to keep track of thread
   state changes. By default, it is mapped to white space.  */

#ifndef TX_THREAD_STATE_CHANGE
#define TX_THREAD_STATE_CHANGE(a, b)
#endif


/* Define the macro to set the current thread pointer. This is particularly useful in SMP
   versions of ThreadX to add additional processing.  The default implementation is to simply
   access the global current thread pointer directly.  */

#ifndef TX_THREAD_SET_CURRENT
#define TX_THREAD_SET_CURRENT(a)            TX_MEMSET(&_tx_thread_current_ptr[0], (a), sizeof(_tx_thread_current_ptr));
#endif


/* Define the get system state macro. By default, it is mapped to white space.  */

#ifndef TX_THREAD_GET_SYSTEM_STATE
#define TX_THREAD_GET_SYSTEM_STATE()        _tx_thread_smp_current_state_get()
#endif


/* Define the check for whether or not to call the _tx_thread_system_return function.  A non-zero value
   indicates that _tx_thread_system_return should not be called.  */

#ifndef TX_THREAD_SYSTEM_RETURN_CHECK
#define TX_THREAD_SYSTEM_RETURN_CHECK(c)    (c) = (ULONG) _tx_thread_preempt_disable; (c) = (c) | TX_THREAD_GET_SYSTEM_STATE();
#endif


/* Define the timeout setup macro used in _tx_thread_create.  */

#ifndef TX_THREAD_CREATE_TIMEOUT_SETUP
#define TX_THREAD_CREATE_TIMEOUT_SETUP(t)    (t) -> tx_thread_timer.tx_timer_internal_timeout_function =  &(_tx_thread_timeout);                \
                                             (t) -> tx_thread_timer.tx_timer_internal_timeout_param =     TX_POINTER_TO_ULONG_CONVERT((t));
#endif


/* Define the thread timeout pointer setup macro used in _tx_thread_timeout.  */

#ifndef TX_THREAD_TIMEOUT_POINTER_SETUP
#define TX_THREAD_TIMEOUT_POINTER_SETUP(t)   (t) =  TX_ULONG_TO_THREAD_POINTER_CONVERT(timeout_input);
#endif


#ifdef TX_THREAD_SMP_SOURCE_CODE


/* Determine if the in-line capability has been disabled.  */

#ifndef TX_DISABLE_INLINE


/* Define the inline option, which is compiler specific. If not defined, it will be resolved as
   "inline".  */

#ifndef INLINE_DECLARE
#define INLINE_DECLARE  inline
#endif


/* Define the lowest bit set macro. Note, that this may be overridden
   by a port specific definition if there is supporting assembly language
   instructions in the architecture.  */

#ifndef TX_LOWEST_SET_BIT_CALCULATE

static INLINE_DECLARE UINT  _tx_thread_lowest_set_bit_calculate(ULONG map)
{
UINT    bit_set;

    if ((map & ((ULONG) 0x1)) != ((ULONG) 0))
    {
        bit_set = ((UINT) 0);
    }
    else
    {
        map =  map & (ULONG) ((~map) + ((ULONG) 1));
        if (map < ((ULONG) 0x100))
        {
            bit_set = ((UINT) 1);
        }
        else if (map < ((ULONG) 0x10000))
        {
            bit_set =  ((UINT) 9);
            map =  map >> ((UINT) 8);
        }
        else if (map < ((ULONG) 0x01000000))
        {
            bit_set = ((UINT) 17);
            map = map >> ((UINT) 16);
        }
        else
        {
            bit_set = ((UINT) 25);
            map = map >> ((UINT) 24);
        }
        if (map >= ((ULONG) 0x10))
        {
            map = map >> ((UINT) 4);
            bit_set = bit_set + ((UINT) 4);
        }
        if (map >= ((ULONG) 0x4))
        {
            map = map >> ((UINT) 2);
            bit_set = bit_set + ((UINT) 2);
        }
        bit_set = bit_set - (UINT) (map & (ULONG) 0x1);
    }

    return(bit_set);
}


#define TX_LOWEST_SET_BIT_CALCULATE(m, b)   (b) =  _tx_thread_lowest_set_bit_calculate((m));

#endif


/* Define the next priority macro. Note, that this may be overridden
   by a port specific definition.  */

#ifndef TX_NEXT_PRIORITY_FIND
#if TX_MAX_PRIORITIES > 32
static INLINE_DECLARE UINT _tx_thread_smp_next_priority_find(UINT priority)
{
ULONG           map_index;
ULONG           local_priority_map_active;
ULONG           local_priority_map;
ULONG           priority_bit;
ULONG           first_bit_set;
ULONG           found_priority;

    found_priority =  ((UINT) TX_MAX_PRIORITIES);
    if (priority < ((UINT) TX_MAX_PRIORITIES))
    {
        map_index =  priority/((UINT) 32);
        local_priority_map =  _tx_thread_priority_maps[map_index];
        priority_bit =        (((ULONG) 1) << (priority % ((UINT) 32)));
        local_priority_map =  local_priority_map & ~(priority_bit - ((UINT)1));
        if (local_priority_map != ((ULONG) 0))
        {
            TX_LOWEST_SET_BIT_CALCULATE(local_priority_map, first_bit_set)
            found_priority =  (map_index * ((UINT) 32)) + first_bit_set;
        }
        else
        {
            /* Move to next map index.  */
            map_index++;
            if (map_index < (((UINT) TX_MAX_PRIORITIES)/((UINT) 32)))
            {
                priority_bit =               (((ULONG) 1) << (map_index));
                local_priority_map_active =  _tx_thread_priority_map_active & ~(priority_bit - ((UINT) 1));
                if (local_priority_map_active != ((ULONG) 0))
                {
                    TX_LOWEST_SET_BIT_CALCULATE(local_priority_map_active, map_index)
                    local_priority_map =  _tx_thread_priority_maps[map_index];
                    TX_LOWEST_SET_BIT_CALCULATE(local_priority_map, first_bit_set)
                    found_priority =  (map_index * ((UINT) 32)) + first_bit_set;
                }
            }
        }
    }
    return(found_priority);
}
#else

static INLINE_DECLARE UINT _tx_thread_smp_next_priority_find(UINT priority)
{
UINT            first_bit_set;
ULONG           local_priority_map;
UINT            next_priority;

    local_priority_map =  _tx_thread_priority_maps[0];
    local_priority_map =  local_priority_map >> priority;
    next_priority =  priority;
    if (local_priority_map == ((ULONG) 0))
    {
        next_priority =  ((UINT) TX_MAX_PRIORITIES);
    }
    else
    {
        if (next_priority >= ((UINT) TX_MAX_PRIORITIES))
        {
            next_priority =  ((UINT) TX_MAX_PRIORITIES);
        }
        else
        {
            TX_LOWEST_SET_BIT_CALCULATE(local_priority_map, first_bit_set)
            next_priority =  priority + first_bit_set;
        }
    }

    return(next_priority);
}
#endif
#endif

static INLINE_DECLARE void  _tx_thread_smp_schedule_list_clear(void)
{
#if TX_THREAD_SMP_MAX_CORES > 6
UINT    i;
#endif


    /* Clear the schedule list.  */
    _tx_thread_smp_schedule_list[0] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 1
    _tx_thread_smp_schedule_list[1] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 2
    _tx_thread_smp_schedule_list[2] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 3
    _tx_thread_smp_schedule_list[3] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 4
    _tx_thread_smp_schedule_list[4] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 5
    _tx_thread_smp_schedule_list[5] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to clear the remainder of the schedule list.  */
    i =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (i < _tx_thread_smp_max_cores)
#endif
    {
        /* Clear entry in schedule list.  */
        _tx_thread_smp_schedule_list[i] =  TX_NULL;

        /* Move to next index.  */
        i++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
}

static INLINE_DECLARE VOID  _tx_thread_smp_execute_list_clear(void)
{
#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif

    /* Clear the execute list.  */
    _tx_thread_execute_ptr[0] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 1
    _tx_thread_execute_ptr[1] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 2
    _tx_thread_execute_ptr[2] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 3
    _tx_thread_execute_ptr[3] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 4
    _tx_thread_execute_ptr[4] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 5
    _tx_thread_execute_ptr[5] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to clear the remainder of the execute list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Clear entry in execute list.  */
        _tx_thread_execute_ptr[j] =  TX_NULL;

        /* Move to next index.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
}


static INLINE_DECLARE VOID  _tx_thread_smp_schedule_list_setup(void)
{
#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif

    _tx_thread_smp_schedule_list[0] =  _tx_thread_execute_ptr[0];
#if TX_THREAD_SMP_MAX_CORES > 1
    _tx_thread_smp_schedule_list[1] =  _tx_thread_execute_ptr[1];
#if TX_THREAD_SMP_MAX_CORES > 2
    _tx_thread_smp_schedule_list[2] =  _tx_thread_execute_ptr[2];
#if TX_THREAD_SMP_MAX_CORES > 3
    _tx_thread_smp_schedule_list[3] =  _tx_thread_execute_ptr[3];
#if TX_THREAD_SMP_MAX_CORES > 4
    _tx_thread_smp_schedule_list[4] =  _tx_thread_execute_ptr[4];
#if TX_THREAD_SMP_MAX_CORES > 5
    _tx_thread_smp_schedule_list[5] =  _tx_thread_execute_ptr[5];
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Setup entry in schedule list.  */
        _tx_thread_smp_schedule_list[j] =  _tx_thread_execute_ptr[j];

        /* Move to next index.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
}


#ifdef TX_THREAD_SMP_INTER_CORE_INTERRUPT
static INLINE_DECLARE VOID  _tx_thread_smp_core_interrupt(TX_THREAD *thread_ptr, UINT current_core, UINT target_core)
{

TX_THREAD   *current_thread;


    /* Make sure this is a different core, since there is no need to interrupt the current core for
       a scheduling change.  */
    if (current_core != target_core)
    {

        /* Yes, a different core is present.  */

        /* Pickup the currently executing thread.  */
        current_thread =  _tx_thread_current_ptr[target_core];

        /* Determine if they are the same.  */
        if ((current_thread != TX_NULL) && (thread_ptr != current_thread))
        {

            /* Not the same and not NULL... determine if the core is running at thread level.  */
            if (_tx_thread_system_state[target_core] < TX_INITIALIZE_IN_PROGRESS)
            {

                /* Preempt the mapped thread.  */
                _tx_thread_smp_core_preempt(target_core);
            }
        }
    }
}
#else

/* Define to whitespace.  */
#define _tx_thread_smp_core_interrupt(a,b,c)

#endif


#ifdef TX_THREAD_SMP_WAKEUP_LOGIC
static INLINE_DECLARE VOID  _tx_thread_smp_core_wakeup(UINT current_core, UINT target_core)
{

    /* Determine if the core specified is not the current core - no need to wakeup the
       current core.  */
    if (target_core != current_core)
    {

        /* Wakeup based on application's macro.  */
        TX_THREAD_SMP_WAKEUP(target_core);
    }
}
#else

/* Define to whitespace.  */
#define _tx_thread_smp_core_wakeup(a,b)

#endif


static INLINE_DECLARE VOID  _tx_thread_smp_execute_list_setup(UINT core_index)
{

TX_THREAD   *schedule_thread;
UINT        i;


    /* Loop to copy the schedule list into the execution list.  */
    i =  ((UINT) 0);
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (i < _tx_thread_smp_max_cores)
#endif
    {

        /* Pickup the thread to schedule.  */
        schedule_thread =  _tx_thread_smp_schedule_list[i];

        /* Copy the schedule list into the execution list.  */
        _tx_thread_execute_ptr[i] =  schedule_thread;

        /* If necessary, interrupt the core with the new thread to schedule.  */
        _tx_thread_smp_core_interrupt(schedule_thread, core_index, i);

#ifdef TX_THREAD_SMP_WAKEUP_LOGIC

        /* Does this need to be waked up?  */
        if ((i != core_index) && (schedule_thread != TX_NULL))
        {

            /* Wakeup based on application's macro.  */
            TX_THREAD_SMP_WAKEUP(i);
        }
#endif
        /* Move to next index.  */
        i++;
    }
}


static INLINE_DECLARE ULONG  _tx_thread_smp_available_cores_get(void)
{

#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif
ULONG   available_cores;

    available_cores =  ((ULONG) 0);
    if (_tx_thread_execute_ptr[0] == TX_NULL)
    {
        available_cores =  ((ULONG) 1);
    }
#if TX_THREAD_SMP_MAX_CORES > 1
    if (_tx_thread_execute_ptr[1] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 2);
    }
#if TX_THREAD_SMP_MAX_CORES > 2
    if (_tx_thread_execute_ptr[2] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 4);
    }
#if TX_THREAD_SMP_MAX_CORES > 3
    if (_tx_thread_execute_ptr[3] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 8);
    }
#if TX_THREAD_SMP_MAX_CORES > 4
    if (_tx_thread_execute_ptr[4] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 0x10);
    }
#if TX_THREAD_SMP_MAX_CORES > 5
    if (_tx_thread_execute_ptr[5] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 0x20);
    }
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Determine if this core is available.  */
        if (_tx_thread_execute_ptr[j] == TX_NULL)
        {
            available_cores =  available_cores | (((ULONG) 1) << j);
        }

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
    return(available_cores);
}


static INLINE_DECLARE ULONG  _tx_thread_smp_possible_cores_get(void)
{

#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif
ULONG       possible_cores;
TX_THREAD   *thread_ptr;

    possible_cores =  ((ULONG) 0);
    thread_ptr =  _tx_thread_execute_ptr[0];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 1
    thread_ptr =  _tx_thread_execute_ptr[1];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 2
    thread_ptr =  _tx_thread_execute_ptr[2];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 3
    thread_ptr =  _tx_thread_execute_ptr[3];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 4
    thread_ptr =  _tx_thread_execute_ptr[4];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 5
    thread_ptr =  _tx_thread_execute_ptr[5];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Determine if this core is available.  */
        thread_ptr =  _tx_thread_execute_ptr[j];
        if (thread_ptr != TX_NULL)
        {
            possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
        }

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
    return(possible_cores);
}


static INLINE_DECLARE UINT  _tx_thread_smp_lowest_priority_get(void)
{

#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif
TX_THREAD   *thread_ptr;
UINT        lowest_priority;

    lowest_priority =  ((UINT) 0);
    thread_ptr =  _tx_thread_execute_ptr[0];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 1
    thread_ptr =  _tx_thread_execute_ptr[1];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 2
    thread_ptr =  _tx_thread_execute_ptr[2];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 3
    thread_ptr =  _tx_thread_execute_ptr[3];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 4
    thread_ptr =  _tx_thread_execute_ptr[4];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 5
    thread_ptr =  _tx_thread_execute_ptr[5];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Determine if this core has a thread scheduled.  */
        thread_ptr =  _tx_thread_execute_ptr[j];
        if (thread_ptr != TX_NULL)
        {

            /* Is this the new lowest priority?  */
            if (thread_ptr -> tx_thread_priority > lowest_priority)
            {
                lowest_priority =  thread_ptr -> tx_thread_priority;
            }
        }

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
    return(lowest_priority);
}


static INLINE_DECLARE UINT  _tx_thread_smp_remap_solution_find(TX_THREAD *schedule_thread, ULONG available_cores, ULONG thread_possible_cores, ULONG test_possible_cores)
{

UINT            core;
UINT            previous_core;
ULONG           test_cores;
ULONG           last_thread_cores;
UINT            queue_first, queue_last;
UINT            core_queue[TX_THREAD_SMP_MAX_CORES-1];
TX_THREAD       *thread_ptr;
TX_THREAD       *last_thread;
TX_THREAD       *thread_remap_list[TX_THREAD_SMP_MAX_CORES];


    /* Clear the last thread cores in the search.  */
    last_thread_cores =  ((ULONG) 0);

    /* Set the last thread pointer to NULL.  */
    last_thread =  TX_NULL;

    /* Setup the core queue indices.  */
    queue_first =  ((UINT) 0);
    queue_last =   ((UINT) 0);

    /* Build a list of possible cores for this thread to execute on, starting
       with the previously mapped core.  */
    core =  schedule_thread -> tx_thread_smp_core_mapped;
    if ((thread_possible_cores & (((ULONG) 1) << core)) != ((ULONG) 0))
    {

        /* Remember this potential mapping.  */
        thread_remap_list[core] =   schedule_thread;
        core_queue[queue_last] =    core;

        /* Move to next slot.  */
        queue_last++;

        /* Clear this core.  */
        thread_possible_cores =  thread_possible_cores & ~(((ULONG) 1) << core);
    }

    /* Loop to add additional possible cores.  */
    while (thread_possible_cores != ((ULONG) 0))
    {

        /* Determine the first possible core.  */
        test_cores =  thread_possible_cores;
        TX_LOWEST_SET_BIT_CALCULATE(test_cores, core)

        /* Clear this core.  */
        thread_possible_cores =  thread_possible_cores & ~(((ULONG) 1) << core);

        /* Remember this potential mapping.  */
        thread_remap_list[core] =  schedule_thread;
        core_queue[queue_last] =   core;

        /* Move to next slot.  */
        queue_last++;
    }

    /* Loop to evaluate the potential thread mappings, against what is already mapped.  */
    do
    {

        /* Pickup the next entry.  */
        core = core_queue[queue_first];

        /* Move to next slot.  */
        queue_first++;

        /* Retrieve the thread from the current mapping.  */
        thread_ptr =  _tx_thread_smp_schedule_list[core];

        /* Determine if there is a thread currently mapped to this core.  */
        if (thread_ptr != TX_NULL)
        {

            /* Determine the cores available for this thread.  */
            thread_possible_cores =  thread_ptr -> tx_thread_smp_cores_allowed;
            thread_possible_cores =  test_possible_cores & thread_possible_cores;

            /* Are there any possible cores for this thread?  */
            if (thread_possible_cores != ((ULONG) 0))
            {

                /* Determine if there are cores available for this thread.  */
                if ((thread_possible_cores & available_cores) != ((ULONG) 0))
                {

                    /* Yes, remember the final thread and cores that are valid for this thread.  */
                    last_thread_cores =  thread_possible_cores & available_cores;
                    last_thread =        thread_ptr;

                    /* We are done - get out of the loop!  */
                    break;
                }
                else
                {

                    /* Remove cores that will be added to the list.  */
                    test_possible_cores =  test_possible_cores & ~(thread_possible_cores);

                    /* Loop to add this thread to the potential mapping list.  */
                    do
                    {

                        /* Calculate the core.  */
                        test_cores =  thread_possible_cores;
                        TX_LOWEST_SET_BIT_CALCULATE(test_cores, core)

                        /* Clear this core.  */
                        thread_possible_cores =  thread_possible_cores & ~(((ULONG) 1) << core);

                        /* Remember this thread for remapping.  */
                        thread_remap_list[core] =  thread_ptr;

                        /* Remember this core.  */
                        core_queue[queue_last] =  core;

                        /* Move to next slot.  */
                        queue_last++;

                    } while (thread_possible_cores != ((ULONG) 0));
                }
            }
        }
    } while (queue_first != queue_last);

    /* Was a remapping solution found?  */
    if (last_thread != TX_NULL)
    {

        /* Pickup the core of the last thread to remap.  */
        core =  last_thread -> tx_thread_smp_core_mapped;

        /* Pickup the thread from the remapping list.  */
        thread_ptr =  thread_remap_list[core];

        /* Loop until we arrive at the thread we have been trying to map.  */
        while (thread_ptr != schedule_thread)
        {

            /* Move this thread in the schedule list.  */
            _tx_thread_smp_schedule_list[core] =  thread_ptr;

            /* Remember the previous core.  */
            previous_core =  core;

            /* Pickup the core of thread to remap.  */
            core =  thread_ptr -> tx_thread_smp_core_mapped;

            /* Save the new core mapping for this thread.  */
            thread_ptr -> tx_thread_smp_core_mapped =  previous_core;

            /* Move the next thread.  */
            thread_ptr =  thread_remap_list[core];
        }

        /* Save the remaining thread in the updated schedule list.  */
        _tx_thread_smp_schedule_list[core] =  thread_ptr;

        /* Update this thread's core mapping.  */
        thread_ptr -> tx_thread_smp_core_mapped =  core;

        /* Finally, setup the last thread in the remapping solution.  */
        test_cores =  last_thread_cores;
        TX_LOWEST_SET_BIT_CALCULATE(test_cores, core)

        /* Setup the last thread.  */
        _tx_thread_smp_schedule_list[core] =     last_thread;

        /* Remember the core mapping for this thread.  */
        last_thread -> tx_thread_smp_core_mapped =  core;
    }
    else
    {

        /* Set core to the maximum value in order to signal a remapping solution was not found.  */
        core =  ((UINT) TX_THREAD_SMP_MAX_CORES);
    }

    /* Return core to the caller.  */
    return(core);
}


static INLINE_DECLARE ULONG  _tx_thread_smp_preemptable_threads_get(UINT priority, TX_THREAD *possible_preemption_list[])
{

UINT        i, j, k;
TX_THREAD   *thread_ptr;
TX_THREAD   *next_thread;
TX_THREAD   *search_thread;
TX_THREAD   *list_head;
ULONG       possible_cores =  ((ULONG) 0);


    /* Clear the possible preemption list.  */
    possible_preemption_list[0] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 1
    possible_preemption_list[1] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 2
    possible_preemption_list[2] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 3
    possible_preemption_list[3] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 4
    possible_preemption_list[4] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 5
    possible_preemption_list[5] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to clear the remainder of the possible preemption list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Clear entry in possible preemption list.  */
        possible_preemption_list[j] =  TX_NULL;

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif

    /* Loop to build a list of threads of less priority.  */
    i =  ((UINT) 0);
    j =  ((UINT) 0);
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (i < _tx_thread_smp_max_cores)
#endif
    {

        /* Pickup the currently mapped thread.  */
        thread_ptr =  _tx_thread_execute_ptr[i];

        /* Is there a thread scheduled for this core?  */
        if (thread_ptr != TX_NULL)
        {

            /* Update the possible cores bit map.  */
            possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;

            /* Can this thread be preempted?  */
            if (priority < thread_ptr -> tx_thread_priority)
            {

                /* Thread that can be added to the preemption possible list.  */

                /* Yes, this scheduled thread is lower priority, so add it to the preemption possible list.  */
                possible_preemption_list[j] =  thread_ptr;

                /* Move to next entry in preemption possible list.  */
                j++;
            }
        }

        /* Move to next core.  */
        i++;
    }

    /* Check to see if there are more than 2 threads that can be preempted.  */
    if (j > ((UINT) 1))
    {

        /* Yes, loop through the preemption possible list and sort by priority.  */
        i =  ((UINT) 0);
        do
        {

            /* Pickup preemptable thread.  */
            thread_ptr =  possible_preemption_list[i];

            /* Initialize the search index.  */
            k =  i + ((UINT) 1);

            /* Loop to get the lowest priority thread at the front of the list.  */
            while (k < j)
            {

                /* Pickup the next thread to evaluate.  */
                next_thread =  possible_preemption_list[k];

                /* Is this thread lower priority?  */
                if (next_thread -> tx_thread_priority > thread_ptr -> tx_thread_priority)
                {

                    /* Yes, swap the threads.  */
                    possible_preemption_list[i] =  next_thread;
                    possible_preemption_list[k] =  thread_ptr;
                    thread_ptr =  next_thread;
                }
                else
                {

                    /* Compare the thread priorities.  */
                    if (next_thread -> tx_thread_priority == thread_ptr -> tx_thread_priority)
                    {

                        /* Equal priority threads...  see which is in the ready list first.  */
                        search_thread =   thread_ptr -> tx_thread_ready_next;

                        /* Pickup the list head.  */
                        list_head =  _tx_thread_priority_list[thread_ptr -> tx_thread_priority];

                        /* Now loop to see if the next thread is after the current thread preemption.  */
                        while (search_thread != list_head)
                        {

                            /* Have we found the next thread?  */
                            if (search_thread == next_thread)
                            {

                                /* Yes, swap the threads.  */
                                possible_preemption_list[i] =  next_thread;
                                possible_preemption_list[k] =  thread_ptr;
                                thread_ptr =  next_thread;
                                break;
                            }

                            /* Move to the next thread.  */
                            search_thread =  search_thread -> tx_thread_ready_next;
                        }
                    }

                    /* Move to examine the next possible preemptable thread.  */
                    k++;
                }
            }

            /* We have found the lowest priority thread to preempt, now find the next lowest.  */
            i++;
        }
        while (i < (j-((UINT) 1)));
    }

    /* Return the possible cores.  */
    return(possible_cores);
}

static INLINE_DECLARE VOID  _tx_thread_smp_simple_priority_change(TX_THREAD *thread_ptr, UINT new_priority)
{

UINT            priority;
ULONG           priority_bit;
TX_THREAD       *head_ptr;
TX_THREAD       *tail_ptr;
#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif

    /* Pickup the priority.  */
    priority =  thread_ptr -> tx_thread_priority;

    /* Determine if there are other threads at this priority that are
       ready.  */
    if (thread_ptr -> tx_thread_ready_next != thread_ptr)
    {

        /* Yes, there are other threads at this priority ready.  */

        /* Just remove this thread from the priority list.  */
        (thread_ptr -> tx_thread_ready_next) -> tx_thread_ready_previous =    thread_ptr -> tx_thread_ready_previous;
        (thread_ptr -> tx_thread_ready_previous) -> tx_thread_ready_next =    thread_ptr -> tx_thread_ready_next;

        /* Determine if this is the head of the priority list.  */
        if (_tx_thread_priority_list[priority] == thread_ptr)
        {

            /* Update the head pointer of this priority list.  */
            _tx_thread_priority_list[priority] =  thread_ptr -> tx_thread_ready_next;
        }
    }
    else
    {

        /* This is the only thread at this priority ready to run.  Set the head
           pointer to NULL.  */
        _tx_thread_priority_list[priority] =    TX_NULL;

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index into the bit map array.  */
        map_index =  priority/((UINT) 32);
#endif

        /* Clear this priority bit in the ready priority bit map.  */
        TX_MOD32_BIT_SET(priority, priority_bit)
        _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

        /* Determine if there are any other bits set in this priority map.  */
        if (_tx_thread_priority_maps[MAP_INDEX] == ((ULONG) 0))
        {

            /* No, clear the active bit to signify this priority map has nothing set.  */
            TX_DIV32_BIT_SET(priority, priority_bit)
            _tx_thread_priority_map_active =  _tx_thread_priority_map_active & (~(priority_bit));
        }
#endif
    }

    /* Determine if the actual thread priority should be setup, which is the
       case if the new priority is higher than the priority inheritance.  */
    if (new_priority < thread_ptr -> tx_thread_inherit_priority)
    {

        /* Change thread priority to the new user's priority.  */
        thread_ptr -> tx_thread_priority =           new_priority;
        thread_ptr -> tx_thread_preempt_threshold =  new_priority;
    }
    else
    {

        /* Change thread priority to the priority inheritance.  */
        thread_ptr -> tx_thread_priority =           thread_ptr -> tx_thread_inherit_priority;
        thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_inherit_priority;
    }

    /* Now, place the thread at the new priority level.  */

    /* Determine if there are other threads at this priority that are
       ready.  */
    head_ptr =  _tx_thread_priority_list[new_priority];
    if (head_ptr != TX_NULL)
    {

        /* Yes, there are other threads at this priority already ready.  */

        /* Just add this thread to the priority list.  */
        tail_ptr =                                 head_ptr -> tx_thread_ready_previous;
        tail_ptr -> tx_thread_ready_next =         thread_ptr;
        head_ptr -> tx_thread_ready_previous =     thread_ptr;
        thread_ptr -> tx_thread_ready_previous =   tail_ptr;
        thread_ptr -> tx_thread_ready_next =       head_ptr;
    }
    else
    {

        /* First thread at this priority ready.  Add to the front of the list.  */
        _tx_thread_priority_list[new_priority] =   thread_ptr;
        thread_ptr -> tx_thread_ready_next =       thread_ptr;
        thread_ptr -> tx_thread_ready_previous =   thread_ptr;

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index into the bit map array.  */
        map_index =  new_priority/((UINT) 32);

        /* Set the active bit to remember that the priority map has something set.  */
        TX_DIV32_BIT_SET(new_priority, priority_bit)
        _tx_thread_priority_map_active =  _tx_thread_priority_map_active | priority_bit;
#endif

        /* Or in the thread's priority bit.  */
        TX_MOD32_BIT_SET(new_priority, priority_bit)
        _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] | priority_bit;
    }
}
#else

/* In-line was disabled.  All of the above helper fuctions must be defined as actual functions.  */

UINT   _tx_thread_lowest_set_bit_calculate(ULONG map);
#define TX_LOWEST_SET_BIT_CALCULATE(m, b)   (b) =  _tx_thread_lowest_set_bit_calculate((m));

UINT   _tx_thread_smp_next_priority_find(UINT priority);
VOID   _tx_thread_smp_schedule_list_clear(void);
VOID   _tx_thread_smp_execute_list_clear(void);
VOID   _tx_thread_smp_schedule_list_setup(void);

#ifdef TX_THREAD_SMP_INTER_CORE_INTERRUPT
VOID   _tx_thread_smp_core_interrupt(TX_THREAD *thread_ptr, UINT current_core, UINT target_core);
#else
/* Define to whitespace.  */
#define _tx_thread_smp_core_interrupt(a,b,c)
#endif

#ifdef TX_THREAD_SMP_WAKEUP_LOGIC
VOID   _tx_thread_smp_core_wakeup(UINT current_core, UINT target_core);
#else
/* Define to whitespace.  */
#define _tx_thread_smp_core_wakeup(a,b)
#endif

VOID   _tx_thread_smp_execute_list_setup(UINT core_index);
ULONG  _tx_thread_smp_available_cores_get(void);
ULONG  _tx_thread_smp_possible_cores_get(void);
UINT   _tx_thread_smp_lowest_priority_get(void);
UINT   _tx_thread_smp_remap_solution_find(TX_THREAD *schedule_thread, ULONG available_cores, ULONG thread_possible_cores, ULONG test_possible_cores);
ULONG  _tx_thread_smp_preemptable_threads_get(UINT priority, TX_THREAD *possible_preemption_list[]);
VOID   _tx_thread_smp_simple_priority_change(TX_THREAD *thread_ptr, UINT new_priority);

#endif


#endif

#endif

