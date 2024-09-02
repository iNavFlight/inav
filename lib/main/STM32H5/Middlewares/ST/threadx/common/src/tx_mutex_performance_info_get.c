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
/*    _tx_mutex_performance_info_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    mutex.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*    puts                              Destination for the number of     */
/*                                        puts on to this mutex           */
/*    gets                              Destination for the number of     */
/*                                        gets on this mutex              */
/*    suspensions                       Destination for the number of     */
/*                                        suspensions on this mutex       */
/*    timeouts                          Destination for number of timeouts*/
/*                                        on this mutex                   */
/*    inversions                        Destination for number of priority*/
/*                                        inversions on this mutex        */
/*    inheritances                      Destination for number of priority*/
/*                                        inheritances on this mutex      */
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
UINT  _tx_mutex_performance_info_get(TX_MUTEX *mutex_ptr, ULONG *puts, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts, ULONG *inversions, ULONG *inheritances)
{

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Determine if this is a legal request.  */
    if (mutex_ptr == TX_NULL)
    {

        /* Mutex pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the mutex ID is invalid.  */
    else if (mutex_ptr -> tx_mutex_id != TX_MUTEX_ID)
    {

        /* Mutex pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_PERFORMANCE_INFO_GET, mutex_ptr, 0, 0, 0, TX_TRACE_MUTEX_EVENTS)

        /* Log this kernel call.  */
        TX_EL_MUTEX_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve the number of puts on this mutex.  */
        if (puts != TX_NULL)
        {

            *puts =  mutex_ptr -> tx_mutex_performance_put_count;
        }

        /* Retrieve the number of gets on this mutex.  */
        if (gets != TX_NULL)
        {

            *gets =  mutex_ptr -> tx_mutex_performance_get_count;
        }

        /* Retrieve the number of suspensions on this mutex.  */
        if (suspensions != TX_NULL)
        {

            *suspensions =  mutex_ptr -> tx_mutex_performance_suspension_count;
        }

        /* Retrieve the number of timeouts on this mutex.  */
        if (timeouts != TX_NULL)
        {

            *timeouts =  mutex_ptr -> tx_mutex_performance_timeout_count;
        }

        /* Retrieve the number of priority inversions on this mutex.  */
        if (inversions != TX_NULL)
        {

            *inversions =  mutex_ptr -> tx_mutex_performance_priority_inversion_count;
        }

        /* Retrieve the number of priority inheritances on this mutex.  */
        if (inheritances != TX_NULL)
        {

            *inheritances =  mutex_ptr -> tx_mutex_performance__priority_inheritance_count;
        }

        /* Restore interrupts.  */
        TX_RESTORE
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (mutex_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (puts != TX_NULL)
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
#endif

    /* Return completion status.  */
    return(status);
}

