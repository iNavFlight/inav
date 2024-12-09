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
#include "tx_trace.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_deactivate                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deactivates the specified application timer.          */
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
UINT  _tx_timer_deactivate(TX_TIMER *timer_ptr)
{
TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL   *internal_ptr;
TX_TIMER_INTERNAL   **list_head;
TX_TIMER_INTERNAL   *next_timer;
TX_TIMER_INTERNAL   *previous_timer;
ULONG               ticks_left;
UINT                active_timer_list;


    /* Setup internal timer pointer.  */
    internal_ptr =  &(timer_ptr -> tx_timer_internal);

    /* Disable interrupts while the remaining time before expiration is
       calculated.  */
    TX_DISABLE

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

    /* Increment the total deactivations counter.  */
    _tx_timer_performance_deactivate_count++;

    /* Increment the number of deactivations on this timer.  */
    timer_ptr -> tx_timer_performance_deactivate_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_DEACTIVATE, timer_ptr, TX_POINTER_TO_ULONG_CONVERT(&ticks_left), 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Log this kernel call.  */
    TX_EL_TIMER_DEACTIVATE_INSERT

    /* Pickup the list head.  */
    list_head =  internal_ptr -> tx_timer_internal_list_head;

    /* Is the timer active?  */
    if (list_head != TX_NULL)
    {

        /* Default the active timer list flag to false.  */
        active_timer_list =  TX_FALSE;

        /* Determine if the head pointer is within the timer expiration list.  */
        if (TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(list_head) >= TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(_tx_timer_list_start))
        {

            /* Now check to make sure the list head is before the end of the list.  */
            if (TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(list_head) < TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(_tx_timer_list_end))
            {

                /* Set the active timer list flag to true.  */
                active_timer_list =  TX_TRUE;
            }
        }

        /* Determine if the timer is on active timer list.  */
        if (active_timer_list == TX_TRUE)
        {

            /* This timer is active and has not yet expired.  */

            /* Calculate the amount of time that has elapsed since the timer
               was activated.  */

            /* Is this timer's entry after the current timer pointer?  */
            if (TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(list_head) >= TX_TIMER_INDIRECT_TO_VOID_POINTER_CONVERT(_tx_timer_current_ptr))
            {

                /* Calculate ticks left to expiration - just the difference between this
                   timer's entry and the current timer pointer.  */
                ticks_left =  (ULONG) (TX_TIMER_POINTER_DIF(list_head,_tx_timer_current_ptr)) + ((ULONG) 1);
            }
            else
            {

                /* Calculate the ticks left with a wrapped list condition.  */
                ticks_left =  (ULONG) (TX_TIMER_POINTER_DIF(list_head,_tx_timer_list_start));

                ticks_left =  ticks_left + (ULONG) ((TX_TIMER_POINTER_DIF(_tx_timer_list_end, _tx_timer_current_ptr)) + ((ULONG) 1));
            }

            /* Adjust the remaining ticks accordingly.  */
            if (internal_ptr -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
            {

                /* Subtract off the last full pass through the timer list and add the
                   time left.  */
                internal_ptr -> tx_timer_internal_remaining_ticks =
                        (internal_ptr -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES) + ticks_left;
            }
            else
            {

                /* Just put the ticks left into the timer's remaining ticks.  */
                internal_ptr -> tx_timer_internal_remaining_ticks =  ticks_left;
            }
        }
        else
        {

            /* Determine if this is timer has just expired.  */
            if (_tx_timer_expired_timer_ptr != internal_ptr)
            {

                /* No, it hasn't expired. Now check for remaining time greater than the list
                   size.  */
                if (internal_ptr -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
                {

                    /* Adjust the remaining ticks.  */
                    internal_ptr -> tx_timer_internal_remaining_ticks =
                                            internal_ptr -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES;
                }
                else
                {

                    /* Set the remaining time to the reactivation time.  */
                    internal_ptr -> tx_timer_internal_remaining_ticks =  internal_ptr -> tx_timer_internal_re_initialize_ticks;
                }
            }
            else
            {

                /* Set the remaining time to the reactivation time.  */
                internal_ptr -> tx_timer_internal_remaining_ticks =  internal_ptr -> tx_timer_internal_re_initialize_ticks;
            }
        }

        /* Pickup the next timer.  */
        next_timer =  internal_ptr -> tx_timer_internal_active_next;

        /* See if this is the only timer in the list.  */
        if (internal_ptr == next_timer)
        {

            /* Yes, the only timer on the list.  */

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == internal_ptr)
            {

                /* Update the head pointer.  */
                *(list_head) =  TX_NULL;
            }
        }
        else
        {

            /* At least one more timer is on the same expiration list.  */

            /* Update the links of the adjacent timers.  */
            previous_timer =                                   internal_ptr -> tx_timer_internal_active_previous;
            next_timer -> tx_timer_internal_active_previous =  previous_timer;
            previous_timer -> tx_timer_internal_active_next =  next_timer;

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == internal_ptr)
            {

                /* Update the next timer in the list with the list head
                   pointer.  */
                next_timer -> tx_timer_internal_list_head =  list_head;

                /* Update the head pointer.  */
                *(list_head) =  next_timer;
            }
        }

        /* Clear the timer's list head pointer.  */
        internal_ptr -> tx_timer_internal_list_head =  TX_NULL;
    }

    /* Restore interrupts to previous posture.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

