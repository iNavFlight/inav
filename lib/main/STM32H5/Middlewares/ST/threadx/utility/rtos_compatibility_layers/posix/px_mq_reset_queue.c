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
/*    posix_reset_queue                                  PORTABLE C       */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets a message queue structure                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    q_ptr                                 q_ptr                         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_byte_pool_delete                   Deletes byte pool             */
/*    posix_internal_error                  Returns generic error         */
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
VOID posix_reset_queue(POSIX_MSG_QUEUE * q_ptr)
{
    /* Indicate this entry is not in use.  */
    q_ptr->in_use = TX_FALSE;

    /* Reset thread name to NULL string.  */
    q_ptr -> name = NULL;

    /* Reset open count.  */
    q_ptr -> open_count = 0;

    /* Reset storage */
    q_ptr -> storage = NULL;

    /* Delete message area.  */
    if (tx_byte_pool_delete(&(q_ptr ->vq_message_area)))
    {
        /* Internal error.  */
        posix_internal_error(444);
    }
    /* Reset queue id.  */
    q_ptr -> px_queue_id = 0;

    /* Reset Unlink Flag  */
    q_ptr -> unlink_flag = TX_FALSE;
}
