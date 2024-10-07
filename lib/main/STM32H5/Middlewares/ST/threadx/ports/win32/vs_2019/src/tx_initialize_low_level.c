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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


/* Define various Win32 objects used by the ThreadX port.  */

TX_WIN32_CRITICAL_SECTION       _tx_win32_critical_section;
HANDLE                          _tx_win32_scheduler_semaphore;
DWORD                           _tx_win32_scheduler_id;
ULONG                           _tx_win32_global_int_disabled_flag;
LARGE_INTEGER                   _tx_win32_time_stamp;
ULONG                           _tx_win32_system_error;
extern TX_THREAD                *_tx_thread_current_ptr;


/* Define simulated timer interrupt.  This is done inside a thread, which is
   how other interrupts may be defined as well.  See code below for an 
   example.  */

HANDLE                          _tx_win32_timer_handle;
DWORD                           _tx_win32_timer_id;
DWORD WINAPI                    _tx_win32_timer_interrupt(LPVOID p);


#ifdef TX_WIN32_DEBUG_ENABLE

extern ULONG                    _tx_thread_system_state;
extern UINT                     _tx_thread_preempt_disable;
extern TX_THREAD                *_tx_thread_current_ptr;
extern TX_THREAD                *_tx_thread_execute_ptr;


/* Define the maximum size of the Win32 debug array.  */

#ifndef TX_WIN32_DEBUG_EVENT_SIZE
#define TX_WIN32_DEBUG_EVENT_SIZE           400
#endif


/* Define debug log in order to debug Win32 issues with this port.  */

typedef struct TX_WIN32_DEBUG_ENTRY_STRUCT
{
    char                                    *tx_win32_debug_entry_action;
    LARGE_INTEGER                           tx_win32_debug_entry_timestamp;
    char                                    *tx_win32_debug_entry_file;
    unsigned long                           tx_win32_debug_entry_line;
    TX_WIN32_CRITICAL_SECTION               tx_win32_debug_entry_critical_section;
    unsigned long                           tx_win32_debug_entry_int_disabled_flag;
    ULONG                                   tx_win32_debug_entry_system_state;
    UINT                                    tx_win32_debug_entry_preempt_disable;
    TX_THREAD                               *tx_win32_debug_entry_current_thread;
    DWORD                                   tx_win32_debug_entry_current_thread_id;
    TX_THREAD                               *tx_win32_debug_entry_execute_thread;
    DWORD                                   tx_win32_debug_entry_execute_thread_id;
    DWORD                                   tx_win32_debug_entry_running_id;
} TX_WIN32_DEBUG_ENTRY;


/* Define the circular array of Win32 debug entries.  */

TX_WIN32_DEBUG_ENTRY    _tx_win32_debug_entry_array[TX_WIN32_DEBUG_EVENT_SIZE];


/* Define the Win32 debug index.  */

unsigned long           _tx_win32_debug_entry_index =  0;


/* Now define the debug entry function.  */
void    _tx_win32_debug_entry_insert(char *action, char *file, unsigned long line)
{


    /* Get the time stamp.  */
    QueryPerformanceCounter((LARGE_INTEGER *)&_tx_win32_time_stamp);

    /* Setup the debug entry.  */
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_action =             action;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_timestamp =          _tx_win32_time_stamp;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_file =               file;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_line =               line;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_critical_section =   _tx_win32_critical_section;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_int_disabled_flag =  _tx_win32_global_int_disabled_flag;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_system_state =       _tx_thread_system_state;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_preempt_disable =    _tx_thread_preempt_disable;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_current_thread =     _tx_thread_current_ptr;
    if (_tx_thread_current_ptr)
        _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_current_thread_id =  _tx_thread_current_ptr -> tx_thread_win32_thread_id;
    else 
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_current_thread_id =  0;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_execute_thread =     _tx_thread_execute_ptr;
    if (_tx_thread_execute_ptr)
        _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_execute_thread_id =  _tx_thread_execute_ptr -> tx_thread_win32_thread_id;
    else 
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_execute_thread_id =  0;
    _tx_win32_debug_entry_array[_tx_win32_debug_entry_index].tx_win32_debug_entry_running_id =         GetCurrentThreadId();

    /* Now move to the next entry.  */
    _tx_win32_debug_entry_index++;
    
    /* Determine if we need to wrap the list.  */
    if (_tx_win32_debug_entry_index >= TX_WIN32_DEBUG_EVENT_SIZE)
    {
    
        /* Yes, wrap the list!  */
        _tx_win32_debug_entry_index =  0;
    }
}

#endif


