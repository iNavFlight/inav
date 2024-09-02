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
#if defined(TX_MISRA_ENABLE) || defined(TX_ENABLE_STACK_CHECKING) || defined(TX_PORT_THREAD_STACK_ERROR_HANDLING)
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_stack_error_handler                      PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes stack errors detected during run-time. The  */
/*    processing currently consists of a spin loop.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX internal code                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            update misra support,       */
/*                                            resulting in version 6.1    */
/*  10-16-2020     William E. Lamie         Modified comment(s),          */
/*                                            fixed link issue,           */
/*                                            resulting in version 6.1.1  */
/*  06-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            fixed link issue, added     */
/*                                            conditional compilation     */
/*                                            for ARMv8-M (Cortex M23/33) */
/*                                            resulting in version 6.1.7  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            stack check error handling, */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_stack_error_handler(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

#if defined(TX_ENABLE_STACK_CHECKING) || defined(TX_PORT_THREAD_STACK_ERROR_HANDLING)

    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine if the application has registered an error handler.  */
    if (_tx_thread_application_stack_error_handler != TX_NULL)
    {

        /* Yes, an error handler is present, simply call the application error handler.  */
        (_tx_thread_application_stack_error_handler)(thread_ptr);
    }

    /* Restore interrupts.  */
    TX_RESTORE

#else

    /* Access input argument just for the sake of lint, MISRA, etc.  */
    if (thread_ptr != TX_NULL)
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Restore interrupts.  */
        TX_RESTORE
    }
#endif
}
#endif
