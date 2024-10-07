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
/*    _tx_timer_smp_core_exclude_get                     PROTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the current exclusion list for the timer.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                             Pointer to the timer          */
/*    exclusion_map_ptr                     Destination for the current   */
/*                                            exclusion list              */
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
UINT  _tx_timer_smp_core_exclude_get(TX_TIMER *timer_ptr, ULONG *exclusion_map_ptr)
{

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

    /* Is the destination pointer NULL?  */
    else if (exclusion_map_ptr == TX_NULL)
    {

        /* Return pointer error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Save the current exclusion map in the destination.  */
        *exclusion_map_ptr =  (timer_ptr -> tx_timer_internal.tx_timer_internal_smp_cores_excluded) & ((ULONG) TX_THREAD_SMP_CORE_MASK);

        /* Set the status to success.  */
        status =  TX_SUCCESS;
    }

    /* Return success.  */
    return(status);
}

