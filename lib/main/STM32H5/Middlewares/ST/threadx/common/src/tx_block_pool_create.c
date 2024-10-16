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
#include "tx_trace.h"
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_block_pool_create                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a pool of fixed-size memory blocks in the     */
/*    specified memory area.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    name_ptr                          Pointer to block pool name        */
/*    block_size                        Number of bytes in each block     */
/*    pool_start                        Address of beginning of pool area */
/*    pool_size                         Number of bytes in the block pool */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
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
UINT  _tx_block_pool_create(TX_BLOCK_POOL *pool_ptr, CHAR *name_ptr, ULONG block_size,
                    VOID *pool_start, ULONG pool_size)
{

TX_INTERRUPT_SAVE_AREA

UINT                blocks;
UINT                status;
ULONG               total_blocks;
UCHAR               *block_ptr;
UCHAR               **block_link_ptr;
UCHAR               *next_block_ptr;
TX_BLOCK_POOL       *next_pool;
TX_BLOCK_POOL       *previous_pool;


    /* Initialize block pool control block to all zeros.  */
    TX_MEMSET(pool_ptr, 0, (sizeof(TX_BLOCK_POOL)));

    /* Round the block size up to something that is evenly divisible by
       an ALIGN_TYPE (typically this is a 32-bit ULONG). This helps guarantee proper alignment.  */
    block_size =  (((block_size + (sizeof(ALIGN_TYPE))) - ((ALIGN_TYPE) 1))/(sizeof(ALIGN_TYPE))) * (sizeof(ALIGN_TYPE));

    /* Round the pool size down to something that is evenly divisible by
       an ALIGN_TYPE (typically this is a 32-bit ULONG).  */
    pool_size =   (pool_size/(sizeof(ALIGN_TYPE))) * (sizeof(ALIGN_TYPE));

    /* Setup the basic block pool fields.  */
    pool_ptr -> tx_block_pool_name =             name_ptr;
    pool_ptr -> tx_block_pool_start =            TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    pool_ptr -> tx_block_pool_size =             pool_size;
    pool_ptr -> tx_block_pool_block_size =       (UINT) block_size;

    /* Calculate the total number of blocks.  */
    total_blocks =  pool_size/(block_size + (sizeof(UCHAR *)));

    /* Walk through the pool area, setting up the available block list.  */
    blocks =            ((UINT) 0);
    block_ptr =         TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    next_block_ptr =    TX_UCHAR_POINTER_ADD(block_ptr, (block_size + (sizeof(UCHAR *))));
    while(blocks < (UINT) total_blocks)
    {

        /* Yes, we have another block.  Increment the block count.  */
        blocks++;

        /* Setup the link to the next block.  */
        block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(block_ptr);
        *block_link_ptr =  next_block_ptr;

        /* Advance to the next block.  */
        block_ptr =   next_block_ptr;

        /* Update the next block pointer.  */
        next_block_ptr =  TX_UCHAR_POINTER_ADD(block_ptr, (block_size + (sizeof(UCHAR *))));
    }

    /* Save the remaining information in the pool control block.  */
    pool_ptr -> tx_block_pool_available =  blocks;
    pool_ptr -> tx_block_pool_total =      blocks;

    /* Quickly check to make sure at least one block is in the pool.  */
    if (blocks != ((UINT) 0))
    {

        /* Backup to the last block in the pool.  */
        block_ptr =  TX_UCHAR_POINTER_SUB(block_ptr,(block_size + (sizeof(UCHAR *))));

        /* Set the last block's forward pointer to NULL.  */
        block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(block_ptr);
        *block_link_ptr =  TX_NULL;

        /* Setup the starting pool address.  */
        pool_ptr -> tx_block_pool_available_list =  TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);

        /* Disable interrupts to place the block pool on the created list.  */
        TX_DISABLE

        /* Setup the block pool ID to make it valid.  */
        pool_ptr -> tx_block_pool_id =  TX_BLOCK_POOL_ID;

        /* Place the block pool on the list of created block pools.  First,
           check for an empty list.  */
        if (_tx_block_pool_created_count == TX_EMPTY)
        {

            /* The created block pool list is empty.  Add block pool to empty list.  */
            _tx_block_pool_created_ptr =                  pool_ptr;
            pool_ptr -> tx_block_pool_created_next =      pool_ptr;
            pool_ptr -> tx_block_pool_created_previous =  pool_ptr;
        }
        else
        {

            /* This list is not NULL, add to the end of the list.  */
            next_pool =      _tx_block_pool_created_ptr;
            previous_pool =  next_pool -> tx_block_pool_created_previous;

            /* Place the new block pool in the list.  */
            next_pool -> tx_block_pool_created_previous =  pool_ptr;
            previous_pool -> tx_block_pool_created_next =  pool_ptr;

            /* Setup this block pool's created links.  */
            pool_ptr -> tx_block_pool_created_previous =  previous_pool;
            pool_ptr -> tx_block_pool_created_next =      next_pool;
        }

        /* Increment the created count.  */
        _tx_block_pool_created_count++;

        /* Optional block pool create extended processing.  */
        TX_BLOCK_POOL_CREATE_EXTENSION(pool_ptr)

        /* If trace is enabled, register this object.  */
        TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_BLOCK_POOL, pool_ptr, name_ptr, pool_size, block_size)

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_BLOCK_POOL_CREATE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(pool_start), blocks, block_size, TX_TRACE_BLOCK_POOL_EVENTS)

        /* Log this kernel call.  */
        TX_EL_BLOCK_POOL_CREATE_INSERT

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return successful status.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* Not enough memory for one block, return appropriate error.  */
        status =  TX_SIZE_ERROR;
    }

    /* Return completion status.  */
    return(status);
}

