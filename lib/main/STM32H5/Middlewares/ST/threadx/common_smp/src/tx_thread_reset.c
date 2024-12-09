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
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_reset                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function prepares the thread to run again from the entry       */
/*    point specified during thread creation. The application must        */
/*    call tx_thread_resume after this call completes for the thread      */
/*    to actually run.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to reset    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_stack_build                Build initial thread stack    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_reset(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *current_thread;
UINT            status;


    /* Default a successful completion status.  */
    status =  TX_SUCCESS;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* Check for a call from the current thread, which is not allowed!  */
    if (current_thread == thread_ptr)
    {

        /* Thread not completed or terminated - return an error!  */
        status =  TX_NOT_DONE;
    }
    else
    {

        /* Check for proper status of this thread to reset.  */
        if (thread_ptr -> tx_thread_state != TX_COMPLETED)
        {

            /* Now check for terminated state.  */
            if (thread_ptr -> tx_thread_state != TX_TERMINATED)
            {

                /* Thread not completed or terminated - return an error!  */
                status =  TX_NOT_DONE;
            }
        }
    }

    /* Is the request valid?  */
    if (status == TX_SUCCESS)
    {

        /* Modify the thread status to prevent additional reset calls.  */
        thread_ptr -> tx_thread_state =  TX_NOT_DONE;

        /* Execute Port-Specific completion processing. If needed, it is typically defined in tx_port.h.  */
        TX_THREAD_RESET_PORT_COMPLETION(thread_ptr)

        /* Restore interrupts.  */
        TX_RESTORE

#ifndef TX_DISABLE_STACK_FILLING

        /* Set the thread stack to a pattern prior to creating the initial
           stack frame.  This pattern is used by the stack checking routines
           to see how much has been used.  */
        TX_MEMSET(thread_ptr -> tx_thread_stack_start, ((UCHAR) TX_STACK_FILL), thread_ptr -> tx_thread_stack_size);
#endif

        /* Call the target specific stack frame building routine to build the
           thread's initial stack and to setup the actual stack pointer in the
           control block.  */
        _tx_thread_stack_build(thread_ptr, _tx_thread_shell_entry);

        /* Disable interrupts.  */
        TX_DISABLE

        /* Finally, move into a suspended state to allow for the thread to be resumed.  */
        thread_ptr -> tx_thread_state =  TX_SUSPENDED;

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESET, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

        /* Log this kernel call.  */
        TX_EL_THREAD_RESET_INSERT

        /* Log the thread status change.  */
        TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_SUSPENDED)
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status to caller.  */
    return(status);
}

