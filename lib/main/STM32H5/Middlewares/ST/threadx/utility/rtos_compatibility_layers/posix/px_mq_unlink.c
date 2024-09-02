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
/*    mq_unlink                                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine removes the named message queue                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    const CHAR * mqName           Message Queue name.                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                            If successful                         */
/*    ERROR                         IF fails                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_internal_error          Generic error handler                 */
/*    posix_find_queue              Finds the required queue              */
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
INT  mq_unlink(const CHAR * mqName)
{

POSIX_MSG_QUEUE    *q_ptr;
ULONG               len;
ULONG               temp1;

    /* Checking for the invalid length.  */
    len = strlen(mqName);
    if(len > 10)
    {
        /* Return appropriate error.  */
        posix_errno = ENAMETOOLONG;
	    posix_set_pthread_errno(ENAMETOOLONG);

        /* Return Error.  */
        return(ERROR);
    }

    /* For checking the name.  */
    if(!(q_ptr = posix_find_queue(mqName)))
    {
        /* Set Posix error if name exist.  */
        posix_errno = EEXIST;
	    posix_set_pthread_errno(EEXIST);

        /* Return error.  */
        return(ERROR);
    }

    if(q_ptr)
        /* Unlinks the message Queue.  */
        q_ptr->unlink_flag = TX_TRUE;

    /* check if the message queue is not open in any task.  */
    if(q_ptr->open_count == 0)
    {
        /* Free the system resource allocated by open call.  */
        temp1 = posix_queue_delete( q_ptr );

        if( temp1 != TX_SUCCESS)
        {
            /*. Return generic error.  */
            posix_internal_error(100);

            /* Return error.  */
            return(ERROR);
        }
    }
    return(OK);
}
