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
/**   Application Interface (API)                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    tx_api.h                                            PORTABLE SMP    */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance ThreadX real-time kernel.  All service prototypes  */
/*    and data structure definitions are defined in this file.            */
/*    Please note that basic data type definitions and other architecture-*/
/*    specific information is contained in the file tx_port.h.            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      William E. Lamie        Initial Version 6.1           */
/*  10-16-2020      William E. Lamie        Modified comment(s), and      */
/*                                            increased patch version,    */
/*                                            resulting in version 6.1.1  */
/*  12-31-2020      William E. Lamie        Modified comment(s), and      */
/*                                            increased patch version,    */
/*                                            resulting in version 6.1.3  */
/*  03-02-2021      Scott Larson            Modified comment(s), and      */
/*                                            order defines numerically,  */
/*                                            add option to remove FileX  */
/*                                            pointer, fix whitespace,    */
/*                                            resulting in version 6.1.5  */
/*  04-02-2021      Scott Larson            Modified comment(s), and      */
/*                                            update patch number,        */
/*                                            resulting in version 6.1.6  */
/*  06-02-2021      Scott Larson            Added options for multiple    */
/*                                            block pool search & delay,  */
/*                                            resulting in version 6.1.7  */
/*  08-02-2021      Scott Larson            Modified comment(s), and      */
/*                                            update patch number,        */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021      Yuxin Zhou              Modified comment(s),          */
/*                                            update patch number,        */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022      Scott Larson            Modified comment(s),          */
/*                                            add unused parameter macro, */
/*                                            update patch number,        */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022      Wenhui Xie              Modified comment(s),          */
/*                                            optimized the definition of */
/*                                            TX_TIMER_TICKS_PER_SECOND,  */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022      Scott Larson            Modified comment(s),          */
/*                                            update patch number,        */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022      Scott Larson            Modified comment(s),          */
/*                                            add extension macros,       */
/*                                            update version numbers,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef TX_API_H
#define TX_API_H


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Disable warning of parameter not used. */
#ifndef TX_PARAMETER_NOT_USED
#define TX_PARAMETER_NOT_USED(p) ((void)(p))
#endif /* TX_PARAMETER_NOT_USED */

/* Include the port-specific data type file.  */

#include "tx_port.h"


/* If not defined by tx_port.h, default the port-specific memory synchronization macro to whitespace.  */

#ifndef TX_PORT_SPECIFIC_MEMORY_SYNCHRONIZATION
#define TX_PORT_SPECIFIC_MEMORY_SYNCHRONIZATION
#endif


/* Define default block pool search and delay values.  */
#ifndef TX_BYTE_POOL_MULTIPLE_BLOCK_SEARCH
#define TX_BYTE_POOL_MULTIPLE_BLOCK_SEARCH    20
#endif
#ifndef TX_BTYE_POOL_DELAY_VALUE
#define TX_BYTE_POOL_DELAY_VALUE              3
#endif


/* Define basic constants for the ThreadX kernel.  */


/* Define the major/minor version information that can be used by the application
   and the ThreadX source as well.  */

#define AZURE_RTOS_THREADX
#define THREADX_MAJOR_VERSION           6
#define THREADX_MINOR_VERSION           2
#define THREADX_PATCH_VERSION           0

/* Define the following symbol for backward compatibility */
#define EL_PRODUCT_THREADX


/* API input parameters and general constants.  */

#define TX_NO_WAIT                      ((ULONG)  0)
#define TX_WAIT_FOREVER                 ((ULONG)  0xFFFFFFFFUL)
#define TX_AND                          ((UINT)   2)
#define TX_AND_CLEAR                    ((UINT)   3)
#define TX_OR                           ((UINT)   0)
#define TX_OR_CLEAR                     ((UINT)   1)
#define TX_1_ULONG                      ((UINT)   1)
#define TX_2_ULONG                      ((UINT)   2)
#define TX_4_ULONG                      ((UINT)   4)
#define TX_8_ULONG                      ((UINT)   8)
#define TX_16_ULONG                     ((UINT)   16)
#define TX_NO_TIME_SLICE                ((ULONG)  0)
#define TX_AUTO_START                   ((UINT)   1)
#define TX_DONT_START                   ((UINT)   0)
#define TX_AUTO_ACTIVATE                ((UINT)   1)
#define TX_NO_ACTIVATE                  ((UINT)   0)
#define TX_TRUE                         ((UINT)   1)
#define TX_FALSE                        ((UINT)   0)
#define TX_NULL                         ((void *) 0)
#define TX_INHERIT                      ((UINT)   1)
#define TX_NO_INHERIT                   ((UINT)   0)
#define TX_THREAD_ENTRY                 ((UINT)   0)
#define TX_THREAD_EXIT                  ((UINT)   1)
#define TX_NO_SUSPENSIONS               ((UINT)   0)
#define TX_NO_MESSAGES                  ((UINT)   0)
#define TX_EMPTY                        ((ULONG)  0)
#define TX_CLEAR_ID                     ((ULONG)  0)
#define TX_STACK_FILL                   ((ULONG)  0xEFEFEFEFUL)


/* Thread execution state values.  */

#define TX_READY                        ((UINT) 0)
#define TX_COMPLETED                    ((UINT) 1)
#define TX_TERMINATED                   ((UINT) 2)
#define TX_SUSPENDED                    ((UINT) 3)
#define TX_SLEEP                        ((UINT) 4)
#define TX_QUEUE_SUSP                   ((UINT) 5)
#define TX_SEMAPHORE_SUSP               ((UINT) 6)
#define TX_EVENT_FLAG                   ((UINT) 7)
#define TX_BLOCK_MEMORY                 ((UINT) 8)
#define TX_BYTE_MEMORY                  ((UINT) 9)
#define TX_IO_DRIVER                    ((UINT) 10)
#define TX_FILE                         ((UINT) 11)
#define TX_TCP_IP                       ((UINT) 12)
#define TX_MUTEX_SUSP                   ((UINT) 13)
#define TX_PRIORITY_CHANGE              ((UINT) 14)


/* API return values.  */

#define TX_SUCCESS                      ((UINT) 0x00)
#define TX_DELETED                      ((UINT) 0x01)
#define TX_POOL_ERROR                   ((UINT) 0x02)
#define TX_PTR_ERROR                    ((UINT) 0x03)
#define TX_WAIT_ERROR                   ((UINT) 0x04)
#define TX_SIZE_ERROR                   ((UINT) 0x05)
#define TX_GROUP_ERROR                  ((UINT) 0x06)
#define TX_NO_EVENTS                    ((UINT) 0x07)
#define TX_OPTION_ERROR                 ((UINT) 0x08)
#define TX_QUEUE_ERROR                  ((UINT) 0x09)
#define TX_QUEUE_EMPTY                  ((UINT) 0x0A)
#define TX_QUEUE_FULL                   ((UINT) 0x0B)
#define TX_SEMAPHORE_ERROR              ((UINT) 0x0C)
#define TX_NO_INSTANCE                  ((UINT) 0x0D)
#define TX_THREAD_ERROR                 ((UINT) 0x0E)
#define TX_PRIORITY_ERROR               ((UINT) 0x0F)
#define TX_NO_MEMORY                    ((UINT) 0x10)
#define TX_START_ERROR                  ((UINT) 0x10)
#define TX_DELETE_ERROR                 ((UINT) 0x11)
#define TX_RESUME_ERROR                 ((UINT) 0x12)
#define TX_CALLER_ERROR                 ((UINT) 0x13)
#define TX_SUSPEND_ERROR                ((UINT) 0x14)
#define TX_TIMER_ERROR                  ((UINT) 0x15)
#define TX_TICK_ERROR                   ((UINT) 0x16)
#define TX_ACTIVATE_ERROR               ((UINT) 0x17)
#define TX_THRESH_ERROR                 ((UINT) 0x18)
#define TX_SUSPEND_LIFTED               ((UINT) 0x19)
#define TX_WAIT_ABORTED                 ((UINT) 0x1A)
#define TX_WAIT_ABORT_ERROR             ((UINT) 0x1B)
#define TX_MUTEX_ERROR                  ((UINT) 0x1C)
#define TX_NOT_AVAILABLE                ((UINT) 0x1D)
#define TX_NOT_OWNED                    ((UINT) 0x1E)
#define TX_INHERIT_ERROR                ((UINT) 0x1F)
#define TX_NOT_DONE                     ((UINT) 0x20)
#define TX_CEILING_EXCEEDED             ((UINT) 0x21)
#define TX_INVALID_CEILING              ((UINT) 0x22)
#define TX_FEATURE_NOT_ENABLED          ((UINT) 0xFF)


#ifdef TX_64_BIT

#ifndef TX_THREAD_EXTENSION_PTR_SET
#define TX_THREAD_EXTENSION_PTR_SET(a, b)                   { \
                                                                TX_THREAD *thread_ptr; \
                                                                thread_ptr = (TX_THREAD *) (a); \
                                                                (thread_ptr -> tx_thread_extension_ptr) = (VOID *)(b); \
                                                            }
#endif /* TX_THREAD_EXTENSION_PTR_SET  */

#ifndef TX_THREAD_EXTENSION_PTR_GET
#define TX_THREAD_EXTENSION_PTR_GET(a, b, c)                { \
                                                                TX_PARAMETER_NOT_USED(c); \
                                                                TX_THREAD *thread_ptr; \
                                                                thread_ptr = tx_thread_identify(); \
                                                                while(1)\
                                                                { \
                                                                    if (thread_ptr -> tx_thread_extension_ptr) \
                                                                    { \
                                                                        (a) = (b *)(thread_ptr -> tx_thread_extension_ptr); \
                                                                        break; \
                                                                    } \
                                                                    tx_thread_sleep(1); \
                                                                } \
                                                            }
#endif /* TX_THREAD_EXTENSION_PTR_GET  */

#ifndef TX_TIMER_EXTENSION_PTR_SET
#define TX_TIMER_EXTENSION_PTR_SET(a, b)                    { \
                                                                TX_TIMER *timer_ptr; \
                                                                timer_ptr = (TX_TIMER *) (a);   \
                                                                (timer_ptr -> tx_timer_internal.tx_timer_internal_extension_ptr) = (VOID *)(b); \
                                                            }
#endif /* TX_TIMER_EXTENSION_PTR_SET  */

#ifndef TX_TIMER_EXTENSION_PTR_GET
#define TX_TIMER_EXTENSION_PTR_GET(a, b, c)                 { \
                                                                TX_PARAMETER_NOT_USED(c); \
                                                                if (!_tx_timer_expired_timer_ptr -> tx_timer_internal_extension_ptr) \
                                                                    return; \
                                                                (a) = (b *)(_tx_timer_expired_timer_ptr -> tx_timer_internal_extension_ptr); \
                                                            }
#endif /* TX_TIMER_EXTENSION_PTR_GET  */

#else   /* not 64 bit */

#ifndef TX_THREAD_EXTENSION_PTR_SET
#define TX_THREAD_EXTENSION_PTR_SET(a, b)
#endif /* TX_THREAD_EXTENSION_PTR_SET  */

#ifndef TX_THREAD_EXTENSION_PTR_GET
#define TX_THREAD_EXTENSION_PTR_GET(a, b, c)                { \
                                                                (a) = (b *)(c); \
                                                            }
#endif /* TX_THREAD_EXTENSION_PTR_GET  */

#ifndef TX_TIMER_EXTENSION_PTR_SET
#define TX_TIMER_EXTENSION_PTR_SET(a, b)
#endif /* TX_TIMER_EXTENSION_PTR_SET  */

#ifndef TX_TIMER_EXTENSION_PTR_GET
#define TX_TIMER_EXTENSION_PTR_GET(a, b, c)                 { \
                                                                (a) = (b *)(c); \
                                                            }
#endif /* TX_TIMER_EXTENSION_PTR_GET  */

#endif  /* TX_64_BIT */



/* Define the common timer tick reference for use by other middleware components. The default
   value is 10ms, but may be replaced by a port specific version in tx_port.h or by the user
   as a compilation option.  */

#ifndef TX_TIMER_TICKS_PER_SECOND
#define TX_TIMER_TICKS_PER_SECOND       (100UL)
#endif


/* Event numbers 0 through 4095 are reserved by Azure RTOS. Specific event assignments are:

                                ThreadX events:     1-199
                                FileX events:       200-299
                                NetX events:        300-599
                                USBX events:        600-999
                                GUIX events:        1000-1500

   User-defined event numbers start at 4096 and continue through 65535, as defined by the constants
   TX_TRACE_USER_EVENT_START and TX_TRACE_USER_EVENT_END, respectively. User events should be based
   on these constants in case the user event number assignment is changed in future releases.  */

#define TX_TRACE_USER_EVENT_START           4096            /* I1, I2, I3, I4 are user defined           */
#define TX_TRACE_USER_EVENT_END             65535           /* I1, I2, I3, I4 are user defined           */


/* Define event filters that can be used to selectively disable certain events or groups of events.  */

#define TX_TRACE_ALL_EVENTS                 0x000007FF      /* All ThreadX events                        */
#define TX_TRACE_INTERNAL_EVENTS            0x00000001      /* ThreadX internal events                   */
#define TX_TRACE_BLOCK_POOL_EVENTS          0x00000002      /* ThreadX Block Pool events                 */
#define TX_TRACE_BYTE_POOL_EVENTS           0x00000004      /* ThreadX Byte Pool events                  */
#define TX_TRACE_EVENT_FLAGS_EVENTS         0x00000008      /* ThreadX Event Flags events                */
#define TX_TRACE_INTERRUPT_CONTROL_EVENT    0x00000010      /* ThreadX Interrupt Control events          */
#define TX_TRACE_MUTEX_EVENTS               0x00000020      /* ThreadX Mutex events                      */
#define TX_TRACE_QUEUE_EVENTS               0x00000040      /* ThreadX Queue events                      */
#define TX_TRACE_SEMAPHORE_EVENTS           0x00000080      /* ThreadX Semaphore events                  */
#define TX_TRACE_THREAD_EVENTS              0x00000100      /* ThreadX Thread events                     */
#define TX_TRACE_TIME_EVENTS                0x00000200      /* ThreadX Time events                       */
#define TX_TRACE_TIMER_EVENTS               0x00000400      /* ThreadX Timer events                      */
#define TX_TRACE_USER_EVENTS                0x80000000UL    /* ThreadX User Events                       */


