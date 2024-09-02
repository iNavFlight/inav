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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_system_deactivate                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deactivates, or removes the timer from the active     */
/*    timer expiration list.  If the timer is already deactivated, this   */
/*    function just returns.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Always returns success            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_system_resume          Thread resume function            */
/*    _tx_timer_thread_entry            Timer thread processing           */
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
VOID  _tx_timer_system_deactivate(TX_TIMER_INTERNAL *timer_ptr)
{

TX_TIMER_INTERNAL   **list_head;
TX_TIMER_INTERNAL   *next_timer;
TX_TIMER_INTERNAL   *previous_timer;


    /* Pickup the list head pointer.  */
    list_head =  timer_ptr -> tx_timer_internal_list_head;

    /* Determine if the timer still needs deactivation.  */
    if (list_head != TX_NULL)
    {

        /* Deactivate the timer.  */

        /* Pickup the next active timer.  */
        next_timer =  timer_ptr -> tx_timer_internal_active_next;

        /* See if this is the only timer in the list.  */
        if (timer_ptr == next_timer)
        {

            /* Yes, the only timer on the list.  */

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == timer_ptr)
            {

                /* Update the head pointer.  */
                *(list_head) =  TX_NULL;
            }
        }
        else
        {

            /* At least one more timer is on the same expiration list.  */

            /* Update the links of the adjacent timers.  */
            previous_timer =                                   timer_ptr -> tx_timer_internal_active_previous;
            next_timer -> tx_timer_internal_active_previous =  previous_timer;
            previous_timer -> tx_timer_internal_active_next =  next_timer;

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == timer_ptr)
            {

                /* Update the next timer in the list with the list head pointer.  */
                next_timer -> tx_timer_internal_list_head =  list_head;

                /* Update the head pointer.  */
                *(list_head) =  next_timer;
            }
        }

        /* Clear the timer's list head pointer.  */
        timer_ptr -> tx_timer_internal_list_head =  TX_NULL;
    }
}

