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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_event_flags.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_event_flags_get                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the event flags get function     */
/*    call.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to group control block    */
/*    requested_event_flags             Event flags requested             */
/*    get_option                        Specifies and/or and clear options*/
/*    actual_flags_ptr                  Pointer to place the actual flags */
/*                                        the service retrieved           */
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_GROUP_ERROR                    Invalid event flags group pointer */
/*    TX_PTR_ERROR                      Invalid actual flags pointer      */
/*    TX_WAIT_ERROR                     Invalid wait option               */
/*    TX_OPTION_ERROR                   Invalid get option                */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_event_flags_get               Actual event flags get function   */
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
UINT  _txe_event_flags_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG requested_flags,
                    UINT get_option, ULONG *actual_flags_ptr, ULONG wait_option)
{

UINT            status;

#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *current_thread;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid event flag group pointer.  */
    if (group_ptr == TX_NULL)
    {

        /* Event flags group pointer is invalid, return appropriate error code.  */
        status =  TX_GROUP_ERROR;
    }

    /* Now check for invalid event group ID.  */
    else if (group_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID)
    {

        /* Event flags group pointer is invalid, return appropriate error code.  */
        status =  TX_GROUP_ERROR;
    }

    /* Check for an invalid destination for actual flags.  */
    else if (actual_flags_ptr == TX_NULL)
    {

        /* Null destination pointer, return appropriate error.  */
        status =  TX_PTR_ERROR;
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

    /* Is everything still okay?  */
    if (status == TX_SUCCESS)
    {

        /* Check for invalid get option.  */
        if (get_option > TX_AND_CLEAR)
        {

            /* Invalid get events option, return appropriate error.  */
            status =  TX_OPTION_ERROR;
        }
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual event flags get function.  */
        status =  _tx_event_flags_get(group_ptr, requested_flags, get_option, actual_flags_ptr, wait_option);
    }

    /* Return completion status.  */
    return(status);
}