/* Define basic alignment type used in block and byte pool operations. This data type must
   be at least 32-bits in size and also be large enough to hold a pointer type.  */

#ifndef ALIGN_TYPE_DEFINED
#define ALIGN_TYPE      ULONG
#endif


/* Define the control block definitions for all system objects.  */


/* Define the basic timer management structures.  These are the structures
   used to manage thread sleep, timeout, and user timer requests.  */

/* Determine if the internal timer control block has an extension defined. If not,
   define the extension to whitespace.  */

#ifndef TX_TIMER_INTERNAL_EXTENSION
#define TX_TIMER_INTERNAL_EXTENSION
#endif


/* Define the common internal timer control block.  */

typedef struct TX_TIMER_INTERNAL_STRUCT
{

    /* Define the remaining ticks and re-initialization tick values.  */
    ULONG               tx_timer_internal_remaining_ticks;
    ULONG               tx_timer_internal_re_initialize_ticks;

    /* Define the timeout function and timeout function parameter.  */
    VOID                (*tx_timer_internal_timeout_function)(ULONG id);
    ULONG               tx_timer_internal_timeout_param;


    /* Define the next and previous internal link pointers for active
       internal timers.  */
    struct TX_TIMER_INTERNAL_STRUCT
                        *tx_timer_internal_active_next,
                        *tx_timer_internal_active_previous;

    /* Keep track of the pointer to the head of this list as well.  */
    struct TX_TIMER_INTERNAL_STRUCT
                        **tx_timer_internal_list_head;

    /************* Define ThreadX SMP timer control block extensions.  *************/

    /* Define the timer SMP core exclusion for timer callback.  */
    ULONG               tx_timer_internal_smp_cores_excluded;

    /************* End of ThreadX SMP timer control block extensions.  *************/

    /* Define optional extension to internal timer control block.  */
    TX_TIMER_INTERNAL_EXTENSION

} TX_TIMER_INTERNAL;


/* Determine if the timer control block has an extension defined. If not,
   define the extension to whitespace.  */

#ifndef TX_TIMER_EXTENSION
#define TX_TIMER_EXTENSION
#endif


/* Define the timer structure utilized by the application.  */

typedef struct TX_TIMER_STRUCT
{

    /* Define the timer ID used for error checking.  */
    ULONG               tx_timer_id;

    /* Define the timer's name.  */
    CHAR                *tx_timer_name;

    /* Define the actual contents of the timer.  This is the block that
       is used in the actual timer expiration processing.  */
    TX_TIMER_INTERNAL   tx_timer_internal;

    /* Define the pointers for the created list.  */
    struct TX_TIMER_STRUCT
                        *tx_timer_created_next,
                        *tx_timer_created_previous;

    /* Define optional extension to timer control block.  */
    TX_TIMER_EXTENSION

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

    /* Define the number of timer activations.  */
    ULONG               tx_timer_performance_activate_count;

    /* Define the number of timer reactivations.  */
    ULONG               tx_timer_performance_reactivate_count;

    /* Define the number of timer deactivations.  */
    ULONG               tx_timer_performance_deactivate_count;

    /* Define the number of timer expirations.  */
    ULONG               tx_timer_performance_expiration_count;

    /* Define the total number of timer expiration adjustments.  */
    ULONG               tx_timer_performance__expiration_adjust_count;
#endif

} TX_TIMER;


/* Determine if the thread priority change extension is defined. If not, define the
   extension to whitespace.  */

#ifndef TX_THREAD_PRIORITY_CHANGE_EXTENSION
#define TX_THREAD_PRIORITY_CHANGE_EXTENSION
#endif


/* ThreadX thread control block structure follows.  Additional fields
   can be added providing they are added after the information that is
   referenced in the port-specific assembly code.  */

typedef struct TX_THREAD_STRUCT
{
    /* The first section of the control block contains critical
       information that is referenced by the port-specific
       assembly language code.  Any changes in this section could
       necessitate changes in the assembly language.  */

    ULONG               tx_thread_id;                   /* Control block ID         */
    ULONG               tx_thread_run_count;            /* Thread's run counter     */
    VOID                *tx_thread_stack_ptr;           /* Thread's stack pointer   */
    VOID                *tx_thread_stack_start;         /* Stack starting address   */
    VOID                *tx_thread_stack_end;           /* Stack ending address     */
    ULONG               tx_thread_stack_size;           /* Stack size               */
    ULONG               tx_thread_time_slice;           /* Current time-slice       */
    ULONG               tx_thread_new_time_slice;       /* New time-slice           */

    /* Define pointers to the next and previous ready threads.  */
    struct TX_THREAD_STRUCT
                        *tx_thread_ready_next,
                        *tx_thread_ready_previous;

    /***************************************************************/

    /* Define the first port extension in the thread control block. This
       is typically defined to whitespace or a pointer type in tx_port.h.  */
    TX_THREAD_EXTENSION_0

    CHAR                *tx_thread_name;                /* Pointer to thread's name     */
    UINT                tx_thread_priority;             /* Priority of thread (0-1023)  */
    UINT                tx_thread_state;                /* Thread's execution state     */
    UINT                tx_thread_delayed_suspend;      /* Delayed suspend flag         */
    UINT                tx_thread_suspending;           /* Thread suspending flag       */
    UINT                tx_thread_preempt_threshold;    /* Preemption threshold         */

    /* Define the thread schedule hook. The usage of this is port/application specific,
       but when used, the function pointer designated is called whenever the thread is
       scheduled and unscheduled.  */
    VOID                (*tx_thread_schedule_hook)(struct TX_THREAD_STRUCT *thread_ptr, ULONG id);

    /* Nothing after this point is referenced by the target-specific
       assembly language.  Hence, information after this point can
       be added to the control block providing the complete system
       is recompiled.  */

    /* Define the thread's entry point and input parameter.  */
    VOID                (*tx_thread_entry)(ULONG id);
    ULONG               tx_thread_entry_parameter;

    /* Define the thread's timer block.   This is used for thread
       sleep and timeout requests.  */
    TX_TIMER_INTERNAL   tx_thread_timer;

    /* Define the thread's cleanup function and associated data.  This
       is used to cleanup various data structures when a thread
       suspension is lifted or terminated either by the user or
       a timeout.  */
    VOID                (*tx_thread_suspend_cleanup)(struct TX_THREAD_STRUCT *thread_ptr, ULONG suspension_sequence);
    VOID                *tx_thread_suspend_control_block;
    struct TX_THREAD_STRUCT
                        *tx_thread_suspended_next,
                        *tx_thread_suspended_previous;
    ULONG               tx_thread_suspend_info;
    VOID                *tx_thread_additional_suspend_info;
    UINT                tx_thread_suspend_option;
    UINT                tx_thread_suspend_status;

    /* Define the second port extension in the thread control block. This
       is typically defined to whitespace or a pointer type in tx_port.h.  */
    TX_THREAD_EXTENSION_1

    /* Define pointers to the next and previous threads in the
       created list.  */
    struct TX_THREAD_STRUCT
                        *tx_thread_created_next,
                        *tx_thread_created_previous;

    /************* Define ThreadX SMP thread control block extensions.  *************/

    UINT                tx_thread_smp_core_mapped;
    ULONG               tx_thread_smp_core_control;
    UINT                tx_thread_smp_core_executing;

    /************* End of ThreadX SMP thread control block extensions.  *************/

    /* Define the third port extension in the thread control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_THREAD_EXTENSION_2

    /************* Define ThreadX SMP thread control block extensions.  *************/

    ULONG               tx_thread_smp_cores_excluded;
    ULONG               tx_thread_smp_cores_allowed;
    ULONG               tx_thread_smp_lock_ready_bit;

    /************* End of ThreadX SMP thread control block extensions.  *************/

    /* Define a pointer type for FileX extensions.  */
#ifndef TX_NO_FILEX_POINTER
    VOID                *tx_thread_filex_ptr;
#endif

    /* Define the priority inheritance variables. These will be used
       to manage priority inheritance changes applied to this thread
       as a result of mutex get operations.  */
    UINT                tx_thread_user_priority;
    UINT                tx_thread_user_preempt_threshold;
    UINT                tx_thread_inherit_priority;

    /* Define the owned mutex count and list head pointer.  */
    UINT                tx_thread_owned_mutex_count;
    struct TX_MUTEX_STRUCT
                        *tx_thread_owned_mutex_list;

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

    /* Define the number of times this thread is resumed.  */
    ULONG               tx_thread_performance_resume_count;

    /* Define the number of times this thread suspends.  */
    ULONG               tx_thread_performance_suspend_count;

    /* Define the number of times this thread is preempted by calling
       a ThreadX API service.  */
    ULONG               tx_thread_performance_solicited_preemption_count;

    /* Define the number of times this thread is preempted by an
       ISR calling a ThreadX API service.  */
    ULONG               tx_thread_performance_interrupt_preemption_count;

    /* Define the number of priority inversions for this thread.  */
    ULONG               tx_thread_performance_priority_inversion_count;

    /* Define the last thread pointer to preempt this thread.  */
    struct TX_THREAD_STRUCT
                        *tx_thread_performance_last_preempting_thread;

    /* Define the total number of times this thread was time-sliced.  */
    ULONG               tx_thread_performance_time_slice_count;

    /* Define the total number of times this thread relinquishes.  */
    ULONG               tx_thread_performance_relinquish_count;

    /* Define the total number of times this thread had a timeout.  */
    ULONG               tx_thread_performance_timeout_count;

    /* Define the total number of times this thread had suspension lifted
       because of the tx_thread_wait_abort service.  */
    ULONG               tx_thread_performance_wait_abort_count;
#endif

    /* Define the highest stack pointer variable.  */
    VOID                *tx_thread_stack_highest_ptr;   /* Stack highest usage pointer  */


#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Define the application callback routine used to notify the application when
       the thread is entered or exits.  */
    VOID                (*tx_thread_entry_exit_notify)(struct TX_THREAD_STRUCT *thread_ptr, UINT type);
#endif

    /* Define the fourth port extension in the thread control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_THREAD_EXTENSION_3

    /* Define suspension sequence number.  This is used to ensure suspension is still valid when
       cleanup routine executes.  */
    ULONG               tx_thread_suspension_sequence;

    /* Define the user extension field.  This typically is defined
       to white space, but some ports of ThreadX may need to have
       additional fields in the thread control block.  This is
       defined in the file tx_port.h.  */
    TX_THREAD_USER_EXTENSION

} TX_THREAD;


/* Define the block memory pool structure utilized by the application.  */

