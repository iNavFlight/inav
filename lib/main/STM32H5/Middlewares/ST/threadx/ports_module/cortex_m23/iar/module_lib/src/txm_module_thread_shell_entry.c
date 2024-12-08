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
/**   Module                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef TXM_MODULE
#define TXM_MODULE
#endif

#ifndef TX_SOURCE_CODE
#define TX_SOURCE_CODE
#endif


/* Include necessary system files.  */

#include "txm_module.h"
#include "tx_thread.h"

/* Define the global module entry pointer from the start thread of the module.  */

TXM_MODULE_THREAD_ENTRY_INFO    *_txm_module_entry_info;


/* Define the dispatch function pointer used in the module implementation.  */

ULONG                           (*_txm_module_kernel_call_dispatcher)(ULONG kernel_request, ULONG param_1, ULONG param_2, ULONG param3);


/* Define the IAR startup code that clears the uninitialized global data and sets up the
   preset global variables.  */

extern VOID __iar_data_init3(VOID);


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_thread_shell_entry                   Cortex-M23/IAR     */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calls the specified entry function of the thread.  It */
/*    also provides a place for the thread's entry function to return.    */
/*    If the thread returns, this function places the thread in a         */
/*    "COMPLETED" state.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to current thread         */
/*    thread_info                       Pointer to thread entry info      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    __iar_data_init3                  IAR global initialization         */
/*    thread_entry                      Thread's entry function           */
/*    tx_thread_resume                  Resume the module callback thread */
/*    _txm_module_thread_system_suspend Module thread suspension routine  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Initial thread stack frame                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-02-2021      Scott Larson            Initial Version 6.1.6         */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_thread_shell_entry(TX_THREAD *thread_ptr, TXM_MODULE_THREAD_ENTRY_INFO *thread_info)
{

#ifndef TX_DISABLE_NOTIFY_CALLBACKS
    VOID            (*entry_exit_notify)(TX_THREAD *, UINT);
#endif


    /* Determine if this is the start thread.  If so, we must prepare the module for
       execution.  If not, simply skip the C startup code.  */
    if (thread_info -> txm_module_thread_entry_info_start_thread)
    {
        /* Initialize the IAR C environment.  */
        __iar_data_init3();
        
        /* Save the entry info pointer, for later use.  */
        _txm_module_entry_info =  thread_info;
        
        /* Save the kernel function dispatch address. This is used to make all resident calls from
           the module.  */
        _txm_module_kernel_call_dispatcher =  thread_info -> txm_module_thread_entry_info_kernel_call_dispatcher;
        
        /* Ensure that we have a valid pointer.  */
        while (!_txm_module_kernel_call_dispatcher)
        {
            /* Loop here, if an error is present getting the dispatch function pointer!
               An error here typically indicates the resident portion of _tx_thread_schedule
               is not supporting the trap to obtain the function pointer.   */
        }
        
        /* Resume the module's callback thread, already created in the manager.  */
        _txe_thread_resume(thread_info -> txm_module_thread_entry_info_callback_request_thread);
    }

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Pickup the entry/exit application callback routine.  */
    entry_exit_notify =  thread_info -> txm_module_thread_entry_info_exit_notify;

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has been entered!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_ENTRY);
    }
#endif

    /* Call current thread's entry function.  */
    (thread_info -> txm_module_thread_entry_info_entry) (thread_info -> txm_module_thread_entry_info_parameter);

    /* Suspend thread with a "completed" state.  */


#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Pickup the entry/exit application callback routine again.  */
    entry_exit_notify =  thread_info -> txm_module_thread_entry_info_exit_notify;

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has exited!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
    }
#endif

    /* Call actual thread suspension routine.  */
    _txm_module_thread_system_suspend(thread_ptr);

#ifdef TX_SAFETY_CRITICAL

    /* If we ever get here, raise safety critical exception.  */
    TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);
#endif
}

