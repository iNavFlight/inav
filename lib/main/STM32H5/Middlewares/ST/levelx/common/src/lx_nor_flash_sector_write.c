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
/*    _lx_nor_flash_sector_write                          PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes a logical sector to the NOR flash.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    logical_sector                        Logical sector number         */ 
/*    buffer                                Pointer to buffer to write    */ 
/*                                            (the size is 512 bytes)     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nor_flash_driver_write            Driver flash sector write     */ 
/*    _lx_nor_flash_driver_read             Driver flash sector read      */ 
/*    _lx_nor_flash_block_reclaim           Reclaim one flash block       */ 
/*    _lx_nor_flash_logical_sector_find     Find logical sector           */ 
/*    _lx_nor_flash_physical_sector_allocate                              */ 
/*                                          Allocate new physical sector  */ 
/*    _lx_nor_flash_sector_mapping_cache_invalidate                       */ 
/*                                          Invalidate cache entry        */ 
/*    _lx_nor_flash_system_error            Internal system error handler */ 
/*    tx_mutex_get                          Get thread protection         */ 
/*    tx_mutex_put                          Release thread protection     */ 
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
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nor_flash_sector_write(LX_NOR_FLASH *nor_flash, ULONG logical_sector, VOID *buffer)
{

ULONG                           *old_mapping_address;
ULONG                           *old_sector_address;
ULONG                           old_mapping_entry;
ULONG                           *new_mapping_address;
ULONG                           *new_sector_address;
ULONG                           new_mapping_entry;
ULONG                           i;
LX_NOR_SECTOR_MAPPING_CACHE_ENTRY  *sector_mapping_cache_entry_ptr;
UINT                            status;


#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nor_flash -> lx_nor_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Determine if there are less than two block's worth of free sectors.  */
    i =  0;
    while (nor_flash -> lx_nor_flash_free_physical_sectors <= nor_flash -> lx_nor_flash_physical_sectors_per_block)
    {
     
        /* Attempt to reclaim one physical block.  */
        _lx_nor_flash_block_reclaim(nor_flash);

        /* Increment the block count.  */
        i++;

        /* Have we exceeded the number of blocks in the system?  */
        if (i >= nor_flash -> lx_nor_flash_total_blocks)
        { 
          
            /* Yes, break out of the loop.  */
            break;
        }
    }

    /* Increment the number of write requests.  */
    nor_flash -> lx_nor_flash_write_requests++;

    /* See if we can find the sector in the current mapping.  */
    _lx_nor_flash_logical_sector_find(nor_flash, logical_sector, LX_FALSE, &old_mapping_address, &old_sector_address);
       
    /* Allocate a new physical sector for this write.  */
    _lx_nor_flash_physical_sector_allocate(nor_flash, logical_sector, &new_mapping_address, &new_sector_address);

    /* Determine if the new sector allocation was successful.  */
    if (new_mapping_address)
    {
    
        /* Yes, we were able to allocate a new physical sector.  */

        /* Update the number of free physical sectors.  */
        nor_flash -> lx_nor_flash_free_physical_sectors--;

        /* Write the sector data to the new physical sector.  */
        status =  _lx_nor_flash_driver_write(nor_flash, new_sector_address, buffer, LX_NOR_SECTOR_SIZE);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

            /* Return status.  */
            return(LX_ERROR);
        }

        /* Was there a previously mapped sector?  */
        if (old_mapping_address)
        {

            /* Now deprecate the old sector mapping.  */
            
            /* Read in the old sector mapping.  */
#ifdef LX_DIRECT_READ
        
            /* Read the word directly.  */
            old_mapping_entry =  *(old_mapping_address);
#else
            status =  _lx_nor_flash_driver_read(nor_flash, old_mapping_address, &old_mapping_entry, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

                /* Return status.  */
                return(LX_ERROR);
            }
#endif
            
            /* Clear bit 30, which indicates this sector is superceded.  */
            old_mapping_entry =  old_mapping_entry & ~((ULONG) LX_NOR_PHYSICAL_SECTOR_SUPERCEDED);
            
            /* Write the value back to the flash to clear bit 30.  */
            status =  _lx_nor_flash_driver_write(nor_flash, old_mapping_address, &old_mapping_entry, 1);

            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

                /* Return status.  */
                return(LX_ERROR);
            }
        }
        
        /* Now build the new mapping entry - with the not valid bit set initially.  */
        new_mapping_entry =  ((ULONG) LX_NOR_PHYSICAL_SECTOR_VALID) | ((ULONG) LX_NOR_PHYSICAL_SECTOR_SUPERCEDED) | ((ULONG) LX_NOR_PHYSICAL_SECTOR_MAPPING_NOT_VALID) | logical_sector;
            
        /* Write out the new mapping entry.  */
        status =  _lx_nor_flash_driver_write(nor_flash, new_mapping_address, &new_mapping_entry, 1);

        /* Check for an error from flash driver. Drivers should never return an error..  */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nor_flash_system_error(nor_flash, status);

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

            /* Return status.  */
            return(LX_ERROR);
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

