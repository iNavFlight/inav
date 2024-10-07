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
/*    mq_close                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    close a message queue                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqdes                         message queue descriptor              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                            If successful                         */
/*    ERROR                         If error occurs                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_internal_error          Generic error handler                 */
/*    tx_byte_release               Release bytes                         */
/*    tx_thread_identify            Returns currently running thread      */
/*    posix_queue_delete            Deletes specific queue                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
INT  mq_close(mqd_t mqdes)
{

TX_INTERRUPT_SAVE_AREA

TX_QUEUE          * Queue; 
POSIX_MSG_QUEUE   * q_ptr;

    /* Assign a temporary variable for clarity.  */ 
    Queue = &(mqdes->f_data->queue); 
    q_ptr = (POSIX_MSG_QUEUE * )Queue; 

    /* First, check for an invalid queue pointer.  */
    if ( (!q_ptr) || ( (q_ptr -> px_queue_id) != PX_QUEUE_ID))
    {
        /* Queue pointer is invalid, return appropriate error code.  */
        posix_errno = EBADF;
	    posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* If we are not calling this routine from a thread context.  */
    if (!(tx_thread_identify()))
    {
        /* return appropriate error code.  */
        posix_errno = EBADF;
	    posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* Free the system resource allocated by open call.  */
    if( tx_byte_release( ( VOID *) mqdes ))
    {
        /* return generic error.  */
        posix_internal_error(100);

        /* Return error.  */
        return(ERROR);
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Decrement ref count.  */
    if( q_ptr->open_count )

        q_ptr ->open_count--;

    /* Destroy the basic Queue is the ref count is zero and 
        it is marked by unlink.  */
    if( (! q_ptr ->open_count ) && (q_ptr->unlink_flag == TX_TRUE))
    {
        /* Free the system resource allocated by open call.  */
        if( posix_queue_delete( q_ptr ))
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* return generic type.  */
            posix_internal_error(100);

            /* Return error.  */
            return(ERROR);
        }
    }
    /* Restore interrupts.  */
    TX_RESTORE

    /* Return OK.  */
    return(OK);
}
