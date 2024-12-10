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
#include "tx_thread.h"
#include "tx_byte_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_byte_pool_search                               PORTABLE SMP     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches a byte pool for a memory block to satisfy    */
/*    the requested number of bytes.  Merging of adjacent free blocks     */
/*    takes place during the search and a split of the block that         */
/*    satisfies the request may occur before this function returns.       */
/*                                                                        */
/*    It is assumed that this function is called with interrupts enabled  */
/*    and with the tx_pool_owner field set to the thread performing the   */
/*    search.  Also note that the search can occur during allocation and  */
/*    release of a memory block.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    memory_size                       Number of bytes required          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    UCHAR *                           Pointer to the allocated memory,  */
/*                                        if successful.  Otherwise, a    */
/*                                        NULL is returned                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_byte_allocate                 Allocate bytes of memory          */
/*    _tx_byte_release                  Release bytes of memory           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      William E. Lamie        Initial Version 6.1           */
/*  06-02-2021      Scott Larson            Improve possible free bytes   */
/*                                            calculation, and reduced    */
/*                                            number of search resets,    */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UCHAR  *_tx_byte_pool_search(TX_BYTE_POOL *pool_ptr, ULONG memory_size)
{

TX_INTERRUPT_SAVE_AREA

UCHAR           *current_ptr;
UCHAR           *next_ptr;
UCHAR           **this_block_link_ptr;
UCHAR           **next_block_link_ptr;
ULONG           available_bytes;
UINT            examine_blocks;
UINT            first_free_block_found =  TX_FALSE;
TX_THREAD       *thread_ptr;
ALIGN_TYPE      *free_ptr;
UCHAR           *work_ptr;
volatile ULONG  delay_count;
ULONG           total_theoretical_available;
#ifdef TX_BYTE_POOL_MULTIPLE_BLOCK_SEARCH
UINT            blocks_searched =  ((UINT) 0);
#endif


    /* Disable interrupts.  */
    TX_DISABLE

    /* First, determine if there are enough bytes in the pool.  */
    /* Theoretical bytes available = free bytes + ((fragments-2) * overhead of each block) */
    total_theoretical_available = pool_ptr -> tx_byte_pool_available + ((pool_ptr -> tx_byte_pool_fragments - 2) * ((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE))));
    if (memory_size >= total_theoretical_available)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Not enough memory, return a NULL pointer.  */
        current_ptr =  TX_NULL;
    }
    else
    {

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Setup ownership of the byte pool.  */
        pool_ptr -> tx_byte_pool_owner =  thread_ptr;

        /* Walk through the memory pool in search for a large enough block.  */
        current_ptr =      pool_ptr -> tx_byte_pool_search;
        examine_blocks =   pool_ptr -> tx_byte_pool_fragments + ((UINT) 1);
        available_bytes =  ((ULONG) 0);
        do
        {


#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

            /* Increment the total fragment search counter.  */
            _tx_byte_pool_performance_search_count++;

            /* Increment the number of fragments searched on this pool.  */
            pool_ptr -> tx_byte_pool_performance_search_count++;
#endif

            /* Check to see if this block is free.  */
            work_ptr =  TX_UCHAR_POINTER_ADD(current_ptr, (sizeof(UCHAR *)));
            free_ptr =  TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(work_ptr);
            if ((*free_ptr) == TX_BYTE_BLOCK_FREE)
            {

                /* Determine if this is the first free block.  */
                if (first_free_block_found == TX_FALSE)
                {
                    /* This is the first free block.  */
                    pool_ptr->tx_byte_pool_search =  current_ptr;

                    /* Set the flag to indicate we have found the first free
                       block.  */
                    first_free_block_found =  TX_TRUE;
                }

                /* Block is free, see if it is large enough.  */

                /* Pickup the next block's pointer.  */
                this_block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(current_ptr);
                next_ptr =             *this_block_link_ptr;

                /* Calculate the number of bytes available in this block.  */
                available_bytes =   TX_UCHAR_POINTER_DIF(next_ptr, current_ptr);
                available_bytes =   available_bytes - ((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE)));

                /* If this is large enough, we are done because our first-fit algorithm
                   has been satisfied!  */
                if (available_bytes >= memory_size)
                {
                    /* Get out of the search loop!  */
                    break;
                }
                else
                {

                    /* Clear the available bytes variable.  */
                    available_bytes =  ((ULONG) 0);

                    /* Not enough memory, check to see if the neighbor is
                       free and can be merged.  */
                    work_ptr =  TX_UCHAR_POINTER_ADD(next_ptr, (sizeof(UCHAR *)));
                    free_ptr =  TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(work_ptr);
                    if ((*free_ptr) == TX_BYTE_BLOCK_FREE)
                    {

                        /* Yes, neighbor block can be merged!  This is quickly accomplished
                           by updating the current block with the next blocks pointer.  */
                        next_block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(next_ptr);
                        *this_block_link_ptr =  *next_block_link_ptr;

                        /* Reduce the fragment total.  We don't need to increase the bytes
                           available because all free headers are also included in the available
                           count.  */
                        pool_ptr -> tx_byte_pool_fragments--;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

                        /* Increment the total merge counter.  */
                        _tx_byte_pool_performance_merge_count++;

                        /* Increment the number of blocks merged on this pool.  */
                        pool_ptr -> tx_byte_pool_performance_merge_count++;
#endif

                        /* See if the search pointer is affected.  */
                        if (pool_ptr -> tx_byte_pool_search ==  next_ptr)
                        {
                            /* Yes, update the search pointer.   */
                            pool_ptr -> tx_byte_pool_search =  current_ptr;
                        }
                    }
                    else
                    {
                        /* Neighbor is not free so we can skip over it!  */
                        next_block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(next_ptr);
                        current_ptr =  *next_block_link_ptr;

                        /* Decrement the examined block count to account for this one.  */
                        if (examine_blocks != ((UINT) 0))
                        {
                            examine_blocks--;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

                            /* Increment the total fragment search counter.  */
                            _tx_byte_pool_performance_search_count++;

                            /* Increment the number of fragments searched on this pool.  */
                            pool_ptr -> tx_byte_pool_performance_search_count++;
#endif
                        }
                    }
                }
            }
            else
            {

                /* Block is not free, move to next block.  */
                this_block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(current_ptr);
                current_ptr =  *this_block_link_ptr;
            }

            /* Another block has been searched... decrement counter.  */
            if (examine_blocks != ((UINT) 0))
            {

                examine_blocks--;
            }

#ifdef TX_BYTE_POOL_MULTIPLE_BLOCK_SEARCH

            /* When this is enabled, multiple blocks are searched while holding the protection.  */

            /* Increment the number of blocks searched.  */
            blocks_searched =  blocks_searched + ((UINT) 1);

            /* Have we reached the maximum number of blocks to search while holding the protection?  */
            if (blocks_searched >= ((UINT) TX_BYTE_POOL_MULTIPLE_BLOCK_SEARCH))
            {

                /* Yes, we have exceeded the multiple block search limit.  */

                /* Restore interrupts temporarily.  */
                TX_RESTORE

                /* Disable interrupts.  */
                TX_DISABLE

                /* Reset the number of blocks searched counter.  */
                blocks_searched =  ((UINT) 0);
            }
#else
            /* Restore interrupts temporarily.  */
            TX_RESTORE

            /* Disable interrupts.  */
            TX_DISABLE
#endif
            /* Determine if anything has changed in terms of pool ownership.  */
            if (pool_ptr -> tx_byte_pool_owner != thread_ptr)
            {
                /* Loop to delay changing the ownership back to avoid thrashing.  */
                delay_count =  0;
                do
                {
                    /* Restore interrupts temporarily.  */
                    TX_RESTORE

                    /* Increment the delay counter.  */
                    delay_count++;

                    /* Disable interrupts.  */
                    TX_DISABLE
                } while (delay_count < ((ULONG) TX_BYTE_POOL_DELAY_VALUE));
                /* Pool changed ownership in the brief period interrupts were
                   enabled.  Reset the search.  */
                current_ptr =      pool_ptr -> tx_byte_pool_search;
                examine_blocks =   pool_ptr -> tx_byte_pool_fragments + ((UINT) 1);

                /* Setup our ownership again.  */
                pool_ptr -> tx_byte_pool_owner =  thread_ptr;
            }
        } while(examine_blocks != ((UINT) 0));

        /* Determine if a block was found.  If so, determine if it needs to be
           split.  */
        if (available_bytes != ((ULONG) 0))
        {

            /* Determine if we need to split this block.  */
            if ((available_bytes - memory_size) >= ((ULONG) TX_BYTE_BLOCK_MIN))
            {

                /* Split the block.  */
                next_ptr =  TX_UCHAR_POINTER_ADD(current_ptr, (memory_size + ((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE)))));

                /* Setup the new free block.  */
                next_block_link_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(next_ptr);
                this_block_link_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(current_ptr);
                *next_block_link_ptr =  *this_block_link_ptr;
                work_ptr =              TX_UCHAR_POINTER_ADD(next_ptr, (sizeof(UCHAR *)));
                free_ptr =              TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(work_ptr);
                *free_ptr =             TX_BYTE_BLOCK_FREE;

                /* Increase the total fragment counter.  */
                pool_ptr -> tx_byte_pool_fragments++;

                /* Update the current pointer to point at the newly created block.  */
                *this_block_link_ptr =  next_ptr;

                /* Set available equal to memory size for subsequent calculation.  */
                available_bytes =  memory_size;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

                /* Increment the total split counter.  */
                _tx_byte_pool_performance_split_count++;

                /* Increment the number of blocks split on this pool.  */
                pool_ptr -> tx_byte_pool_performance_split_count++;
#endif
            }

            /* In any case, mark the current block as allocated.  */
            work_ptr =              TX_UCHAR_POINTER_ADD(current_ptr, (sizeof(UCHAR *)));
            this_block_link_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(work_ptr);
            *this_block_link_ptr =  TX_BYTE_POOL_TO_UCHAR_POINTER_CONVERT(pool_ptr);

            /* Reduce the number of available bytes in the pool.  */
            pool_ptr -> tx_byte_pool_available =  (pool_ptr -> tx_byte_pool_available - available_bytes) - ((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE)));

            /* Determine if the search pointer needs to be updated. This is only done
               if the search pointer matches the block to be returned.  */
            if (current_ptr == pool_ptr -> tx_byte_pool_search)
            {

                /* Yes, update the search pointer to the next block.  */
                this_block_link_ptr =   TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(current_ptr);
                pool_ptr -> tx_byte_pool_search =  *this_block_link_ptr;
            }

            /* Restore interrupts.  */
            TX_RESTORE

            /* Adjust the pointer for the application.  */
            current_ptr =  TX_UCHAR_POINTER_ADD(current_ptr, (((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE)))));
        }
        else
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Set current pointer to NULL to indicate nothing was found.  */
            current_ptr =  TX_NULL;
        }
    }

    /* Return the search pointer.  */
    return(current_ptr);
}

