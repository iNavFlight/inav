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
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    tx_port.h                                         SMP/Linux/GCC     */
/*                                                           6.1.9        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains data type definitions that make the ThreadX      */
/*    real-time kernel function identically on a variety of different     */
/*    processor architectures.  For example, the size or number of bits   */
/*    in an "int" data type vary between microprocessor architectures and */
/*    even C compilers for the same microprocessor.  ThreadX does not     */
/*    directly use native C data types.  Instead, ThreadX creates its     */
/*    own special types that can be mapped to actual data types by this   */
/*    file to guarantee consistency in the interface and functionality.   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*  04-02-2021     Bhupendra Naphade        Modified comment(s),updated   */
/*                                            macro definition,           */
/*                                            resulting in version 6.1.6  */
/*  10-15-2021     William E. Lamie         Modified comment(s), added    */
/*                                            symbol ULONG64_DEFINED,     */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef TX_PORT_H
#define TX_PORT_H



/************* Define ThreadX SMP constants.  *************/

#define TX_DISABLE_INLINE


/* Define the ThreadX SMP maximum number of cores.  */

#ifndef TX_THREAD_SMP_MAX_CORES
#define TX_THREAD_SMP_MAX_CORES                 4
#endif



/* Define the ThreadX SMP core mask. */

#ifndef TX_THREAD_SMP_CORE_MASK
#define TX_THREAD_SMP_CORE_MASK                 0xF            /* Where bit 0 represents Core 0, bit 1 represents Core 1, etc.  */
#endif

/* Define dynamic number of cores option.  When commented out, the number of cores is static.  */

/* #define TX_THREAD_SMP_DYNAMIC_CORE_MAX  */


/* Define ThreadX SMP initialization macro.  */

#define TX_PORT_SPECIFIC_PRE_INITIALIZATION


/* Enable the inter-core interrupt logic.  */

#define TX_THREAD_SMP_INTER_CORE_INTERRUPT


/* Determine if there is customer-specific wakeup logic needed.  */

#ifdef TX_THREAD_SMP_WAKEUP_LOGIC

/* Include customer-specific wakeup code.  */

#include "tx_thread_smp_core_wakeup.h"
#else

#ifdef TX_THREAD_SMP_DEFAULT_WAKEUP_LOGIC

/* Default wakeup code.  */
#define TX_THREAD_SMP_WAKEUP_LOGIC
#define TX_THREAD_SMP_WAKEUP(i)                _tx_thread_smp_core_preempt(i)
#endif
#endif


/* Ensure that the in-line resume/suspend define is not allowed.  */

#ifdef TX_INLINE_THREAD_RESUME_SUSPEND
#undef TX_INLINE_THREAD_RESUME_SUSPEND
#endif


/* Overide inline keyword.  */

#define INLINE_DECLARE  __inline


/************* End ThreadX SMP constants.  *************/


/* Determine if the optional ThreadX user define file should be used.  */

#ifdef TX_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in tx_user.h. The defines in this file may
   alternately be defined on the command line.  */

#include "tx_user.h"
#endif


/* Define compiler library include files.  */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#undef __USE_POSIX199309
#else /* __USE_POSIX199309 */
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#endif /* __USE_POSIX199309 */


/* Define ThreadX basic types for this port.  */

typedef void                                    VOID;
typedef char                                    CHAR;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;
typedef uint64_t                                ULONG64;
#define ULONG64_DEFINED


/* Define automated coverage test extensions...  These are required for the 
   ThreadX regression test.  */

typedef unsigned int    TEST_FLAG;
extern TEST_FLAG        threadx_byte_allocate_loop_test;
extern TEST_FLAG        threadx_byte_release_loop_test;
extern TEST_FLAG        threadx_mutex_suspension_put_test;
extern TEST_FLAG        threadx_mutex_suspension_priority_test;
#ifndef TX_TIMER_PROCESS_IN_ISR
extern TEST_FLAG        threadx_delete_timer_thread;
#endif

