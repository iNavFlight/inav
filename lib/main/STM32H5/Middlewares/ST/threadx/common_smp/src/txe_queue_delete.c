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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"
#include "tx_queue.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_queue_delete                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the queue delete function call.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    queue_ptr                         Pointer to queue control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_QUEUE_ERROR                    Invalid queue pointer             */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_queue_delete                  Actual queue delete function      */
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
UINT  _txe_queue_delete(TX_QUEUE *queue_ptr)
{

UINT            status;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid queue pointer.  */
    if (queue_ptr == TX_NULL)
    {

        /* Queue pointer is invalid, return appropriate error code.  */
        status =  TX_QUEUE_ERROR;
    }

    /* Now check for a valid queue ID.  */
    else if (queue_ptr -> tx_queue_id != TX_QUEUE_ID)
    {

        /* Queue pointer is invalid, return appropriate error code.  */
        status =  TX_QUEUE_ERROR;
    }
    else
    {

        /* Check for invalid caller of this function.  */

        /* Is the caller an ISR or Initialization?  */
        if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
        {

            /* Invalid caller of this function, return appropriate error code.  */
            status =  TX_CALLER_ERROR;
        }

#ifndef TX_TIMER_PROCESS_IN_ISR
        else
        {

            /* Pickup thread pointer.  */
            TX_THREAD_GET_CURRENT(thread_ptr)

            /* Is the caller the system timer thread?  */
            if (thread_ptr == &_tx_timer_thread)
            {

                /* Invalid caller of this function, return appropriate error code.  */
                status =  TX_CALLER_ERROR;
            }
        }
#endif
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual queue delete function.  */
        status =  _tx_queue_delete(queue_ptr);
    }

    /* Return completion status.  */
    return(status);
}

