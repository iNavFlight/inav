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
/*    posix_arrange_msg                                   PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Return the oldest, highest priority message from the queue.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Queue                         queue descriptor                      */
/*   *pMsgPrio                      If not NULL, priority of message      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                            Always return successful              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
/*  10-31-2022      Scott Larson            Modified comments,            */
/*                                            fixed message swap logic,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
ULONG posix_arrange_msg(TX_QUEUE *Queue, ULONG *pMsgPrio)
{
    ULONG*  q_read;             /* to store read ptr of the queue        */
    ULONG*  temp_q = TX_NULL;   /* temp storage for the message pointer  */
    ULONG   numMsgs;            /* no of messages queued                 */
    ULONG   msg;                /* temp variable for thr for loop        */
    ULONG   priority;           /* priority of the message               */
    ULONG   maxPrio;            /* max. priority of the messages in queue*/
    ULONG   number2;            /* messages                              */
    ULONG   minNo;              /* oldest message in the same priority   */
    ULONG   swap;               /* temp.variable for the swapping of the */
                                /* messages                              */

    /* initialize the priority to the lowest priority.  */
    maxPrio = 0;
    minNo = 0;

    /* Copy read pointer to the temporary variable.  */
    q_read = Queue -> tx_queue_read;

    /* Copy no. of messages in the queue to the temporary variable.  */
    numMsgs = Queue -> tx_queue_enqueued;

    /* If there is 0 or 1 message, no rearranging is needed.  */
    if (numMsgs < 2)
    {
        return(OK);
    }

    for (msg = 0; msg < numMsgs; msg++)
    {
        /* Advance q_read to read the priority of the message.  */
        q_read = q_read + TX_POSIX_QUEUE_PRIORITY_OFFSET;

        /* Priority of the message queued.  */
        priority = *q_read;

        /* check with maxpriority.  */
        if (priority > maxPrio)
        {
            /* copy read pointer to temporary pointer.  */
            temp_q = q_read-TX_POSIX_QUEUE_PRIORITY_OFFSET;

            /* increment read pointer to point to order.  */
            q_read++;

            /* copy FIFO order to the message  */
            minNo = *q_read;
            
            /* Found higher priority message.  */
            maxPrio = priority;

            q_read++;
        }

        /* if more than one message of the same priority is in the queue
           then check if this the oldest message.  */
        else if (priority == maxPrio)
        {
            /* increment read pointer to point to read FIFO order */
            q_read++;

            /* copy number to the local variable.  */
            number2 = *q_read;
            
            /* Go to next message.  */
            q_read++;
            
            /* find the oldest of the messages in this priority level.  */
            if( number2 < minNo )
            {
                /* founder older one  */
                minNo = number2;
                /* copy read pointer to temporary buffer.  */
                temp_q = q_read - (TX_POSIX_MESSAGE_SIZE);
            }
        }

        else
        {
            /* Not highest priority, go to next message.  */
            q_read = q_read + (TX_POSIX_MESSAGE_SIZE - TX_POSIX_QUEUE_PRIORITY_OFFSET);
        }

        /* Determine if we are at the end.  */
        if (q_read >= Queue -> tx_queue_end)
        {
            /* Yes, wrap around to the beginning.  */
            q_read = Queue -> tx_queue_start;
        }
    }

    /* Output priority if non-null */
    if (pMsgPrio != NULL)
    {
        /* copy message priority.  */
        *pMsgPrio = maxPrio;
    }

    /* All messages checked, temp_q holds address of oldest highest priority message
       and maxPrio holds the highest priority.  */
    /* Get the current queue read pointer */
    q_read = Queue -> tx_queue_read;

    if((temp_q != TX_NULL) && (temp_q != q_read))
    {
        /* Swap the messages.  */
        for (msg = 0; msg < TX_POSIX_MESSAGE_SIZE; msg++)
        {
            swap = *temp_q;
            *temp_q = *q_read;
            *q_read = swap;
            temp_q++;
            q_read++;
        }
    }

    return(OK);
}
