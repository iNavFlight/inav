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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_semaphore.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_semaphore_get                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the semaphore get function call. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore_ptr                     Pointer to semaphore control block*/
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SEMAPHORE_ERROR                Invalid semaphore pointer         */
/*    TX_WAIT_ERROR                     Invalid wait option               */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_semaphore_get                 Actual get semaphore function     */
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
UINT  _txe_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option)
{

UINT        status;

#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD   *current_thread;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid semaphore pointer.  */
    if (semaphore_ptr == TX_NULL)
    {

        /* Semaphore pointer is invalid, return appropriate error code.  */
        status =  TX_SEMAPHORE_ERROR;
    }

    /* Now check for invalid semaphore ID.  */
    else if (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID)
    {

        /* Semaphore pointer is invalid, return appropriate error code.  */
        status =  TX_SEMAPHORE_ERROR;
    }
    else
    {

        /* Check for a wait option error.  Only threads are allowed any form of
           suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Is the call from an ISR or Initialization?  */
            if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
            {

                /* A non-thread is trying to suspend, return appropriate error code.  */
                status =  TX_WAIT_ERROR;
            }

#ifndef TX_TIMER_PROCESS_IN_ISR
            else
            {

                /* Pickup thread pointer.  */
                TX_THREAD_GET_CURRENT(current_thread)

                /* Is the current thread the timer thread?  */
                if (current_thread == &_tx_timer_thread)
                {

                    /* A non-thread is trying to suspend, return appropriate error code.  */
                    status =  TX_WAIT_ERROR;
                }
            }
#endif
        }
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual get semaphore function.  */
        status =  _tx_semaphore_get(semaphore_ptr, wait_option);
    }

    /* Return completion status.  */
    return(status);
}

