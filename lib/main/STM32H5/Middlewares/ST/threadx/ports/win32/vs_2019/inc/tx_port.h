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
/*    tx_port.h                                          Win32/Visual     */ 
/*                                                           6.1          */
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
/*                                                                        */
/**************************************************************************/

#ifndef TX_PORT_H
#define TX_PORT_H


/* Determine if the optional ThreadX user define file should be used.  */

#ifdef TX_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in tx_user.h. The defines in this file may 
   alternately be defined on the command line.  */

#include "tx_user.h"
#endif


/* Define compiler library include files.  */

#include <stdlib.h>
#include <string.h>


/* Define performance metric symbols.  */

#ifndef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO
#define TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
#define TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
#define TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_MUTEX_ENABLE_PERFORMANCE_INFO
#define TX_MUTEX_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_QUEUE_ENABLE_PERFORMANCE_INFO
#define TX_QUEUE_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO
#define TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_THREAD_ENABLE_PERFORMANCE_INFO
#define TX_THREAD_ENABLE_PERFORMANCE_INFO
#endif

#ifndef TX_TIMER_ENABLE_PERFORMANCE_INFO
#define TX_TIMER_ENABLE_PERFORMANCE_INFO
#endif


/* Enable trace info.  */

#ifndef TX_ENABLE_EVENT_TRACE
#define TX_ENABLE_EVENT_TRACE
#endif


/* Define ThreadX basic types for this port.  */ 

#define VOID                                    void
typedef char                                    CHAR;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;


/* Add Win32 debug insert prototype.  */
 
void    _tx_win32_debug_entry_insert(char *action, char *file, unsigned long line);

#ifndef TX_WIN32_DEBUG_ENABLE

/* If Win32 debug is not enabled, turn logging into white-space.  */

#define _tx_win32_debug_entry_insert(a, b, c)

#endif



/* Define the TX_MEMSET macro to remove library reference.  */

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
                                               

/* Include windows include file.  */

#include <windows.h>


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
#define TX_TIMER_THREAD_STACK_SIZE              400         /* Default timer thread stack size - Not used in Win32 port!  */
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

#ifndef TX_TRACE_TIME_SOURCE
#define TX_TRACE_TIME_SOURCE                    ((ULONG) (_tx_win32_time_stamp.LowPart));
#endif
#ifndef TX_TRACE_TIME_MASK
#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL
#endif


/* Define the port-specific trace extension to pickup the Windows timer.  */

#define TX_TRACE_PORT_EXTENSION                 QueryPerformanceCounter((LARGE_INTEGER *)&_tx_win32_time_stamp); 


/* Define the port specific options for the _tx_build_options variable. This variable indicates
   how the ThreadX library was built.  */

#define TX_PORT_SPECIFIC_BUILD_OPTIONS          0


/* Define the in-line initialization constant so that modules with in-line
   initialization capabilities can prevent their initialization from being
   a function call.  */

#define TX_INLINE_INITIALIZATION


/* Define the Win32-specific initialization code that is expanded in the generic source.  */

void    _tx_initialize_start_interrupts(void);

#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION                       _tx_initialize_start_interrupts();


/* Determine whether or not stack checking is enabled. By default, ThreadX stack checking is 
   disabled. When the following is defined, ThreadX thread stack checking is enabled.  If stack
   checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking
   logic.  */

#ifdef TX_ENABLE_STACK_CHECKING
#undef TX_DISABLE_STACK_FILLING
#endif


/* Define the TX_THREAD control block extensions for this port. The main reason
   for the multiple macros is so that backward compatibility can be maintained with 
   existing ThreadX kernel awareness modules.  */

#define TX_THREAD_EXTENSION_0                                               HANDLE tx_thread_win32_thread_handle; \
                                                                            DWORD  tx_thread_win32_thread_id; \
                                                                            HANDLE tx_thread_win32_thread_run_semaphore; \
                                                                            UINT   tx_thread_win32_suspension_type; \
                                                                            UINT   tx_thread_win32_int_disabled_flag;
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


/* Define the ThreadX object deletion extensions for the remaining objects.  */

#define TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)
#define TX_MUTEX_DELETE_EXTENSION(mutex_ptr)
#define TX_QUEUE_DELETE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)
#define TX_TIMER_DELETE_EXTENSION(timer_ptr)


struct TX_THREAD_STRUCT;

/* Define the Win32 critical section data structure.  */

typedef struct TX_WIN32_CRITICAL_SECTION_STRUCT
{
    HANDLE                                      tx_win32_critical_section_mutex_handle;
    DWORD                                       tx_win32_critical_section_owner;
    ULONG                                       tx_win32_critical_section_nested_count;
} TX_WIN32_CRITICAL_SECTION;


/* Define Win32-specific critical section APIs.  */

void  _tx_win32_critical_section_obtain(TX_WIN32_CRITICAL_SECTION *critical_section);
void  _tx_win32_critical_section_release(TX_WIN32_CRITICAL_SECTION *critical_section);
void  _tx_win32_critical_section_release_all(TX_WIN32_CRITICAL_SECTION *critical_section);


