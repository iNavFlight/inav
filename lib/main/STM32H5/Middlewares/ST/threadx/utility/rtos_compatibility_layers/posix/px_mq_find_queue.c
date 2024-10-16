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
/*    posix_find_queue                                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine returns queue descriptor indicating that name of       */
/*    in the message queue.exists.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    const char * mq_name                     Name of the message queue  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    q_ptr                                    If successful              */
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
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
POSIX_MSG_QUEUE * posix_find_queue(const CHAR *mq_name)
{

POSIX_MSG_QUEUE        *q_ptr;
ULONG                   index;
ULONG                   match;
CHAR                   *dummy_name;
CHAR                   *dummy_queue_name;
ULONG                   namelength;

    q_ptr = (POSIX_MSG_QUEUE*)NULL;

    /* For checking the name.  */
    for(index = 0,q_ptr = posix_queue_pool;index < POSIX_MAX_QUEUES ;index ++,q_ptr ++)
    {
        /* Assume the worst case.  */
        match = TX_FALSE;
        if(q_ptr->in_use == TX_TRUE)
        {
            dummy_queue_name = q_ptr->name;
            for(namelength = 0 ,dummy_name = (CHAR *) mq_name ; namelength < PATH_MAX ;
                  namelength ++, dummy_name++,dummy_queue_name ++)
            {
                /*  Copy name.  */
                if (*dummy_name == *dummy_queue_name)
                {
                    /* End of the string.  */
                    if(*dummy_name == '\0')
                    {
                        match = TX_TRUE;
                        break;
                    }
                }/* Copy name.  */
                else
                    break;
            }
        }
        if(match==TX_TRUE)
            break;
   }
   /* For each message queue.  */
   if(match==TX_TRUE)
       return(q_ptr);
   /* Returns NULL if match not found. */
   q_ptr = (POSIX_MSG_QUEUE*)NULL;
   return (q_ptr);
}
