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
/*    tx_port.h                                         Cortex-M33/AC6    */
/*                                                           6.1.12       */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
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
/*    This file replaces the previous Cortex-M33 files. It unifies        */
/*    the Cortex-M33 compilers into one common file.                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  03-02-2021      Scott Larson            Modified comment(s), added    */
/*                                            ULONG64_DEFINED,            */
/*                                            resulting in version 6.1.5  */
/*  06-02-2021      Scott Larson            Modified comment(s), removed  */
/*                                            unneeded header file, funcs */
/*                                            set_control and get_control */
/*                                            changed to inline,          */
/*                                            added symbol to enable      */
/*                                            stack error handler,        */
/*                                            resulting in version 6.1.7  */
/*  10-15-2021      Scott Larson            Modified comment(s), improved */
/*                                            stack check error handling, */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022      Scott Larson            Modified comment(s), unified  */
/*                                            this file across compilers, */
/*                                            fixed predefined macro,     */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022      Scott Larson            Modified comments and added   */
/*                                            volatile to registers,      */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022      Scott Larson            Modified comments and changed */
/*                                            secure stack initialization */
/*                                            macro to port-specific,     */
/*                                            described BASEPRI usage,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#ifndef TX_PORT_H
#define TX_PORT_H

/* Determine if the optional ThreadX user define file should be used.  */
#ifdef TX_INCLUDE_USER_DEFINE_FILE

/* Yes, include the user defines in tx_user.h. The defines in this file may
   alternately be defined on the command line.  */

#include "tx_user.h"
#endif /* TX_INCLUDE_USER_DEFINE_FILE */

/* Define compiler library include files.  */

#include <stdlib.h>
#include <string.h>

#ifdef __ICCARM__
#include <intrinsics.h>                         /* IAR Intrinsics */
#define __asm__                 __asm           /* Define to make all inline asm from each compiler look similar */
#define _tx_control_get         __get_CONTROL
#define _tx_control_set         __set_CONTROL
#define _tx_ipsr_get            __get_IPSR
#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
#include <yvals.h>
#endif /* TX_ENABLE_IAR_LIBRARY_SUPPORT */
#endif /* __ICCARM__ */

#ifdef __ARMCOMPILER_VERSION
#include <arm_compat.h>
#endif


/* Define ThreadX basic types for this port.  */

#define VOID                                    void
typedef char                                    CHAR;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef unsigned long long                      ULONG64;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;
#define ULONG64_DEFINED

/* Function prototypes for this port. */
struct  TX_THREAD_STRUCT;
UINT    _txe_thread_secure_stack_allocate(struct TX_THREAD_STRUCT *thread_ptr, ULONG stack_size);
UINT    _txe_thread_secure_stack_free(struct TX_THREAD_STRUCT *thread_ptr);
UINT    _tx_thread_secure_stack_allocate(struct TX_THREAD_STRUCT *tx_thread, ULONG stack_size);
UINT    _tx_thread_secure_stack_free(struct TX_THREAD_STRUCT *tx_thread);

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

#define tx_thread_secure_stack_allocate   _tx_thread_secure_stack_allocate
#define tx_thread_secure_stack_free       _tx_thread_secure_stack_free

#else

/* Services with error checking.  */

#define tx_thread_secure_stack_allocate   _txe_thread_secure_stack_allocate
#define tx_thread_secure_stack_free       _txe_thread_secure_stack_free

#endif  /* TX_DISABLE_ERROR_CHECKING */
#endif  /* TX_SOURCE_CODE */

/* This port has a usage fault handler in _tx_initialize_low_level for stack exceptions.  */
#define TX_PORT_THREAD_STACK_ERROR_HANDLING

/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */

#ifndef TX_MAX_PRIORITIES
#define TX_MAX_PRIORITIES                       32
#endif


/* Define the minimum stack for a ThreadX thread on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */

#ifndef TX_MINIMUM_STACK
#define TX_MINIMUM_STACK                        200         /* Minimum stack size for this port  */
#endif


/* Define the system timer thread's default stack size and priority.  These are only applicable
   if TX_TIMER_PROCESS_IN_ISR is not defined.  */

#ifndef TX_TIMER_THREAD_STACK_SIZE
#define TX_TIMER_THREAD_STACK_SIZE              1024        /* Default timer thread stack size  */
#endif

#ifndef TX_TIMER_THREAD_PRIORITY
#define TX_TIMER_THREAD_PRIORITY                0           /* Default timer thread priority    */
#endif

