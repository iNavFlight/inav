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
/*    sem_open                                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates/initialize a semaphore.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    name                                  Semaphore name                */
/*    oflags                                Flags                         */
/*    mode                                  Optional parameter            */
/*    value                                 Optional parameter            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    sem                                   Semaphore descriptor          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_in_thread_context               Make sure caller is thread    */
/*    posix_find_sem                        Finds the required semaphore  */
/*    posix_get_new_sem                     Get new semaphore block       */
/*    posix_internal_error                  Internal posix error          */
/*    tx_semaphore_create                   ThreadX semaphore create      */
/*    posix_set_sem_name                    Sets name for semaphore       */
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
sem_t * sem_open(const CHAR * name, ULONG oflag, ...)
{

TX_INTERRUPT_SAVE_AREA

TX_SEMAPHORE     *TheSem; 
sem_t            *semid;
ULONG             retval;
ULONG             len;
sem_t            *posix_sem;
va_list           vl;
UINT              value;
mode_t            mode;

    /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
        /* return POSIX error.  */
        posix_internal_error(444);
        
        /* return error.  */
        return (( sem_t * )SEM_FAILED);
    }

    /* Find length of the name. The actual length is not known yet.  */
    len = strlen(name);

    if(len > SEM_NAME_MAX)
    {
        /* Set appropriate errno.  */
        posix_errno = ENAMETOOLONG;
        posix_set_pthread_errno(ENAMETOOLONG);

        /* Return ERROR.  */
        return (( sem_t * ) SEM_FAILED);
    }

    /* Check if semaphore exists.  */
    if((semid = posix_find_sem(name)) != NULL)
    {
        if(semid->unlink_flag ==TX_TRUE )
        {
            /* Return error.  */
            posix_errno = EINVAL;
            posix_set_pthread_errno(EINVAL);
            return(( sem_t * )SEM_FAILED);
       }
    }

    /* Semaphore not found.  */
    if(!(semid))
    {
        /* Check if flag set is O_EXCL.  */
        if( oflag == O_EXCL )
        {

            /* Set error for sem does not exist and O_CREAT not set.  */
            posix_errno = ENOENT;
            posix_set_pthread_errno(ENOENT);

            /* Return the SEM_FAILED error.  */
            return(( sem_t * )SEM_FAILED);    
        }
        if( (oflag == O_CREAT) || ( (oflag & (O_CREAT|O_EXCL )) == (O_CREAT|O_EXCL) ) )
        {

            /* Get the variable arguments pointers.  */

            va_start( vl, oflag );

            mode = va_arg( vl, mode_t);
            mode = mode;  /* just to keep compiler happy */

            value = va_arg( vl, ULONG);

            if(value > SEM_VALUE_MAX)
            {
                /* Semaphore value large.  */
               posix_errno = EINVAL;
               posix_set_pthread_errno(EINVAL);

               /* Return ERROR  */
               return (( sem_t * )SEM_FAILED);

            }

            /* Disable interrupts.  */
            TX_DISABLE

            /* Get a new semaphore from the POSIX semaphore pool.  */
            TheSem = posix_get_new_sem();

            /* Make sure we actually got a semaphore.  */
            if (!TheSem)
            {
                /* Semaphore cannot be initialized due to resource constraint.  */
                posix_errno = ENOSPC;
                posix_set_pthread_errno(ENOSPC);

                /* Restore interrupts.  */
                TX_RESTORE

                /* return appropriate error code.  */
                return(( sem_t * )SEM_FAILED);
            }

            /* Set the semaphore name.  */
            posix_set_sem_name((sem_t *)TheSem, ( CHAR * ) name);

            /* Now actually create the ThreadX semaphore.  */
            posix_sem = (struct POSIX_SEMAPHORE_STRUCT *)TheSem;
            retval = tx_semaphore_create(TheSem, ( CHAR * ) posix_sem->sem_name , value);

            /* Make sure it worked.  */
            if (retval)
            {
                /* Return generic error.  */
                posix_internal_error(100);

                /* Restore interrupts.  */
                TX_RESTORE

                /* Return appropriate error code.  */
                return(( sem_t * )SEM_FAILED);
            }

            /* Add the calling thread to the thread list.  */
            posix_sem -> count += 1;

            /* Set initial count  */
            posix_sem->refCnt = value;

            /* Give the caller the semaphore ID.  */
            semid = (sem_t * )TheSem; 

            /* Restore interrupts.  */
            TX_RESTORE

            /* All done.  */
            return(semid);
        }

    }
    /* Semaphore found.  */
    if(semid)
    {

        /* Check if flags are O_CREAT|O_EXCL.  */
        if( (oflag == O_EXCL) || (oflag & (O_CREAT|O_EXCL )) == (O_CREAT|O_EXCL) )
        {
            /* Set appropriate errno.  */
            posix_errno = EEXIST;
            posix_set_pthread_errno(EEXIST);

            /* Return ERROR.  */
            return (( sem_t * ) SEM_FAILED);
        }

        /* Check if flag is only O_CREAT.  */
        if( (oflag == O_CREAT) || (oflag==0))
        {
            /* Disable interrupts.  */
            TX_DISABLE

            /* Add the calling thread to the thread list.  */
            semid -> count++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Return semaphore.  */
            return (( sem_t * ) semid);
        }
    }

    /* Semaphore value large.  */
    posix_errno = EINVAL;
    posix_set_pthread_errno(EINVAL);

    /* Return ERROR.  */
    return (( sem_t * ) SEM_FAILED);
}
