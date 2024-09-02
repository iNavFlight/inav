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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_mutex_delete                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the mutex delete function        */
/*    call.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_MUTEX_ERROR                    Invalid mutex pointer             */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_mutex_delete                  Actual delete mutex function      */
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
UINT  _txe_mutex_delete(TX_MUTEX *mutex_ptr)
{

UINT            status;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Default status to success.  */
    status =  TX_SUCCESS;
#endif

    /* Check for an invalid mutex pointer.  */
    if (mutex_ptr == TX_NULL)
    {

        /* Mutex pointer is invalid, return appropriate error code.  */
        status =  TX_MUTEX_ERROR;
    }

    /* Now check for a valid mutex ID.  */
    else if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
    {

        /* Mutex pointer is invalid, return appropriate error code.  */
        status =  TX_MUTEX_ERROR;
    }

    /* Check for invalid caller of this function.  */

    /* Is the caller an ISR or Initialization?  */
    else if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
    {

        /* Invalid caller of this function, return appropriate error code.  */
        status =  TX_CALLER_ERROR;
    }
    else
    {

#ifndef TX_TIMER_PROCESS_IN_ISR

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Is the caller the system timer thread?  */
        if (thread_ptr == &_tx_timer_thread)
        {

            /* Invalid caller of this function, return appropriate error code.  */
            status =  TX_CALLER_ERROR;
        }

        /* Determine if everything is okay.  */
        if (status == TX_SUCCESS)
        {
#endif

            /* Call actual mutex delete function.  */
            status =  _tx_mutex_delete(mutex_ptr);

#ifndef TX_TIMER_PROCESS_IN_ISR
        }
#endif
    }

    /* Return completion status.  */
    return(status);
}