typedef struct TX_BLOCK_POOL_STRUCT
{

    /* Define the block pool ID used for error checking.  */
    ULONG               tx_block_pool_id;

    /* Define the block pool's name.  */
    CHAR                *tx_block_pool_name;

    /* Define the number of available memory blocks in the pool.  */
    UINT                tx_block_pool_available;

    /* Save the initial number of blocks.  */
    UINT                tx_block_pool_total;

    /* Define the head pointer of the available block pool.  */
    UCHAR               *tx_block_pool_available_list;

    /* Save the start address of the block pool's memory area.  */
    UCHAR               *tx_block_pool_start;

    /* Save the block pool's size in bytes.  */
    ULONG               tx_block_pool_size;

    /* Save the individual memory block size - rounded for alignment.  */
    UINT                tx_block_pool_block_size;

    /* Define the block pool suspension list head along with a count of
       how many threads are suspended.  */
    struct TX_THREAD_STRUCT
                        *tx_block_pool_suspension_list;
    UINT                tx_block_pool_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct TX_BLOCK_POOL_STRUCT
                        *tx_block_pool_created_next,
                        *tx_block_pool_created_previous;

#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO

    /* Define the number of block allocates.  */
    ULONG               tx_block_pool_performance_allocate_count;

    /* Define the number of block releases.  */
    ULONG               tx_block_pool_performance_release_count;

    /* Define the number of block pool suspensions.  */
    ULONG               tx_block_pool_performance_suspension_count;

    /* Define the number of block pool timeouts.  */
    ULONG               tx_block_pool_performance_timeout_count;
#endif

    /* Define the port extension in the block pool control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_BLOCK_POOL_EXTENSION

} TX_BLOCK_POOL;


/* Determine if the byte allocate extension is defined. If not, define the
   extension to whitespace.  */

#ifndef TX_BYTE_ALLOCATE_EXTENSION
#define TX_BYTE_ALLOCATE_EXTENSION
#endif


/* Determine if the byte release extension is defined. If not, define the
   extension to whitespace.  */

#ifndef TX_BYTE_RELEASE_EXTENSION
#define TX_BYTE_RELEASE_EXTENSION
#endif


/* Define the byte memory pool structure utilized by the application.  */

typedef struct TX_BYTE_POOL_STRUCT
{

    /* Define the byte pool ID used for error checking.  */
    ULONG               tx_byte_pool_id;

    /* Define the byte pool's name.  */
    CHAR                *tx_byte_pool_name;

    /* Define the number of available bytes in the pool.  */
    ULONG               tx_byte_pool_available;

    /* Define the number of fragments in the pool.  */
    UINT                tx_byte_pool_fragments;

    /* Define the head pointer of byte pool.  */
    UCHAR               *tx_byte_pool_list;

    /* Define the search pointer used for initial searching for memory
       in a byte pool.  */
    UCHAR               *tx_byte_pool_search;

    /* Save the start address of the byte pool's memory area.  */
    UCHAR               *tx_byte_pool_start;

    /* Save the byte pool's size in bytes.  */
    ULONG               tx_byte_pool_size;

    /* This is used to mark the owner of the byte memory pool during
       a search.  If this value changes during the search, the local search
       pointer must be reset.  */
    struct TX_THREAD_STRUCT
                        *tx_byte_pool_owner;

    /* Define the byte pool suspension list head along with a count of
       how many threads are suspended.  */
    struct TX_THREAD_STRUCT
                        *tx_byte_pool_suspension_list;
    UINT                tx_byte_pool_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct TX_BYTE_POOL_STRUCT
                        *tx_byte_pool_created_next,
                        *tx_byte_pool_created_previous;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

    /* Define the number of allocates.  */
    ULONG               tx_byte_pool_performance_allocate_count;

    /* Define the number of releases.  */
    ULONG               tx_byte_pool_performance_release_count;

    /* Define the number of adjacent memory fragment merges.  */
    ULONG               tx_byte_pool_performance_merge_count;

    /* Define the number of memory fragment splits.  */
    ULONG               tx_byte_pool_performance_split_count;

    /* Define the number of memory fragments searched that either were not free or could not satisfy the
       request.  */
    ULONG               tx_byte_pool_performance_search_count;

    /* Define the number of byte pool suspensions.  */
    ULONG               tx_byte_pool_performance_suspension_count;

    /* Define the number of byte pool timeouts.  */
    ULONG               tx_byte_pool_performance_timeout_count;
#endif

    /* Define the port extension in the byte pool control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_BYTE_POOL_EXTENSION

} TX_BYTE_POOL;


/* Define the event flags group structure utilized by the application.  */

typedef struct TX_EVENT_FLAGS_GROUP_STRUCT
{

    /* Define the event flags group ID used for error checking.  */
    ULONG               tx_event_flags_group_id;

    /* Define the event flags group's name.  */
    CHAR                *tx_event_flags_group_name;

    /* Define the actual current event flags in this group. A zero in a
       particular bit indicates the event flag is not set.  */
    ULONG               tx_event_flags_group_current;

    /* Define the reset search flag that is set when an ISR sets flags during
       the search of the suspended threads list.  */
    UINT                tx_event_flags_group_reset_search;

    /* Define the event flags group suspension list head along with a count of
       how many threads are suspended.  */
    struct TX_THREAD_STRUCT
                        *tx_event_flags_group_suspension_list;
    UINT                tx_event_flags_group_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct TX_EVENT_FLAGS_GROUP_STRUCT
                        *tx_event_flags_group_created_next,
                        *tx_event_flags_group_created_previous;

    /* Define the delayed clearing event flags.  */
    ULONG               tx_event_flags_group_delayed_clear;

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

    /* Define the number of event flag sets.  */
    ULONG               tx_event_flags_group_performance_set_count;

    /* Define the number of event flag gets.  */
    ULONG               tx_event_flags_group__performance_get_count;

    /* Define the number of event flag suspensions.  */
    ULONG               tx_event_flags_group___performance_suspension_count;

    /* Define the number of event flag timeouts.  */
    ULONG               tx_event_flags_group____performance_timeout_count;
#endif

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Define the application callback routine used to notify the application when
       an event flag is set.  */
    VOID                (*tx_event_flags_group_set_notify)(struct TX_EVENT_FLAGS_GROUP_STRUCT *group_ptr);
#endif

    /* Define the port extension in the event flags group control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_EVENT_FLAGS_GROUP_EXTENSION

} TX_EVENT_FLAGS_GROUP;


/* Determine if the mutex put extension 1 is defined. If not, define the
   extension to whitespace.  */

#ifndef TX_MUTEX_PUT_EXTENSION_1
#define TX_MUTEX_PUT_EXTENSION_1
#endif


/* Determine if the mutex put extension 2 is defined. If not, define the
   extension to whitespace.  */

#ifndef TX_MUTEX_PUT_EXTENSION_2
#define TX_MUTEX_PUT_EXTENSION_2
#endif


/* Determine if the mutex priority change extension is defined. If not, define the
   extension to whitespace.  */

#ifndef TX_MUTEX_PRIORITY_CHANGE_EXTENSION
#define TX_MUTEX_PRIORITY_CHANGE_EXTENSION
#endif


/* Define the mutex structure utilized by the application.  */

typedef struct TX_MUTEX_STRUCT
{

    /* Define the mutex ID used for error checking.  */
    ULONG               tx_mutex_id;

    /* Define the mutex's name.  */
    CHAR                *tx_mutex_name;

    /* Define the mutex ownership count.  */
    UINT                tx_mutex_ownership_count;

    /* Define the mutex ownership pointer.  This pointer points to the
       the thread that owns the mutex.  */
    TX_THREAD           *tx_mutex_owner;

    /* Define the priority inheritance flag.  If this flag is set, priority
       inheritance will be in effect.  */
    UINT                tx_mutex_inherit;

    /* Define the save area for the owning thread's original priority.  */
    UINT                tx_mutex_original_priority;

    /* Define the mutex suspension list head along with a count of
       how many threads are suspended.  */
    struct TX_THREAD_STRUCT
                        *tx_mutex_suspension_list;
    UINT                tx_mutex_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct TX_MUTEX_STRUCT
                        *tx_mutex_created_next,
                        *tx_mutex_created_previous;

    /* Define the priority of the highest priority thread waiting for
       this mutex.  */
    UINT                tx_mutex_highest_priority_waiting;

    /* Define the owned list next and previous pointers.  */
    struct TX_MUTEX_STRUCT
                        *tx_mutex_owned_next,
                        *tx_mutex_owned_previous;

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

    /* Define the number of mutex puts.  */
    ULONG               tx_mutex_performance_put_count;

    /* Define the total number of mutex gets.  */
    ULONG               tx_mutex_performance_get_count;

    /* Define the total number of mutex suspensions.  */
    ULONG               tx_mutex_performance_suspension_count;

    /* Define the total number of mutex timeouts.  */
    ULONG               tx_mutex_performance_timeout_count;

    /* Define the total number of priority inversions.  */
    ULONG               tx_mutex_performance_priority_inversion_count;

    /* Define the total number of priority inheritance conditions.  */
    ULONG               tx_mutex_performance__priority_inheritance_count;
#endif

    /* Define the port extension in the mutex control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_MUTEX_EXTENSION

} TX_MUTEX;


/* Define the queue structure utilized by the application.  */

typedef struct TX_QUEUE_STRUCT
{

    /* Define the queue ID used for error checking.  */
    ULONG               tx_queue_id;

    /* Define the queue's name.  */
    CHAR                *tx_queue_name;

    /* Define the message size that was specified in queue creation.  */
    UINT                tx_queue_message_size;

    /* Define the total number of messages in the queue.  */
    UINT                tx_queue_capacity;

    /* Define the current number of messages enqueued and the available
       queue storage space.  */
    UINT                tx_queue_enqueued;
    UINT                tx_queue_available_storage;

    /* Define pointers that represent the start and end for the queue's
       message area.  */
    ULONG               *tx_queue_start;
    ULONG               *tx_queue_end;

    /* Define the queue read and write pointers.  Send requests use the write
       pointer while receive requests use the read pointer.  */
    ULONG               *tx_queue_read;
    ULONG               *tx_queue_write;

    /* Define the queue suspension list head along with a count of
       how many threads are suspended.  */
    struct TX_THREAD_STRUCT
                        *tx_queue_suspension_list;
    UINT                tx_queue_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct TX_QUEUE_STRUCT
                        *tx_queue_created_next,
                        *tx_queue_created_previous;

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

    /* Define the number of messages sent to this queue.  */
    ULONG               tx_queue_performance_messages_sent_count;

    /* Define the number of messages received from this queue.  */
    ULONG               tx_queue_performance_messages_received_count;

    /* Define the number of empty suspensions on this queue.  */
    ULONG               tx_queue_performance_empty_suspension_count;

    /* Define the number of full suspensions on this queue.  */
    ULONG               tx_queue_performance_full_suspension_count;

    /* Define the number of full non-suspensions on this queue. These
       messages are rejected with an appropriate error code.  */
    ULONG               tx_queue_performance_full_error_count;

    /* Define the number of queue timeouts.  */
    ULONG               tx_queue_performance_timeout_count;
#endif

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Define the application callback routine used to notify the application when
       the a message is sent to the queue.  */
    VOID                (*tx_queue_send_notify)(struct TX_QUEUE_STRUCT *queue_ptr);
#endif

    /* Define the port extension in the queue control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_QUEUE_EXTENSION

} TX_QUEUE;


/* Define the semaphore structure utilized by the application.  */

typedef struct TX_SEMAPHORE_STRUCT
{

    /* Define the semaphore ID used for error checking.  */
    ULONG               tx_semaphore_id;

    /* Define the semaphore's name.  */
    CHAR                *tx_semaphore_name;

    /* Define the actual semaphore count.  A zero means that no semaphore
       instance is available.  */
    ULONG               tx_semaphore_count;

    /* Define the semaphore suspension list head along with a count of
       how many threads are suspended.  */
    struct TX_THREAD_STRUCT
                        *tx_semaphore_suspension_list;
    UINT                tx_semaphore_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct TX_SEMAPHORE_STRUCT
                        *tx_semaphore_created_next,
                        *tx_semaphore_created_previous;

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Define the number of semaphore puts.  */
    ULONG               tx_semaphore_performance_put_count;

    /* Define the number of semaphore gets.  */
    ULONG               tx_semaphore_performance_get_count;

    /* Define the number of semaphore suspensions.  */
    ULONG               tx_semaphore_performance_suspension_count;

    /* Define the number of semaphore timeouts.  */
    ULONG               tx_semaphore_performance_timeout_count;
#endif

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Define the application callback routine used to notify the application when
       the a semaphore is put.  */
    VOID                (*tx_semaphore_put_notify)(struct TX_SEMAPHORE_STRUCT *semaphore_ptr);
#endif

    /* Define the port extension in the semaphore control block. This
       is typically defined to whitespace in tx_port.h.  */
    TX_SEMAPHORE_EXTENSION

} TX_SEMAPHORE;


/************* Define ThreadX SMP function prototypes and remapping.  *************/

/* Re-map user API to internal API for SMP routines.  */

#ifndef TX_SOURCE_CODE
#define tx_thread_smp_core_exclude              _tx_thread_smp_core_exclude
#define tx_thread_smp_core_exclude_get          _tx_thread_smp_core_exclude_get
#define tx_thread_smp_core_get                  _tx_thread_smp_core_get
#define tx_timer_smp_core_exclude               _tx_timer_smp_core_exclude
#define tx_timer_smp_core_exclude_get           _tx_timer_smp_core_exclude_get
#endif


#ifndef TX_MISRA_ENABLE

/* Define external data references for SMP.  */
#ifndef TX_SOURCE_CODE
extern ULONG volatile  _tx_timer_system_clock;
#endif
#endif


/* Define all SMP prototypes for calling from C.  */

UINT        _tx_thread_smp_core_exclude(TX_THREAD *thread_ptr, ULONG exclusion_map);
UINT        _tx_thread_smp_core_exclude_get(TX_THREAD *thread_ptr, ULONG *exclusion_map_ptr);
UINT        _tx_thread_smp_core_get(void);
UINT        _tx_timer_smp_core_exclude(TX_TIMER *timer_ptr, ULONG exclusion_map);
UINT        _tx_timer_smp_core_exclude_get(TX_TIMER *timer_ptr, ULONG *exclusion_map_ptr);

/* Define ThreadX SMP low-level assembly routines.   */

TX_THREAD   *_tx_thread_smp_current_thread_get(void);
UINT        _tx_thread_smp_protect(void);
void        _tx_thread_smp_unprotect(UINT interrupt_save);
ULONG       _tx_thread_smp_current_state_get(void);
ULONG       _tx_thread_smp_time_get(void);


/* Determine if SMP Debug is selected.  If so, the function prototype is setup. Otherwise, the debug call is
   simply mapped to whitespace.  */

#ifdef TX_THREAD_SMP_DEBUG_ENABLE
void        _tx_thread_smp_debug_entry_insert(ULONG id, ULONG suspend, VOID *thread_ptr);
#endif


/* Define the get thread macro.  */

#define TX_THREAD_GET_CURRENT(a)                (a) =  (TX_THREAD *) _tx_thread_smp_current_thread_get();


/* Define the get core ID macro.  */

#define TX_SMP_CORE_ID                          _tx_thread_smp_core_get()


/************* End of ThreadX SMP function prototypes and remapping.  *************/


/* Define the system API mappings based on the error checking
   selected by the user.  Note: this section is only applicable to
   application source code, hence the conditional that turns off this
   stuff when the include file is processed by the ThreadX source. */

#ifndef TX_SOURCE_CODE


/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef TX_DISABLE_ERROR_CHECKING


/* Services without error checking.  */

#define tx_kernel_enter                             _tx_initialize_kernel_enter

#define tx_block_allocate                           _tx_block_allocate
#define tx_block_pool_create                        _tx_block_pool_create
#define tx_block_pool_delete                        _tx_block_pool_delete
#define tx_block_pool_info_get                      _tx_block_pool_info_get
#define tx_block_pool_performance_info_get          _tx_block_pool_performance_info_get
#define tx_block_pool_performance_system_info_get   _tx_block_pool_performance_system_info_get
#define tx_block_pool_prioritize                    _tx_block_pool_prioritize
#define tx_block_release                            _tx_block_release

#define tx_byte_allocate                            _tx_byte_allocate
#define tx_byte_pool_create                         _tx_byte_pool_create
#define tx_byte_pool_delete                         _tx_byte_pool_delete
#define tx_byte_pool_info_get                       _tx_byte_pool_info_get
#define tx_byte_pool_performance_info_get           _tx_byte_pool_performance_info_get
#define tx_byte_pool_performance_system_info_get    _tx_byte_pool_performance_system_info_get
#define tx_byte_pool_prioritize                     _tx_byte_pool_prioritize
#define tx_byte_release                             _tx_byte_release

#define tx_event_flags_create                       _tx_event_flags_create
#define tx_event_flags_delete                       _tx_event_flags_delete
#define tx_event_flags_get                          _tx_event_flags_get
#define tx_event_flags_info_get                     _tx_event_flags_info_get
#define tx_event_flags_performance_info_get         _tx_event_flags_performance_info_get
#define tx_event_flags_performance_system_info_get  _tx_event_flags_performance_system_info_get
#define tx_event_flags_set                          _tx_event_flags_set
#define tx_event_flags_set_notify                   _tx_event_flags_set_notify

#ifdef TX_ENABLE_EVENT_LOGGING
UINT    _tx_el_interrupt_control(UINT new_posture);
#define tx_interrupt_control                        _tx_el_interrupt_control
#else
#ifdef TX_ENABLE_EVENT_TRACE
#define tx_interrupt_control                        _tx_trace_interrupt_control
#else
#define tx_interrupt_control                        _tx_thread_interrupt_control
#endif
#endif

#define tx_mutex_create                             _tx_mutex_create
#define tx_mutex_delete                             _tx_mutex_delete
#define tx_mutex_get                                _tx_mutex_get
#define tx_mutex_info_get                           _tx_mutex_info_get
#define tx_mutex_performance_info_get               _tx_mutex_performance_info_get
#define tx_mutex_performance_system_info_get        _tx_mutex_performance_system_info_get
#define tx_mutex_prioritize                         _tx_mutex_prioritize
#define tx_mutex_put                                _tx_mutex_put

#define tx_queue_create                             _tx_queue_create
#define tx_queue_delete                             _tx_queue_delete
#define tx_queue_flush                              _tx_queue_flush
#define tx_queue_info_get                           _tx_queue_info_get
#define tx_queue_performance_info_get               _tx_queue_performance_info_get
#define tx_queue_performance_system_info_get        _tx_queue_performance_system_info_get
#define tx_queue_receive                            _tx_queue_receive
#define tx_queue_send                               _tx_queue_send
#define tx_queue_send_notify                        _tx_queue_send_notify
#define tx_queue_front_send                         _tx_queue_front_send
#define tx_queue_prioritize                         _tx_queue_prioritize

#define tx_semaphore_ceiling_put                    _tx_semaphore_ceiling_put
#define tx_semaphore_create                         _tx_semaphore_create
#define tx_semaphore_delete                         _tx_semaphore_delete
#define tx_semaphore_get                            _tx_semaphore_get
#define tx_semaphore_info_get                       _tx_semaphore_info_get
#define tx_semaphore_performance_info_get           _tx_semaphore_performance_info_get
#define tx_semaphore_performance_system_info_get    _tx_semaphore_performance_system_info_get
#define tx_semaphore_prioritize                     _tx_semaphore_prioritize
#define tx_semaphore_put                            _tx_semaphore_put
#define tx_semaphore_put_notify                     _tx_semaphore_put_notify

#define tx_thread_create                            _tx_thread_create
#define tx_thread_delete                            _tx_thread_delete
#define tx_thread_entry_exit_notify                 _tx_thread_entry_exit_notify
#define tx_thread_identify                          _tx_thread_identify
#define tx_thread_info_get                          _tx_thread_info_get
#define tx_thread_performance_info_get              _tx_thread_performance_info_get
#define tx_thread_performance_system_info_get       _tx_thread_performance_system_info_get
#define tx_thread_preemption_change                 _tx_thread_preemption_change
#define tx_thread_priority_change                   _tx_thread_priority_change
#define tx_thread_relinquish                        _tx_thread_relinquish
#define tx_thread_reset                             _tx_thread_reset
#define tx_thread_resume                            _tx_thread_resume
#define tx_thread_sleep                             _tx_thread_sleep
#define tx_thread_stack_error_notify                _tx_thread_stack_error_notify
#define tx_thread_suspend                           _tx_thread_suspend
#define tx_thread_terminate                         _tx_thread_terminate
#define tx_thread_time_slice_change                 _tx_thread_time_slice_change
#define tx_thread_wait_abort                        _tx_thread_wait_abort

/************* Define ThreadX SMP remapping of tx_time_get.  *************/
#ifndef TX_MISRA_ENABLE
#define tx_time_get()                               (_tx_timer_system_clock)
#else
#define tx_time_get                                 _tx_time_get
#endif
/************* End ThreadX SMP remapping of tx_time_get.  *************/
#define tx_time_set                                 _tx_time_set
#define tx_timer_activate                           _tx_timer_activate
#define tx_timer_change                             _tx_timer_change
#define tx_timer_create                             _tx_timer_create
#define tx_timer_deactivate                         _tx_timer_deactivate
#define tx_timer_delete                             _tx_timer_delete
#define tx_timer_info_get                           _tx_timer_info_get
#define tx_timer_performance_info_get               _tx_timer_performance_info_get
#define tx_timer_performance_system_info_get        _tx_timer_performance_system_info_get

#define tx_trace_enable                             _tx_trace_enable
#define tx_trace_event_filter                       _tx_trace_event_filter
#define tx_trace_event_unfilter                     _tx_trace_event_unfilter
#define tx_trace_disable                            _tx_trace_disable
#define tx_trace_isr_enter_insert                   _tx_trace_isr_enter_insert
#define tx_trace_isr_exit_insert                    _tx_trace_isr_exit_insert
#define tx_trace_buffer_full_notify                 _tx_trace_buffer_full_notify
#define tx_trace_user_event_insert                  _tx_trace_user_event_insert

#else

/* Services with error checking.  */

#define tx_kernel_enter                             _tx_initialize_kernel_enter

/* Define the system API mappings depending on the runtime error
   checking behavior selected by the user.  */

#ifdef TX_ENABLE_MULTI_ERROR_CHECKING

/* Services with MULTI runtime error checking ThreadX.  */

#define tx_block_allocate                           _txr_block_allocate
#define tx_block_pool_create(p,n,b,s,l)             _txr_block_pool_create((p),(n),(b),(s),(l),(sizeof(TX_BLOCK_POOL)))
#define tx_block_pool_delete                        _txr_block_pool_delete
#define tx_block_pool_info_get                      _txr_block_pool_info_get
#define tx_block_pool_performance_info_get          _tx_block_pool_performance_info_get
#define tx_block_pool_performance_system_info_get   _tx_block_pool_performance_system_info_get
#define tx_block_pool_prioritize                    _txr_block_pool_prioritize
#define tx_block_release                            _txr_block_release

#define tx_byte_allocate                            _txr_byte_allocate
#define tx_byte_pool_create(p,n,s,l)                _txr_byte_pool_create((p),(n),(s),(l),(sizeof(TX_BYTE_POOL)))
#define tx_byte_pool_delete                         _txr_byte_pool_delete
#define tx_byte_pool_info_get                       _txr_byte_pool_info_get
#define tx_byte_pool_performance_info_get           _tx_byte_pool_performance_info_get
#define tx_byte_pool_performance_system_info_get    _tx_byte_pool_performance_system_info_get
#define tx_byte_pool_prioritize                     _txr_byte_pool_prioritize
#define tx_byte_release                             _txr_byte_release

#define tx_event_flags_create(g,n)                  _txr_event_flags_create((g),(n),(sizeof(TX_EVENT_FLAGS_GROUP)))
#define tx_event_flags_delete                       _txr_event_flags_delete
#define tx_event_flags_get                          _txr_event_flags_get
#define tx_event_flags_info_get                     _txr_event_flags_info_get
#define tx_event_flags_performance_info_get         _tx_event_flags_performance_info_get
#define tx_event_flags_performance_system_info_get  _tx_event_flags_performance_system_info_get
#define tx_event_flags_set                          _txr_event_flags_set
#define tx_event_flags_set_notify                   _txr_event_flags_set_notify

#ifdef TX_ENABLE_EVENT_LOGGING
UINT    _tx_el_interrupt_control(UINT new_posture);
#define tx_interrupt_control                        _tx_el_interrupt_control
#else
#ifdef TX_ENABLE_EVENT_TRACE
#define tx_interrupt_control                        _tx_trace_interrupt_control
#else
#define tx_interrupt_control                        _tx_thread_interrupt_control
#endif
#endif

#define tx_mutex_create(m,n,i)                      _txr_mutex_create((m),(n),(i),(sizeof(TX_MUTEX)))
#define tx_mutex_delete                             _txr_mutex_delete
#define tx_mutex_get                                _txr_mutex_get
#define tx_mutex_info_get                           _txr_mutex_info_get
#define tx_mutex_performance_info_get               _tx_mutex_performance_info_get
#define tx_mutex_performance_system_info_get        _tx_mutex_performance_system_info_get
#define tx_mutex_prioritize                         _txr_mutex_prioritize
#define tx_mutex_put                                _txr_mutex_put

#define tx_queue_create(q,n,m,s,l)                  _txr_queue_create((q),(n),(m),(s),(l),(sizeof(TX_QUEUE)))
#define tx_queue_delete                             _txr_queue_delete
#define tx_queue_flush                              _txr_queue_flush
#define tx_queue_info_get                           _txr_queue_info_get
#define tx_queue_performance_info_get               _tx_queue_performance_info_get
#define tx_queue_performance_system_info_get        _tx_queue_performance_system_info_get
#define tx_queue_receive                            _txr_queue_receive
#define tx_queue_send                               _txr_queue_send
#define tx_queue_send_notify                        _txr_queue_send_notify
#define tx_queue_front_send                         _txr_queue_front_send
#define tx_queue_prioritize                         _txr_queue_prioritize

#define tx_semaphore_ceiling_put                    _txr_semaphore_ceiling_put
#define tx_semaphore_create(s,n,i)                  _txr_semaphore_create((s),(n),(i),(sizeof(TX_SEMAPHORE)))
#define tx_semaphore_delete                         _txr_semaphore_delete
#define tx_semaphore_get                            _txr_semaphore_get
#define tx_semaphore_info_get                       _txr_semaphore_info_get
#define tx_semaphore_performance_info_get           _tx_semaphore_performance_info_get
#define tx_semaphore_performance_system_info_get    _tx_semaphore_performance_system_info_get
#define tx_semaphore_prioritize                     _txr_semaphore_prioritize
#define tx_semaphore_put                            _txr_semaphore_put
#define tx_semaphore_put_notify                     _txr_semaphore_put_notify

#define tx_thread_create(t,n,e,i,s,l,p,r,c,a)       _txr_thread_create((t),(n),(e),(i),(s),(l),(p),(r),(c),(a),(sizeof(TX_THREAD)))
#define tx_thread_delete                            _txr_thread_delete
#define tx_thread_entry_exit_notify                 _txr_thread_entry_exit_notify
#define tx_thread_identify                          _tx_thread_identify
#define tx_thread_info_get                          _txr_thread_info_get
#define tx_thread_performance_info_get              _tx_thread_performance_info_get
#define tx_thread_performance_system_info_get       _tx_thread_performance_system_info_get
#define tx_thread_preemption_change                 _txr_thread_preemption_change
#define tx_thread_priority_change                   _txr_thread_priority_change
#define tx_thread_relinquish                        _txe_thread_relinquish
#define tx_thread_reset                             _txr_thread_reset
#define tx_thread_resume                            _txr_thread_resume
#define tx_thread_sleep                             _tx_thread_sleep
#define tx_thread_stack_error_notify                _tx_thread_stack_error_notify
#define tx_thread_suspend                           _txr_thread_suspend
#define tx_thread_terminate                         _txr_thread_terminate
#define tx_thread_time_slice_change                 _txr_thread_time_slice_change
#define tx_thread_wait_abort                        _txr_thread_wait_abort

/************* Define ThreadX SMP remapping of tx_time_get.  *************/
#ifndef TX_MISRA_ENABLE
#define tx_time_get()                               (_tx_timer_system_clock)
#else
#define tx_time_get                                 _tx_time_get
#endif
/************* End ThreadX SMP remapping of tx_time_get.  *************/
#define tx_time_set                                 _tx_time_set
#define tx_timer_activate                           _txr_timer_activate
#define tx_timer_change                             _txr_timer_change
#define tx_timer_create(t,n,e,i,c,r,a)              _txr_timer_create((t),(n),(e),(i),(c),(r),(a),(sizeof(TX_TIMER)))
#define tx_timer_deactivate                         _txr_timer_deactivate
#define tx_timer_delete                             _txr_timer_delete
#define tx_timer_info_get                           _txr_timer_info_get
#define tx_timer_performance_info_get               _tx_timer_performance_info_get
#define tx_timer_performance_system_info_get        _tx_timer_performance_system_info_get

#define tx_trace_enable                             _tx_trace_enable
#define tx_trace_event_filter                       _tx_trace_event_filter
#define tx_trace_event_unfilter                     _tx_trace_event_unfilter
#define tx_trace_disable                            _tx_trace_disable
#define tx_trace_isr_enter_insert                   _tx_trace_isr_enter_insert
#define tx_trace_isr_exit_insert                    _tx_trace_isr_exit_insert
#define tx_trace_buffer_full_notify                 _tx_trace_buffer_full_notify
#define tx_trace_user_event_insert                  _tx_trace_user_event_insert

#else

#define tx_block_allocate                           _txe_block_allocate
#define tx_block_pool_create(p,n,b,s,l)             _txe_block_pool_create((p),(n),(b),(s),(l),(sizeof(TX_BLOCK_POOL)))
#define tx_block_pool_delete                        _txe_block_pool_delete
#define tx_block_pool_info_get                      _txe_block_pool_info_get
#define tx_block_pool_performance_info_get          _tx_block_pool_performance_info_get
#define tx_block_pool_performance_system_info_get   _tx_block_pool_performance_system_info_get
#define tx_block_pool_prioritize                    _txe_block_pool_prioritize
#define tx_block_release                            _txe_block_release

#define tx_byte_allocate                            _txe_byte_allocate
#define tx_byte_pool_create(p,n,s,l)                _txe_byte_pool_create((p),(n),(s),(l),(sizeof(TX_BYTE_POOL)))
#define tx_byte_pool_delete                         _txe_byte_pool_delete
#define tx_byte_pool_info_get                       _txe_byte_pool_info_get
#define tx_byte_pool_performance_info_get           _tx_byte_pool_performance_info_get
#define tx_byte_pool_performance_system_info_get    _tx_byte_pool_performance_system_info_get
#define tx_byte_pool_prioritize                     _txe_byte_pool_prioritize
#define tx_byte_release                             _txe_byte_release

#define tx_event_flags_create(g,n)                  _txe_event_flags_create((g),(n),(sizeof(TX_EVENT_FLAGS_GROUP)))
#define tx_event_flags_delete                       _txe_event_flags_delete
#define tx_event_flags_get                          _txe_event_flags_get
#define tx_event_flags_info_get                     _txe_event_flags_info_get
#define tx_event_flags_performance_info_get         _tx_event_flags_performance_info_get
#define tx_event_flags_performance_system_info_get  _tx_event_flags_performance_system_info_get
#define tx_event_flags_set                          _txe_event_flags_set
#define tx_event_flags_set_notify                   _txe_event_flags_set_notify

#ifdef TX_ENABLE_EVENT_LOGGING
UINT    _tx_el_interrupt_control(UINT new_posture);
#define tx_interrupt_control                        _tx_el_interrupt_control
#else
#ifdef TX_ENABLE_EVENT_TRACE
#define tx_interrupt_control                        _tx_trace_interrupt_control
#else
#define tx_interrupt_control                        _tx_thread_interrupt_control
#endif
#endif

#define tx_mutex_create(m,n,i)                      _txe_mutex_create((m),(n),(i),(sizeof(TX_MUTEX)))
#define tx_mutex_delete                             _txe_mutex_delete
#define tx_mutex_get                                _txe_mutex_get
#define tx_mutex_info_get                           _txe_mutex_info_get
#define tx_mutex_performance_info_get               _tx_mutex_performance_info_get
#define tx_mutex_performance_system_info_get        _tx_mutex_performance_system_info_get
#define tx_mutex_prioritize                         _txe_mutex_prioritize
#define tx_mutex_put                                _txe_mutex_put

#define tx_queue_create(q,n,m,s,l)                  _txe_queue_create((q),(n),(m),(s),(l),(sizeof(TX_QUEUE)))
#define tx_queue_delete                             _txe_queue_delete
#define tx_queue_flush                              _txe_queue_flush
#define tx_queue_info_get                           _txe_queue_info_get
#define tx_queue_performance_info_get               _tx_queue_performance_info_get
#define tx_queue_performance_system_info_get        _tx_queue_performance_system_info_get
#define tx_queue_receive                            _txe_queue_receive
#define tx_queue_send                               _txe_queue_send
#define tx_queue_send_notify                        _txe_queue_send_notify
#define tx_queue_front_send                         _txe_queue_front_send
#define tx_queue_prioritize                         _txe_queue_prioritize

#define tx_semaphore_ceiling_put                    _txe_semaphore_ceiling_put
#define tx_semaphore_create(s,n,i)                  _txe_semaphore_create((s),(n),(i),(sizeof(TX_SEMAPHORE)))
#define tx_semaphore_delete                         _txe_semaphore_delete
#define tx_semaphore_get                            _txe_semaphore_get
#define tx_semaphore_info_get                       _txe_semaphore_info_get
#define tx_semaphore_performance_info_get           _tx_semaphore_performance_info_get
#define tx_semaphore_performance_system_info_get    _tx_semaphore_performance_system_info_get
#define tx_semaphore_prioritize                     _txe_semaphore_prioritize
#define tx_semaphore_put                            _txe_semaphore_put
#define tx_semaphore_put_notify                     _txe_semaphore_put_notify

#define tx_thread_create(t,n,e,i,s,l,p,r,c,a)       _txe_thread_create((t),(n),(e),(i),(s),(l),(p),(r),(c),(a),(sizeof(TX_THREAD)))
#define tx_thread_delete                            _txe_thread_delete
#define tx_thread_entry_exit_notify                 _txe_thread_entry_exit_notify
#define tx_thread_identify                          _tx_thread_identify
#define tx_thread_info_get                          _txe_thread_info_get
#define tx_thread_performance_info_get              _tx_thread_performance_info_get
#define tx_thread_performance_system_info_get       _tx_thread_performance_system_info_get
#define tx_thread_preemption_change                 _txe_thread_preemption_change
#define tx_thread_priority_change                   _txe_thread_priority_change
#define tx_thread_relinquish                        _txe_thread_relinquish
#define tx_thread_reset                             _txe_thread_reset
#define tx_thread_resume                            _txe_thread_resume
#define tx_thread_sleep                             _tx_thread_sleep
#define tx_thread_stack_error_notify                _tx_thread_stack_error_notify
#define tx_thread_suspend                           _txe_thread_suspend
#define tx_thread_terminate                         _txe_thread_terminate
#define tx_thread_time_slice_change                 _txe_thread_time_slice_change
#define tx_thread_wait_abort                        _txe_thread_wait_abort

/************* Define ThreadX SMP remapping of tx_time_get.  *************/
#ifndef TX_MISRA_ENABLE
#define tx_time_get()                               (_tx_timer_system_clock)
#else
#define tx_time_get                                 _tx_time_get
#endif
/************* End ThreadX SMP remapping of tx_time_get.  *************/
#define tx_time_set                                 _tx_time_set
#define tx_timer_activate                           _txe_timer_activate
#define tx_timer_change                             _txe_timer_change
#define tx_timer_create(t,n,e,i,c,r,a)              _txe_timer_create((t),(n),(e),(i),(c),(r),(a),(sizeof(TX_TIMER)))
#define tx_timer_deactivate                         _txe_timer_deactivate
#define tx_timer_delete                             _txe_timer_delete
#define tx_timer_info_get                           _txe_timer_info_get
#define tx_timer_performance_info_get               _tx_timer_performance_info_get
#define tx_timer_performance_system_info_get        _tx_timer_performance_system_info_get

#define tx_trace_enable                             _tx_trace_enable
#define tx_trace_event_filter                       _tx_trace_event_filter
#define tx_trace_event_unfilter                     _tx_trace_event_unfilter
#define tx_trace_disable                            _tx_trace_disable
#define tx_trace_isr_enter_insert                   _tx_trace_isr_enter_insert
#define tx_trace_isr_exit_insert                    _tx_trace_isr_exit_insert
#define tx_trace_buffer_full_notify                 _tx_trace_buffer_full_notify
#define tx_trace_user_event_insert                  _tx_trace_user_event_insert

#endif
#endif

#endif


/* Declare the tx_application_define function as having C linkage.  */

VOID        tx_application_define(VOID *first_unused_memory);


/* Define the function prototypes of the ThreadX API.  */


/* Define block memory pool management function prototypes.  */

UINT        _tx_block_allocate(TX_BLOCK_POOL *pool_ptr, VOID **block_ptr, ULONG wait_option);
UINT        _tx_block_pool_create(TX_BLOCK_POOL *pool_ptr, CHAR *name_ptr, ULONG block_size,
                    VOID *pool_start, ULONG pool_size);
UINT        _tx_block_pool_delete(TX_BLOCK_POOL *pool_ptr);
UINT        _tx_block_pool_info_get(TX_BLOCK_POOL *pool_ptr, CHAR **name, ULONG *available_blocks,
                    ULONG *total_blocks, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BLOCK_POOL **next_pool);
UINT        _tx_block_pool_performance_info_get(TX_BLOCK_POOL *pool_ptr, ULONG *allocates, ULONG *releases,
                    ULONG *suspensions, ULONG *timeouts);
UINT        _tx_block_pool_performance_system_info_get(ULONG *allocates, ULONG *releases,
                    ULONG *suspensions, ULONG *timeouts);
UINT        _tx_block_pool_prioritize(TX_BLOCK_POOL *pool_ptr);
UINT        _tx_block_release(VOID *block_ptr);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_block_allocate(TX_BLOCK_POOL *pool_ptr, VOID **block_ptr, ULONG wait_option);
UINT        _txe_block_pool_create(TX_BLOCK_POOL *pool_ptr, CHAR *name_ptr, ULONG block_size,
                    VOID *pool_start, ULONG pool_size, UINT pool_control_block_size);
UINT        _txe_block_pool_delete(TX_BLOCK_POOL *pool_ptr);
UINT        _txe_block_pool_info_get(TX_BLOCK_POOL *pool_ptr, CHAR **name, ULONG *available_blocks,
                    ULONG *total_blocks, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BLOCK_POOL **next_pool);
UINT        _txe_block_pool_prioritize(TX_BLOCK_POOL *pool_ptr);
UINT        _txe_block_release(VOID *block_ptr);


/* Define byte memory pool management function prototypes.  */

UINT        _tx_byte_allocate(TX_BYTE_POOL *pool_ptr, VOID **memory_ptr, ULONG memory_size,
                    ULONG wait_option);
UINT        _tx_byte_pool_create(TX_BYTE_POOL *pool_ptr, CHAR *name_ptr, VOID *pool_start,
                    ULONG pool_size);
UINT        _tx_byte_pool_delete(TX_BYTE_POOL *pool_ptr);
UINT        _tx_byte_pool_info_get(TX_BYTE_POOL *pool_ptr, CHAR **name, ULONG *available_bytes,
                    ULONG *fragments, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BYTE_POOL **next_pool);
UINT        _tx_byte_pool_performance_info_get(TX_BYTE_POOL *pool_ptr, ULONG *allocates, ULONG *releases,
                    ULONG *fragments_searched, ULONG *merges, ULONG *splits, ULONG *suspensions, ULONG *timeouts);
UINT        _tx_byte_pool_performance_system_info_get(ULONG *allocates, ULONG *releases,
                    ULONG *fragments_searched, ULONG *merges, ULONG *splits, ULONG *suspensions, ULONG *timeouts);
UINT        _tx_byte_pool_prioritize(TX_BYTE_POOL *pool_ptr);
UINT        _tx_byte_release(VOID *memory_ptr);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_byte_allocate(TX_BYTE_POOL *pool_ptr, VOID **memory_ptr, ULONG memory_size,
                    ULONG wait_option);
UINT        _txe_byte_pool_create(TX_BYTE_POOL *pool_ptr, CHAR *name_ptr, VOID *pool_start,
                    ULONG pool_size, UINT pool_control_block_size);
UINT        _txe_byte_pool_delete(TX_BYTE_POOL *pool_ptr);
UINT        _txe_byte_pool_info_get(TX_BYTE_POOL *pool_ptr, CHAR **name, ULONG *available_bytes,
                    ULONG *fragments, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BYTE_POOL **next_pool);
UINT        _txe_byte_pool_prioritize(TX_BYTE_POOL *pool_ptr);
UINT        _txe_byte_release(VOID *memory_ptr);


/* Define event flags management function prototypes.  */

UINT        _tx_event_flags_create(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR *name_ptr);
UINT        _tx_event_flags_delete(TX_EVENT_FLAGS_GROUP *group_ptr);
UINT        _tx_event_flags_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG requested_flags,
                    UINT get_option, ULONG *actual_flags_ptr, ULONG wait_option);
UINT        _tx_event_flags_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR **name, ULONG *current_flags,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_EVENT_FLAGS_GROUP **next_group);
UINT        _tx_event_flags_performance_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG *sets, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts);
UINT        _tx_event_flags_performance_system_info_get(ULONG *sets, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts);
UINT        _tx_event_flags_set(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG flags_to_set,
                    UINT set_option);
UINT        _tx_event_flags_set_notify(TX_EVENT_FLAGS_GROUP *group_ptr, VOID (*events_set_notify)(TX_EVENT_FLAGS_GROUP *notify_group_ptr));


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_event_flags_create(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR *name_ptr, UINT event_control_block_size);
UINT        _txe_event_flags_delete(TX_EVENT_FLAGS_GROUP *group_ptr);
UINT        _txe_event_flags_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG requested_flags,
                    UINT get_option, ULONG *actual_flags_ptr, ULONG wait_option);
UINT        _txe_event_flags_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR **name, ULONG *current_flags,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_EVENT_FLAGS_GROUP **next_group);
UINT        _txe_event_flags_set(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG flags_to_set,
                    UINT set_option);
