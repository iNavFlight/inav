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
/** LevelX Component                                                      */ 
/**                                                                       */
/**   NOR Flash                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define LX_SOURCE_CODE


/* Disable ThreadX error checking.  */

#ifndef LX_DISABLE_ERROR_CHECKING
#define LX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include "lx_api.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _lx_nor_flash_next_block_to_erase_find              PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds the next block to erase in the NOR flash.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    return_erase_block                    Returned block to erase       */ 
/*    return_erase_count                    Returned erase count of block */ 
/*    return_mapped_sectors                 Returned number of mapped     */ 
/*                                            sectors                     */ 
/*    return_obsolete_sectors               Returned number of obsolete   */ 
/*                                            sectors                     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nor_flash_driver_read             Driver flash sector read      */ 
/*    _lx_nor_flash_system_error            Internal system error handler */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Internal LevelX                                                     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nor_flash_next_block_to_erase_find(LX_NOR_FLASH *nor_flash, ULONG *return_erase_block, ULONG *return_erase_count, ULONG *return_mapped_sectors, ULONG *return_obsolete_sectors)
{

ULONG   *block_word_ptr;
ULONG   *list_word_ptr;
ULONG   list_word;
ULONG   i, j;
ULONG   mapped_sectors;
ULONG   erase_count;
ULONG   obsolete_sectors;
ULONG   min_block_erase = 0;
ULONG   min_block_erase_count;
ULONG   min_block_obsolete_count = 0;
ULONG   min_block_mapped_count = 0;
ULONG   max_obsolete_sectors;
ULONG   max_obsolete_block = 0;
ULONG   max_obsolete_erase_count = 0;
ULONG   max_obsolete_mapped_count = 0;
ULONG   min_system_block_erase_count;
ULONG   max_system_block_erase_count;
ULONG   erase_count_threshold;
#ifndef LX_DIRECT_READ
UINT    status;
#endif


    /* Setup the block word pointer to the first word of the search block.  */
    block_word_ptr =  nor_flash -> lx_nor_flash_base_address;

    /* Initialize the minimum erase count.  */
    min_block_erase_count =  LX_ALL_ONES;
    
    /* Initialize the system minimum and maximum erase counts.  */
    min_system_block_erase_count =  LX_ALL_ONES;
    max_system_block_erase_count =  0;
        
    /* Initialize the maximum obsolete sector count.  */
    max_obsolete_sectors =  0;
        
    /* Calculate the erase count threshold.  */
    if (nor_flash -> lx_nor_flash_free_physical_sectors >= nor_flash -> lx_nor_flash_physical_sectors_per_block)
    {
        
        /* Calculate erase count threshold by adding constant to the current minimum.  */
        erase_count_threshold =  nor_flash -> lx_nor_flash_minimum_erase_count + LX_NOR_FLASH_MAX_ERASE_COUNT_DELTA;
    }
    else
    {
      
        /* When the number of free sectors is low, simply pick the block that has the most number of obsolete sectors.  */
        erase_count_threshold =  LX_ALL_ONES;
    }
    
    /* Loop through the blocks to attempt to find the mapped logical sector.  */
    for (i = 0; i < nor_flash -> lx_nor_flash_total_blocks; i++)
    {

        /* Read the erase count of this block.  */
#ifdef LX_DIRECT_READ
        
        /* Read the word directly.  */
        erase_count =  *(block_word_ptr);
#else
        status =  _lx_nor_flash_driver_read(nor_flash, block_word_ptr, &erase_count, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }
#endif

        /* Update the system minimum and maximum erase counts.  */
        if (erase_count < min_system_block_erase_count)
            min_system_block_erase_count =  erase_count;
        if (erase_count > max_system_block_erase_count)
            max_system_block_erase_count =  erase_count;

        /* Compute the number of obsolete and mapped sectors for this block.  */
        obsolete_sectors =  0;
        mapped_sectors =    0;
        
        /* Setup a pointer to the mapped list.  */
        list_word_ptr =  block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset;

        /* Loop through the mapped list for this block.  */
        for (j = 0; j < nor_flash -> lx_nor_flash_physical_sectors_per_block; j++)
        {
        
            /* Read the current mapping entry.  */
#ifdef LX_DIRECT_READ
        
            /* Read the word directly.  */
            list_word =  *(list_word_ptr);
#else
            status =  _lx_nor_flash_driver_read(nor_flash, list_word_ptr, &list_word, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }
#endif
            
            /* Determine if the entry hasn't been used.  */
            if (list_word == LX_NOR_PHYSICAL_SECTOR_FREE)
            {
                
                /* Since allocations are done sequentially in the block, we know nothing
                   else exists after this point.  */
                break;
            }
            
            /* Is this entry obsolete?  */
            if ((list_word & LX_NOR_PHYSICAL_SECTOR_VALID) == 0)
            {
                
                /* Increment the number of obsolete sectors.  */
                obsolete_sectors++;    
            }
            else
            {
                
                /* Increment the number of mapped sectors.  */
                mapped_sectors++;
            }

            /* Move the list pointer ahead.  */
            list_word_ptr++;
        }

        /* Determine if we have a block with a new maximum number of obsolete sectors.  */
        if ((obsolete_sectors > max_obsolete_sectors) && (erase_count <= erase_count_threshold))
        {
        
            /* Update the new maximum obsolete sectors and related information.  */
            max_obsolete_sectors =      obsolete_sectors;
            max_obsolete_block =        i;
            max_obsolete_erase_count =  erase_count;
            max_obsolete_mapped_count = mapped_sectors;
        }
        else if ((max_obsolete_sectors) && (obsolete_sectors == max_obsolete_sectors) && (erase_count <= erase_count_threshold))
        {
        
            /* Another block has the same number of obsolete sectors.  Does this new block have a smaller erase
               count?  */
            if (erase_count < max_obsolete_erase_count)
            {
            
                /* Yes, erase the block with the smaller erase count.  */
                max_obsolete_sectors =      obsolete_sectors;
                max_obsolete_block =        i;
                max_obsolete_erase_count =  erase_count;
                max_obsolete_mapped_count = mapped_sectors;
            }
        }
        
        /* Determine if we have a new minimum erase count.  */
        if (erase_count < min_block_erase_count)
        {
            
            /* Update the new minimum erase count and related information.  */
            min_block_erase_count =     erase_count;
            min_block_erase =           i;
            min_block_obsolete_count =  obsolete_sectors;
            min_block_mapped_count =    mapped_sectors;
        }
          
        /* Move to the next block.  */
        block_word_ptr =  block_word_ptr + nor_flash -> lx_nor_flash_words_per_block;
    }

    /* Determine if we can erase the block with the most obsolete sectors.  */
    if (max_obsolete_sectors)
    {
    
        /* Erase the block with the most obsolete sectors.  */
        *return_erase_block =       max_obsolete_block;
        *return_erase_count =       max_obsolete_erase_count;
        *return_obsolete_sectors =  max_obsolete_sectors;
        *return_mapped_sectors =    max_obsolete_mapped_count;
    }
    else
    {
      
        /* Otherwise, choose the block with the smallest erase count.  */
        *return_erase_block =       min_block_erase;
        *return_erase_count =       min_block_erase_count;
        *return_obsolete_sectors =  min_block_obsolete_count;
        *return_mapped_sectors =    min_block_mapped_count;
    }

    /* Update the overall minimum and maximum erase count.  */
    nor_flash -> lx_nor_flash_minimum_erase_count =  min_system_block_erase_count;
    nor_flash -> lx_nor_flash_maximum_erase_count =  max_system_block_erase_count;
    
    /* Return success.  */
    return(LX_SUCCESS);
}