extern void             abort_and_resume_byte_allocating_thread(void);
extern void             abort_all_threads_suspended_on_mutex(void);
extern void             suspend_lowest_priority(void);
#ifndef TX_TIMER_PROCESS_IN_ISR
extern void             delete_timer_thread(void);
#endif
extern TEST_FLAG        test_stack_analyze_flag;
extern TEST_FLAG        test_initialize_flag;
extern TEST_FLAG        test_forced_mutex_timeout;
extern UINT             mutex_priority_change_extension_selection;
extern UINT             priority_change_extension_selection;


#ifdef TX_REGRESSION_TEST

/* Define extension macros for automated coverage tests.  */


#define TX_PORT_SPECIFIC_MEMORY_SYNCHRONIZATION other_core_status =  other_core_status + _tx_thread_system_state[0]; \
                                                _tx_thread_system_state[0] =  0;


#define TX_BYTE_ALLOCATE_EXTENSION              if (threadx_byte_allocate_loop_test == ((TEST_FLAG) 1))         \
                                                {                                                               \
                                                    pool_ptr -> tx_byte_pool_owner =  TX_NULL;                  \
                                                    threadx_byte_allocate_loop_test = ((TEST_FLAG) 0);          \
                                                }

#define TX_BYTE_RELEASE_EXTENSION               if (threadx_byte_release_loop_test == ((TEST_FLAG) 1))          \
                                                {                                                               \
                                                    threadx_byte_release_loop_test = ((TEST_FLAG) 0);           \
                                                    abort_and_resume_byte_allocating_thread();                  \
                                                }

#define TX_MUTEX_PUT_EXTENSION_1                if (threadx_mutex_suspension_put_test == ((TEST_FLAG) 1))       \
                                                {                                                               \
                                                    threadx_mutex_suspension_put_test = ((TEST_FLAG) 0);        \
                                                    abort_all_threads_suspended_on_mutex();                     \
                                                }


#define TX_MUTEX_PUT_EXTENSION_2                if (test_forced_mutex_timeout == ((TEST_FLAG) 1))               \
                                                {                                                               \
                                                    test_forced_mutex_timeout = ((TEST_FLAG) 0);                \
                                                    _tx_thread_wait_abort(mutex_ptr -> tx_mutex_suspension_list); \
                                                }


#define TX_MUTEX_PRIORITY_CHANGE_EXTENSION      if (threadx_mutex_suspension_priority_test == ((TEST_FLAG) 1))  \
                                                {                                                               \
                                                    threadx_mutex_suspension_priority_test = ((TEST_FLAG) 0);   \
                                                    if (mutex_priority_change_extension_selection == 2)         \
                                                        original_priority = new_priority;                       \
                                                    if (mutex_priority_change_extension_selection == 3)         \
                                                        original_pt_thread =  thread_ptr;                       \
                                                    if (mutex_priority_change_extension_selection == 4)         \
                                                    {                                                           \
                                                        execute_ptr =  thread_ptr;                              \
                                                        _tx_thread_preemption__threshold_scheduled = TX_NULL;   \
                                                    }                                                           \
                                                    suspend_lowest_priority();                                  \
                                                }

#define TX_THREAD_PRIORITY_CHANGE_EXTENSION     if (priority_change_extension_selection != ((TEST_FLAG) 0))     \
                                                {                                                               \
                                                    if (priority_change_extension_selection == 1)               \
                                                        thread_ptr -> tx_thread_smp_core_mapped =  TX_THREAD_SMP_MAX_CORES; \
                                                    else if (priority_change_extension_selection == 2)          \
                                                    {                                                           \
                                                        original_priority =  new_priority;                      \
                                                        _tx_thread_execute_ptr[0] =  TX_NULL;                   \
                                                    }                                                           \
                                                    else if (priority_change_extension_selection == 3)          \
                                                    {                                                           \
                                                        original_pt_thread =  thread_ptr;                       \
                                                    }                                                           \
                                                    else                                                        \
                                                    {                                                           \
                                                        _tx_thread_preemption__threshold_scheduled = TX_NULL;   \
                                                    }                                                           \
                                                    priority_change_extension_selection =  0;                   \
                                                }


