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
/*    posix_memory_allocate                               PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to obtain the specified amount of memory     */
/*    from the POSIX heap.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    size                                  Number of bytes in the pool   */
/*    memory_ptr                            Pointer to memory ptr variable*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ERROR                                 If error occurs               */
/*    retval                                Return value                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_byte_allocate                      Allocate from the pool        */
/*    posix_internal_error                  Generic posix error           */
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
INT posix_memory_allocate(ULONG size, VOID **memory_ptr)
{

INT    retval;

    /* Init in case we fail.  */
    *memory_ptr = (VOID *)0;

    /* Force all alignments to long word boundaries to be safe.  */
    if (size & 0x03)
    {
        /* Bump size up to next 4 byte boundary.  */
        size = ((size + 0x03) & ~0x03);
    }
    /* Attempt to allocate the desired memory from the POSIX heap.
       Do not wait - if memory isn't available, flag an error.  */
    retval = tx_byte_allocate((TX_BYTE_POOL *)&posix_heap_byte_pool, memory_ptr,
                              size, TX_NO_WAIT); 

    /* Make sure the memory was obtained successfully.  */
    if (retval)
    {
        /* Error obtaining memory.  */
        posix_internal_error(555);
        /* return error.  */
        return(ERROR);
    }
    /* Return to caller.  */
    return(retval); 
}
