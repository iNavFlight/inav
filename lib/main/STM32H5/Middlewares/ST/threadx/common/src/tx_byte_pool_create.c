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
/**   Byte Pool                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_byte_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_byte_pool_create                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a pool of memory bytes in the specified       */
/*    memory area.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    name_ptr                          Pointer to byte pool name         */
/*    pool_start                        Address of beginning of pool area */
/*    pool_size                         Number of bytes in the byte pool  */
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
UINT  _tx_byte_pool_create(TX_BYTE_POOL *pool_ptr, CHAR *name_ptr, VOID *pool_start, ULONG pool_size)
{

TX_INTERRUPT_SAVE_AREA

UCHAR               *block_ptr;
UCHAR               **block_indirect_ptr;
UCHAR               *temp_ptr;
TX_BYTE_POOL        *next_pool;
TX_BYTE_POOL        *previous_pool;
ALIGN_TYPE          *free_ptr;


    /* Initialize the byte pool control block to all zeros.  */
    TX_MEMSET(pool_ptr, 0, (sizeof(TX_BYTE_POOL)));

    /* Round the pool size down to something that is evenly divisible by
       an ULONG.  */
    pool_size =   (pool_size/(sizeof(ALIGN_TYPE))) * (sizeof(ALIGN_TYPE));

    /* Setup the basic byte pool fields.  */
    pool_ptr -> tx_byte_pool_name =              name_ptr;

    /* Save the start and size of the pool.  */
    pool_ptr -> tx_byte_pool_start =   TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    pool_ptr -> tx_byte_pool_size =    pool_size;

    /* Setup memory list to the beginning as well as the search pointer.  */
    pool_ptr -> tx_byte_pool_list =    TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    pool_ptr -> tx_byte_pool_search =  TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);

    /* Initially, the pool will have two blocks.  One large block at the
       beginning that is available and a small allocated block at the end
       of the pool that is there just for the algorithm.  Be sure to count
       the available block's header in the available bytes count.  */
    pool_ptr -> tx_byte_pool_available =   pool_size - ((sizeof(VOID *)) + (sizeof(ALIGN_TYPE)));
    pool_ptr -> tx_byte_pool_fragments =   ((UINT) 2);

    /* Each block contains a "next" pointer that points to the next block in the pool followed by a ALIGN_TYPE
       field that contains either the constant TX_BYTE_BLOCK_FREE (if the block is free) or a pointer to the
       owning pool (if the block is allocated).  */

    /* Calculate the end of the pool's memory area.  */
    block_ptr =  TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    block_ptr =  TX_UCHAR_POINTER_ADD(block_ptr, pool_size);

    /* Backup the end of the pool pointer and build the pre-allocated block.  */
    block_ptr =  TX_UCHAR_POINTER_SUB(block_ptr, (sizeof(ALIGN_TYPE)));

    /* Cast the pool pointer into a ULONG.  */
    temp_ptr =             TX_BYTE_POOL_TO_UCHAR_POINTER_CONVERT(pool_ptr);
    block_indirect_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(block_ptr);
    *block_indirect_ptr =  temp_ptr;

    block_ptr =            TX_UCHAR_POINTER_SUB(block_ptr, (sizeof(UCHAR *)));
    block_indirect_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(block_ptr);
    *block_indirect_ptr =  TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);

    /* Now setup the large available block in the pool.  */
    temp_ptr =             TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    block_indirect_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(temp_ptr);
    *block_indirect_ptr =  block_ptr;
    block_ptr =            TX_VOID_TO_UCHAR_POINTER_CONVERT(pool_start);
    block_ptr =            TX_UCHAR_POINTER_ADD(block_ptr, (sizeof(UCHAR *)));
    free_ptr =             TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(block_ptr);
    *free_ptr =            TX_BYTE_BLOCK_FREE;

    /* Clear the owner id.  */
    pool_ptr -> tx_byte_pool_owner =  TX_NULL;

    /* Disable interrupts to place the byte pool on the created list.  */
    TX_DISABLE

    /* Setup the byte pool ID to make it valid.  */
    pool_ptr -> tx_byte_pool_id =  TX_BYTE_POOL_ID;

    /* Place the byte pool on the list of created byte pools.  First,
       check for an empty list.  */
    if (_tx_byte_pool_created_count == TX_EMPTY)
    {

        /* The created byte pool list is empty.  Add byte pool to empty list.  */
        _tx_byte_pool_created_ptr =                  pool_ptr;
        pool_ptr -> tx_byte_pool_created_next =      pool_ptr;
        pool_ptr -> tx_byte_pool_created_previous =  pool_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_pool =      _tx_byte_pool_created_ptr;
        previous_pool =  next_pool -> tx_byte_pool_created_previous;

        /* Place the new byte pool in the list.  */
        next_pool -> tx_byte_pool_created_previous =  pool_ptr;
        previous_pool -> tx_byte_pool_created_next =  pool_ptr;

        /* Setup this byte pool's created links.  */
        pool_ptr -> tx_byte_pool_created_previous =  previous_pool;
        pool_ptr -> tx_byte_pool_created_next =      next_pool;
    }

    /* Increment the number of created byte pools.  */
    _tx_byte_pool_created_count++;

    /* Optional byte pool create extended processing.  */
    TX_BYTE_POOL_CREATE_EXTENSION(pool_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_BYTE_POOL, pool_ptr, name_ptr, pool_size, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_POOL_CREATE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(pool_start), pool_size, TX_POINTER_TO_ULONG_CONVERT(&block_ptr), TX_TRACE_BYTE_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BYTE_POOL_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

