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
/** POSIX wrapper for THREADX                                             */ 
/**                                                                       */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Include necessary system files.  */

#include "tx_api.h"    /* Threadx API */
#include "pthread.h"  /* Posix API */
#include "px_int.h"    /* Posix helper functions */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_memory_release                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to return the specified memory region back   */
/*    into the POSIX heap.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    memory_ptr                            Memory pointer to return      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_byte_release                       Release the memory            */
/*    posix_internal_error                  internal error handler        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    POSIX internal code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
VOID posix_memory_release(VOID * memory_ptr)
{

    /* Check if memory_ptr is part of the POSIX byte pool, 
     * if not just return  */ 
    if (((CHAR *)memory_ptr >= 
                (CHAR *)&posix_heap_byte_pool.tx_byte_pool_start)  || 
        ((CHAR *)memory_ptr <= 
                (CHAR *)posix_heap_byte_pool.tx_byte_pool_start  
                      + posix_heap_byte_pool.tx_byte_pool_size)) {
      tx_byte_release(memory_ptr);
    }

    /* Return to caller.  */
    return;
}
