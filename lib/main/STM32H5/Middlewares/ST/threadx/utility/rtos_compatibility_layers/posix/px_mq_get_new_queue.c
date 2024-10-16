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

#include "tx_api.h"     /* Threadx API */
#include "pthread.h"    /* Posix API */
#include "px_int.h"     /* Posix helper functions */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_get_new_queue                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to build a ThreadX message queue             */
/*    for variable length message queues                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    maxnum                                Number of queue entries       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    q_ptr                                 Pointer to posix queue        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_memory_allocate                 Memory allocate               */
/*    posix_memory_release                  Memory free                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    POSIX internal Code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
POSIX_MSG_QUEUE  * posix_get_new_queue(ULONG maxnum)
{

ULONG                   i; 
POSIX_MSG_QUEUE        *q_ptr; 
VOID                   *bp; 
INT                     retval; 
ULONG                   size; 


    /* Determine how much memory we need for the queue. 
       The queue holds "maxnum" entries;  each entry is 2 ULONG.  */
    size   = (maxnum * (TX_4_ULONG * sizeof(ULONG)));

    /* Now attempt to allocate memory for the queue.  */
    retval = posix_memory_allocate(size, &bp);

    /* Make sure we obtained the memory we requested.  */
    if (retval)
    {
        return((POSIX_MSG_QUEUE  *)NULL);
    }
    /* Loop through the list of queues -  */
    /* try to find one that is not "in use".  */
    /* Search the queue pool for an available queue.  */
   for (i = 0, q_ptr = &(posix_queue_pool[0]);
            i < POSIX_MAX_QUEUES; 
            i++, q_ptr++)
   {
        /* Make sure the queue is not "in use".   */
        if (q_ptr->in_use == TX_FALSE)
        {
            /* This queue is now in use.  */
            q_ptr->in_use = TX_TRUE; 

            /* Point to allocated memory.  */
            q_ptr->storage = bp;

            q_ptr->px_queue_id = PX_QUEUE_ID;

            /* Stop searching.  */
            break; 
        }
   }
   /* If we didn't find a free queue, free the allocated memory. */ 
   if ( i >= POSIX_MAX_QUEUES)
   {
       posix_memory_release(bp);
        return((POSIX_MSG_QUEUE  *)0);
   }
   /* Return home.  */
   return(q_ptr);
}