/* By default, ThreadX for Cortex-M uses the PRIMASK register to enable/disable interrupts.
If using BASEPRI is desired, define the following two symbols for both c and assembly files:
TX_PORT_USE_BASEPRI - This tells ThreadX to use BASEPRI instead of PRIMASK.
TX_PORT_BASEPRI = (priority_mask << (8 - number_priority_bits)) - this defines the maximum priority level to mask.
Any interrupt with a higher priority than priority_mask will not be masked, thus the interrupt will run.
*/

/* Define various constants for the ThreadX Cortex-M port.  */

#define TX_INT_DISABLE                          1           /* Disable interrupts               */
#define TX_INT_ENABLE                           0           /* Enable interrupts                */


/* Define the clock source for trace event entry time stamp. The following two item are port specific.
   For example, if the time source is at the address 0x0a800024 and is 16-bits in size, the clock
   source constants would be:

#define TX_TRACE_TIME_SOURCE                    *((volatile ULONG *) 0x0a800024)
#define TX_TRACE_TIME_MASK                      0x0000FFFFUL

*/

#ifndef TX_MISRA_ENABLE
#ifndef TX_TRACE_TIME_SOURCE
#define TX_TRACE_TIME_SOURCE                    *((volatile ULONG *) 0xE0001004)
#endif
#else
ULONG   _tx_misra_time_stamp_get(VOID);
#define TX_TRACE_TIME_SOURCE                    _tx_misra_time_stamp_get()
#endif

#ifndef TX_TRACE_TIME_MASK
#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL
#endif


/* Define the port specific options for the _tx_build_options variable. This variable indicates
   how the ThreadX library was built.  */

#define TX_PORT_SPECIFIC_BUILD_OPTIONS          (0)


/* Define the in-line initialization constant so that modules with in-line
   initialization capabilities can prevent their initialization from being
   a function call.  */

#ifdef TX_MISRA_ENABLE
#define TX_DISABLE_INLINE
#else
#define TX_INLINE_INITIALIZATION
#endif


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

#define TX_THREAD_EXTENSION_0
#define TX_THREAD_EXTENSION_1

#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
/* IAR library support */
#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
/* ThreadX in non-secure zone with calls to secure zone. */
#define TX_THREAD_EXTENSION_2           VOID    *tx_thread_secure_stack_context;    \
                                        VOID    *tx_thread_iar_tls_pointer;
#else
/* ThreadX in only one zone. */
#define TX_THREAD_EXTENSION_2           VOID    *tx_thread_iar_tls_pointer;
#endif

#else
/* No IAR library support */
#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
/* ThreadX in non-secure zone with calls to secure zone. */
#define TX_THREAD_EXTENSION_2           VOID    *tx_thread_secure_stack_context;
#else
/* ThreadX in only one zone. */
#define TX_THREAD_EXTENSION_2
#endif

#endif  /* TX_ENABLE_IAR_LIBRARY_SUPPORT */

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
#ifdef  TX_ENABLE_IAR_LIBRARY_SUPPORT
void    *_tx_iar_create_per_thread_tls_area(void);
void    _tx_iar_destroy_per_thread_tls_area(void *tls_ptr);
void    __iar_Initlocks(void);
#define TX_THREAD_CREATE_EXTENSION(thread_ptr)                      thread_ptr -> tx_thread_iar_tls_pointer = _tx_iar_create_per_thread_tls_area();

#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                      do {_tx_iar_destroy_per_thread_tls_area(thread_ptr -> tx_thread_iar_tls_pointer);   \
                                                                        thread_ptr -> tx_thread_iar_tls_pointer = TX_NULL; } while(0);                  \
                                                                    if(thread_ptr -> tx_thread_secure_stack_context){_tx_thread_secure_stack_free(thread_ptr);}
#else
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                      do {_tx_iar_destroy_per_thread_tls_area(thread_ptr -> tx_thread_iar_tls_pointer);   \
                                                                        thread_ptr -> tx_thread_iar_tls_pointer = TX_NULL; } while(0);
#endif
#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION               do {__iar_Initlocks();} while(0);

#else   /* No IAR library support. */
#define TX_THREAD_CREATE_EXTENSION(thread_ptr)
#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)                      if(thread_ptr -> tx_thread_secure_stack_context){_tx_thread_secure_stack_free(thread_ptr);}
#else
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)
#endif
#endif  /* TX_ENABLE_IAR_LIBRARY_SUPPORT */

#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
/* Define the size of the secure stack for the timer thread and use the extension to allocate the secure stack. */
#define TX_TIMER_THREAD_SECURE_STACK_SIZE       256
#define TX_TIMER_INITIALIZE_EXTENSION(status)   _tx_thread_secure_stack_allocate(&_tx_timer_thread, TX_TIMER_THREAD_SECURE_STACK_SIZE);
#endif