UINT        _txe_event_flags_set_notify(TX_EVENT_FLAGS_GROUP *group_ptr, VOID (*events_set_notify)(TX_EVENT_FLAGS_GROUP *notify_group_ptr));


/* Define initialization function prototypes.  */

VOID        _tx_initialize_kernel_enter(VOID);


/* Define mutex management function prototypes.  */

UINT        _tx_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit);
UINT        _tx_mutex_delete(TX_MUTEX *mutex_ptr);
UINT        _tx_mutex_get(TX_MUTEX *mutex_ptr, ULONG wait_option);
UINT        _tx_mutex_info_get(TX_MUTEX *mutex_ptr, CHAR **name, ULONG *count, TX_THREAD **owner,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_MUTEX **next_mutex);
UINT        _tx_mutex_performance_info_get(TX_MUTEX *mutex_ptr, ULONG *puts, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts, ULONG *inversions, ULONG *inheritances);
UINT        _tx_mutex_performance_system_info_get(ULONG *puts, ULONG *gets, ULONG *suspensions, ULONG *timeouts,
                    ULONG *inversions, ULONG *inheritances);
UINT        _tx_mutex_prioritize(TX_MUTEX *mutex_ptr);
UINT        _tx_mutex_put(TX_MUTEX *mutex_ptr);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit, UINT mutex_control_block_size);
UINT        _txe_mutex_delete(TX_MUTEX *mutex_ptr);
UINT        _txe_mutex_get(TX_MUTEX *mutex_ptr, ULONG wait_option);
UINT        _txe_mutex_info_get(TX_MUTEX *mutex_ptr, CHAR **name, ULONG *count, TX_THREAD **owner,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_MUTEX **next_mutex);
UINT        _txe_mutex_prioritize(TX_MUTEX *mutex_ptr);
UINT        _txe_mutex_put(TX_MUTEX *mutex_ptr);


