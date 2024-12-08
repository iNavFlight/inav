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
/*    _lx_nor_flash_extended_cache_enable                 PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables or disables the extended cache.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    memory                                Address of RAM for cache      */ 
/*    size                                  Size of the RAM for cache     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
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
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     William E. Lamie         Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.3  */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            updated product constants   */
/*                                            resulting in version 6.1.7  */
/*  10-15-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            added check for out of      */
/*                                            bound memory access,        */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nor_flash_extended_cache_enable(LX_NOR_FLASH *nor_flash, VOID *memory, ULONG size)
{
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE

UINT    i;
ULONG   cache_size;
ULONG   *cache_memory;


    /* Determine if memory was specified but with an invalid size (less than one NOR sector).  */
    if ((memory) && (size < LX_NOR_SECTOR_SIZE))
    {
    
        /* Error in memory size supplied.  */
        return(LX_ERROR);
    }

#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nor_flash -> lx_nor_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Initialize the internal NOR cache.  */
    nor_flash -> lx_nor_flash_extended_cache_entries =  0;

    /* Calculate cache size in words.  */
    cache_size = size/sizeof(ULONG);

    /* Setup cache memory pointer.  */
    cache_memory =  (ULONG *) memory;

    /* Loop through the memory supplied and assign to cache entries.  */
    i =  0;
    while (cache_size >= LX_NOR_SECTOR_SIZE)
    {
    
        /* Setup this cache entry.  */
        nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_sector_address =  LX_NULL;
        nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_sector_memory =   cache_memory;
        nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_access_count =    0;
        
        /* Move the cache memory forward.   */
        cache_memory =  cache_memory + LX_NOR_SECTOR_SIZE;
        
        /* Decrement the size.  */
        cache_size =  cache_size - LX_NOR_SECTOR_SIZE;
    
        /* Move to next cache entry.  */
        i++;
    }
    
    /* Save the number of cache entries.  */
    if(i > LX_NOR_EXTENDED_CACHE_SIZE)
    {

        nor_flash -> lx_nor_flash_extended_cache_entries =  LX_NOR_EXTENDED_CACHE_SIZE;
    }
    else
    {

        nor_flash -> lx_nor_flash_extended_cache_entries =  i;
    }  

#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

    /* Return successful completion.  */
    return(LX_SUCCESS);
#else

    LX_PARAMETER_NOT_USED(nor_flash);
    LX_PARAMETER_NOT_USED(memory);
    LX_PARAMETER_NOT_USED(size);

    /* Return disabled error message.  */
    return(LX_DISABLED);
#endif
}