#if defined(__ARMVFP__) || defined(__ARM_PCS_VFP) || defined(__ARM_FP) || defined(__TARGET_FPU_VFP) || defined(__VFP__)

#ifdef TX_MISRA_ENABLE

ULONG  _tx_misra_control_get(void);
void   _tx_misra_control_set(ULONG value);
ULONG  _tx_misra_fpccr_get(void);
void   _tx_misra_vfp_touch(void);

#else   /* TX_MISRA_ENABLE not defined */

#ifdef __GNUC__ /* GCC and ARM Compiler 6 */

__attribute__( ( always_inline ) ) static inline ULONG _tx_control_get(void)
{
ULONG  control_value;

    __asm__ volatile (" MRS  %0,CONTROL ": "=r" (control_value) );
    return(control_value);
}

__attribute__( ( always_inline ) ) static inline void _tx_control_set(ULONG control_value)
{
    __asm__ volatile (" MSR  CONTROL,%0": : "r" (control_value): "memory" );
}

#endif  /* __GNUC__ */

/* Touch VFP register in order to flush. Works for AC6/GCC/IAR compilers. */
#define TX_VFP_TOUCH()  __asm__ volatile ("VMOV.F32 s0, s0");

#endif  /* TX_MISRA_ENABLE */

/* A completed thread falls into _thread_shell_entry and we can simply deactivate the FPU via CONTROL.FPCA
   in order to ensure no lazy stacking will occur. */

#ifndef TX_MISRA_ENABLE

#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)   {                                                       \
                                                    ULONG  _tx_vfp_state;                                   \
                                                        _tx_vfp_state = _tx_control_get();                  \
                                                        _tx_vfp_state = _tx_vfp_state & ~((ULONG) 0x4);     \
                                                        _tx_control_set(_tx_vfp_state);                     \
                                                    }
#else

#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)   {                                                       \
                                                    ULONG  _tx_vfp_state;                                   \
                                                        _tx_vfp_state = _tx_misra_control_get();            \
                                                        _tx_vfp_state = _tx_vfp_state & ~((ULONG) 0x4);     \
                                                        _tx_misra_control_set(_tx_vfp_state);               \
                                                    }
#endif

/* A thread can be terminated by another thread, so we first check if it's self-terminating and not in an ISR.
   If so, deactivate the FPU via CONTROL.FPCA. Otherwise we are in an interrupt or another thread is terminating
   this one, so if the FPCCR.LSPACT bit is set, we need to save the CONTROL.FPCA state, touch the FPU to flush
   the lazy FPU save, then restore the CONTROL.FPCA state. */

#ifndef TX_MISRA_ENABLE

#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)  {                                                                                       \
                                                    ULONG  _tx_system_state;                                                                \
                                                        _tx_system_state = TX_THREAD_GET_SYSTEM_STATE();                                    \
                                                        if ((_tx_system_state == ((ULONG) 0)) && ((thread_ptr) == _tx_thread_current_ptr))  \
                                                        {                                                                                   \
                                                        ULONG  _tx_vfp_state;                                                               \
                                                            _tx_vfp_state = _tx_control_get();                                              \
                                                            _tx_vfp_state = _tx_vfp_state & ~((ULONG) 0x4);                                 \
                                                            _tx_control_set(_tx_vfp_state);                                                 \
                                                        }                                                                                   \
                                                        else                                                                                \
                                                        {                                                                                   \
                                                        ULONG  _tx_fpccr;                                                                   \
                                                            _tx_fpccr = *((volatile ULONG *) 0xE000EF34);                                   \
                                                            _tx_fpccr = _tx_fpccr & ((ULONG) 0x01);                                         \
                                                            if (_tx_fpccr == ((ULONG) 0x01))                                                \
                                                            {                                                                               \
                                                            ULONG _tx_vfp_state;                                                            \
                                                                _tx_vfp_state = _tx_control_get();                                          \
                                                                _tx_vfp_state = _tx_vfp_state & ((ULONG) 0x4);                              \
                                                                TX_VFP_TOUCH();                                                             \
                                                                if (_tx_vfp_state == ((ULONG) 0))                                           \
                                                                {                                                                           \
                                                                    _tx_vfp_state = _tx_control_get();                                      \
                                                                    _tx_vfp_state = _tx_vfp_state & ~((ULONG) 0x4);                         \
                                                                    _tx_control_set(_tx_vfp_state);                                         \
                                                                }                                                                           \
                                                            }                                                                               \
                                                        }                                                                                   \
                                                    }