#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

            /* Return status.  */
            return(LX_ERROR);
        }

        /* Increment the number of mapped physical sectors.  */
        nor_flash -> lx_nor_flash_mapped_physical_sectors++;
        
        /* Was there a previously mapped sector?  */
        if (old_mapping_address)
        {
        
            /* Now clear bit 31, which indicates this sector is now obsoleted.  */
            old_mapping_entry =  old_mapping_entry & ~((ULONG) LX_NOR_PHYSICAL_SECTOR_VALID);
            
            /* Write the value back to the flash to clear bit 31.  */
            status =  _lx_nor_flash_driver_write(nor_flash, old_mapping_address, &old_mapping_entry, 1);
            
            /* Check for an error from flash driver. Drivers should never return an error..  */
            if (status)
            {
        
                /* Call system error handler.  */
                _lx_nor_flash_system_error(nor_flash, status);

#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

              /* Return status.  */
                return(LX_ERROR);
            }

            /* Increment the number of obsolete physical sectors.  */
            nor_flash -> lx_nor_flash_obsolete_physical_sectors++;

            /* Decrement the number of mapped physical sectors.  */
            nor_flash -> lx_nor_flash_mapped_physical_sectors--;
            
            /* Invalidate the old sector mapping cache entry.  */
            _lx_nor_flash_sector_mapping_cache_invalidate(nor_flash, logical_sector);
        }

        /* Determine if the sector mapping cache is enabled.  */
        if (nor_flash -> lx_nor_flash_sector_mapping_cache_enabled)
        {
        
            /* Yes, sector mapping cache is enabled, place this sector information in the cache.  */
            
            /* Calculate the starting index of the sector mapping cache for this sector entry.  */
            i =  (logical_sector & LX_NOR_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NOR_SECTOR_MAPPING_CACHE_DEPTH;

            /* Build a pointer to the cache entry.  */
            sector_mapping_cache_entry_ptr =  &nor_flash -> lx_nor_flash_sector_mapping_cache[i];

            /* Move all the cache entries down so the oldest is at the bottom.  */
            *(sector_mapping_cache_entry_ptr + 3) =  *(sector_mapping_cache_entry_ptr + 2);
            *(sector_mapping_cache_entry_ptr + 2) =  *(sector_mapping_cache_entry_ptr + 1);
            *(sector_mapping_cache_entry_ptr + 1) =  *(sector_mapping_cache_entry_ptr);
           
            /* Setup the new sector information in the cache.  */
            sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_logical_sector =             (logical_sector | LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID);
            sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_physical_sector_map_entry =  new_mapping_address;
            sector_mapping_cache_entry_ptr -> lx_nor_sector_mapping_cache_physical_sector_address =    new_sector_address;
        }

        /* Indicate the write was successful.  */
        status =  LX_SUCCESS;        
    }
    else
    {
    
        /* Indicate the write was unsuccessful.  */
        status =  LX_NO_SECTORS;
    }

#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

    /* Return the completion status.  */
    return(status);
}


