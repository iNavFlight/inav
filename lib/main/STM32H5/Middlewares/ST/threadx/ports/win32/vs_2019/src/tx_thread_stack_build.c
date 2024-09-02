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


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include <stdio.h>


/* Prototype for new thread entry function.  */

DWORD WINAPI _tx_win32_thread_entry(LPVOID p);


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_stack_build                            Win32/Visual      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function builds a stack frame on the supplied thread's stack.  */
/*    The stack frame results in a fake interrupt return to the supplied  */
/*    function pointer.                                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread control blk */
/*    function_ptr                          Pointer to return function    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    CreateThread                          Win32 create thread           */ 
/*    ResumeThread                          Win32 resume thread           */ 
/*    SetThreadPriority                     Win32 set thread priority     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_thread_create                     Create thread service         */
/*    _tx_thread_reset                      Reset thread service          */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID))
{

    /* Create a Win32 thread for the application thread.  */
    thread_ptr -> tx_thread_win32_thread_handle =
        CreateThread(NULL, 0, _tx_win32_thread_entry, (LPVOID) thread_ptr, CREATE_SUSPENDED, 
                        &(thread_ptr -> tx_thread_win32_thread_id));

    /* Check for a good thread create.  */
    if (!thread_ptr -> tx_thread_win32_thread_handle)
    {

        /* Display an error message.  */
        printf("ThreadX Win32 error creating thread!\n");
        while(1)
        {
        }
    }

    /* Otherwise, we have a good thread create.  Now set the priority to
       a lower level.  */
    SetThreadPriority(thread_ptr -> tx_thread_win32_thread_handle, THREAD_PRIORITY_LOWEST);

    /* Create the run semaphore for the thread.  This will allow the scheduler
       control over when the thread actually runs.  */
    thread_ptr -> tx_thread_win32_thread_run_semaphore =  CreateSemaphore(NULL, 0, 1, NULL);

    /* Determine if the run semaphore was created successfully.  */
    if (!thread_ptr -> tx_thread_win32_thread_run_semaphore)
    {

        /* Display an error message.  */
        printf("ThreadX Win32 error creating thread running semaphore!\n");
        while(1)
        {
        }
    }

    /* Setup the thread suspension type to solicited thread suspension.  
       Pseudo interrupt handlers will suspend with this field set to 1.  */
    thread_ptr -> tx_thread_win32_suspension_type =  0;

    /* Clear the disabled count that will keep track of the 
       tx_interrupt_control nesting.  */
    thread_ptr -> tx_thread_win32_int_disabled_flag =  0;

    /* Setup a fake thread stack pointer.   */
    thread_ptr -> tx_thread_stack_ptr =  (VOID *) (((CHAR *) thread_ptr -> tx_thread_stack_end) - 8);

    /* Clear the first word of the stack.  */
    *(((ULONG *) thread_ptr -> tx_thread_stack_ptr) - 1) =  0;

    /* Make the thread initially ready so it will run to the initial wait on
       its run semaphore.  */
    ResumeThread(thread_ptr -> tx_thread_win32_thread_handle);
}


DWORD WINAPI _tx_win32_thread_entry(LPVOID ptr)
{

TX_THREAD  *thread_ptr;

    /* Pickup the current thread pointer.  */
    thread_ptr =  (TX_THREAD *) ptr;

    /* Now suspend the thread initially.  If the thread has already
       been scheduled, this will return immediately.  */
    WaitForSingleObject(thread_ptr -> tx_thread_win32_thread_run_semaphore, INFINITE);

    /* Call ThreadX thread entry point.  */
    _tx_thread_shell_entry();

    return EXIT_SUCCESS;
}