/* Define post completion processing for tx_thread_delete, so that the Win32 thread resources are properly removed.  */

#define TX_THREAD_DELETE_PORT_COMPLETION(thread_ptr)                            \
{                                                                               \
BOOL            win32_status;                                                   \
DWORD           exitcode;                                                       \
HANDLE          threadrunsemaphore;                                             \
HANDLE          threadhandle;                                                   \
    threadhandle =       thread_ptr -> tx_thread_win32_thread_handle;           \
    threadrunsemaphore = thread_ptr -> tx_thread_win32_thread_run_semaphore;    \
    _tx_thread_interrupt_restore(tx_saved_posture);                             \
    do                                                                          \
    {                                                                           \
        win32_status =  GetExitCodeThread(threadhandle, &exitcode);             \
        if ((win32_status) && (exitcode != STILL_ACTIVE))                       \
        {                                                                       \
            break;                                                              \
        }                                                                       \
        ResumeThread(threadhandle);                                             \
        ReleaseSemaphore(threadrunsemaphore, 1, NULL);                          \
        Sleep(1);                                                               \
    } while (1);                                                                \
    CloseHandle(threadhandle);                                                  \
    tx_saved_posture =   _tx_thread_interrupt_disable();                        \
}


/* Define post completion processing for tx_thread_reset, so that the Win32 thread resources are properly removed.  */

#define TX_THREAD_RESET_PORT_COMPLETION(thread_ptr)                             \
{                                                                               \
BOOL            win32_status;                                                   \
DWORD           exitcode;                                                       \
HANDLE          threadrunsemaphore;                                             \
HANDLE          threadhandle;                                                   \
    threadhandle =       thread_ptr -> tx_thread_win32_thread_handle;           \
    threadrunsemaphore = thread_ptr -> tx_thread_win32_thread_run_semaphore;    \
    _tx_thread_interrupt_restore(tx_saved_posture);                             \
    do                                                                          \
    {                                                                           \
        win32_status =  GetExitCodeThread(threadhandle, &exitcode);             \
        if ((win32_status) && (exitcode != STILL_ACTIVE))                       \
        {                                                                       \
            break;                                                              \
        }                                                                       \
        ResumeThread(threadhandle);                                             \
        ReleaseSemaphore(threadrunsemaphore, 1, NULL);                          \
        Sleep(1);                                                               \
    } while (1);                                                                \
    CloseHandle(threadhandle);                                                  \
    tx_saved_posture =   _tx_thread_interrupt_disable();                        \
}


/* Define ThreadX interrupt lockout and restore macros for protection on 
   access of critical kernel information.  The restore interrupt macro must 
   restore the interrupt posture of the running thread prior to the value 
   present prior to the disable macro.  In most cases, the save area macro
   is used to define a local function save area for the disable and restore
   macros.  */

UINT   _tx_thread_interrupt_disable(void);
VOID   _tx_thread_interrupt_restore(UINT previous_posture);

#define TX_INTERRUPT_SAVE_AREA UINT             tx_saved_posture;

#define TX_DISABLE                              tx_saved_posture =   _tx_thread_interrupt_disable();

#define TX_RESTORE                              _tx_thread_interrupt_restore(tx_saved_posture);


/* Define the interrupt lockout macros for each ThreadX object.  */

#define TX_BLOCK_POOL_DISABLE                   TX_DISABLE
#define TX_BYTE_POOL_DISABLE                    TX_DISABLE
#define TX_EVENT_FLAGS_GROUP_DISABLE            TX_DISABLE
#define TX_MUTEX_DISABLE                        TX_DISABLE
#define TX_QUEUE_DISABLE                        TX_DISABLE
#define TX_SEMAPHORE_DISABLE                    TX_DISABLE


/* Define the version ID of ThreadX.  This may be utilized by the application.  */

#ifdef TX_THREAD_INIT
CHAR                            _tx_version_id[] = 
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  ThreadX Win32/Visual Studio Version 6.1.9 *";
#else
extern  CHAR                    _tx_version_id[];
#endif


/* Define externals for the Win32 port of ThreadX.  */

extern TX_WIN32_CRITICAL_SECTION                _tx_win32_critical_section;
extern HANDLE                                   _tx_win32_scheduler_semaphore;
extern DWORD                                    _tx_win32_scheduler_id;
extern ULONG                                    _tx_win32_global_int_disabled_flag;
extern LARGE_INTEGER                            _tx_win32_time_stamp;
extern ULONG                                    _tx_win32_system_error;
extern HANDLE                                   _tx_win32_timer_handle;
extern DWORD                                    _tx_win32_timer_id;
extern LARGE_INTEGER                            _tx_win32_time_stamp;


#ifndef TX_WIN32_MEMORY_SIZE
#define TX_WIN32_MEMORY_SIZE                    64000
#endif

#ifndef TX_TIMER_PERIODIC
#define TX_TIMER_PERIODIC                       18
#endif

#endif




