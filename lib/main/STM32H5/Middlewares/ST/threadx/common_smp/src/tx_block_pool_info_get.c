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
#include "tx_trace.h"
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_block_pool_info_get                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information from the specified block pool.  */
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
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _tx_block_pool_info_get(TX_BLOCK_POOL *pool_ptr, CHAR **name, ULONG *available_blocks,
                    ULONG *total_blocks, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BLOCK_POOL **next_pool)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BLOCK_POOL_INFO_GET, pool_ptr, 0, 0, 0, TX_TRACE_BLOCK_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BLOCK_POOL_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the block pool.  */
    if (name != TX_NULL)
    {

        *name =  pool_ptr -> tx_block_pool_name;
    }

    /* Retrieve the number of available blocks in the block pool.  */
    if (available_blocks != TX_NULL)
    {

        *available_blocks =  (ULONG) pool_ptr -> tx_block_pool_available;
    }

    /* Retrieve the total number of blocks in the block pool.  */
    if (total_blocks != TX_NULL)
    {

        *total_blocks =  (ULONG) pool_ptr -> tx_block_pool_total;
    }

    /* Retrieve the first thread suspended on this block pool.  */
    if (first_suspended != TX_NULL)
    {

        *first_suspended =  pool_ptr -> tx_block_pool_suspension_list;
    }

    /* Retrieve the number of threads suspended on this block pool.  */
    if (suspended_count != TX_NULL)
    {

        *suspended_count =  (ULONG) pool_ptr -> tx_block_pool_suspended_count;
    }

    /* Retrieve the pointer to the next block pool created.  */
    if (next_pool != TX_NULL)
    {

        *next_pool =  pool_ptr -> tx_block_pool_created_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

