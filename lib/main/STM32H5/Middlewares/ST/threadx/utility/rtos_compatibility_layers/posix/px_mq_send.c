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
/*    mq_send                                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    The mq_send() function puts a message of size msg_len and pointed to*/
/*    by msg_ptr into the queue indicated by mqdes. The new message has a */
/*    priority of msg_prio.                                               */
/*    The queue maintained is in priority order (priorities may range from*/
/*    0 to MQ_PRIO_MAX), and in FIFO order within the same priority.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqdes                             Queue descriptor                  */
/*    msg_ptr                           Message pointer                   */
/*    msg_len                           length of message                 */
/*    msg_prio                          Priority of the message           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                                no of bytes received              */
/*    ERROR                             If error occurs                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_identify                returns currently running thread  */
/*    tx_byte_allocate                  allocate memory                   */
/*    tx_queue_send                     ThreadX queue send                */
/*    posix_priority_search             search message for same priority  */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
INT  mq_send( mqd_t mqdes, const CHAR * msg_ptr, size_t msg_len, 
                ULONG msg_prio )
{

TX_QUEUE           *Queue; 
UINT                temp1; 
POSIX_MSG_QUEUE    *q_ptr; 
VOID               *bp;
UCHAR              *source;
UCHAR              *destination;
UCHAR              *save_ptr;
ULONG               mycount;
ULONG               msg[TX_POSIX_MESSAGE_SIZE];

    /* Assign a temporary variable for clarity.  */ 
    Queue = &(mqdes->f_data->queue); 
    q_ptr = (POSIX_MSG_QUEUE * )mqdes->f_data; 

    /* First, check for an invalid queue pointer.  */
    if ( (!q_ptr) || ( (q_ptr -> px_queue_id) != PX_QUEUE_ID))
    {
        /* Queue pointer is invalid, return appropriate error code.  */
        posix_errno = EBADF;
        posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(ERROR);
    }
    /* Make sure if we're calling this routine from a ISR timeout 
        is set to zero.  */ 
    if (!(tx_thread_identify())) 
    {
        /* POSIX doesn't have error for this, hence give default.  */
        posix_errno = EINTR ;
        posix_set_pthread_errno(EINTR);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* First, check for an invalid queue pointer.  */
    if ( (!q_ptr) || ( (q_ptr->queue.tx_queue_id) != TX_QUEUE_ID))
    {
        /* Queue descriptor is invalid, set appropriate error code.  */
        posix_errno = EBADF ;
        posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(ERROR);
    }
    if(((mqdes->f_flag & O_WRONLY) != O_WRONLY) && ((mqdes->f_flag & O_RDWR) != O_RDWR))
    {
        /* Queue pointer is invalid, return appropriate error code.  */
        posix_errno = EBADF;
        posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(ERROR);
    }
    if( msg_prio > MQ_PRIO_MAX)
    {
        /* Return appropriate error.  */
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);

        /* Return error.  */
        return(ERROR);
    }
    /* Check for an invalid source for message.  */
    if (! msg_ptr)
    {
        /* POSIX doesn't have error for this, hence give default.  */
        posix_errno = EINTR ;
        posix_set_pthread_errno(EINTR);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* Now check the length of message.  */
    if ( msg_len > (q_ptr->q_attr.mq_msgsize ) )
    {
        /*  Return message length exceeds max length.  */
        posix_errno = EMSGSIZE ;
        posix_set_pthread_errno(EMSGSIZE);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* Now try to allocate memory to save the message from the 
      queue's byte pool.  */
    temp1 = tx_byte_allocate((TX_BYTE_POOL * )&(q_ptr->vq_message_area), &bp,
                             msg_len, TX_NO_WAIT);

    if (temp1 != TX_SUCCESS)
    {
    posix_internal_error(9999);
    }   
    /* Got the memory , Setup source and destination pointers 
       Cast them in UCHAR as message length is in bytes.  */
    source      =  (UCHAR * ) msg_ptr;
    destination =  (UCHAR * ) bp;

    /* Save start of message storage.  */
    save_ptr   = destination;

    /* Copy the message into private buffer.  */
    for ( mycount = 0; mycount < msg_len; mycount++)
    {

        * destination++ =  * source++;
    }
    /* Restore the pointer of save message.  */
    source =  save_ptr ;
    /* Create message that holds saved message pointer and message length.  */
#ifdef TX_64_BIT
    msg[0] = (ULONG)((ALIGN_TYPE)source >> 32);
    msg[1] = (ULONG)((ALIGN_TYPE)source);
    msg[2] =  msg_len;
    msg[3] =  msg_prio;
    msg[4] =  posix_priority_search(mqdes, msg_prio);
#else
    msg[0] = (ULONG)source;
    msg[1] =  msg_len;
    msg[2] =  msg_prio;
    msg[3] =  posix_priority_search(mqdes, msg_prio);
#endif
    /* Attempt to post the message to the queue.  */
    temp1 = tx_queue_send(Queue, msg, TX_WAIT_FOREVER);
    if ( temp1 != TX_SUCCESS)
    {
        /* POSIX doesn't have error for this, hence give default.  */
        posix_errno = EINTR ;
	    posix_set_pthread_errno(EINTR);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* All done.  */
    return(OK);
}
