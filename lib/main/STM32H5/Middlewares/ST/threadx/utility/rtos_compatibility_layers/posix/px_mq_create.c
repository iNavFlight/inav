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
/*    posix_mq_create                                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This subroutine creates and initializes a message queue             */
/*    As the message length is user defined, message pointer and message  */
/*    length is stored in a ThreadX queue(instead of the actual message)  */
/*    The actual message is stored in a dedicated byte pool for the       */
/*    queue                                                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mq_name                               name of the Queue             */
/*    msgq_attr                             Pointer to mq_attr structure  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    posix_q                               If success                    */
/*    NULL                                  If failure                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_in_thread_context               Make sure caller is thread    */
/*    tx_queue_create                       to create a ThreadX Queue     */
/*    posix_internal_error                  Generic error Handler         */
/*    posix_memory_allocate                 to create a byte pool         */
/*    tx_queue_delete                       to delete the queue           */
/*    posix_putback_queue                   to delete the queue           */
/*    tx_byte_pool_create                   to create a byte pool         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    POSIX internal Code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Add 64-bit support,           */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
POSIX_MSG_QUEUE * posix_mq_create (const CHAR * mq_name,
                                      struct mq_attr * msgq_attr)
{

TX_INTERRUPT_SAVE_AREA

POSIX_MSG_QUEUE    *posix_q;
UINT                temp1;
VOID               *bp;
INT                 retval;
ULONG               size;
TX_QUEUE           *TheQ;

    /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
       /* return POSIX error.  */
       posix_internal_error(444);

       /* return error.  */
       return ((POSIX_MSG_QUEUE *)ERROR);
    }
    /* Disable interrupts.*/
    TX_DISABLE

    /* Get a new queue from the POSIX queue pool.  */
    /* Make sure we have enough space for the size.  */

    posix_q = posix_get_new_queue(msgq_attr->mq_maxmsg);

    /* Make sure we actually got a queue.  */
    if (!posix_q)
    {
        /* Restore interrupts.  */
        TX_RESTORE

        /* User configuration error -  not enough memory.  */
        posix_errno = EBADF;
	    posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(TX_NULL);
    }

    /* Now create a ThreadX message queue. 
       to store only the message pointer and message length.  */
    temp1 = tx_queue_create((&(posix_q->queue)),
                             (CHAR *)mq_name,
                             TX_POSIX_MESSAGE_SIZE,
                             posix_q->storage,
                             (msgq_attr->mq_maxmsg * (sizeof(ULONG) * TX_POSIX_MESSAGE_SIZE)));

    /* Make sure it worked.  */
    if (temp1 != TX_SUCCESS)
    {
        /*. Return generic error.  */
        posix_internal_error(188);

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return ERROR.  */
        return(TX_NULL);
    }
    /* Restore no. of maximum messages.  */
    posix_q->q_attr.mq_maxmsg = msgq_attr->mq_maxmsg;

    /* Restore maximum message length.  */
    posix_q->q_attr.mq_msgsize = msgq_attr->mq_msgsize;

    /* Flags are stored in que descriptor structure and 
       not in mq_att structure.  */

    /* Create a byte pool for the  queue.  
       Determine how much memory we need to store all messages in this queue.   
       11 bytes are added to counter overhead as well as alignment problem if any.  */
    size = ( ((msgq_attr->mq_maxmsg) + 1)  * (msgq_attr->mq_msgsize + 11) );

    if(size < 100)
        size = 100;

    /* Now attempt to allocate that much memory for the queue.  */

    retval = posix_memory_allocate(size,&bp);

    /* Make sure we obtained the memory we requested.  */
    if (retval)
    {
        /* Created  queue Control block, got memory to store message pointers
           and lengths which means that created a fixed length message queue but
           not enough memory to store actual messages.  */

        /* Delete the queue.  */

        /* Assign a temporary variable for clarity.  */
        TheQ   = (TX_QUEUE * )posix_q;
        retval = tx_queue_delete(TheQ);

        /* Make sure the queue was deleted.  */
        if (retval != TX_SUCCESS)
        {
            /* Return generic error.  */
            posix_internal_error(799);

            /* Restore interrupts.  */
            TX_RESTORE

            /* Return ERROR.  */
            return(TX_NULL);
        }
        /* Put the queue back into the POSIX queue pool.  */
        posix_putback_queue(TheQ); 

        /* User configuration error -  not enough memory.  */
        posix_errno =  EBADF;
        posix_set_pthread_errno(EBADF);
        TX_RESTORE;

        /* Return ERROR.  */
        return(TX_NULL);
    }
    /* Create a ThreadX byte pool that will provide memory needed by the queue.  */
    retval = tx_byte_pool_create((&(posix_q->vq_message_area)), "POSIX Queue",
                                    bp, size);

    /* Make sure the byte pool was created successfully.  */
    if (retval)
    {
        /* Error creating byte pool.  */
        posix_internal_error(9999);

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return ERROR.*/    
        return(TX_NULL);
    }

    posix_q->name = (CHAR*) mq_name;

    /* Restore interrupts.  */
    TX_RESTORE

    /* All done.  */
    return(posix_q);
}
