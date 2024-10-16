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
/**   Initialization                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_timer.h"
#include "tx_thread.h"



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_high_level_initialize               PORTABLE SMP     */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the ThreadX SMP data structures and       */
/*    CPU registers.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter           ThreadX entry                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*  12-31-2020     William E. Lamie         Modified comments, added      */
/*                                            cast to address a MISRA     */
/*                                            compliant issue,            */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
void  _tx_thread_smp_high_level_initialize(void)
{

    /* Clear the system error flag.  */
    _tx_thread_smp_system_error =  TX_FALSE;

    /* Ensure that the system state variable is set to indicate
       initialization is in progress.  Note that this variable is
       later used to represent interrupt nesting.  */
    _tx_thread_smp_current_state_set(TX_INITIALIZE_IN_PROGRESS);

    /* Clear the thread protection.  */
    TX_MEMSET(&_tx_thread_smp_protection, 0, sizeof(TX_THREAD_SMP_PROTECT));

    /* Set the field of the protection to all ones to indicate it is invalid.  */
    _tx_thread_smp_protection.tx_thread_smp_protect_core =  ((ULONG) 0xFFFFFFFFUL);

    /* Clear the thread schedule list.  */
    TX_MEMSET(&_tx_thread_smp_schedule_list[0], 0, sizeof(_tx_thread_smp_schedule_list));

    /* Initialize core list.  */
    TX_MEMSET(&_tx_thread_smp_protect_wait_list[0], 0xff, sizeof(_tx_thread_smp_protect_wait_list));

    /* Set the wait list size so we can access it from assembly functions.  */
    _tx_thread_smp_protect_wait_list_size =  ((ULONG) TX_THREAD_SMP_PROTECT_WAIT_LIST_SIZE);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    /* Call low-level SMP initialize.  */
    _tx_thread_smp_low_level_initialize(((UINT) TX_THREAD_SMP_MAX_CORES));
#else

    /* Determine if the dynamic maximum number of cores is 0. If so, default it
       to the compile-time maximum.  */
    if (_tx_thread_smp_max_cores == 0)
    {

        /* Default to the compile-time maximum.  */
        _tx_thread_smp_max_cores =  TX_THREAD_SMP_MAX_CORES;
    }

    /* Call low-level SMP initialize.  */
    _tx_thread_smp_low_level_initialize(_tx_thread_smp_max_cores);
#endif
}