#ifndef TX_TIMER_PROCESS_IN_ISR

#define TX_TIMER_INITIALIZE_EXTENSION(a)        if (threadx_delete_timer_thread == ((TEST_FLAG) 1))             \
                                                {                                                               \
                                                    threadx_delete_timer_thread = ((TEST_FLAG) 0);              \
                                                    delete_timer_thread();                                      \
                                                    (a) =  ((UINT) 1);                                          \
                                                }

#endif

#define TX_THREAD_STACK_ANALYZE_EXTENSION       if (test_stack_analyze_flag == ((TEST_FLAG) 1))                 \
                                                {                                                               \
                                                    thread_ptr -> tx_thread_id =  ((TEST_FLAG) 0);              \
                                                    test_stack_analyze_flag =     ((TEST_FLAG) 0);              \
                                                }                                                               \
                                                else if (test_stack_analyze_flag == ((TEST_FLAG) 2))            \
                                                {                                                               \
                                                    stack_ptr =  thread_ptr -> tx_thread_stack_start;           \
                                                    test_stack_analyze_flag =     ((TEST_FLAG) 0);              \
                                                }                                                               \
                                                else if (test_stack_analyze_flag == ((TEST_FLAG) 3))            \
                                                {                                                               \
                                                    *stack_ptr =  TX_STACK_FILL;                                \
                                                    test_stack_analyze_flag =     ((TEST_FLAG) 0);              \
                                                }                                                               \
                                                else                                                            \
                                                {                                                               \
                                                    test_stack_analyze_flag =     ((TEST_FLAG) 0);              \
                                                }                                                               

#define TX_INITIALIZE_KERNEL_ENTER_EXTENSION    if (test_initialize_flag == ((TEST_FLAG) 1))                    \
                                                {                                                               \
                                                    test_initialize_flag =  ((TEST_FLAG) 0);                    \
                                                    return;                                                     \
                                                }

#endif


/* Add Linux debug insert prototype.  */

void    _tx_linux_debug_entry_insert(char *action, char *file, unsigned long line);

#ifndef TX_LINUX_DEBUG_ENABLE

/* If Linux debug is not enabled, turn logging into white-space.  */

#define _tx_linux_debug_entry_insert(a, b, c)

#endif



/* Define the TX_MEMSET macro to remove library reference.  */

#ifndef TX_MISRA_ENABLE
#define TX_MEMSET(a,b,c)                        {                                       \
                                                UCHAR *ptr;                             \
                                                UCHAR value;                            \
                                                UINT  i, size;                          \
                                                    ptr =    (UCHAR *) ((VOID *) a);    \
                                                    value =  (UCHAR) b;                 \
                                                    size =   (UINT) c;                  \
                                                    for (i = 0; i < size; i++)          \
                                                    {                                   \
                                                        *ptr++ =  value;                \
                                                    }                                   \
                                                }
#endif


/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */

#ifndef TX_MAX_PRIORITIES
#define TX_MAX_PRIORITIES                       32
#endif


/* Define the minimum stack for a ThreadX thread on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */

#ifndef TX_MINIMUM_STACK
#define TX_MINIMUM_STACK                        200         /* Minimum stack size for this port */
#endif


/* Define the system timer thread's default stack size and priority.  These are only applicable
   if TX_TIMER_PROCESS_IN_ISR is not defined.  */

#ifndef TX_TIMER_THREAD_STACK_SIZE
#define TX_TIMER_THREAD_STACK_SIZE              400         /* Default timer thread stack size - Not used in Linux port!  */
#endif

