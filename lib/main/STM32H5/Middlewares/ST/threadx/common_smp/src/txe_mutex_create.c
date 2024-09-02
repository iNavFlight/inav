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
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_mutex_create                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the create mutex function        */
/*    call.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*    name_ptr                          Pointer to mutex name             */
/*    inherit                           Initial mutex count               */
/*    mutex_control_block_size          Size of mutex control block       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_MUTEX_ERROR                    Invalid mutex pointer             */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    TX_INHERIT_ERROR                  Invalid inherit option            */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_mutex_create                  Actual create mutex function      */
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
UINT  _txe_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit, UINT mutex_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
ULONG           i;
TX_MUTEX        *next_mutex;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid mutex pointer.  */
    if (mutex_ptr == TX_NULL)
    {

        /* Mutex pointer is invalid, return appropriate error code.  */
        status =  TX_MUTEX_ERROR;
    }

    /* Now check to make sure the control block is the correct size.  */
    else if (mutex_control_block_size != (sizeof(TX_MUTEX)))
    {

        /* Mutex pointer is invalid, return appropriate error code.  */
        status =  TX_MUTEX_ERROR;
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
        next_mutex =   _tx_mutex_created_ptr;
        for (i = ((ULONG) 0); i < _tx_mutex_created_count; i++)
        {

            /* Determine if this mutex matches the mutex in the list.  */
            if (mutex_ptr == next_mutex)
            {

                break;
            }
            else
            {

                /* Move to the next mutex.  */
                next_mutex =  next_mutex -> tx_mutex_created_next;
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

        /* At this point, check to see if there is a duplicate mutex.  */
        if (mutex_ptr == next_mutex)
        {

            /* Mutex is already created, return appropriate error code.  */
            status =  TX_MUTEX_ERROR;
        }
        else
        {

            /* Check for a valid inherit option.  */
            if (inherit != TX_INHERIT)
            {

                if (inherit != TX_NO_INHERIT)
                {

                    /* Inherit option is illegal.  */
                    status =  TX_INHERIT_ERROR;
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

        /* Call actual mutex create function.  */
        status =  _tx_mutex_create(mutex_ptr, name_ptr, inherit);
    }

    /* Return completion status.  */
    return(status);
}

