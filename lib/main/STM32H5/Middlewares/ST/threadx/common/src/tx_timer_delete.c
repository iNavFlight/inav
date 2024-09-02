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
/*    _tx_timer_delete                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes the specified application timer.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_timer_system_deactivate       Timer deactivation function       */
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
UINT  _tx_timer_delete(TX_TIMER *timer_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_TIMER        *next_timer;
TX_TIMER        *previous_timer;


    /* Disable interrupts to remove the timer from the created list.  */
    TX_DISABLE

    /* Determine if the timer needs to be deactivated.  */
    if (timer_ptr -> tx_timer_internal.tx_timer_internal_list_head != TX_NULL)
    {

        /* Yes, deactivate the timer before it is deleted.  */
        _tx_timer_system_deactivate(&(timer_ptr -> tx_timer_internal));
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_DELETE, timer_ptr, 0, 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Optional timer delete extended processing.  */
    TX_TIMER_DELETE_EXTENSION(timer_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(timer_ptr)

    /* Log this kernel call.  */
    TX_EL_TIMER_DELETE_INSERT

    /* Clear the timer ID to make it invalid.  */
    timer_ptr -> tx_timer_id =  TX_CLEAR_ID;

    /* Decrement the number of created timers.  */
    _tx_timer_created_count--;

    /* See if the timer is the only one on the list.  */
    if (_tx_timer_created_count == TX_EMPTY)
    {

        /* Only created timer, just set the created list to NULL.  */
        _tx_timer_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_timer =                               timer_ptr -> tx_timer_created_next;
        previous_timer =                           timer_ptr -> tx_timer_created_previous;
        next_timer -> tx_timer_created_previous =  previous_timer;
        previous_timer -> tx_timer_created_next =  next_timer;

        /* See if we have to update the created list head pointer.  */
        if (_tx_timer_created_ptr == timer_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _tx_timer_created_ptr =  next_timer;
        }
    }

    /* Execute Port-Specific completion processing. If needed, it is typically defined in tx_port.h.  */
    TX_TIMER_DELETE_PORT_COMPLETION(timer_ptr)

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