#ifndef TX_TIMER_THREAD_PRIORITY
#define TX_TIMER_THREAD_PRIORITY                0           /* Default timer thread priority    */
#endif


/* Define various constants for the ThreadX  port.  */

#define TX_INT_DISABLE                          1           /* Disable interrupts               */
#define TX_INT_ENABLE                           0           /* Enable interrupts                */


/* Define the clock source for trace event entry time stamp. The following two item are port specific.
   For example, if the time source is at the address 0x0a800024 and is 16-bits in size, the clock
   source constants would be:

#define TX_TRACE_TIME_SOURCE                    *((ULONG *) 0x0a800024)
#define TX_TRACE_TIME_MASK                      0x0000FFFFUL

*/

#ifndef TX_MISRA_ENABLE
#ifndef TX_TRACE_TIME_SOURCE
#define TX_TRACE_TIME_SOURCE                    ((ULONG) (_tx_linux_time_stamp.tv_nsec))
#endif
#else
ULONG   _tx_misra_time_stamp_get(VOID);
#define TX_TRACE_TIME_SOURCE                    _tx_misra_time_stamp_get()
#endif

#ifndef TX_TRACE_TIME_MASK
#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL
#endif


/* Define the port-specific trace extension to pickup the Windows timer.  */

#define TX_TRACE_PORT_EXTENSION                 clock_gettime(CLOCK_REALTIME, &_tx_linux_time_stamp);


/* Define the port specific options for the _tx_build_options variable. This variable indicates
   how the ThreadX library was built.  */

#define TX_PORT_SPECIFIC_BUILD_OPTIONS          0


/* Define the in-line initialization constant so that modules with in-line
   initialization capabilities can prevent their initialization from being
   a function call.  */

#ifdef TX_MISRA_ENABLE
#define TX_DISABLE_INLINE
#else
#define TX_INLINE_INITIALIZATION
#endif


/* Define the Linux-specific initialization code that is expanded in the generic source.  */

void    _tx_initialize_start_interrupts(void);


#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION                       _tx_initialize_start_interrupts();                  \
                                                                            {                                                   \
                                                                            UINT k;                                             \
                                                                                for (k = 1; k < TX_THREAD_SMP_MAX_CORES; k++)   \
                                                                                {                                               \
                                                                                    _tx_thread_system_state[k] =  0;            \
                                                                                }                                               \
                                                                            }

/* Determine whether or not stack checking is enabled. By default, ThreadX stack checking is
   disabled. When the following is defined, ThreadX thread stack checking is enabled.  If stack
   checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking
   logic.  */

#ifndef TX_MISRA_ENABLE
#ifdef TX_ENABLE_STACK_CHECKING
#undef TX_DISABLE_STACK_FILLING
#endif
#endif


/* Define the TX_THREAD control block extensions for this port. The main reason
   for the multiple macros is so that backward compatibility can be maintained with
   existing ThreadX kernel awareness modules.  */

#define TX_THREAD_EXTENSION_0                                               pthread_t   tx_thread_linux_thread_id; \
                                                                            sem_t       tx_thread_linux_thread_run_semaphore; \
                                                                            UINT        tx_thread_linux_suspension_type; \
                                                                            UINT        tx_thread_linux_mutex_access; \
                                                                            UINT        tx_thread_linux_int_disabled_flag; \
                                                                            UINT        tx_thread_linux_deferred_preempt; \
                                                                            UINT        tx_thread_linux_virtual_core;

#define TX_THREAD_EXTENSION_1
#define TX_THREAD_EXTENSION_2
#define TX_THREAD_EXTENSION_3


/* Define the port extensions of the remaining ThreadX objects.  */

#define TX_BLOCK_POOL_EXTENSION
#define TX_BYTE_POOL_EXTENSION
#define TX_EVENT_FLAGS_GROUP_EXTENSION
#define TX_MUTEX_EXTENSION
#define TX_QUEUE_EXTENSION
#define TX_SEMAPHORE_EXTENSION
#define TX_TIMER_EXTENSION