/* Define queue management function prototypes.  */

UINT        _tx_queue_create(TX_QUEUE *queue_ptr, CHAR *name_ptr, UINT message_size,
                        VOID *queue_start, ULONG queue_size);
UINT        _tx_queue_delete(TX_QUEUE *queue_ptr);
UINT        _tx_queue_flush(TX_QUEUE *queue_ptr);
UINT        _tx_queue_info_get(TX_QUEUE *queue_ptr, CHAR **name, ULONG *enqueued, ULONG *available_storage,
                    TX_THREAD **first_suspended, ULONG *suspended_count, TX_QUEUE **next_queue);
UINT        _tx_queue_performance_info_get(TX_QUEUE *queue_ptr, ULONG *messages_sent, ULONG *messages_received,
                    ULONG *empty_suspensions, ULONG *full_suspensions, ULONG *full_errors, ULONG *timeouts);
UINT        _tx_queue_performance_system_info_get(ULONG *messages_sent, ULONG *messages_received,
                    ULONG *empty_suspensions, ULONG *full_suspensions, ULONG *full_errors, ULONG *timeouts);
UINT        _tx_queue_prioritize(TX_QUEUE *queue_ptr);
UINT        _tx_queue_receive(TX_QUEUE *queue_ptr, VOID *destination_ptr, ULONG wait_option);
UINT        _tx_queue_send(TX_QUEUE *queue_ptr, VOID *source_ptr, ULONG wait_option);
UINT        _tx_queue_send_notify(TX_QUEUE *queue_ptr, VOID (*queue_send_notify)(TX_QUEUE *notify_queue_ptr));
UINT        _tx_queue_front_send(TX_QUEUE *queue_ptr, VOID *source_ptr, ULONG wait_option);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_queue_create(TX_QUEUE *queue_ptr, CHAR *name_ptr, UINT message_size,
                        VOID *queue_start, ULONG queue_size, UINT queue_control_block_size);
