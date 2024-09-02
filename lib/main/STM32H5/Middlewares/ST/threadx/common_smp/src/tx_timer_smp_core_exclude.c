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
/**   Timer - High Level SMP Support                                      */
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
/*    _tx_timer_smp_core_exclude                         PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allows the application to exclude one or more cores   */
/*    from executing the specified timer.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                             Pointer to the timer          */
/*    exclusion_map                         Bit map of exclusion list,    */
/*                                            where bit 0 set means that  */
/*                                            this thread cannot run on   */
/*                                            core0, etc.                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _tx_timer_smp_core_exclude(TX_TIMER *timer_ptr, ULONG exclusion_map)
{

TX_INTERRUPT_SAVE_AREA

UINT    status;


    /* First, make sure the timer pointer is valid.  */
    if (timer_ptr == TX_NULL)
    {

        /* Return pointer error.  */
        status =  TX_TIMER_ERROR;
    }

    /* Check for valid ID.  */
    else if (timer_ptr -> tx_timer_id != TX_TIMER_ID)
    {

        /* Return pointer error.  */
        status =  TX_TIMER_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Now store in the core exclusion information.  */
        timer_ptr -> tx_timer_internal.tx_timer_internal_smp_cores_excluded =  (timer_ptr -> tx_timer_internal.tx_timer_internal_smp_cores_excluded & ~(((ULONG) TX_THREAD_SMP_CORE_MASK))) |
                                                                                   (exclusion_map & ((ULONG) TX_THREAD_SMP_CORE_MASK));

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success.  */
        status =  TX_SUCCESS;
    }

    /* Return success.  */
    return(status);
}

