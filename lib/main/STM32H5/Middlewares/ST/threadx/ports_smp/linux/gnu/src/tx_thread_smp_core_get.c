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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_core_get                           SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the currently running core number and returns it.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Core ID                                                             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_self                          Get Linux thread ID           */
/*    _tx_linux_mutex_obtain                                              */
/*    _tx_linux_mutex_release                                             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX Source                                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_smp_core_get(void)
{

UINT        core;
UINT        i;
pthread_t   thread_id;


    /* Lock Linux mutex.  */
    _tx_linux_mutex_obtain(&_tx_linux_mutex);

    /* Default to core 0 for ISRs and initialization.  */
    core =  0;

    /* Pickup the currently executing thread ID. */
    thread_id =  pthread_self();

    /* Loop through mapping table to find the core running this thread ID.  */
    for (i = 0; i < TX_THREAD_SMP_MAX_CORES; i++)
    {

        /* Does this core match?  */
        if (_tx_linux_virtual_cores[i].tx_thread_smp_core_mapping_linux_thread_id == thread_id)
        {

            /* Yes, we have a match.  */
            core =  i;

            /* Get out of loop.  */
            break;
        }
    }

    /* Unlock linux mutex. */
    _tx_linux_mutex_release(&_tx_linux_mutex);

    /* Return the core ID.  */
    return(core);
}

