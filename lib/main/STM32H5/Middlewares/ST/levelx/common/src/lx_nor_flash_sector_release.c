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
/*    _lx_nor_flash_sector_release                        PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function releases a logical sector from being managed in the   */ 
/*    NOR flash.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    logical_sector                        Logical sector number         */ 
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
/*    _lx_nor_flash_sector_mapping_cache_invalidate                       */ 
/*                                          Invalidate cache entry        */ 
/*    _lx_nor_flash_logical_sector_find     Find logical sector           */ 
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
UINT  _lx_nor_flash_sector_release(LX_NOR_FLASH *nor_flash, ULONG logical_sector)
{

UINT    status;
ULONG   *mapping_address;
ULONG   mapping_entry;
ULONG   *sector_address;
ULONG   i;


#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nor_flash -> lx_nor_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Increment the number of read requests.  */
    nor_flash -> lx_nor_flash_read_requests++;

    /* See if we can find the sector in the current mapping.  */
    _lx_nor_flash_logical_sector_find(nor_flash, logical_sector, LX_FALSE, &mapping_address, &sector_address);
    
    /* Determine if the logical sector was found.  */
    if (mapping_address)
    {
    
        /* Yes, we were able to find the logical sector.  */
        
        /* Read in the old sector mapping.  */
#ifdef LX_DIRECT_READ
        
        /* Read the word directly.  */
        mapping_entry =  *(mapping_address);
#else
        status =  _lx_nor_flash_driver_read(nor_flash, mapping_address, &mapping_entry, 1);

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

        /* Mark this sector as invalid.  */
        /* Now clear bits 31 and 30, which indicates this sector is now obsoleted.  */
        mapping_entry =  mapping_entry & ~(((ULONG) LX_NOR_PHYSICAL_SECTOR_VALID) | ((ULONG) LX_NOR_PHYSICAL_SECTOR_SUPERCEDED));
            
        /* Write the value back to the flash to clear bits 31 & 30.  */
        status =  _lx_nor_flash_driver_write(nor_flash, mapping_address, &mapping_entry, 1);
            
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
            
        /* Ensure the sector mapping cache no longer has this sector.  */
        _lx_nor_flash_sector_mapping_cache_invalidate(nor_flash, logical_sector);

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

        /* Set the status to success.  */
        status =  LX_SUCCESS;
    }
    else
    {

        /* Could not find the logical sector.  */
        status =  LX_SECTOR_NOT_FOUND;
    }
    
#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

    /* Return status.  */
    return(status);
}

