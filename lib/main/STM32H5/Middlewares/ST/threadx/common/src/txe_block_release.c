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
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_block_release                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the block release function call. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    block_ptr                         Pointer to memory block           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_PTR_ERROR                      Invalid memory block pointer      */
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_block_release                 Actual block release function     */
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
UINT  _txe_block_release(VOID *block_ptr)
{

UINT                status;
TX_BLOCK_POOL       *pool_ptr;
UCHAR               **indirect_ptr;
UCHAR               *work_ptr;


    /* First check the supplied pointer.  */
    if (block_ptr == TX_NULL)
    {

        /* The block pointer is invalid, return appropriate status.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Pickup the pool pointer which is just previous to the starting
           address of block that the caller sees.  */
        work_ptr =      TX_VOID_TO_UCHAR_POINTER_CONVERT(block_ptr);
        work_ptr =      TX_UCHAR_POINTER_SUB(work_ptr, (sizeof(UCHAR *)));
        indirect_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(work_ptr);
        work_ptr =      *indirect_ptr;
        pool_ptr =      TX_UCHAR_TO_BLOCK_POOL_POINTER_CONVERT(work_ptr);

        /* Check for an invalid pool pointer.  */
        if (pool_ptr == TX_NULL)
        {

            /* Pool pointer is invalid, return appropriate error code.  */
            status =  TX_PTR_ERROR;
        }

        /* Now check for invalid pool ID.  */
        else if  (pool_ptr -> tx_block_pool_id != TX_BLOCK_POOL_ID)
        {

            /* Pool pointer is invalid, return appropriate error code.  */
            status =  TX_PTR_ERROR;
        }
        else
        {

            /* Call actual block release function.  */
            status =  _tx_block_release(block_ptr);
        }
    }

    /* Return completion status.  */
    return(status);
}

