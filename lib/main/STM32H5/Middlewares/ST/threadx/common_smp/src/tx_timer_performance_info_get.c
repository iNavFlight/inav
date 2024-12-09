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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_performance_info_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    timer.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*    activates                         Destination for the number of     */
/*                                        activations of this timer       */
/*    reactivates                       Destination for the number of     */
/*                                        reactivations of this timer     */
/*    deactivates                       Destination for the number of     */
/*                                        deactivations of this timer     */
/*    expirations                       Destination for the number of     */
/*                                        expirations of this timer       */
/*    expiration_adjusts                Destination for the number of     */
/*                                        expiration adjustments of this  */
/*                                        timer                           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
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
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_timer_performance_info_get(TX_TIMER *timer_ptr, ULONG *activates, ULONG *reactivates,
                    ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts)
{

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (timer_ptr == TX_NULL)
    {

        /* Timer pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the timer ID is invalid.  */
    else if (timer_ptr -> tx_timer_id != TX_TIMER_ID)
    {

        /* Timer pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_PERFORMANCE_INFO_GET, timer_ptr, 0, 0, 0, TX_TRACE_TIMER_EVENTS)

        /* Log this kernel call.  */
        TX_EL_TIMER_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve the number of activations of this timer.  */
        if (activates != TX_NULL)
        {

            *activates =  timer_ptr -> tx_timer_performance_activate_count;
        }

        /* Retrieve the number of reactivations of this timer.  */
        if (reactivates != TX_NULL)
        {

            *reactivates =  timer_ptr -> tx_timer_performance_reactivate_count;
        }

        /* Retrieve the number of deactivations of this timer.  */
        if (deactivates != TX_NULL)
        {

            *deactivates =  timer_ptr -> tx_timer_performance_deactivate_count;
        }

        /* Retrieve the number of expirations of this timer.  */
        if (expirations != TX_NULL)
        {

            *expirations =  timer_ptr -> tx_timer_performance_expiration_count;
        }

        /* Retrieve the number of expiration adjustments of this timer.  */
        if (expiration_adjusts != TX_NULL)
        {

            *expiration_adjusts =  timer_ptr -> tx_timer_performance__expiration_adjust_count;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return successful completion.  */
        status =  TX_SUCCESS;
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (timer_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (activates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (reactivates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (deactivates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (expirations != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (expiration_adjusts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
#endif

    /* Return completion status.  */
    return(status);
}

