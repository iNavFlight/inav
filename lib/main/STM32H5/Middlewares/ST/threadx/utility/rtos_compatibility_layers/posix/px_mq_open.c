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
/*    mq_open                                             PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine establishes connection between a named message queue   */
/*    and the calling a thread                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqName                      name of the queue to open.              */
/*    oflags                      O_RDONLY, O_WRONLY, O_RDWR, or O_CREAT, */
/*                                O_EXCEL,O_NONBLOCK.                     */
/*                                extra optional parameters.              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*     queue_des                  If successful                           */
/*     ERROR                      If fails                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_find_queue            find queue of given name                */
/*    posix_mq_create             create a queue                          */
/*    posix_get_queue_des         gets a queue-descriptor for Queue       */
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
/*  10-31-2022      Scott Larson            Update comparison with NULL,  */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
mqd_t  mq_open(const CHAR * mqName, ULONG oflags,...)
{

POSIX_MSG_QUEUE     *posix_queue;
struct mq_des       *queue_des;
struct mq_attr      *q_attr;
mode_t               mode;
va_list              create_queue;
ULONG                len;
ULONG                temp1; 

    len = strlen(mqName);
    if(len > PATH_MAX)
    {
        /* Return error.  */
        posix_errno = ENAMETOOLONG;
        posix_set_pthread_errno(ENAMETOOLONG);

        /*. Return error.  */
        return((struct mq_des *)ERROR);
    }
    switch(oflags & 0xFF00)
    {
        case O_CREAT:

        case (O_EXCL | O_CREAT):

             va_start(create_queue, oflags);
             mode   = va_arg(create_queue, mode_t);
             mode = mode;  /* just to keep the complier happy */
             q_attr = va_arg(create_queue, struct mq_attr *);
             va_end(create_queue);

             /* Check for valid messages and its size.  */
             if(!q_attr || q_attr->mq_maxmsg > MQ_MAXMSG || q_attr->mq_msgsize > MQ_MSGSIZE)
             {
                 /* return POSIX error.for invalid oflag.  */
                 posix_errno = EINVAL;
                 posix_set_pthread_errno(EINVAL);
                 /* return error.  */
                 return ((struct mq_des *)ERROR);
             }

             /* Check if name is exist. NULL if successful.  */
             if((posix_queue = posix_find_queue(mqName)) != NULL)
             {
                 if(posix_queue->unlink_flag == TX_TRUE)
                 {
                     /* return POSIX error.  */
                     posix_errno = ENOENT;
                 posix_set_pthread_errno(ENOENT);
                    /* return error.  */
                    return ((struct mq_des *)ERROR);
                 }

                 /* Set Posix error if name exist.  */
                 posix_errno = EEXIST;
                 posix_set_pthread_errno(EEXIST);
                 /* return error */
                 return((struct mq_des *)ERROR);
             }

             /* If q_attr is NULL then the default attributes of the struct 
                mq_attr are used */
             if(q_attr == NULL)
             {
                 q_attr = &(posix_qattr_default);
                 temp1 = q_attr->mq_maxmsg;
                 temp1= temp1 ;         /* Just to keep complier happy */ 
             }

             /* Create a queue which returns posix queue if successful and 
                NULL if fails.  */
             if(!(posix_queue = posix_mq_create(mqName, q_attr)))
             {

                /* posix_errno is filled up in mq_create.  */
                return((struct mq_des *)ERROR);
             }
             /* open count incremented by one.  */
             posix_queue->open_count += 1;
             break;

        case O_EXCL:
            /* Check if name is exist. NULL if successful.  */
            if(!(posix_queue = posix_find_queue(mqName)))
            {
                /* return POSIX error.  */
                posix_errno = EBADF;
                posix_set_pthread_errno(EBADF);
                /* return error.  */
                return ((struct mq_des *)ERROR);
            }

            return(OK);

        case O_RDONLY:
        case O_WRONLY:
        case O_RDWR:
        case O_NONBLOCK:
            /* Check if name is exist. NULL if successful.  */
            if((posix_queue = posix_find_queue(mqName)) != NULL)
            {
                if(posix_queue->unlink_flag == TX_TRUE)
                {
                    /* return POSIX error.  */
                    posix_errno = ENOENT;
                    posix_set_pthread_errno(ENOENT);
                    /* return error.  */
                    return ((struct mq_des *)ERROR);
                }
                /* open count incremented by one.  */
                posix_queue->open_count += 1;
            }
            break;

    default:
            /* return POSIX error.for invalid oflag.  */
            posix_errno = EINVAL;
            posix_set_pthread_errno(EINVAL);
            /* return error.  */
            return ((struct mq_des *)ERROR);
    }

    queue_des = posix_get_queue_des(posix_queue);
    /* Store the flags.  */
    queue_des->f_flag = oflags;
    return(queue_des);
}
