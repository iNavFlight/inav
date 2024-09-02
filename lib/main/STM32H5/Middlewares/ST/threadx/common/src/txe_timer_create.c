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
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_timer_create                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the create application timer     */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*    name_ptr                          Pointer to timer name             */
/*    expiration_function               Application expiration function   */
/*    initial_ticks                     Initial expiration ticks          */
/*    reschedule_ticks                  Reschedule ticks                  */
/*    auto_activate                     Automatic activation flag         */
/*    timer_control_block_size          Size of timer control block       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TIMER_ERROR                    Invalid timer control block       */
/*    TX_TICK_ERROR                     Invalid initial expiration count  */
/*    TX_ACTIVATE_ERROR                 Invalid timer activation option   */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check   Check for preemption              */
/*    _tx_timer_create                  Actual timer create function      */
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
UINT  _txe_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr,
            VOID (*expiration_function)(ULONG id), ULONG expiration_input,
            ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate, UINT timer_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
ULONG           i;
TX_TIMER        *next_timer;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for a NULL timer pointer.  */
    if (timer_ptr == TX_NULL)
    {

        /* Timer pointer is invalid, return appropriate error code.  */
        status =  TX_TIMER_ERROR;
    }

    /* Now check for invalid control block size.  */
    else if (timer_control_block_size != (sizeof(TX_TIMER)))
    {

        /* Timer pointer is invalid, return appropriate error code.  */
        status =  TX_TIMER_ERROR;
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
        next_timer =  _tx_timer_created_ptr;
        for (i = ((ULONG) 0); i < _tx_timer_created_count; i++)
        {

            /* Determine if this timer matches the current timer in the list.  */
            if (timer_ptr == next_timer)
            {

                break;
            }
            else
            {

                /* Move to next timer.  */
                next_timer =  next_timer -> tx_timer_created_next;
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

        /* At this point, check to see if there is a duplicate timer.  */
        if (timer_ptr == next_timer)
        {

            /* Timer is already created, return appropriate error code.  */
            status =  TX_TIMER_ERROR;
        }

        /* Check for an illegal initial tick value.  */
        else if (initial_ticks == ((ULONG) 0))
        {

            /* Invalid initial tick value, return appropriate error code.  */
            status =  TX_TICK_ERROR;
        }
        else
        {

            /* Check for an illegal activation.  */
            if (auto_activate != TX_AUTO_ACTIVATE)
            {

                /* And activation is not the other value.  */
                if (auto_activate != TX_NO_ACTIVATE)
                {

                    /* Invalid activation selected, return appropriate error code.  */
                    status =  TX_ACTIVATE_ERROR;
                }
            }
        }
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
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


    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual application timer create function.  */
        status =  _tx_timer_create(timer_ptr, name_ptr, expiration_function, expiration_input,
                                                    initial_ticks, reschedule_ticks, auto_activate);
    }

    /* Return completion status.  */
    return(status);
}

