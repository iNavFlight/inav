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
/*    _lx_nor_flash_block_reclaim                         PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reclaims one block from the NOR flash.                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nor_flash_driver_block_erase      Driver erase block            */ 
/*    _lx_nor_flash_driver_write            Driver flash sector write     */ 
/*    _lx_nor_flash_driver_read             Driver flash sector read      */ 
/*    _lx_nor_flash_next_block_to_erase_find                              */ 
/*                                          Find next block to erase      */ 
/*    _lx_nor_flash_physical_sector_allocate                              */ 
/*                                          Allocate new logical sector   */ 
/*    _lx_nor_flash_sector_mapping_cache_invalidate                       */ 
/*                                          Invalidate cache entry        */ 
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
UINT  _lx_nor_flash_block_reclaim(LX_NOR_FLASH *nor_flash)
{

ULONG   *block_word_ptr;
ULONG   *list_word_ptr;
ULONG   list_word;
ULONG   i;
ULONG   erase_block;
ULONG   erase_count;
ULONG   temp_erase_count;
ULONG   erase_started_value;
ULONG   mapped_sectors;
ULONG   obsolete_sectors;
ULONG   free_sectors;
ULONG   logical_sector;
ULONG   *new_mapping_address;
ULONG   *new_sector_address;
ULONG   new_mapping_entry;
UINT    status;


    /* Determine the next block to erase.  */
    _lx_nor_flash_next_block_to_erase_find(nor_flash, &erase_block, &erase_count, &mapped_sectors, &obsolete_sectors);

    /* Determine if the search pointer is set for this block.  */
    if (nor_flash -> lx_nor_flash_free_block_search == erase_block)
    {
    
        /* Ensure the search block is not the block we are trying to free.  */
        nor_flash -> lx_nor_flash_free_block_search =  erase_block + 1;
     
        /* Check for wrap condition.  */
        if (nor_flash -> lx_nor_flash_free_block_search >= nor_flash -> lx_nor_flash_total_blocks)
            nor_flash -> lx_nor_flash_free_block_search =  0;
    }

    /* Setup the block word pointer to the first word of the search block.  */
    block_word_ptr =  nor_flash -> lx_nor_flash_base_address + (nor_flash -> lx_nor_flash_words_per_block * erase_block);

    /* Determine if this block is completely obsolete.  */
    if (obsolete_sectors == nor_flash -> lx_nor_flash_physical_sectors_per_block)
    {

        /* Write the erased started indication.  */            
        erase_started_value =  LX_BLOCK_ERASE_STARTED;
        status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr, &erase_started_value, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }
       
        /* Erase the entire block.  */
        status =  _lx_nor_flash_driver_block_erase(nor_flash, erase_block, erase_count+1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);
            
            /* Return the error.  */
            return(status);
        }
        
        /* Increment the erase count.  */
        erase_count++;            

        /* Determine if the new erase count exceeds the maximum.  */
        if (erase_count > ((ULONG) LX_BLOCK_ERASE_COUNT_MAX))
        {
                
            /* Yes, erase count is in overflow. Stay at the maximum count.  */
            erase_count =  ((ULONG) LX_BLOCK_ERASE_COUNT_MAX);
        }
        
        /* Determine if we need to update the maximum erase count.  */
        if (erase_count > nor_flash -> lx_nor_flash_maximum_erase_count)
        {
        
            /* Yes, a new maximum is present.  */
            nor_flash -> lx_nor_flash_maximum_erase_count =  erase_count;
        }
        
        /* Setup the free bit map that corresponds to the free physical sectors in this
           block. Note that we only need to setup the portion of the free bit map that doesn't 
           have sectors associated with it.  */            
        status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr+(nor_flash -> lx_nor_flash_block_free_bit_map_offset + (nor_flash -> lx_nor_flash_block_bit_map_words - 1)), 
                                                                        &(nor_flash -> lx_nor_flash_block_bit_map_mask), 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
       
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }
        
        /* Write the initial erase count for the block with upper bit set.  */
        temp_erase_count =  (erase_count | LX_BLOCK_ERASED);
        status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr, &temp_erase_count, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }

        /* Write the final initial erase count for the block.  */
        status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr, &erase_count, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

            /* Return the error.  */
            return(status);
        }
        
        /* Update parameters of this flash.  */
        nor_flash -> lx_nor_flash_free_physical_sectors =      nor_flash -> lx_nor_flash_free_physical_sectors + obsolete_sectors;
        nor_flash -> lx_nor_flash_obsolete_physical_sectors =  nor_flash -> lx_nor_flash_obsolete_physical_sectors - obsolete_sectors;
    }
    else 
    {
    
        /* Calculate the number of free sectors in this block.  */
        free_sectors =  nor_flash -> lx_nor_flash_physical_sectors_per_block - (obsolete_sectors + mapped_sectors);
        
        /* Determine if there are enough free sectors outside of this block to reclaim this block.  */
        if (mapped_sectors <= (nor_flash -> lx_nor_flash_free_physical_sectors - free_sectors))
        {
        
            /* Setup a pointer to the mapped list.  */
            list_word_ptr =  block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset;

            /* Now search through the list to find mapped sectors to move.  */
            for (i = 0; i < nor_flash -> lx_nor_flash_physical_sectors_per_block; i++)
            {

                /* Pickup the mapped sector list entry.  */
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
            
                /* Is this entry mapped?  */
                if (list_word & LX_NOR_PHYSICAL_SECTOR_VALID)
                {
                
                    /* Pickup the logical sector associated with this mapped physical sector.  */
                    logical_sector =  list_word & LX_NOR_LOGICAL_SECTOR_MASK;

                    /* Invalidate the old sector mapping cache entry.  */
                    _lx_nor_flash_sector_mapping_cache_invalidate(nor_flash, logical_sector);
                      
                    /* Allocate a new physical sector for this write.  */
                    _lx_nor_flash_physical_sector_allocate(nor_flash, logical_sector, &new_mapping_address, &new_sector_address);
                    
                    /* Check to see if the new sector is also in the erase block.  */
                    if ((new_sector_address >= block_word_ptr) && (new_sector_address < (block_word_ptr + nor_flash -> lx_nor_flash_words_per_block)))
                    {
                    
                        /* Yes, the new sector was found in the block to be erased. Simply move the search pointer
                           to the block after the erase block and search for another sector from there.  */
                        nor_flash -> lx_nor_flash_free_block_search =  erase_block + 1;
     
                        /* Check for wrap condition.  */
                        if (nor_flash -> lx_nor_flash_free_block_search >= nor_flash -> lx_nor_flash_total_blocks)
                            nor_flash -> lx_nor_flash_free_block_search =  0;

                        /* Allocate a new physical sector for this write.  */
                        _lx_nor_flash_physical_sector_allocate(nor_flash, logical_sector, &new_mapping_address, &new_sector_address);

                        /* Check again for the new sector inside of the block to erase. This should be impossible, since
                           we check previously if there are enough free sectors outside of this block needed to reclaim
                           this block.  */
                        if ((new_sector_address >= block_word_ptr) && (new_sector_address < (block_word_ptr + LX_NOR_SECTOR_SIZE)))
                        {

                            /* System error, a new sector is not available outside of the erase block. 
                               Clear the new sector so we fall through to the error handling. */
                            new_mapping_address =  LX_NULL;
                        }
                    }

                    /* Determine if the new sector allocation was successful.  */
                    if (new_mapping_address)
                    {
    
                        /* Yes, we were able to allocate a new physical sector.  */
        
#ifdef LX_DIRECT_READ
                        /* First, write the sector data to the new physical sector.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, new_sector_address, (block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_offset) +
                                                                        (i * LX_NOR_SECTOR_SIZE), LX_NOR_SECTOR_SIZE);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }
#else

                        /* First, read the sector data into the internal memory of the NOR flash instance. This internal memory
                           is supplied by the underlying driver during initialization.  */
                        status =  _lx_nor_flash_driver_read(nor_flash, (block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_offset) +
                                                                                       (i * LX_NOR_SECTOR_SIZE), nor_flash -> lx_nor_flash_sector_buffer,
                                                                                       LX_NOR_SECTOR_SIZE);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Next, write the sector data from the internal buffer to the new physical sector.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, new_sector_address, nor_flash -> lx_nor_flash_sector_buffer, LX_NOR_SECTOR_SIZE);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }
#endif

                        /* Now deprecate the old sector mapping.  */
            
                        /* Clear bit 30, which indicates this sector is superceded.  */
                        list_word =  list_word & ~((ULONG) LX_NOR_PHYSICAL_SECTOR_SUPERCEDED);
            
                        /* Write the value back to the flash to clear bit 30.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, list_word_ptr, &list_word, 1);
                
                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }
                
                        /* Now build the new mapping entry - with the not valid bit set initially.  */
                        new_mapping_entry =  ((ULONG) LX_NOR_PHYSICAL_SECTOR_VALID) | ((ULONG) LX_NOR_PHYSICAL_SECTOR_SUPERCEDED) | (ULONG) LX_NOR_PHYSICAL_SECTOR_MAPPING_NOT_VALID | logical_sector;
            
                        /* Write out the new mapping entry.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, new_mapping_address, &new_mapping_entry, 1);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Now clear the not valid bit to make this sector mapping valid.  This is done because the writing of the extra bytes itself can 
                           be interrupted and we need to make sure this can be detected when the flash is opened again.  */
                        new_mapping_entry =  new_mapping_entry & ~((ULONG) LX_NOR_PHYSICAL_SECTOR_MAPPING_NOT_VALID);
            
                        /* Clear the not valid bit.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, new_mapping_address, &new_mapping_entry, 1);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }

                        /* Now clear bit 31, which indicates this sector is now obsoleted.  */
                        list_word =  list_word & ~((ULONG) LX_NOR_PHYSICAL_SECTOR_VALID);
            
                        /* Write the value back to the flash to clear bit 31.  */
                        status =  _lx_nor_flash_driver_write(nor_flash, list_word_ptr, &list_word, 1);

                        /* Check for an error from flash driver. Drivers should never return an error..  */
                        if (status)
                        {
        
                            /* Call system error handler.  */
                            _lx_nor_flash_system_error(nor_flash, status);

                            /* Return the error.  */
                            return(status);
                        }
                    }
                    else
                    {

                        /* Call system error handler - the allocation should always succeed at this point.  */
                        _lx_nor_flash_system_error(nor_flash, LX_SYSTEM_ALLOCATION_FAILED);

                        /* Return the error.  */
                        return(status);
                    }
                        
                    /* Decrement the number of mapped sectors.  */
                    mapped_sectors--;
                            
                    /* Determine if we are done.  */
                    if (mapped_sectors == 0)
                        break;
               }

               /* Move the list pointer ahead.  */
               list_word_ptr++;
            }

            /* Write the erased started indication.  */            
            erase_started_value =  LX_BLOCK_ERASE_STARTED;
            status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr, &erase_started_value, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }
        
            /* Erase the entire block.  */
            status =  _lx_nor_flash_driver_block_erase(nor_flash, erase_block, erase_count+1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }
        
            /* Increment the erase count.  */
            erase_count++;            

            /* Determine if the new erase count exceeds the maximum.  */
            if (erase_count > ((ULONG) LX_BLOCK_ERASE_COUNT_MAX))
            {
                
                /* Yes, erase count is in overflow. Stay at the maximum count.  */
                erase_count =  ((ULONG) LX_BLOCK_ERASE_COUNT_MAX);
            }

            /* Determine if we need to update the maximum erase count.  */
            if (erase_count > nor_flash -> lx_nor_flash_maximum_erase_count)
            {
        
                /* Yes, a new maximum is present.  */
                nor_flash -> lx_nor_flash_maximum_erase_count =  erase_count;
            }
        
            /* Setup the free bit map that corresponds to the free physical sectors in this
               block. Note that we only need to setup the portion of the free bit map that doesn't 
               have sectors associated with it.  */            
            status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr+(nor_flash -> lx_nor_flash_block_free_bit_map_offset + (nor_flash -> lx_nor_flash_block_bit_map_words - 1)) , 
                                                                             &(nor_flash -> lx_nor_flash_block_bit_map_mask), 1);
        
            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }

            /* Write the initial erase count for the block with the upper bit set.  */
            temp_erase_count =  (erase_count | LX_BLOCK_ERASED);
            status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr, &temp_erase_count, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }

            /* Write the final initial erase count for the block.  */
            status =  _lx_nor_flash_driver_write(nor_flash, block_word_ptr, &erase_count, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

                /* Return the error.  */
                return(status);
            }

            /* Update parameters of this flash.  */
            nor_flash -> lx_nor_flash_free_physical_sectors =      nor_flash -> lx_nor_flash_free_physical_sectors + obsolete_sectors;
            nor_flash -> lx_nor_flash_obsolete_physical_sectors =  nor_flash -> lx_nor_flash_obsolete_physical_sectors - obsolete_sectors;
        }
    }

    /* Return status.  */
    return(LX_SUCCESS);
}

