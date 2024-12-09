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
/*    _txe_thread_entry_exit_notify                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the thread entry/exit notify     */
/*    callback function call.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread             */
/*    thread_entry_exit_notify              Pointer to notify callback    */
/*                                            function, TX_NULL to disable*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_entry_exit_notify          Actual entry/exit notify      */
/*                                            function                    */
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
UINT  _txe_thread_entry_exit_notify(TX_THREAD *thread_ptr, VOID (*thread_entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT type))
{

UINT    status;


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
    else
    {

        /* Call actual thread entry/exit notify function.  */
        status =  _tx_thread_entry_exit_notify(thread_ptr, thread_entry_exit_notify);
    }

    /* Return completion status.  */
    return(status);
}

