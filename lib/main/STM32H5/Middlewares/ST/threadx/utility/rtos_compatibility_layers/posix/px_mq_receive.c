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
/*    mq_receive                                          PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*                                                                        */
/*    Receive a message from a message queue.                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqdes                         message queue descriptor              */
/*   *pMsg                          buffer to receive message             */
/*    msgLen                        size of buffer, in bytes              */
/*   *pMsgPrio                      If not NULL, return message priority  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    temp1                         no of bytes received                  */
/*    ERROR                         If error occurs                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_internal_error          Generic error handler                 */
/*    tx_queue_receive              ThreadX queue receive                 */
/*    tx_byte_release               Release bytes                         */
/*    posix_memory_allocate         Allocate memory                       */
/*    tx_thread_identify            Returns currently running thread      */
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
ssize_t mq_receive( mqd_t mqdes, VOID * pMsg, size_t msgLen, ULONG *pMsgPrio)
{

TX_QUEUE            * Queue;
POSIX_MSG_QUEUE     * q_ptr;
INT                   temp1, retval = ERROR;
ULONG                 wait_option,length_of_message, priority_of_message,mycount;
ULONG               * my_ptr;
CHAR                * this_ptr;
VOID                * msgbuf1;
UCHAR               * msgbuf2;
VOID                * message_source;

    /* Assign a temporary variable for clarity.  */ 
    Queue = &(mqdes->f_data->queue); 
    q_ptr = (POSIX_MSG_QUEUE * )mqdes->f_data; 

    /* First, check for an invalid queue pointer.  */
    if ((!q_ptr) || ( (q_ptr -> px_queue_id) != PX_QUEUE_ID))
    {
        /* Queue pointer is invalid, return appropriate error code.  */
        posix_errno = EBADF;
            posix_set_pthread_errno(EBADF);
        /* Return ERROR.  */
        return(ERROR);
    }

    if(((mqdes ->f_flag & O_RDONLY) != O_RDONLY ) && ((mqdes->f_flag & O_RDWR) != O_RDWR))
    {
        /* Queue pointer is invalid, return appropriate error code.  */
        posix_errno = EBADF;
        posix_set_pthread_errno(EBADF);

        /* Return ERROR.  */
        return(ERROR);
    }

    /* Check if message length provided is less q size provided while creation.  */
    if( msgLen < q_ptr -> q_attr.mq_msgsize )
    {
        /* Return appropriate error.  */
        posix_errno = EMSGSIZE;
        posix_set_pthread_errno(EMSGSIZE);

        /* Return error.  */
        return(ERROR);
    }

    /* User has indicated a wait is acceptable.  */
    /* if a message is not immediately available.  */
    /* If we are not calling this routine from a thread context.  */
    if (!(tx_thread_identify()))
    {
        /* return appropriate error code.  */
        posix_errno = EBADF;
        posix_set_pthread_errno(EBADF);
        /* Return ERROR.  */
        return(ERROR);
    }
    if ( ( mqdes ->f_flag & O_NONBLOCK ) == O_NONBLOCK )
            wait_option = TX_NO_WAIT;
    else
            wait_option = TX_WAIT_FOREVER;

    
    /* Try to get a message from the message queue.  */
    /* Create a temporary buffer to get message pointer and message length.  */
    temp1 = posix_memory_allocate((sizeof(ULONG)) * TX_POSIX_MESSAGE_SIZE, (VOID**)&msgbuf1);
    if(temp1 != TX_SUCCESS )
    {
        /* Return generic error.  */
        posix_internal_error(100);

        /* Return error.  */
        return(ERROR);
    }
    /* Arrange the messages in the queue as per the required priority.  */
    temp1 = posix_arrange_msg( Queue, pMsgPrio );
    /* Receive the message */
    temp1 = tx_queue_receive(Queue, msgbuf1, wait_option);
   /* Some ThreadX error codes map to posix error codes.  */
    switch(temp1)
   {
        case TX_SUCCESS:
        {

           /* All ok  */
            temp1 = OK;
            break; 
        }

        case TX_DELETED:
        {
            break; 
        }

        case TX_QUEUE_EMPTY:
        {
            if (wait_option == TX_NO_WAIT)
            {
                /* No message to receive while NO_WAIT option is set  */
                posix_errno = EAGAIN;
                posix_set_pthread_errno(EAGAIN);

                /* Return error  */
                temp1 = ERROR;
                return(temp1);
            }
            break;
        }
        case TX_QUEUE_ERROR:
        case TX_PTR_ERROR:
        {
            /* Queue pointer is invalid, return appropriate error code.  */
            posix_errno = EBADF;
            posix_set_pthread_errno(EBADF);

            /* Return ERROR.  */
            temp1 = ERROR;
            return(temp1);
        }

        default:
        {
            /* should not come here  but send the default error.  */
            posix_errno = EBADF;
            posix_set_pthread_errno(EBADF);

            /* Return error.  */
            temp1 = ERROR;
            return(temp1);
        }
    }
   
    /* Assign a variable for clarity.  */
    my_ptr = ( ULONG *)msgbuf1;

    /* Retrieve Message pointer, message Length and message priority.  */
#ifdef TX_64_BIT
    this_ptr          = (CHAR *)((((ALIGN_TYPE)my_ptr[0]) << 32) | my_ptr[1]);
    length_of_message =  my_ptr[2];
    priority_of_message = my_ptr[3];
#else
    this_ptr          =   (CHAR *)(*my_ptr);
    length_of_message =    *(++my_ptr);
    priority_of_message = *(++my_ptr);
    
#endif
    message_source    = (VOID *)this_ptr;
    
    /* Copy message into supplied buffer.  */
    msgbuf2 = (UCHAR *)pMsg;

    /* Release memory after storing data into destination.  */
    retval = tx_byte_release(msgbuf1);

    if( retval)
    {
        /* return generic error.  */
        posix_internal_error(100);

        /* Return error.  */
        return(ERROR);
    }

    if ( temp1 == OK)
    {
        for (mycount = 0; ( (mycount < length_of_message) && (mycount < msgLen)); mycount++)
        {
            *(msgbuf2++) = *((UCHAR *)(this_ptr++));
        }
 
        temp1 = mycount;
    }

    retval = tx_byte_release(message_source);

    if( retval)
    {
        /* return generic error.  */
        posix_internal_error(100);

        /* Return error.  */
        return(ERROR);
    }

    /* Copy message priority */ 
    if (pMsgPrio) 
    {
        *pMsgPrio = priority_of_message;
    }
    
    /* All done  */
    return(length_of_message);
}