UINT        _txe_queue_delete(TX_QUEUE *queue_ptr);
UINT        _txe_queue_flush(TX_QUEUE *queue_ptr);
UINT        _txe_queue_info_get(TX_QUEUE *queue_ptr, CHAR **name, ULONG *enqueued, ULONG *available_storage,
                    TX_THREAD **first_suspended, ULONG *suspended_count, TX_QUEUE **next_queue);
UINT        _txe_queue_prioritize(TX_QUEUE *queue_ptr);
UINT        _txe_queue_receive(TX_QUEUE *queue_ptr, VOID *destination_ptr, ULONG wait_option);
UINT        _txe_queue_send(TX_QUEUE *queue_ptr, VOID *source_ptr, ULONG wait_option);
UINT        _txe_queue_send_notify(TX_QUEUE *queue_ptr, VOID (*queue_send_notify)(TX_QUEUE *notify_queue_ptr));
UINT        _txe_queue_front_send(TX_QUEUE *queue_ptr, VOID *source_ptr, ULONG wait_option);


/* Define semaphore management function prototypes.  */

UINT        _tx_semaphore_ceiling_put(TX_SEMAPHORE *semaphore_ptr, ULONG ceiling);
UINT        _tx_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count);
UINT        _tx_semaphore_delete(TX_SEMAPHORE *semaphore_ptr);
UINT        _tx_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option);
UINT        _tx_semaphore_info_get(TX_SEMAPHORE *semaphore_ptr, CHAR **name, ULONG *current_value,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_SEMAPHORE **next_semaphore);
UINT        _tx_semaphore_performance_info_get(TX_SEMAPHORE *semaphore_ptr, ULONG *puts, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts);
UINT        _tx_semaphore_performance_system_info_get(ULONG *puts, ULONG *gets, ULONG *suspensions, ULONG *timeouts);
UINT        _tx_semaphore_prioritize(TX_SEMAPHORE *semaphore_ptr);
UINT        _tx_semaphore_put(TX_SEMAPHORE *semaphore_ptr);
UINT        _tx_semaphore_put_notify(TX_SEMAPHORE *semaphore_ptr, VOID (*semaphore_put_notify)(TX_SEMAPHORE *notify_semaphore_ptr));


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_semaphore_ceiling_put(TX_SEMAPHORE *semaphore_ptr, ULONG ceiling);
UINT        _txe_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count, UINT semaphore_control_block_size);
UINT        _txe_semaphore_delete(TX_SEMAPHORE *semaphore_ptr);
UINT        _txe_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option);
UINT        _txe_semaphore_info_get(TX_SEMAPHORE *semaphore_ptr, CHAR **name, ULONG *current_value,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_SEMAPHORE **next_semaphore);
UINT        _txe_semaphore_prioritize(TX_SEMAPHORE *semaphore_ptr);
UINT        _txe_semaphore_put(TX_SEMAPHORE *semaphore_ptr);
UINT        _txe_semaphore_put_notify(TX_SEMAPHORE *semaphore_ptr, VOID (*semaphore_put_notify)(TX_SEMAPHORE *notify_semaphore_ptr));


/* Define thread control function prototypes.  */

VOID        _tx_thread_context_save(VOID);
VOID        _tx_thread_context_restore(VOID);
UINT        _tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr,
                VOID (*entry_function)(ULONG entry_input), ULONG entry_input,
                VOID *stack_start, ULONG stack_size,
                UINT priority, UINT preempt_threshold,
                ULONG time_slice, UINT auto_start);
UINT        _tx_thread_delete(TX_THREAD *thread_ptr);
UINT        _tx_thread_entry_exit_notify(TX_THREAD *thread_ptr, VOID (*thread_entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT type));
TX_THREAD  *_tx_thread_identify(VOID);
UINT        _tx_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state, ULONG *run_count,
                UINT *priority, UINT *preemption_threshold, ULONG *time_slice,
                TX_THREAD **next_thread, TX_THREAD **next_suspended_thread);
UINT        _tx_thread_interrupt_control(UINT new_posture);
UINT        _tx_thread_performance_info_get(TX_THREAD *thread_ptr, ULONG *resumptions, ULONG *suspensions,
                ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions,
                ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts, TX_THREAD **last_preempted_by);
UINT        _tx_thread_performance_system_info_get(ULONG *resumptions, ULONG *suspensions,
                ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions,
                ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts,
                ULONG *non_idle_returns, ULONG *idle_returns);
UINT        _tx_thread_preemption_change(TX_THREAD *thread_ptr, UINT new_threshold,
                        UINT *old_threshold);
UINT        _tx_thread_priority_change(TX_THREAD *thread_ptr, UINT new_priority,
                        UINT *old_priority);
VOID        _tx_thread_relinquish(VOID);
UINT        _tx_thread_reset(TX_THREAD *thread_ptr);
UINT        _tx_thread_resume(TX_THREAD *thread_ptr);
UINT        _tx_thread_sleep(ULONG timer_ticks);
UINT        _tx_thread_stack_error_notify(VOID (*stack_error_handler)(TX_THREAD *thread_ptr));
UINT        _tx_thread_suspend(TX_THREAD *thread_ptr);
UINT        _tx_thread_terminate(TX_THREAD *thread_ptr);
UINT        _tx_thread_time_slice_change(TX_THREAD *thread_ptr, ULONG new_time_slice, ULONG *old_time_slice);
UINT        _tx_thread_wait_abort(TX_THREAD *thread_ptr);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr,
                VOID (*entry_function)(ULONG entry_input), ULONG entry_input,
                VOID *stack_start, ULONG stack_size,
                UINT priority, UINT preempt_threshold,
                ULONG time_slice, UINT auto_start, UINT thread_control_block_size);
UINT        _txe_thread_delete(TX_THREAD *thread_ptr);
UINT        _txe_thread_entry_exit_notify(TX_THREAD *thread_ptr, VOID (*thread_entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT type));
UINT        _txe_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state, ULONG *run_count,
                UINT *priority, UINT *preemption_threshold, ULONG *time_slice,
                TX_THREAD **next_thread, TX_THREAD **next_suspended_thread);
UINT        _txe_thread_preemption_change(TX_THREAD *thread_ptr, UINT new_threshold,
                        UINT *old_threshold);
UINT        _txe_thread_priority_change(TX_THREAD *thread_ptr, UINT new_priority,
                        UINT *old_priority);
VOID        _txe_thread_relinquish(VOID);
UINT        _txe_thread_reset(TX_THREAD *thread_ptr);
UINT        _txe_thread_resume(TX_THREAD *thread_ptr);
UINT        _txe_thread_suspend(TX_THREAD *thread_ptr);
UINT        _txe_thread_terminate(TX_THREAD *thread_ptr);
UINT        _txe_thread_time_slice_change(TX_THREAD *thread_ptr, ULONG new_time_slice, ULONG *old_time_slice);
UINT        _txe_thread_wait_abort(TX_THREAD *thread_ptr);


/* Define timer management function prototypes.  */

UINT        _tx_timer_activate(TX_TIMER *timer_ptr);
UINT        _tx_timer_change(TX_TIMER *timer_ptr, ULONG initial_ticks, ULONG reschedule_ticks);
UINT        _tx_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr,
                VOID (*expiration_function)(ULONG input), ULONG expiration_input,
                ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate);
UINT        _tx_timer_deactivate(TX_TIMER *timer_ptr);
UINT        _tx_timer_delete(TX_TIMER *timer_ptr);
UINT        _tx_timer_info_get(TX_TIMER *timer_ptr, CHAR **name, UINT *active, ULONG *remaining_ticks,
                ULONG *reschedule_ticks, TX_TIMER **next_timer);
UINT        _tx_timer_performance_info_get(TX_TIMER *timer_ptr, ULONG *activates, ULONG *reactivates,
                ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts);
UINT        _tx_timer_performance_system_info_get(ULONG *activates, ULONG *reactivates,
                ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts);

ULONG       _tx_time_get(VOID);
VOID        _tx_time_set(ULONG new_time);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT        _txe_timer_activate(TX_TIMER *timer_ptr);
UINT        _txe_timer_change(TX_TIMER *timer_ptr, ULONG initial_ticks, ULONG reschedule_ticks);
UINT        _txe_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr,
                VOID (*expiration_function)(ULONG input), ULONG expiration_input,
                ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate, UINT timer_control_block_size);
UINT        _txe_timer_deactivate(TX_TIMER *timer_ptr);
UINT        _txe_timer_delete(TX_TIMER *timer_ptr);
UINT        _txe_timer_info_get(TX_TIMER *timer_ptr, CHAR **name, UINT *active, ULONG *remaining_ticks,
                ULONG *reschedule_ticks, TX_TIMER **next_timer);


/* Define trace API function prototypes.  */

UINT        _tx_trace_enable(VOID *trace_buffer_start, ULONG trace_buffer_size, ULONG registry_entries);
UINT        _tx_trace_event_filter(ULONG event_filter_bits);
UINT        _tx_trace_event_unfilter(ULONG event_unfilter_bits);
UINT        _tx_trace_disable(VOID);
VOID        _tx_trace_isr_enter_insert(ULONG isr_id);
VOID        _tx_trace_isr_exit_insert(ULONG isr_id);
UINT        _tx_trace_buffer_full_notify(VOID (*full_buffer_callback)(VOID *buffer));
UINT        _tx_trace_user_event_insert(ULONG event_id, ULONG info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4);
UINT        _tx_trace_interrupt_control(UINT new_posture);


/* Add a default macro that can be re-defined in tx_port.h to add default processing when a thread starts. Common usage
   would be for enabling floating point for a thread by default, however, the additional processing could be anything
   defined in tx_port.h.  */

#ifndef TX_THREAD_STARTED_EXTENSION
#define TX_THREAD_STARTED_EXTENSION(thread_ptr)
#endif


/* Add a default macro that can be re-defined in tx_port.h to add processing to the thread stack analyze function.
   By default, this is simply defined as whitespace.  */

#ifndef TX_THREAD_STACK_ANALYZE_EXTENSION
#define TX_THREAD_STACK_ANALYZE_EXTENSION
#endif


/* Add a default macro that can be re-defined in tx_port.h to add processing to the initialize kernel enter function.
   By default, this is simply defined as whitespace.  */

#ifndef TX_INITIALIZE_KERNEL_ENTER_EXTENSION
#define TX_INITIALIZE_KERNEL_ENTER_EXTENSION
#endif


/* Check for MISRA compliance requirements.  */

#ifdef TX_MISRA_ENABLE

/* Define MISRA-specific routines.  */

