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
/*    _lx_nor_flash_physical_sector_allocate              PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allocates a free physical sector for mapping to a     */ 
/*    logical sector.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    logical_sector                        Logical sector number         */ 
/*    physical_sector_map_entry             Pointer to sector map entry   */ 
/*    physical_sector_address               Address of physical sector    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nor_flash_driver_write            Driver flash sector write     */ 
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
UINT  _lx_nor_flash_physical_sector_allocate(LX_NOR_FLASH *nor_flash, ULONG logical_sector, ULONG **physical_sector_map_entry, ULONG **physical_sector_address)
{

ULONG   search_block;
ULONG   *block_word_ptr;
ULONG   block_word;
ULONG   min_logical_sector;
ULONG   max_logical_sector;
ULONG   *list_word_ptr;
ULONG   list_word;
ULONG   i, j, k, l;
UINT    status;


    /* Increment the number of physical sector allocation requests.  */
    nor_flash -> lx_nor_flash_physical_block_allocates++;

    /* Initialize the return parameters.  */
    *physical_sector_map_entry =  (ULONG *) 0;
    *physical_sector_address =    (ULONG *) 0;
    
    /* Determine if there are any free physical sectors.  */
    if (nor_flash -> lx_nor_flash_free_physical_sectors == 0)
    {

        /* Increment the number of failed allocations.  */
        nor_flash -> lx_nor_flash_physical_block_allocate_errors++;

        /* No free physical sectors, return .  */
        return(LX_NO_SECTORS);
    }

    /* Pickup the search for a free physical sector at the specified block.  */
    search_block =  nor_flash -> lx_nor_flash_free_block_search;

    /* Loop through the blocks to find a free physical sector.  */
    for (i = 0; i < nor_flash -> lx_nor_flash_total_blocks; i++)
    {

        /* Setup the block word pointer to the first word of the search block.  */
        block_word_ptr =  nor_flash -> lx_nor_flash_base_address + (search_block * nor_flash -> lx_nor_flash_words_per_block);

        /* Find the first free physical sector from the free sector bit map of this block.  */
        for (j = 0; j < nor_flash -> lx_nor_flash_block_bit_map_words; j++)
        {
                
            /* Read this word of the free sector bit map.  */
#ifdef LX_DIRECT_READ
        
            /* Read the word directly.  */
            block_word =  *(block_word_ptr + nor_flash -> lx_nor_flash_block_free_bit_map_offset + j);
#else
            status =  _lx_nor_flash_driver_read(nor_flash, (block_word_ptr + nor_flash -> lx_nor_flash_block_free_bit_map_offset + j), &block_word, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);
                
                /* Return the error.  */
                return(status);
            }
#endif
                    
            /* Are there any free sectors in this word?  */
            if (block_word)
            {
            
                /* Yes, there are free sectors in this word.  */
                for (k = 0; k < 32; k++)
                {
                    
                    /* Is this sector free?  */
                    if (block_word & 1)
                    {
                        
                        /* Yes, this sector is free, clear the bit for this sector in the free sector map.  */
                        
                        /* Read this word of the free sector bit map again.  */
#ifdef LX_DIRECT_READ
        
                        /* Read the word directly.  */
                        block_word =  *(block_word_ptr + nor_flash -> lx_nor_flash_block_free_bit_map_offset + j);
#else
                        status =  _lx_nor_flash_driver_read(nor_flash, (block_word_ptr + nor_flash -> lx_nor_flash_block_free_bit_map_offset + j), &block_word, 1);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }
#endif
                        
                        /* Clear the bit associated with the free sector to indicate it is not free.  */
                        block_word =  block_word & ~(((ULONG) 1) << k);
                        
                        /* Now write back free bit map word with the bit for this sector cleared.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, (block_word_ptr + nor_flash -> lx_nor_flash_block_free_bit_map_offset + j), &block_word, 1);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Determine if this is the last entry available in this block.  */
                        if (((block_word >> 1) == 0) && (j == (nor_flash -> lx_nor_flash_block_bit_map_words - 1)))
                        {
                        
                            /* This is the last physical sector in the block.  Now we need to calculate the minimum valid logical
                               sector and the maximum valid logical sector.  */

                            /* Setup the minimum and maximum logical sectors to the current logical sector.  */
                            min_logical_sector =  logical_sector;
                            max_logical_sector =  logical_sector;
                               
                            /* Setup a pointer to the mapped list.  */
                            list_word_ptr =  block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset;

                            /* Loop to search the mapped list.  */
                            for (l = 0; l < nor_flash -> lx_nor_flash_physical_sectors_per_block; l++)
                            {

                                /* Read the mapped sector entry.  */
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
            
                                /* Is this entry valid?  */
                                if (list_word & LX_NOR_PHYSICAL_SECTOR_VALID)
                                {

                                    /* Isolate the logical sector.  */
                                    list_word =  list_word & LX_NOR_LOGICAL_SECTOR_MASK;

                                    /* Determine if a new minimum has been found.  */
                                    if (list_word < min_logical_sector)
                                        min_logical_sector =  list_word;
                
                                    /* Determine if a new maximum has been found.  */
                                    if (list_word != LX_NOR_LOGICAL_SECTOR_MASK)
                                    {
                                        if (list_word > max_logical_sector)
                                            max_logical_sector =  list_word;                    
                                    }
                                }

                                /* Move the list pointer ahead.  */
                                list_word_ptr++;
                            }
                            
                            /* Move the search pointer forward, since we know this block is exhausted.  */
                            search_block++;
                            
                            /* Check for wrap condition on the search block.  */
                            if (search_block >= nor_flash -> lx_nor_flash_total_blocks)
                            {
                            
                                /* Reset search block to the beginning.  */
                                search_block =  0;
                            }
                            
                            /* Now write the minimum and maximum logical sector in this block.  */
                            status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr + LX_NOR_FLASH_MIN_LOGICAL_SECTOR_OFFSET, &min_logical_sector, 1);

                            /* Check for an error from flash driver. Drivers should never return an error..  */
                            if (status)
                            {
        
                                /* Call system error handler.  */
                                _lx_nor_flash_system_error(nor_flash, status);

                                /* Return the error.  */
                                return(status);
                            }
                            
                            status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr + LX_NOR_FLASH_MAX_LOGICAL_SECTOR_OFFSET, &max_logical_sector, 1);

                            /* Check for an error from flash driver. Drivers should never return an error..  */
                            if (status)
                            {
        
                                /* Call system error handler.  */
                                _lx_nor_flash_system_error(nor_flash, status);

                                /* Return the error.  */
                                return(status);
                            }
                        }
                                                
                        /* Remember the block to search.  */
                        nor_flash -> lx_nor_flash_free_block_search =  search_block;
                                                
                        /* Prepare the return information.  */
                        *physical_sector_map_entry =  block_word_ptr + (nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset + (j * 32)) + k;
                        *physical_sector_address =    block_word_ptr + (nor_flash -> lx_nor_flash_block_physical_sector_offset) + (((j * 32) + k) * LX_NOR_SECTOR_SIZE);

                        /* Return success!  */
                        return(LX_SUCCESS);                     
                    }
                        
                    /* Shift down the bit map.  */
                    block_word =  block_word >> 1;
                    
                    /* Determine if there are any more bits set? If not, we can break out of the search of this word.  */
                    if (block_word == 0)
                        break;
                }
            }
        }
            
        /* Move to the next flash block.  */
        search_block++;
        
        /* Determine if we have to wrap the search block.  */
        if (search_block >= nor_flash -> lx_nor_flash_total_blocks)
        {
        
            /* Set the search block to the beginning.  */
            search_block =  0;
        }
    }

    /* Increment the number of failed allocations.  */
    nor_flash -> lx_nor_flash_physical_block_allocate_errors++;

    /* Return no sector completion.  */
    return(LX_NO_SECTORS);
}

