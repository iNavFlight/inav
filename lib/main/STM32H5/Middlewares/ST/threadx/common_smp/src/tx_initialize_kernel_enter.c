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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"


/* Define any port-specific scheduling data structures.  */

TX_PORT_SPECIFIC_DATA


#ifdef TX_SAFETY_CRITICAL
TX_SAFETY_CRITICAL_EXCEPTION_HANDLER
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_kernel_enter                        PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the first ThreadX function called during           */
/*    initialization.  It is called from the application's "main()"       */
/*    function.  It is important to note that this routine never          */
/*    returns.  The processing of this function is relatively simple:     */
/*    it calls several ThreadX initialization functions (if needed),      */
/*    calls the application define function, and then invokes the         */
/*    scheduler.                                                          */
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
/*    _tx_thread_smp_high_level_initialize  SMP initialization            */
/*    _tx_thread_smp_current_state_set  Set system state for all cores    */
/*    _tx_initialize_low_level          Low-level initialization          */
/*    _tx_initialize_high_level         High-level initialization         */
/*    tx_application_define             Application define function       */
/*    _tx_thread_scheduler              ThreadX scheduling loop           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    main                              Application main program          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_initialize_kernel_enter(VOID)
{

ULONG   other_core_status, i;


    /* Determine if the compiler has pre-initialized ThreadX.  */
    if (_tx_thread_system_state[0] != TX_INITIALIZE_ALMOST_DONE)
    {

        /* No, the initialization still needs to take place.  */

        /* Ensure that the system state variable is set to indicate
           initialization is in progress.  Note that this variable is
           later used to represent interrupt nesting.  */
        _tx_thread_smp_current_state_set(TX_INITIALIZE_IN_PROGRESS);

        /* Call any port specific preprocessing.  */
        TX_PORT_SPECIFIC_PRE_INITIALIZATION

        /* Invoke the low-level initialization to handle all processor specific
           initialization issues.  */
        _tx_initialize_low_level();

        /* Call the high-level SMP  Initialization.  */
        _tx_thread_smp_high_level_initialize();

        /* Invoke the high-level initialization to exercise all of the
           ThreadX components and the application's initialization
           function.  */
        _tx_initialize_high_level();

        /* Call any port specific post-processing.  */
        TX_PORT_SPECIFIC_POST_INITIALIZATION
    }

    /* Optional processing extension.  */
    TX_INITIALIZE_KERNEL_ENTER_EXTENSION

    /* Ensure that the system state variable is set to indicate
       initialization is in progress.  Note that this variable is
       later used to represent interrupt nesting.  */
    _tx_thread_system_state[0] =  TX_INITIALIZE_IN_PROGRESS;

    /* Call the application provided initialization function.  Pass the
       first available memory address to it.  */
    tx_application_define(_tx_initialize_unused_memory);

    /* Call any port specific pre-scheduler processing.  */
    TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION

    /* Now wait for all cores to become ready for scheduling.  */
    do
    {

        /* Release the other cores from initialization.  */
        _tx_thread_smp_release_cores_flag =  TX_TRUE;

        /* Add all the status together...  Other cores must clear their system
           state before they they are released.  */
        other_core_status =  ((ULONG) 0);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
        for (i = ((ULONG) 1); i < ((ULONG) TX_THREAD_SMP_MAX_CORES); i++)
#else
        for (i = ((ULONG) 1); i < _tx_thread_smp_max_cores; i++)
#endif
        {


            /* Call port-specific memory synchronization primitive.  */
            TX_PORT_SPECIFIC_MEMORY_SYNCHRONIZATION

            /* Add the states of each subsequent core.  */
            other_core_status =  other_core_status + _tx_thread_system_state[i];
        }

    } while (other_core_status != ((ULONG) 0));

    /* Set the system state in preparation for entering the thread
       scheduler.  */
    _tx_thread_system_state[0] =  TX_INITIALIZE_IS_FINISHED;

    /* Call port-specific memory synchronization primitive.  */
    TX_PORT_SPECIFIC_MEMORY_SYNCHRONIZATION

    /* Enter the scheduling loop to start executing threads!  */
    _tx_thread_schedule();

#ifdef TX_SAFETY_CRITICAL

    /* If we ever get here, raise safety critical exception.  */
    TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);
#endif
}

