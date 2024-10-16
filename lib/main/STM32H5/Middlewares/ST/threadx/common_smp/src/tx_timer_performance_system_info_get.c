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
/*    _tx_timer_performance_system_info_get               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves timer performance information.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    activates                         Destination for total number of   */
/*                                        activations                     */
/*    reactivates                       Destination for total number of   */
/*                                        reactivations                   */
/*    deactivates                       Destination for total number of   */
/*                                        deactivations                   */
/*    expirations                       Destination for total number of   */
/*                                        expirations                     */
/*    expiration_adjusts                Destination for total number of   */
/*                                        expiration adjustments          */
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
UINT  _tx_timer_performance_system_info_get(ULONG *activates, ULONG *reactivates,
                    ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts)
{

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_PERFORMANCE_SYSTEM_INFO_GET, 0, 0, 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Log this kernel call.  */
    TX_EL_TIMER_PERFORMANCE_SYSTEM_INFO_GET_INSERT

    /* Retrieve the total number of timer activations.  */
    if (activates != TX_NULL)
    {

        *activates =  _tx_timer_performance_activate_count;
    }

    /* Retrieve the total number of timer reactivations.  */
    if (reactivates != TX_NULL)
    {

        *reactivates =  _tx_timer_performance_reactivate_count;
    }

    /* Retrieve the total number of timer deactivations.  */
    if (deactivates != TX_NULL)
    {

        *deactivates =  _tx_timer_performance_deactivate_count;
    }

    /* Retrieve the total number of timer expirations.  */
    if (expirations != TX_NULL)
    {

        *expirations =  _tx_timer_performance_expiration_count;
    }

    /* Retrieve the total number of timer expiration adjustments.  */
    if (expiration_adjusts != TX_NULL)
    {

        *expiration_adjusts =  _tx_timer_performance__expiration_adjust_count;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);

#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (activates != TX_NULL)
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

    /* Return completion status.  */
    return(status);
#endif
}