VOID                    _tx_misra_memset(VOID *ptr, UINT value, UINT size);
UCHAR                   *_tx_misra_uchar_pointer_add(UCHAR *ptr, ULONG amount);
UCHAR                   *_tx_misra_uchar_pointer_sub(UCHAR *ptr, ULONG amount);
ULONG                   _tx_misra_uchar_pointer_dif(UCHAR *ptr1, UCHAR *ptr2);
ULONG                   _tx_misra_pointer_to_ulong_convert(VOID *ptr);
ULONG                   *_tx_misra_ulong_pointer_add(ULONG *ptr, ULONG amount);
ULONG                   *_tx_misra_ulong_pointer_sub(ULONG *ptr, ULONG amount);
ULONG                   _tx_misra_ulong_pointer_dif(ULONG *ptr1, ULONG *ptr2);
VOID                    *_tx_misra_ulong_to_pointer_convert(ULONG input);
VOID                    _tx_misra_message_copy(ULONG **source, ULONG **destination, UINT size);
ULONG                   _tx_misra_timer_pointer_dif(TX_TIMER_INTERNAL **ptr1, TX_TIMER_INTERNAL **ptr2);
TX_TIMER_INTERNAL       **_tx_misra_timer_pointer_add(TX_TIMER_INTERNAL **ptr1, ULONG size);
VOID                    _tx_misra_user_timer_pointer_get(TX_TIMER_INTERNAL *internal_timer, TX_TIMER **user_timer);
VOID                    _tx_misra_thread_stack_check(TX_THREAD *thread_ptr, VOID **highest_stack);
VOID                    _tx_misra_trace_event_insert(ULONG event_id, VOID *info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4, ULONG filter, ULONG time_stamp);
UINT                    _tx_misra_always_true(void);
UCHAR                   **_tx_misra_indirect_void_to_uchar_pointer_convert(VOID **pointer);
UCHAR                   **_tx_misra_uchar_to_indirect_uchar_pointer_convert(UCHAR *pointer);
UCHAR                   *_tx_misra_block_pool_to_uchar_pointer_convert(TX_BLOCK_POOL *pool);
TX_BLOCK_POOL           *_tx_misra_void_to_block_pool_pointer_convert(VOID *pointer);
UCHAR                   *_tx_misra_void_to_uchar_pointer_convert(VOID *pointer);
TX_BLOCK_POOL           *_tx_misra_uchar_to_block_pool_pointer_convert(UCHAR *pointer);
UCHAR                   **_tx_misra_void_to_indirect_uchar_pointer_convert(VOID *pointer);
TX_BYTE_POOL            *_tx_misra_void_to_byte_pool_pointer_convert(VOID *pointer);
UCHAR                   *_tx_misra_byte_pool_to_uchar_pointer_convert(TX_BYTE_POOL *pool);
ALIGN_TYPE              *_tx_misra_uchar_to_align_type_pointer_convert(UCHAR *pointer);
TX_BYTE_POOL            **_tx_misra_uchar_to_indirect_byte_pool_pointer_convert(UCHAR *pointer);
TX_EVENT_FLAGS_GROUP    *_tx_misra_void_to_event_flags_pointer_convert(VOID *pointer);
ULONG                   *_tx_misra_void_to_ulong_pointer_convert(VOID *pointer);
TX_MUTEX                *_tx_misra_void_to_mutex_pointer_convert(VOID *pointer);
UINT                    _tx_misra_status_get(UINT status);
TX_QUEUE                *_tx_misra_void_to_queue_pointer_convert(VOID *pointer);
TX_SEMAPHORE            *_tx_misra_void_to_semaphore_pointer_convert(VOID *pointer);
VOID                    *_tx_misra_uchar_to_void_pointer_convert(UCHAR  *pointer);
TX_THREAD               *_tx_misra_ulong_to_thread_pointer_convert(ULONG value);
VOID                    *_tx_misra_timer_indirect_to_void_pointer_convert(TX_TIMER_INTERNAL **pointer);
CHAR                    *_tx_misra_const_char_to_char_pointer_convert(const char *pointer);
TX_THREAD               *_tx_misra_void_to_thread_pointer_convert(VOID  *pointer);
UCHAR                   *_tx_misra_char_to_uchar_pointer_convert(CHAR  *pointer);
VOID                    _tx_misra_event_flags_group_not_used(TX_EVENT_FLAGS_GROUP *group_ptr);
VOID                    _tx_misra_event_flags_set_notify_not_used(VOID (*events_set_notify)(TX_EVENT_FLAGS_GROUP *notify_group_ptr));
VOID                    _tx_misra_queue_not_used(TX_QUEUE *queue_ptr);
VOID                    _tx_misra_queue_send_notify_not_used(VOID (*queue_send_notify)(TX_QUEUE *notify_queue_ptr));
VOID                    _tx_misra_semaphore_not_used(TX_SEMAPHORE *semaphore_ptr);
VOID                    _tx_misra_semaphore_put_notify_not_used(VOID (*semaphore_put_notify)(TX_SEMAPHORE *notify_semaphore_ptr));
VOID                    _tx_misra_thread_not_used(TX_THREAD *thread_ptr);
VOID                    _tx_misra_thread_entry_exit_notify_not_used(VOID (*thread_entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT id));

#define TX_MEMSET(a,b,c)                                _tx_misra_memset((a), (UINT) (b), (UINT) (c))
#define TX_UCHAR_POINTER_ADD(a,b)                       _tx_misra_uchar_pointer_add((UCHAR *) (a), (ULONG) (b))
#define TX_UCHAR_POINTER_SUB(a,b)                       _tx_misra_uchar_pointer_sub((UCHAR *) (a), (ULONG) (b))
#define TX_UCHAR_POINTER_DIF(a,b)                       _tx_misra_uchar_pointer_dif((UCHAR *) (a), (UCHAR *) (b))
#define TX_ULONG_POINTER_ADD(a,b)                       _tx_misra_ulong_pointer_add((ULONG *) (a), (ULONG) (b))
#define TX_ULONG_POINTER_SUB(a,b)                       _tx_misra_ulong_pointer_sub((ULONG *) (a), (ULONG) (b))
#define TX_ULONG_POINTER_DIF(a,b)                       _tx_misra_ulong_pointer_dif((ULONG *) (a), (ULONG *) (b))
#define TX_POINTER_TO_ULONG_CONVERT(a)                  _tx_misra_pointer_to_ulong_convert((VOID *) (a))
#define TX_ULONG_TO_POINTER_CONVERT(a)                  _tx_misra_ulong_to_pointer_convert((ULONG) (a))
#define TX_QUEUE_MESSAGE_COPY(s,d,z)                    _tx_misra_message_copy(&(s), &(d), (z));
#define TX_TIMER_POINTER_DIF(a,b)                       _tx_misra_timer_pointer_dif((TX_TIMER_INTERNAL **) (a), (TX_TIMER_INTERNAL **) (b))
#define TX_TIMER_POINTER_ADD(a,b)                       _tx_misra_timer_pointer_add((TX_TIMER_INTERNAL **) (a), (ULONG) (b))
#define TX_USER_TIMER_POINTER_GET(a,b)                  _tx_misra_user_timer_pointer_get((TX_TIMER_INTERNAL *) (a), (TX_TIMER **) &(b));
#define TX_THREAD_STACK_CHECK(a)                        _tx_misra_thread_stack_check((a), &((a)->tx_thread_stack_highest_ptr));
#ifdef TX_ENABLE_EVENT_TRACE
#define TX_TRACE_IN_LINE_INSERT(i,a,b,c,d,e)            _tx_misra_trace_event_insert((ULONG) (i), (VOID *) (a), (ULONG) (b), (ULONG) (c), (ULONG) (d), (ULONG) (e), ((ULONG) TX_TRACE_TIME_SOURCE));
#endif
#define TX_LOOP_FOREVER                                 (_tx_misra_always_true() == TX_TRUE)
#define TX_INDIRECT_VOID_TO_UCHAR_POINTER_CONVERT(a)    _tx_misra_indirect_void_to_uchar_pointer_convert((a))
#define TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(a)   _tx_misra_uchar_to_indirect_uchar_pointer_convert((a))
#define TX_BLOCK_POOL_TO_UCHAR_POINTER_CONVERT(a)       _tx_misra_block_pool_to_uchar_pointer_convert((a))
#define TX_VOID_TO_BLOCK_POOL_POINTER_CONVERT(a)        _tx_misra_void_to_block_pool_pointer_convert((a))
#define TX_VOID_TO_UCHAR_POINTER_CONVERT(a)             _tx_misra_void_to_uchar_pointer_convert((a))
#define TX_UCHAR_TO_BLOCK_POOL_POINTER_CONVERT(a)       _tx_misra_uchar_to_block_pool_pointer_convert((a))
#define TX_VOID_TO_INDIRECT_UCHAR_POINTER_CONVERT(a)    _tx_misra_void_to_indirect_uchar_pointer_convert((a))
#define TX_VOID_TO_BYTE_POOL_POINTER_CONVERT(a)         _tx_misra_void_to_byte_pool_pointer_convert((a))
#define TX_BYTE_POOL_TO_UCHAR_POINTER_CONVERT(a)        _tx_misra_byte_pool_to_uchar_pointer_convert((a))
#define TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(a)       _tx_misra_uchar_to_align_type_pointer_convert((a))
#define TX_UCHAR_TO_INDIRECT_BYTE_POOL_POINTER(a)       _tx_misra_uchar_to_indirect_byte_pool_pointer_convert((a))
#define TX_VOID_TO_EVENT_FLAGS_POINTER_CONVERT(a)       _tx_misra_void_to_event_flags_pointer_convert((a))
#define TX_VOID_TO_ULONG_POINTER_CONVERT(a)             _tx_misra_void_to_ulong_pointer_convert((a))
#define TX_VOID_TO_MUTEX_POINTER_CONVERT(a)             _tx_misra_void_to_mutex_pointer_convert((a))
#define TX_MUTEX_PRIORITIZE_MISRA_EXTENSION(a)          _tx_misra_status_get((a))
#define TX_VOID_TO_QUEUE_POINTER_CONVERT(a)             _tx_misra_void_to_queue_pointer_convert((a))
#define TX_VOID_TO_SEMAPHORE_POINTER_CONVERT(a)         _tx_misra_void_to_semaphore_pointer_convert((a))
#define TX_UCHAR_TO_VOID_POINTER_CONVERT(a)             _tx_misra_uchar_to_void_pointer_convert((a))
#define TX_ULONG_TO_THREAD_POINTER_CONVERT(a)           _tx_misra_ulong_to_thread_pointer_convert((a))
#define TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(a)    _tx_misra_timer_indirect_to_void_pointer_convert((a))
#ifndef TX_TIMER_INITIALIZE_EXTENSION
#define TX_TIMER_INITIALIZE_EXTENSION(a)                status =  _tx_misra_status_get((a));
#endif
#define TX_CONST_CHAR_TO_CHAR_POINTER_CONVERT(a)        _tx_misra_const_char_to_char_pointer_convert((a))
#define TX_VOID_TO_THREAD_POINTER_CONVERT(a)            _tx_misra_void_to_thread_pointer_convert((a))
#define TX_CHAR_TO_UCHAR_POINTER_CONVERT(a)             _tx_misra_char_to_uchar_pointer_convert((a))
#define TX_EVENT_FLAGS_GROUP_NOT_USED(a)                _tx_misra_event_flags_group_not_used((a))
#define TX_EVENT_FLAGS_SET_NOTIFY_NOT_USED(a)           _tx_misra_event_flags_set_notify_not_used((a))
#define TX_QUEUE_NOT_USED(a)                            _tx_misra_queue_not_used((a))
#define TX_QUEUE_SEND_NOTIFY_NOT_USED(a)                _tx_misra_queue_send_notify_not_used((a))
#define TX_SEMAPHORE_NOT_USED(a)                        _tx_misra_semaphore_not_used((a))
#define TX_SEMAPHORE_PUT_NOTIFY_NOT_USED(a)             _tx_misra_semaphore_put_notify_not_used((a))
#define TX_THREAD_NOT_USED(a)                           _tx_misra_thread_not_used((a))
#define TX_THREAD_ENTRY_EXIT_NOTIFY_NOT_USED(a)         _tx_misra_thread_entry_exit_notify_not_used((a))

#else

/* Define the TX_MEMSET macro to the standard library function, if not already defined.  */

#ifndef TX_MEMSET
#define TX_MEMSET(a,b,c)                                memset((a),(b),(c))
#endif

#define TX_UCHAR_POINTER_ADD(a,b)                       (((UCHAR *) (a)) + ((UINT) (b)))
#define TX_UCHAR_POINTER_SUB(a,b)                       (((UCHAR *) (a)) - ((UINT) (b)))
#define TX_UCHAR_POINTER_DIF(a,b)                       ((ULONG)(((UCHAR *) (a)) - ((UCHAR *) (b))))
#define TX_ULONG_POINTER_ADD(a,b)                       (((ULONG *) (a)) + ((UINT) (b)))
#define TX_ULONG_POINTER_SUB(a,b)                       (((ULONG *) (a)) - ((UINT) (b)))
#define TX_ULONG_POINTER_DIF(a,b)                       ((ULONG)(((ULONG *) (a)) - ((ULONG *) (b))))
#define TX_POINTER_TO_ULONG_CONVERT(a)                  ((ULONG) ((VOID *) (a)))
#define TX_ULONG_TO_POINTER_CONVERT(a)                  ((VOID *) ((ULONG) (a)))
#define TX_POINTER_TO_ALIGN_TYPE_CONVERT(a)             ((ALIGN_TYPE) ((VOID *) (a)))
#define TX_ALIGN_TYPE_TO_POINTER_CONVERT(a)             ((VOID *) ((ALIGN_TYPE) (a)))
#define TX_TIMER_POINTER_DIF(a,b)                       ((ULONG)(((TX_TIMER_INTERNAL **) (a)) - ((TX_TIMER_INTERNAL **) (b))))
#define TX_TIMER_POINTER_ADD(a,b)                       (((TX_TIMER_INTERNAL **) (a)) + ((ULONG) (b)))
#define TX_USER_TIMER_POINTER_GET(a,b)                  { \
                                                            UCHAR   *working_ptr; \
                                                            working_ptr =  (UCHAR *) (a);   \
                                                            (b) =  (TX_TIMER *) working_ptr; \
                                                            working_ptr =  working_ptr - (((UCHAR *) &(b) -> tx_timer_internal) - ((UCHAR *) &(b) -> tx_timer_id)); \
                                                            (b) =  (TX_TIMER *) working_ptr; \
                                                        }
