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
/*    posix_queue_delete                                  PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    deletes a message queue                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    q_ptr                         message queue pointer                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ERROR                         If fails                              */
/*    OK                            If successful                         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_queue_delete               Detectes a Queue                      */
/*    posix_internal_error          Generic error                         */
/*    posix_reset queue             Resets queue structure                */
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
INT posix_queue_delete(POSIX_MSG_QUEUE  * q_ptr)
{

TX_QUEUE          *queue;

    queue = &(q_ptr->queue);

    /* Delete the Threadx queue.  */
    if(tx_queue_delete(queue))
    {
        /* return generic error.  */
        posix_internal_error(444);
        /* return error.  */
        return(ERROR);
    }

    /* Release the queue back to the pool.  */
    posix_reset_queue(q_ptr);
    q_ptr = NULL;
    return(OK);
}