/* Define the user extension field of the thread control block.  Nothing
   additional is needed for this port so it is defined as white space.  */

#ifndef TX_THREAD_USER_EXTENSION
#define TX_THREAD_USER_EXTENSION
#endif


/* Define the macros for processing extensions in tx_thread_create, tx_thread_delete,
   tx_thread_shell_entry, and tx_thread_terminate.  */


#define TX_THREAD_CREATE_EXTENSION(thread_ptr)
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)
#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)
#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)


/* Define the ThreadX object creation extensions for the remaining objects.  */

#define TX_BLOCK_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION(group_ptr)
#define TX_MUTEX_CREATE_EXTENSION(mutex_ptr)
#define TX_QUEUE_CREATE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)
#define TX_TIMER_CREATE_EXTENSION(timer_ptr)


/* Define the Linux mutex data structure.  */

typedef struct
{
    pthread_mutex_t tx_linux_mutex;
    pthread_t       tx_linux_mutex_owner;
    ULONG           tx_linux_mutex_nested_count;
} TX_LINUX_MUTEX;


/* Define Linux-specific critical section APIs.  */

void _tx_linux_mutex_obtain(TX_LINUX_MUTEX *mutex);
void _tx_linux_mutex_release(TX_LINUX_MUTEX *mutex);
void _tx_linux_mutex_release_all(TX_LINUX_MUTEX *mutex);

typedef struct TX_THREAD_STRUCT TX_THREAD;

/* Define post completion processing for tx_thread_delete, so that the Linux thread resources are properly removed.  */

void _tx_thread_delete_port_completion(TX_THREAD *thread_ptr, UINT tx_interrupt_save);
#define TX_THREAD_DELETE_PORT_COMPLETION(thread_ptr) _tx_thread_delete_port_completion(thread_ptr, tx_interrupt_save);


/* Define post completion processing for tx_thread_reset, so that the Linux thread resources are properly removed.  */

void _tx_thread_reset_port_completion(TX_THREAD *thread_ptr, UINT tx_interrupt_save);
#define TX_THREAD_RESET_PORT_COMPLETION(thread_ptr) _tx_thread_reset_port_completion(thread_ptr, tx_interrupt_save);


/************* Define ThreadX SMP data types and function prototypes.  *************/

struct TX_THREAD_STRUCT;


/* Define the ThreadX SMP protection structure.   */

typedef struct TX_THREAD_SMP_PROTECT_STRUCT
{
    ULONG                   tx_thread_smp_protect_in_force;
    struct TX_THREAD_STRUCT *tx_thread_smp_protect_thread;
    ULONG                   tx_thread_smp_protect_core;
    ULONG                   tx_thread_smp_protect_count;
    pthread_t               tx_thread_smp_protect_linux_thread_id;
} TX_THREAD_SMP_PROTECT;


/* Define the virtual core structure for ThreadX SMP Linux.  This is where we keep the mapping of the core to
   the actual thread running.  All ISRs are assumed to be running on core 0 for Linux.  */

typedef struct TX_THREAD_SMP_CORE_MAPPING_STRUCT
{
    pthread_t               tx_thread_smp_core_mapping_linux_thread_id;
    struct TX_THREAD_STRUCT *tx_thread_smp_core_mapping_thread;
} TX_THREAD_SMP_CORE_MAPPING;


/* Define ThreadX SMP low-level assembly routines.   */

struct TX_THREAD_STRUCT *   _tx_thread_smp_current_thread_get(void);
UINT                        _tx_thread_smp_protect(void);
void                        _tx_thread_smp_unprotect(UINT interrupt_save);
ULONG                       _tx_thread_smp_current_state_get(void);
ULONG                       _tx_thread_smp_time_get(void);


/* Determine if SMP Debug is selected.  If so, the function prototype is setup. Otherwise, the debug call is
   simply mapped to whitespace.  */