#else

#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)  {                                                                                       \
                                                    ULONG  _tx_system_state;                                                                \
                                                        _tx_system_state = TX_THREAD_GET_SYSTEM_STATE();                                    \
                                                        if ((_tx_system_state == ((ULONG) 0)) && ((thread_ptr) == _tx_thread_current_ptr))  \
                                                        {                                                                                   \
                                                        ULONG  _tx_vfp_state;                                                               \
                                                            _tx_vfp_state = _tx_misra_control_get();                                        \
                                                            _tx_vfp_state = _tx_vfp_state & ~((ULONG) 0x4);                                 \
                                                            _tx_misra_control_set(_tx_vfp_state);                                           \
                                                        }                                                                                   \
                                                        else                                                                                \
                                                        {                                                                                   \
                                                        ULONG  _tx_fpccr;                                                                   \
                                                            _tx_fpccr = _tx_misra_fpccr_get();                                              \
                                                            _tx_fpccr = _tx_fpccr & ((ULONG) 0x01);                                         \
                                                            if (_tx_fpccr == ((ULONG) 0x01))                                                \
                                                            {                                                                               \
                                                            ULONG _tx_vfp_state;                                                            \
                                                                _tx_vfp_state = _tx_misra_control_get();                                    \
                                                                _tx_vfp_state = _tx_vfp_state & ((ULONG) 0x4);                              \
                                                                _tx_misra_vfp_touch();                                                      \
                                                                if (_tx_vfp_state == ((ULONG) 0))                                           \
                                                                {                                                                           \
                                                                    _tx_vfp_state = _tx_misra_control_get();                                \
                                                                    _tx_vfp_state = _tx_vfp_state & ~((ULONG) 0x4);                         \
                                                                    _tx_misra_control_set(_tx_vfp_state);                                   \
                                                                }                                                                           \
                                                            }                                                                               \
                                                        }                                                                                   \
                                                    }
#endif

#else   /* No VFP in use */

#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)
#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)

#endif  /* defined(__ARMVFP__) || defined(__ARM_PCS_VFP) || defined(__ARM_FP) || defined(__TARGET_FPU_VFP) || defined(__VFP__) */


/* Define the ThreadX object creation extensions for the remaining objects.  */

#define TX_BLOCK_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION(group_ptr)
#define TX_MUTEX_CREATE_EXTENSION(mutex_ptr)
#define TX_QUEUE_CREATE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)
#define TX_TIMER_CREATE_EXTENSION(timer_ptr)


/* Define the ThreadX object deletion extensions for the remaining objects.  */

#define TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)
#define TX_MUTEX_DELETE_EXTENSION(mutex_ptr)
#define TX_QUEUE_DELETE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)
#define TX_TIMER_DELETE_EXTENSION(timer_ptr)


/* Define the get system state macro.  */

#ifndef TX_THREAD_GET_SYSTEM_STATE
#ifndef TX_MISRA_ENABLE

#if defined(__GNUC__) /* GCC and AC6 */
__attribute__( ( always_inline ) ) static inline UINT _tx_ipsr_get(void)
{
UINT ipsr_value;
    __asm__ volatile (" MRS  %0,IPSR ": "=r" (ipsr_value) );
    return(ipsr_value);
}
#endif  /* GCC and AC6 IPSR_get function. */

#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | _tx_ipsr_get())

#else   /* TX_MISRA_ENABLE is defined, use MISRA function. */
ULONG   _tx_misra_ipsr_get(VOID);
#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | _tx_misra_ipsr_get())
#endif  /* TX_MISRA_ENABLE */
#endif  /* TX_THREAD_GET_SYSTEM_STATE */


/* Define the check for whether or not to call the _tx_thread_system_return function.  A non-zero value
   indicates that _tx_thread_system_return should not be called. This overrides the definition in tx_thread.h
   for Cortex-M since so we don't waste time checking the _tx_thread_system_state variable that is always
   zero after initialization for Cortex-M ports. */

#ifndef TX_THREAD_SYSTEM_RETURN_CHECK
#define TX_THREAD_SYSTEM_RETURN_CHECK(c)    (c) = ((ULONG) _tx_thread_preempt_disable);
#endif

#if !defined(TX_SINGLE_MODE_SECURE) && !defined(TX_SINGLE_MODE_NON_SECURE)
/* Initialize secure stacks for threads calling secure functions. */
extern void    _tx_thread_secure_stack_initialize(void);
#define TX_PORT_SPECIFIC_PRE_INITIALIZATION             _tx_thread_secure_stack_initialize();
#endif

