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
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_event_flags.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_event_flags_create                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the event flag creation function */
/*    call.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to event flags group      */
/*                                        control block                   */
/*    name_ptr                          Pointer to event flags name       */
/*    event_control_block_size          Size of event flags control block */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_GROUP_ERROR                    Invalid event flag group pointer  */
/*    TX_CALLER_ERROR                   Invalid calling function          */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_event_flags_create            Actual create function            */
/*    _tx_thread_system_preempt_check   Check for preemption              */
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
UINT  _txe_event_flags_create(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR *name_ptr, UINT event_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT                        status;
ULONG                       i;
TX_EVENT_FLAGS_GROUP        *next_group;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD                   *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid event flags group pointer.  */
    if (group_ptr == TX_NULL)
    {

        /* Event flags group pointer is invalid, return appropriate error code.  */
        status =  TX_GROUP_ERROR;
    }

    /* Now check for proper control block size.  */
    else if (event_control_block_size != (sizeof(TX_EVENT_FLAGS_GROUP)))
    {

        /* Event flags group pointer is invalid, return appropriate error code.  */
        status =  TX_GROUP_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Increment the preempt disable flag.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Next see if it is already in the created list.  */
        next_group =   _tx_event_flags_created_ptr;
        for (i = ((ULONG) 0); i < _tx_event_flags_created_count; i++)
        {

            /* Determine if this group matches the event flags group in the list.  */
            if (group_ptr == next_group)
            {

                break;
            }
            else
            {

                /* Move to the next group.  */
                next_group =  next_group -> tx_event_flags_group_created_next;
            }
        }

        /* Disable interrupts.  */
        TX_DISABLE

        /* Decrement the preempt disable flag.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();

        /* At this point, check to see if there is a duplicate event flag group.  */
        if (group_ptr == next_group)
        {

            /* Group is already created, return appropriate error code.  */
            status =  TX_GROUP_ERROR;
        }
        else
        {

#ifndef TX_TIMER_PROCESS_IN_ISR

            /* Pickup thread pointer.  */
            TX_THREAD_GET_CURRENT(thread_ptr)

            /* Check for invalid caller of this function.  First check for a calling thread.  */
            if (thread_ptr == &_tx_timer_thread)
            {

                /* Invalid caller of this function, return appropriate error code.  */
                status =  TX_CALLER_ERROR;
            }
#endif

            /* Check for interrupt call.  */
            if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
            {

                /* Now, make sure the call is from an interrupt and not initialization.  */
                if (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)
                {

                    /* Invalid caller of this function, return appropriate error code.  */
                    status =  TX_CALLER_ERROR;
                }
            }
        }
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual event flags create function.  */
        status =  _tx_event_flags_create(group_ptr, name_ptr);
    }

    /* Return completion status.  */
    return(status);
}

