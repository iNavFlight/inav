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
#ifdef TX_ENABLE_STACK_CHECKING
#include "tx_trace.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_stack_error_notify                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers an application stack error handler. If      */
/*    ThreadX detects a stack error, this application handler is called.  */
/*                                                                        */
/*    Note: stack checking must be enabled for this routine to serve any  */
/*    purpose via the TX_ENABLE_STACK_CHECKING define.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    stack_error_handler                   Pointer to stack error        */
/*                                            handler, TX_NULL to disable */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_stack_error_notify(VOID (*stack_error_handler)(TX_THREAD *thread_ptr))
{

#ifndef TX_ENABLE_STACK_CHECKING

UINT        status;


    /* Access input argument just for the sake of lint, MISRA, etc.  */
    if (stack_error_handler != TX_NULL)
    {

        /* Stack checking is not enabled, just return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Stack checking is not enabled, just return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
#else

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Make entry in event log.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_STACK_ERROR_NOTIFY, 0, 0, 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Make entry in event log.  */
    TX_EL_THREAD_STACK_ERROR_NOTIFY_INSERT

    /* Setup global thread stack error handler.  */
    _tx_thread_application_stack_error_handler =  stack_error_handler;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return success to caller.  */
    return(TX_SUCCESS);
#endif
}