#define TX_LOOP_FOREVER                                 ((UINT) 1)
#define TX_INDIRECT_VOID_TO_UCHAR_POINTER_CONVERT(a)    ((UCHAR **) ((VOID *) (a)))
#define TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(a)   ((UCHAR **) ((VOID *) (a)))
#define TX_BLOCK_POOL_TO_UCHAR_POINTER_CONVERT(a)       ((UCHAR *) ((VOID *) (a)))
#define TX_VOID_TO_BLOCK_POOL_POINTER_CONVERT(a)        ((TX_BLOCK_POOL *) ((VOID *) (a)))
#define TX_VOID_TO_UCHAR_POINTER_CONVERT(a)             ((UCHAR *) ((VOID *) (a)))
#define TX_UCHAR_TO_BLOCK_POOL_POINTER_CONVERT(a)       ((TX_BLOCK_POOL *) ((VOID *) (a)))
#define TX_VOID_TO_INDIRECT_UCHAR_POINTER_CONVERT(a)    ((UCHAR **) ((VOID *) (a)))
#define TX_VOID_TO_BYTE_POOL_POINTER_CONVERT(a)         ((TX_BYTE_POOL *) ((VOID *) (a)))
#define TX_BYTE_POOL_TO_UCHAR_POINTER_CONVERT(a)        ((UCHAR *) ((VOID *) (a)))
#ifndef TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT
#define TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(a)       ((ALIGN_TYPE *) ((VOID *) (a)))
#endif
#define TX_UCHAR_TO_INDIRECT_BYTE_POOL_POINTER(a)       ((TX_BYTE_POOL **) ((VOID *) (a)))
#define TX_VOID_TO_EVENT_FLAGS_POINTER_CONVERT(a)       ((TX_EVENT_FLAGS_GROUP *) ((VOID *) (a)))
#define TX_VOID_TO_ULONG_POINTER_CONVERT(a)             ((ULONG *) ((VOID *) (a)))
#define TX_VOID_TO_MUTEX_POINTER_CONVERT(a)             ((TX_MUTEX *) ((VOID *) (a)))
#define TX_VOID_TO_QUEUE_POINTER_CONVERT(a)             ((TX_QUEUE *) ((VOID *) (a)))
#define TX_VOID_TO_SEMAPHORE_POINTER_CONVERT(a)         ((TX_SEMAPHORE *) ((VOID *) (a)))
#define TX_UCHAR_TO_VOID_POINTER_CONVERT(a)             ((VOID *) (a))
#define TX_ULONG_TO_THREAD_POINTER_CONVERT(a)           ((TX_THREAD *) ((VOID *) (a)))
#ifndef TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT
#define TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(a)    ((VOID *) (a))
#endif
#ifndef TX_TIMER_INITIALIZE_EXTENSION
#define TX_TIMER_INITIALIZE_EXTENSION(a)
#endif
#define TX_CONST_CHAR_TO_CHAR_POINTER_CONVERT(a)        ((CHAR *) ((VOID *) (a)))
#define TX_VOID_TO_THREAD_POINTER_CONVERT(a)            ((TX_THREAD *) ((VOID *) (a)))
#define TX_CHAR_TO_UCHAR_POINTER_CONVERT(a)             ((UCHAR *) ((VOID *) (a)))
#ifndef TX_EVENT_FLAGS_GROUP_NOT_USED
#define TX_EVENT_FLAGS_GROUP_NOT_USED(a)                ((void)(a))
#endif
#ifndef TX_EVENT_FLAGS_SET_NOTIFY_NOT_USED
#define TX_EVENT_FLAGS_SET_NOTIFY_NOT_USED(a)           ((void)(a))
#endif
#ifndef TX_QUEUE_NOT_USED
#define TX_QUEUE_NOT_USED(a)                            ((void)(a))
#endif
#ifndef TX_QUEUE_SEND_NOTIFY_NOT_USED
#define TX_QUEUE_SEND_NOTIFY_NOT_USED(a)                ((void)(a))
#endif
#ifndef TX_SEMAPHORE_NOT_USED
#define TX_SEMAPHORE_NOT_USED(a)                        ((void)(a))
#endif
#ifndef TX_SEMAPHORE_PUT_NOTIFY_NOT_USED
#define TX_SEMAPHORE_PUT_NOTIFY_NOT_USED(a)             ((void)(a))
#endif
#ifndef TX_THREAD_NOT_USED
#define TX_THREAD_NOT_USED(a)                           ((void)(a))
#endif
#ifndef TX_THREAD_ENTRY_EXIT_NOTIFY_NOT_USED
#define TX_THREAD_ENTRY_EXIT_NOTIFY_NOT_USED(a)         ((void)(a))
#endif

#endif


/* Determine if there is an tx_api.h extension file to include.  */

#ifdef TX_THREAD_API_EXTENSION

/* Yes, bring in the tx_api.h extension file.  */
#include "tx_api_extension.h"

#endif


/* Define safety critical configuration and exception handling.  */

#ifdef TX_SAFETY_CRITICAL

/* Ensure the maximum number of priorities is defined in safety critical mode.  */
#ifndef TX_MAX_PRIORITIES
#error "tx_port.h: TX_MAX_PRIORITIES not defined."
#endif

/* Ensure the maximum number of priorities is a multiple of 32.  */
#if (TX_MAX_PRIORITIES  %32) != 0
#error "tx_port.h: TX_MAX_PRIORITIES must be a multiple of 32."
#endif

/* Ensure error checking is enabled.  */
#ifdef TX_DISABLE_ERROR_CHECKING
#error "TX_DISABLE_ERROR_CHECKING must not be defined."
#endif

/* Ensure timer ISR processing is not defined.  */
#ifdef TX_TIMER_PROCESS_IN_ISR
#error "TX_TIMER_PROCESS_IN_ISR must not be defined."
#endif

/* Ensure timer reactivation in-line is not defined.  */
#ifdef TX_REACTIVATE_INLINE
#error "TX_REACTIVATE_INLINE must not be defined."
#endif

/* Ensure disable stack filling is not defined.  */
#ifdef TX_DISABLE_STACK_FILLING
#error "TX_DISABLE_STACK_FILLING must not be defined."
#endif

/* Ensure enable stack checking is not defined.  */
#ifdef TX_ENABLE_STACK_CHECKING
#error "TX_ENABLE_STACK_CHECKING must not be defined."
#endif

/* Ensure disable preemption-threshold is not defined.  */
#ifdef TX_DISABLE_PREEMPTION_THRESHOLD
#error "TX_DISABLE_PREEMPTION_THRESHOLD must not be defined."
#endif

/* Ensure disable redundant clearing is not defined.  */
#ifdef TX_DISABLE_REDUNDANT_CLEARING
#error "TX_DISABLE_REDUNDANT_CLEARING must not be defined."
#endif

/* Ensure no timer is not defined.  */
#ifdef TX_NO_TIMER
#error "TX_NO_TIMER must not be defined."
#endif

/* Ensure disable notify callbacks is not defined.  */
#ifdef TX_DISABLE_NOTIFY_CALLBACKS
#error "TX_DISABLE_NOTIFY_CALLBACKS must not be defined."
#endif

/* Ensure in-line thread suspend/resume is not defined.  */
#ifdef TX_INLINE_THREAD_RESUME_SUSPEND
#error "TX_INLINE_THREAD_RESUME_SUSPEND must not be defined."
#endif

/* Ensure not interruptable is not defined.  */
#ifdef TX_NOT_INTERRUPTABLE
#error "TX_NOT_INTERRUPTABLE must not be defined."
#endif

/* Ensure event trace enable is not defined.  */
#ifdef TX_ENABLE_EVENT_TRACE
#error "TX_ENABLE_EVENT_TRACE must not be defined."
#endif

/* Ensure block pool performance info enable is not defined.  */
#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO
#error "TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure byte pool performance info enable is not defined.  */
#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
#error "TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure event flag performance info enable is not defined.  */
#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
#error "TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure mutex performance info enable is not defined.   */
#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO
#error "TX_MUTEX_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure queue performance info enable is not defined.  */
#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO
#error "TX_QUEUE_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure semaphore performance info enable is not defined.  */
#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO
#error "TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure thread performance info enable is not defined.  */
#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO
#error "TX_THREAD_ENABLE_PERFORMANCE_INFO must not be defined."
#endif

/* Ensure timer performance info enable is not defined.  */
#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO
#error "TX_TIMER_ENABLE_PERFORMANCE_INFO must not be defined."
#endif


/* Now define the safety critical exception handler.  */

VOID    _tx_safety_critical_exception_handler(CHAR *file_name, INT line_number, UINT status);


#ifndef TX_SAFETY_CRITICAL_EXCEPTION
#define TX_SAFETY_CRITICAL_EXCEPTION(a, b, c)   _tx_safety_critical_exception_handler(a, b, c);
#endif

#ifndef TX_SAFETY_CRITICAL_EXCEPTION_HANDLER
#define TX_SAFETY_CRITICAL_EXCEPTION_HANDLER    VOID  _tx_safety_critical_exception_handler(CHAR *file_name, INT line_number, UINT status) \
                                                { \
                                                    while(1) \
                                                    { \
                                                    } \
                                                }
#endif
#endif


#ifdef TX_ENABLE_MULTI_ERROR_CHECKING

/* Define ThreadX API MULTI run-time error checking function.  */
void __ghs_rnerr(char *errMsg, int stackLevels, int stackTraceDisplay, void *hexVal);

#endif

/* Bring in the event logging constants and prototypes.  Note that
   TX_ENABLE_EVENT_LOGGING must be defined when building the ThreadX
   library components in order to enable event logging.  */

#ifdef TX_ENABLE_EVENT_LOGGING
#include "tx_el.h"
#else
#ifndef TX_SOURCE_CODE
#ifndef TX_MISRA_ENABLE
#define _tx_el_user_event_insert(a,b,c,d,e)
#endif
#endif
#define TX_EL_INITIALIZE
#define TX_EL_THREAD_REGISTER(a)
#define TX_EL_THREAD_UNREGISTER(a)
#define TX_EL_THREAD_STATUS_CHANGE_INSERT(a, b)
#define TX_EL_BYTE_ALLOCATE_INSERT
#define TX_EL_BYTE_POOL_CREATE_INSERT
#define TX_EL_BYTE_POOL_DELETE_INSERT
#define TX_EL_BYTE_RELEASE_INSERT
#define TX_EL_BLOCK_ALLOCATE_INSERT
#define TX_EL_BLOCK_POOL_CREATE_INSERT
#define TX_EL_BLOCK_POOL_DELETE_INSERT
#define TX_EL_BLOCK_RELEASE_INSERT
#define TX_EL_EVENT_FLAGS_CREATE_INSERT
#define TX_EL_EVENT_FLAGS_DELETE_INSERT
#define TX_EL_EVENT_FLAGS_GET_INSERT
#define TX_EL_EVENT_FLAGS_SET_INSERT
#define TX_EL_INTERRUPT_CONTROL_INSERT
#define TX_EL_QUEUE_CREATE_INSERT
#define TX_EL_QUEUE_DELETE_INSERT
#define TX_EL_QUEUE_FLUSH_INSERT
#define TX_EL_QUEUE_RECEIVE_INSERT
#define TX_EL_QUEUE_SEND_INSERT
#define TX_EL_SEMAPHORE_CREATE_INSERT
#define TX_EL_SEMAPHORE_DELETE_INSERT
#define TX_EL_SEMAPHORE_GET_INSERT
#define TX_EL_SEMAPHORE_PUT_INSERT
#define TX_EL_THREAD_CREATE_INSERT
#define TX_EL_THREAD_DELETE_INSERT
#define TX_EL_THREAD_IDENTIFY_INSERT
#define TX_EL_THREAD_PREEMPTION_CHANGE_INSERT
#define TX_EL_THREAD_PRIORITY_CHANGE_INSERT
#define TX_EL_THREAD_RELINQUISH_INSERT
#define TX_EL_THREAD_RESUME_INSERT
#define TX_EL_THREAD_SLEEP_INSERT
#define TX_EL_THREAD_SUSPEND_INSERT
#define TX_EL_THREAD_TERMINATE_INSERT
#define TX_EL_THREAD_TIME_SLICE_CHANGE_INSERT
#define TX_EL_TIME_GET_INSERT
#define TX_EL_TIME_SET_INSERT
#define TX_EL_TIMER_ACTIVATE_INSERT
#define TX_EL_TIMER_CHANGE_INSERT
#define TX_EL_TIMER_CREATE_INSERT
#define TX_EL_TIMER_DEACTIVATE_INSERT
#define TX_EL_TIMER_DELETE_INSERT
#define TX_EL_BLOCK_POOL_INFO_GET_INSERT
#define TX_EL_BLOCK_POOL_PRIORITIZE_INSERT
#define TX_EL_BYTE_POOL_INFO_GET_INSERT
#define TX_EL_BYTE_POOL_PRIORITIZE_INSERT
#define TX_EL_EVENT_FLAGS_INFO_GET_INSERT
#define TX_EL_MUTEX_CREATE_INSERT
#define TX_EL_MUTEX_DELETE_INSERT
#define TX_EL_MUTEX_GET_INSERT
#define TX_EL_MUTEX_INFO_GET_INSERT
#define TX_EL_MUTEX_PRIORITIZE_INSERT
#define TX_EL_MUTEX_PUT_INSERT
#define TX_EL_QUEUE_INFO_GET_INSERT
#define TX_EL_QUEUE_FRONT_SEND_INSERT
#define TX_EL_QUEUE_PRIORITIZE_INSERT
#define TX_EL_SEMAPHORE_INFO_GET_INSERT
#define TX_EL_SEMAPHORE_PRIORITIZE_INSERT
#define TX_EL_THREAD_INFO_GET_INSERT
#define TX_EL_THREAD_WAIT_ABORT_INSERT
#define TX_EL_TIMER_INFO_GET_INSERT
#define TX_EL_BLOCK_POOL_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_BYTE_POOL_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_EVENT_FLAGS_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_EVENT_FLAGS__PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_EVENT_FLAGS_SET_NOTIFY_INSERT
#define TX_EL_MUTEX_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_QUEUE_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_QUEUE_PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_QUEUE_SEND_NOTIFY_INSERT
#define TX_EL_SEMAPHORE_CEILING_PUT_INSERT
#define TX_EL_SEMAPHORE_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_SEMAPHORE_PUT_NOTIFY_INSERT
#define TX_EL_THREAD_ENTRY_EXIT_NOTIFY_INSERT
#define TX_EL_THREAD_RESET_INSERT
#define TX_EL_THREAD_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_THREAD_PERFORMANCE_SYSTEM_INFO_GET_INSERT
#define TX_EL_THREAD_STACK_ERROR_NOTIFY_INSERT
#define TX_EL_TIMER_PERFORMANCE_INFO_GET_INSERT
#define TX_EL_TIMER_PERFORMANCE_SYSTEM_INFO_GET_INSERT

#endif



/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
        }
#endif

#endif