#ifdef TX_THREAD_SMP_DEBUG_ENABLE
void                        _tx_thread_smp_debug_entry_insert(ULONG id, ULONG suspend, VOID *thread_ptr);
#else
#define                     _tx_thread_smp_debug_entry_insert(a, b, c)
#endif


/* Define the get core ID macro.  */

#define TX_SMP_CORE_ID                          _tx_thread_smp_core_get()




/* Define the ThreadX object deletion extensions for the remaining objects.  */

#define TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)
#define TX_MUTEX_DELETE_EXTENSION(mutex_ptr)
#define TX_QUEUE_DELETE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)
#define TX_TIMER_DELETE_EXTENSION(timer_ptr)


/* Define ThreadX interrupt lockout and restore macros for protection on
   access of critical kernel information.  The restore interrupt macro must
   restore the interrupt posture of the running thread prior to the value
   present prior to the disable macro.  In most cases, the save area macro
   is used to define a local function save area for the disable and restore
   macros.  */

#define TX_INTERRUPT_SAVE_AREA                  UINT tx_interrupt_save;

#define TX_DISABLE                              tx_interrupt_save =  _tx_thread_smp_protect();
#define TX_RESTORE                              _tx_thread_smp_unprotect(tx_interrupt_save);


/************* End ThreadX SMP data type and function prototype definitions.  *************/


#define tx_linux_sem_post(p)                sem_post(p)
#define tx_linux_sem_wait(p)                sem_wait(p)
#define tx_linux_sem_timedwait(p, t)        sem_timedwait(p, t)


/* Define the interrupt lockout macros for each ThreadX object.  */

#define TX_BLOCK_POOL_DISABLE               TX_DISABLE
#define TX_BYTE_POOL_DISABLE                TX_DISABLE
#define TX_EVENT_FLAGS_GROUP_DISABLE        TX_DISABLE
#define TX_MUTEX_DISABLE                    TX_DISABLE
#define TX_QUEUE_DISABLE                    TX_DISABLE
#define TX_SEMAPHORE_DISABLE                TX_DISABLE


/* Define the version ID of ThreadX.  This may be utilized by the application.  */

#ifdef TX_THREAD_INIT
CHAR                            _tx_version_id[] =
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  ThreadX SMP/Linux/gcc Version 6.1.9 *";
#else
extern  CHAR                    _tx_version_id[];
#endif


/* Define externals for the Linux port of ThreadX.  */

extern TX_LINUX_MUTEX                           _tx_linux_mutex;
extern sem_t                                    _tx_linux_scheduler_semaphore;
extern pthread_t                                _tx_linux_scheduler_id;
extern ULONG                                    _tx_linux_global_int_disabled_flag;
extern struct timespec                          _tx_linux_time_stamp;
extern ULONG                                    _tx_linux_system_error;
extern TX_THREAD_SMP_CORE_MAPPING               _tx_linux_virtual_cores[TX_THREAD_SMP_MAX_CORES];
extern __thread int                             _tx_linux_threadx_thread;

/* Define functions for linux thread. */
void    _tx_linux_thread_suspend(pthread_t thread_id);
void    _tx_linux_thread_resume(pthread_t thread_id);
void    _tx_linux_thread_init();
void    _tx_linux_thread_sleep(long ns);

#ifndef TX_LINUX_MEMORY_SIZE
#define TX_LINUX_MEMORY_SIZE                    100000
#endif

#ifndef TX_TIMER_TICKS_PER_SECOND
#define TX_TIMER_TICKS_PER_SECOND               100UL
#endif

#ifndef TX_LINUX_THREAD_STACK_SIZE
#define TX_LINUX_THREAD_STACK_SIZE              65536
#endif

/* Define priorities of pthreads. */
#define TX_LINUX_PRIORITY_SCHEDULE              (3)
#define TX_LINUX_PRIORITY_ISR                   (2)
#define TX_LINUX_PRIORITY_USER_THREAD           (1)

#endif







