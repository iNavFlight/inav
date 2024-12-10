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
/**   Block Pool                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_block_pool_create                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the create block memory pool     */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    name_ptr                          Pointer to block pool name        */
/*    block_size                        Number of bytes in each block     */
/*    pool_start                        Address of beginning of pool area */
/*    pool_size                         Number of bytes in the block pool */
/*    pool_control_block_size           Size of block pool control block  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_POOL_ERROR                     Invalid pool pointer              */
/*    TX_PTR_ERROR                      Invalid starting address          */
/*    TX_SIZE_ERROR                     Invalid pool size                 */
/*    TX_CALLER_ERROR                   Invalid caller of pool            */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_block_pool_create             Actual block pool create function */
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
UINT  _txe_block_pool_create(TX_BLOCK_POOL *pool_ptr, CHAR *name_ptr, ULONG block_size,
                    VOID *pool_start, ULONG pool_size, UINT pool_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
ULONG           i;
TX_BLOCK_POOL   *next_pool;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid pool pointer.  */
    if (pool_ptr == TX_NULL)
    {

        /* Pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Check for invalid control block size.  */
    else if (pool_control_block_size != (sizeof(TX_BLOCK_POOL)))
    {

        /* Pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
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
        next_pool =   _tx_block_pool_created_ptr;
        for (i = ((ULONG) 0); i < _tx_block_pool_created_count; i++)
        {

            /* Determine if this block pool matches the pool in the list.  */
            if (pool_ptr == next_pool)
            {

                break;
            }
            else
            {
                /* Move to the next pool.  */
                next_pool =  next_pool -> tx_block_pool_created_next;
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

        /* At this point, check to see if there is a duplicate pool.  */
        if (pool_ptr == next_pool)
        {

            /* Pool is already created, return appropriate error code.  */
            status =  TX_POOL_ERROR;
        }

        /* Check for an invalid starting address.  */
        else if (pool_start == TX_NULL)
        {

            /* Null starting address pointer, return appropriate error.  */
            status =  TX_PTR_ERROR;
        }
        else
        {

            /* Check for invalid pool size.  */
            if ((((block_size/(sizeof(void *)))*(sizeof(void *))) + (sizeof(void *))) >
                                            ((pool_size/(sizeof(void *)))*(sizeof(void *))))
            {

                /* Not enough memory for one block, return appropriate error.  */
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
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual block pool create function.  */
        status =  _tx_block_pool_create(pool_ptr, name_ptr, block_size, pool_start, pool_size);
    }

    /* Return completion status.  */
    return(status);
}

