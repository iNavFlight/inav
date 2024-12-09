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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_thread_preemption_change                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the preemption threshold change  */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread             */
/*    new_threshold                         New preemption threshold      */
/*    old_threshold                         Old preemption threshold      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_THREAD_ERROR                       Invalid thread pointer        */
/*    TX_PTR_ERROR                          Invalid old threshold pointer */
/*    TX_CALLER_ERROR                       Invalid caller of function    */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_preemption_change          Actual preempt change function*/
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
UINT  _txe_thread_preemption_change(TX_THREAD *thread_ptr, UINT new_threshold, UINT *old_threshold)
{

UINT        status;


    /* Check for an invalid thread pointer.  */
    if (thread_ptr == TX_NULL)
    {

        /* Thread pointer is invalid, return appropriate error code.  */
        status =  TX_THREAD_ERROR;
    }

    /* Now check for invalid thread ID.  */
    else if (thread_ptr -> tx_thread_id != TX_THREAD_ID)
    {

        /* Thread pointer is invalid, return appropriate error code.  */
        status =  TX_THREAD_ERROR;
    }

    /* Check for a valid old threshold pointer.  */
    else if (old_threshold == TX_NULL)
    {

        /* Invalid destination pointer, return appropriate error code.  */
        status =  TX_PTR_ERROR;
    }

    /* Check for invalid caller of this function.  */
    else if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
    {

        /* Invalid caller of this function, return appropriate error code.  */
        status =  TX_CALLER_ERROR;
    }

    /* Determine if the preemption-threshold is valid.  */
    else if (new_threshold > thread_ptr -> tx_thread_user_priority)
    {

        /* Return an error status.  */
        status =  TX_THRESH_ERROR;
    }
    else
    {

        /* Call actual change thread preemption function.  */
        status =  _tx_thread_preemption_change(thread_ptr, new_threshold, old_threshold);
    }

    /* Return completion status.  */
    return(status);
}

