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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_mutex.h"
#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_performance_system_info_get               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves system mutex performance information.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    puts                              Destination for total number of   */
/*                                        mutex puts                      */
/*    gets                              Destination for total number of   */
/*                                        mutex gets                      */
/*    suspensions                       Destination for total number of   */
/*                                        mutex suspensions               */
/*    timeouts                          Destination for total number of   */
/*                                        mutex timeouts                  */
/*    inversions                        Destination for total number of   */
/*                                        mutex priority inversions       */
/*    inheritances                      Destination for total number of   */
/*                                        mutex priority inheritances     */
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
UINT  _tx_mutex_performance_system_info_get(ULONG *puts, ULONG *gets, ULONG *suspensions,
                                ULONG *timeouts, ULONG *inversions, ULONG *inheritances)
{

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_PERFORMANCE_SYSTEM_INFO_GET, 0, 0, 0, 0, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the total number of mutex puts.  */
    if (puts != TX_NULL)
    {

        *puts =  _tx_mutex_performance_put_count;
    }

    /* Retrieve the total number of mutex gets.  */
    if (gets != TX_NULL)
    {

        *gets =  _tx_mutex_performance_get_count;
    }

    /* Retrieve the total number of mutex suspensions.  */
    if (suspensions != TX_NULL)
    {

        *suspensions =  _tx_mutex_performance_suspension_count;
    }

    /* Retrieve the total number of mutex timeouts.  */
    if (timeouts != TX_NULL)
    {

        *timeouts =  _tx_mutex_performance_timeout_count;
    }

    /* Retrieve the total number of mutex priority inversions.  */
    if (inversions != TX_NULL)
    {

        *inversions =  _tx_mutex_performance_priority_inversion_count;
    }

    /* Retrieve the total number of mutex priority inheritances.  */
    if (inheritances != TX_NULL)
    {

        *inheritances =  _tx_mutex_performance__priority_inheritance_count;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);

#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (puts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (gets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (inversions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (inheritances != TX_NULL)
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

