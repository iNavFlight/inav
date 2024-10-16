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
/*    _lx_nand_flash_sector_read                          PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reads a logical sector from NAND flash.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    buffer                                Pointer to buffer to read into*/ 
/*                                            (the size is number of      */ 
/*                                             bytes in a page)           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_driver_read            Driver page read              */ 
/*    _lx_nand_flash_logical_sector_find    Find logical sector           */ 
/*    _lx_nand_flash_system_error           Internal system error handler */ 
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
UINT  _lx_nand_flash_sector_read(LX_NAND_FLASH *nand_flash, ULONG logical_sector, VOID *buffer)
{

UINT        status;
ULONG       found_block;
ULONG       found_page;
ULONG       i;
ULONG       *word_ptr;


#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nand_flash -> lx_nand_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Increment the number of read requests.  */
    nand_flash -> lx_nand_flash_diagnostic_sector_read_requests++;

    /* See if we can find the sector in the current mapping.  */
    _lx_nand_flash_logical_sector_find(nand_flash, logical_sector, LX_FALSE, &found_block, &found_page);
    
    /* Determine if the logical sector mapping was found.  */
    if (found_page)
    {
    
        /* Yes, we were able to find the logical sector to page mapping.  */
        
        /* Read the data from the page.  */
        status =  _lx_nand_flash_driver_read(nand_flash, found_block, found_page, buffer, nand_flash -> lx_nand_flash_words_per_page);

        /* Check for an error from flash driver.   */
        if (status)
        {
        
            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, found_block, found_page);
            
            /* Determine if the error was corrected.  */
            if (status == LX_NAND_ERROR_CORRECTED)
            {
            
                /* Yes, we were able to read this page successfully with error correction.  */
                
                /* Change the status to LX_SUCCESS.  */
                status =  LX_SUCCESS;
            }
            else
            {
            
                /* Adjust return status.  */
                status =  LX_ERROR;
            }
        }
        else
        {

            /* Set the status to success.  */
            status =  LX_SUCCESS;
        }
    }
    else
    {
        
        /* Sector hasn't been written. Simply fill the destination buffer with ones and return success.  */

        /* Setup pointer to users buffer.  */
        word_ptr =  (ULONG *) buffer;
        
        /* Put all ones in he buffer.  */
        for (i = 0; i < nand_flash -> lx_nand_flash_words_per_page; i++)
        {
        
            /* Copy a word.  */
            *word_ptr++ =  LX_ALL_ONES;
        }       

        /* Set the status to success.  */
        status =  LX_SUCCESS;
    }
    
#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

    /* Return status.  */
    return(status);
}

