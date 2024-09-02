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
#include "tx_initialize.h"
#include "tx_timer.h"
#include "tx_thread.h"
#include "tx_queue.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_queue_create                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the queue create function call.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    queue_ptr                         Pointer to queue control block    */
/*    name_ptr                          Pointer to queue name             */
/*    message_size                      Size of each queue message        */
/*    queue_start                       Starting address of the queue area*/
/*    queue_size                        Number of bytes in the queue      */
/*    queue_control_block_size          Size of queue control block       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_QUEUE_ERROR                    Invalid queue pointer             */
/*    TX_PTR_ERROR                      Invalid starting address of queue */
/*    TX_SIZE_ERROR                     Invalid message queue size        */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_queue_create                  Actual queue create function      */
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
UINT  _txe_queue_create(TX_QUEUE *queue_ptr, CHAR *name_ptr, UINT message_size,
                        VOID *queue_start, ULONG queue_size, UINT queue_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
ULONG           i;
TX_QUEUE        *next_queue;
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

    /* Now check for a valid control block size.  */
    else if (queue_control_block_size != (sizeof(TX_QUEUE)))
    {

        /* Queue pointer is invalid, return appropriate error code.  */
        status =  TX_QUEUE_ERROR;
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
        next_queue =   _tx_queue_created_ptr;
        for (i = ((ULONG) 0); i < _tx_queue_created_count; i++)
        {

            /* Determine if this queue matches the queue in the list.  */
            if (queue_ptr == next_queue)
            {

                break;
            }
            else
            {

                /* Move to the next queue.  */
                next_queue =  next_queue -> tx_queue_created_next;
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

        /* At this point, check to see if there is a duplicate queue.  */
        if (queue_ptr == next_queue)
        {

            /* Queue is already created, return appropriate error code.  */
            status =  TX_QUEUE_ERROR;
        }

        /* Check the starting address of the queue.  */
        else if (queue_start == TX_NULL)
        {

            /* Invalid starting address of queue.  */
            status =  TX_PTR_ERROR;
        }

        /* Check for an invalid message size - less than 1.  */
        else if (message_size < TX_1_ULONG)
        {

            /* Invalid message size specified.  */
            status =  TX_SIZE_ERROR;
        }

        /* Check for an invalid message size - greater than 16.  */
        else if (message_size > TX_16_ULONG)
        {

            /* Invalid message size specified.  */
            status =  TX_SIZE_ERROR;
        }

        /* Check on the queue size.  */
        else if ((queue_size/(sizeof(ULONG))) < message_size)
        {

            /* Invalid queue size specified.  */
            status =  TX_SIZE_ERROR;
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

        /* Call actual queue create function.  */
        status =  _tx_queue_create(queue_ptr, name_ptr, message_size, queue_start, queue_size);
    }

    /* Return completion status.  */
    return(status);
}
