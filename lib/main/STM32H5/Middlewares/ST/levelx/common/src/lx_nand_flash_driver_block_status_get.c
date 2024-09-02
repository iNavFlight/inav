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
/**   NAND Flash                                                          */
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
/*    _lx_nand_flash_driver_block_status_get              PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the driver to get the block status and          */ 
/*    updates the internal cache.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    bad_block_flag                        Pointer to Bad block flag     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_block_status_get)                             */ 
/*                                          NAND flash block status get   */ 
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
UINT  _lx_nand_flash_driver_block_status_get(LX_NAND_FLASH *nand_flash, ULONG block, UCHAR *bad_block_flag)
{

UINT    status;


    /* Determine if the block status cache is disabled.  */
    if (nand_flash -> lx_nand_flash_block_status_cache == LX_NULL)
    {
    
        /* Increment the block status get count.  */
        nand_flash -> lx_nand_flash_diagnostic_block_status_gets++;

        /* Call driver block status get function.  */
        status =  (nand_flash -> lx_nand_flash_driver_block_status_get)(block, bad_block_flag);
    }
    else
    {

        /* Determine if the block status cache entry is valid.  */
        if (nand_flash -> lx_nand_flash_block_status_cache[block] != 0)
        {
      
            /* Simply return this value.  */
            *bad_block_flag =  nand_flash -> lx_nand_flash_block_status_cache[block];
               
            /* Increment the number of status byte cache hits.  */
            nand_flash -> lx_nand_flash_diagnostic_block_status_cache_hits++;
        
            /* Return successful status.  */
            status =  LX_SUCCESS;
        }
        else
        {

            /* Increment the block status get count.  */
            nand_flash -> lx_nand_flash_diagnostic_block_status_gets++;

            /* Call driver block status get function.  */
            status =  (nand_flash -> lx_nand_flash_driver_block_status_get)(block, bad_block_flag);

            /* Increment the number of status byte cache misses.  */
            nand_flash -> lx_nand_flash_diagnostic_block_status_cache_misses++;

            /* Save the block status value in the cache.  */
            nand_flash -> lx_nand_flash_block_status_cache[block] =  *bad_block_flag;
        }
    }

    /* Return status.  */
    return(status);
}