/* Define the macro to ensure _tx_thread_preempt_disable is set early in initialization in order to
   prevent early scheduling on Cortex-M parts.  */

#define TX_PORT_SPECIFIC_POST_INITIALIZATION    _tx_thread_preempt_disable++;




#ifndef TX_DISABLE_INLINE

/* Define the TX_LOWEST_SET_BIT_CALCULATE macro for each compiler. */
#ifdef __ICCARM__       /* IAR Compiler */
#define TX_LOWEST_SET_BIT_CALCULATE(m, b)       (b) = (UINT) __CLZ(__RBIT((m)));
#elif defined(__GNUC__) /* GCC and AC6 Compiler */
#define TX_LOWEST_SET_BIT_CALCULATE(m, b)       __asm__ volatile (" RBIT %0,%1 ": "=r" (m) : "r" (m) ); \
                                                __asm__ volatile (" CLZ  %0,%1 ": "=r" (b) : "r" (m) );
#else
#error "Compiler not supported."
#endif



/* Define the interrupt disable/restore macros. */

__attribute__( ( always_inline ) ) static inline UINT __get_interrupt_posture(void)
{
UINT posture;
#ifdef TX_PORT_USE_BASEPRI
    __asm__ volatile ("MRS  %0, BASEPRI ": "=r" (posture));
#else
    __asm__ volatile ("MRS  %0, PRIMASK ": "=r" (posture));
#endif
    return(posture);
}

#ifdef TX_PORT_USE_BASEPRI
__attribute__( ( always_inline ) ) static inline void __set_basepri_value(UINT basepri_value)
{
    __asm__ volatile ("MSR  BASEPRI,%0 ": : "r" (basepri_value));
}
#else
__attribute__( ( always_inline ) ) static inline void __enable_interrupts(void)
{
    __asm__ volatile ("CPSIE  i": : : "memory");
}
#endif

__attribute__( ( always_inline ) ) static inline void __restore_interrupt(UINT int_posture)
{
#ifdef TX_PORT_USE_BASEPRI
    __set_basepri_value(int_posture);
#else
    __asm__ volatile ("MSR  PRIMASK,%0": : "r" (int_posture): "memory");
#endif
}

__attribute__( ( always_inline ) ) static inline UINT __disable_interrupts(void)
{
UINT int_posture;

    int_posture = __get_interrupt_posture();

#ifdef TX_PORT_USE_BASEPRI
    __set_basepri_value(TX_PORT_BASEPRI);
#else
    __asm__ volatile ("CPSID i" : : : "memory");
#endif
    return(int_posture);
}

__attribute__( ( always_inline ) ) static inline void _tx_thread_system_return_inline(void)
{
UINT interrupt_save;

    /* Set PendSV to invoke ThreadX scheduler.  */
    *((volatile ULONG *) 0xE000ED04) = ((ULONG) 0x10000000);
    if (_tx_ipsr_get() == 0)
    {
        interrupt_save = __get_interrupt_posture();
#ifdef TX_PORT_USE_BASEPRI
        __set_basepri_value(0);
#else
        __enable_interrupts();
#endif
        __restore_interrupt(interrupt_save);
    }
}

#define TX_INTERRUPT_SAVE_AREA                  UINT interrupt_save;
#define TX_DISABLE                              interrupt_save =  __disable_interrupts();
#define TX_RESTORE                              __restore_interrupt(interrupt_save);

/* Redefine _tx_thread_system_return for improved performance.  */
#define _tx_thread_system_return                _tx_thread_system_return_inline

#else   /* TX_DISABLE_INLINE is defined */

UINT                                            _tx_thread_interrupt_disable(VOID);
VOID                                            _tx_thread_interrupt_restore(UINT previous_posture);

#define TX_INTERRUPT_SAVE_AREA                  register UINT interrupt_save;

#define TX_DISABLE                              interrupt_save = _tx_thread_interrupt_disable();
#define TX_RESTORE                              _tx_thread_interrupt_restore(interrupt_save);
#endif  /* TX_DISABLE_INLINE */

/* Define the version ID of ThreadX.  This may be utilized by the application.  */

#ifdef TX_THREAD_INIT
CHAR                            _tx_version_id[] =
                                    "Copyright (c) Microsoft Corporation. All rights reserved. * ThreadX Cortex-M33/AC6 Version 6.1.12 *";
#else
#ifdef TX_MISRA_ENABLE
extern  CHAR                    _tx_version_id[100];
#else
extern  CHAR                    _tx_version_id[];
#endif
#endif

#endif
