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
/**   Block Memory                                                        */
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
/*    _txe_block_pool_info_get                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the block pool information get   */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to block pool control blk */
/*    name                              Destination for the pool name     */
/*    available_blocks                  Number of free blocks in pool     */
/*    total_blocks                      Total number of blocks in pool    */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on block pool  */
/*    suspended_count                   Destination for suspended count   */
/*    next_pool                         Destination for pointer to next   */
/*                                        block pool on the created list  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_POOL_ERROR                     Invalid block pool pointer        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_block_pool_info_get           Actual block pool info get service*/
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
UINT  _txe_block_pool_info_get(TX_BLOCK_POOL *pool_ptr, CHAR **name, ULONG *available_blocks,
                    ULONG *total_blocks, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BLOCK_POOL **next_pool)
{


UINT    status;


    /* Check for an invalid block pool pointer.  */
    if (pool_ptr == TX_NULL)
    {

        /* Block pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Now check the pool ID.  */
    else if (pool_ptr -> tx_block_pool_id != TX_BLOCK_POOL_ID)
    {

        /* Block pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }
    else
    {

        /* Otherwise, call the actual block pool information get service.  */
        status =  _tx_block_pool_info_get(pool_ptr, name, available_blocks,
                        total_blocks, first_suspended, suspended_count, next_pool);
    }

    /* Return completion status.  */
    return(status);
}

