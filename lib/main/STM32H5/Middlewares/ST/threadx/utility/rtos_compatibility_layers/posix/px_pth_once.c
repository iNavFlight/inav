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
/*    pthread_once                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    The pthread_once function shall call the init_routine with no       */
/*    arguments. Subsequent calls of pthread_once() with the same         */
/*    once_control shall not call the init_routine. On return from        */
/*    pthread_once(), init_routine shall have completed. The              */
/*    once_control parameter shall determine whether the associated       */
/*    initialization routine has been called.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*    once_control                                                        */
/*    init_routine                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*    zero                        If successful                           */
/*    error number                If fails                                */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*    tx_thread_preemption_change                                         */
/*    tx_event_flags_create                                               */
/*    tx_event_flags_set                                                  */
/*    tx_event_flags_get                                                  */
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
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
INT pthread_once (pthread_once_t * once_control, VOID (*init_routine) (VOID))
{

    INT result;
    UINT old_treshold, temp;

    if (once_control == NULL || init_routine == NULL)
    {
        result = EINVAL;
    }
    else
    {
        if ( once_control->state==PTH_ONCE_DONE) 
        {
            result = 0;
        }

        /* suspend preemption */
        tx_thread_preemption_change( tx_thread_identify(), 0, &old_treshold);

        if (once_control->state==PTH_ONCE_INIT)
        {
            once_control->state=PTH_ONCE_STARTED;
            tx_event_flags_create(&(once_control->event),"once flags");

            /* enable preemption */
            tx_thread_preemption_change( tx_thread_identify(),old_treshold, &temp);
            (*init_routine)();

            tx_event_flags_set(&(once_control->event), PTH_ONCE_INIT, TX_AND);

            once_control->state=PTH_ONCE_DONE;
        }

        /* enable preemption */   
        tx_thread_preemption_change( tx_thread_identify(),old_treshold, &temp);

        if (once_control->state==PTH_ONCE_STARTED)
        {
            tx_event_flags_get(&(once_control->event), PTH_ONCE_INIT, TX_AND, &(once_control->flags) ,TX_WAIT_FOREVER);
        }
    }

    /* note: this routine will not handle the case where the init routine is a cancellation point */
    return (result);
}
