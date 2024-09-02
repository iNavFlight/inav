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
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_byte_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_byte_allocate                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in allocate bytes function call.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    memory_ptr                        Pointer to place allocated bytes  */
/*                                        pointer                         */
/*    memory_size                       Number of bytes to allocate       */
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_POOL_ERROR                     Invalid memory pool pointer       */
/*    TX_PTR_ERROR                      Invalid destination pointer       */
/*    TX_WAIT_ERROR                     Invalid wait option               */
/*    TX_CALLER_ERROR                   Invalid caller of this function   */
/*    TX_SIZE_ERROR                     Invalid size of memory request    */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_byte_allocate                 Actual byte allocate function     */
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
UINT  _txe_byte_allocate(TX_BYTE_POOL *pool_ptr, VOID **memory_ptr,
                                    ULONG memory_size,  ULONG wait_option)
{

UINT            status;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid byte pool pointer.  */
    if (pool_ptr == TX_NULL)
    {

        /* Byte pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Now check for invalid pool ID.  */
    else if  (pool_ptr -> tx_byte_pool_id != TX_BYTE_POOL_ID)
    {

        /* Byte pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Check for an invalid destination for return pointer.  */
    else if (memory_ptr == TX_NULL)
    {

        /* Null destination pointer, return appropriate error.  */
        status =  TX_PTR_ERROR;
    }

    /* Check for an invalid memory size.  */
    else if (memory_size == ((ULONG) 0))
    {

        /* Error in size, return appropriate error.  */
        status =  TX_SIZE_ERROR;
    }

    /* Determine if the size is greater than the pool size.  */
    else if (memory_size > pool_ptr -> tx_byte_pool_size)
    {

        /* Error in size, return appropriate error.  */
        status =  TX_SIZE_ERROR;
    }

    else
    {

        /* Check for a wait option error.  Only threads are allowed any form of
           suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Is call from ISR or Initialization?  */
            if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
            {

                /* A non-thread is trying to suspend, return appropriate error code.  */
                status =  TX_WAIT_ERROR;
            }
        }
    }
#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Check for timer execution.  */
    if (status == TX_SUCCESS)
    {

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Check for invalid caller of this function.  First check for a calling thread.  */
        if (thread_ptr == &_tx_timer_thread)
        {

            /* Invalid caller of this function, return appropriate error code.  */
            status =  TX_CALLER_ERROR;
        }
    }
#endif

    /* Is everything still okay?  */
    if (status == TX_SUCCESS)
    {

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

        /* Call actual byte memory allocate function.  */
        status =  _tx_byte_allocate(pool_ptr, memory_ptr, memory_size,  wait_option);
    }

    /* Return completion status.  */
    return(status);
}

