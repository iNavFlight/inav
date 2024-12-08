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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_secure_stack_allocate                    Cortex-M33      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the secure stack allocate        */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*    stack_size                            Size of secure stack to       */
/*                                            allocate                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_THREAD_ERROR                       Invalid thread pointer        */
/*    TX_CALLER_ERROR                       Invalid caller of function    */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_secure_stack_allocate      Actual stack alloc function   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT    _txe_thread_secure_stack_allocate(TX_THREAD *thread_ptr, ULONG stack_size)
{
#if defined(TX_SINGLE_MODE_SECURE) || defined(TX_SINGLE_MODE_NON_SECURE)
    return(TX_FEATURE_NOT_ENABLED);
#else
UINT    status;
    
    /* Default status to success.  */
    status =  TX_SUCCESS;
    
    /* Check for an invalid thread pointer.  */
    if (thread_ptr == TX_NULL)
    {
        /* Thread pointer is invalid, return appropriate error code.  */
        status =  TX_THREAD_ERROR;
    }

    /* Now check for invalid thread ID.  */
    else if (thread_ptr -> tx_thread_id != TX_THREAD_ID)
    {
        /* Thread pointer is invalid, return appropriate error code.  */
        status =  TX_THREAD_ERROR;
    }
    
    /* Check for interrupt call.  */
    if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
    {
        /* Is call from an interrupt and not initialization?  */
        if (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)
        {
            /* Invalid caller of this function, return appropriate error code.  */
            status =  TX_CALLER_ERROR;
        }
    }
    
    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {
        /* Call actual secure stack allocate function.  */
        status =  _tx_thread_secure_stack_allocate(thread_ptr, stack_size);
    }

    /* Return completion status.  */
    return(status);
#endif
}