/* Define the ThreadX timer interrupt handler.  */

void            _tx_timer_interrupt(void);


/* Define other external function references.  */

VOID            _tx_initialize_low_level(VOID);
VOID            _tx_thread_context_save(VOID);
VOID            _tx_thread_context_restore(VOID);


/* Define other external variable references.  */

extern VOID     *_tx_initialize_unused_memory;


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_initialize_low_level                          Win32/Visual      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function is responsible for any low-level processor            */ 
/*    initialization, including setting up interrupt vectors, setting     */ 
/*    up a periodic timer interrupt source, saving the system stack       */ 
/*    pointer for use in ISR processing later, and finding the first      */ 
/*    available RAM memory address for tx_application_define.             */ 
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
/*    CreateMutex                           Win32 create mutex            */ 
/*    CreateThread                          Win32 create thread           */ 
/*    CreateSemaphore                       Win32 create semaphore        */ 
/*    GetCurrentThreadId                    Win32 get current thread ID   */ 
/*    SetProcessAffinityMask                Win32 process affinity set    */ 
/*    SetThreadPriority                     Win32 set thread priority     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_initialize_kernel_enter           ThreadX entry function        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_initialize_low_level(VOID)
{

/* Deprecate TX_WIN32_MULTI_CORE build option and default to restricting
   execution to one core.  */

#ifndef TX_WIN32_BYPASS_AFFINITY_SETUP

    /* Limit this ThreadX simulation on Win32 to a single core.  */
    if (SetProcessAffinityMask( GetCurrentProcess(), 1 ) == 0)
    {
    
        /* Error restricting the process to one core.  */
        printf("ThreadX Win32 error restricting the process to one core!\n");
        while(1)
        {
        }
    }
#endif

    /* Pickup the first available memory address.  */

    /* Save the first available memory address.  */
    _tx_initialize_unused_memory =  malloc(TX_WIN32_MEMORY_SIZE);

    /* Pickup the unique Id of the current thread, which will also be the Id of the scheduler.  */
    _tx_win32_scheduler_id =        GetCurrentThreadId();

    /* Create the system critical section mutex. This is used by the system to block all other access,
       analogous to an interrupt lockout on an embedded target.  */
    _tx_win32_critical_section.tx_win32_critical_section_mutex_handle =  CreateMutex(NULL, FALSE, NULL);
    _tx_win32_critical_section.tx_win32_critical_section_nested_count =  0;
    _tx_win32_critical_section.tx_win32_critical_section_owner =         0;
       
    /* Create the semaphore that regulates when the scheduler executes.  */
    _tx_win32_scheduler_semaphore =  CreateSemaphore(NULL, 0, 1, NULL);

    /* Initialize the global interrupt disabled flag.  */
    _tx_win32_global_int_disabled_flag =  TX_FALSE;
    
    /* Setup periodic timer interrupt.  */
    _tx_win32_timer_handle =
        CreateThread(NULL, 0, _tx_win32_timer_interrupt, (LPVOID) &_tx_win32_timer_handle,CREATE_SUSPENDED, &_tx_win32_timer_id);

    /* Check for a good thread create.  */
    if (!_tx_win32_timer_handle)
    {

        /* Error creating the timer interrupt.  */
        printf("ThreadX Win32 error creating timer interrupt thread!\n");
        while(1)
        {
        }
    }

    /* Otherwise, we have a good thread create.  Now set the priority to
       a level lower than the system thread but higher than the application
       threads.  */
    SetThreadPriority(_tx_win32_timer_handle, THREAD_PRIORITY_BELOW_NORMAL);

    /* Done, return to caller.  */
}


/* This routine is called after initialization is complete in order to start
   all interrupt threads.  Interrupt threads in addition to the timer may 
   be added to this routine as well.  */

void    _tx_initialize_start_interrupts(void)
{

    /* Kick the timer thread off to generate the ThreadX periodic interrupt
       source.  */
    ResumeThread(_tx_win32_timer_handle);
}


/* Define the ThreadX system timer interrupt.  Other interrupts may be simulated
   in a similar way.  */


DWORD WINAPI _tx_win32_timer_interrupt(LPVOID p)
{

    while(1)
    {

        /* Sleep for the desired time.  */
        Sleep(TX_TIMER_PERIODIC);

        /* Call ThreadX context save for interrupt preparation.  */
        _tx_thread_context_save();


        /* Call the ThreadX system timer interrupt processing.  */
        _tx_timer_interrupt();

        /* Call ThreadX context restore for interrupt completion.  */
        _tx_thread_context_restore();
    } 
}
