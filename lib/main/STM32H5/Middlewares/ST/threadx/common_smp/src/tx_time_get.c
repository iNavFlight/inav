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
#include "tx_trace.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_time_get                                        PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the internal, free-running, system clock    */
/*    and returns it to the caller.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    _tx_timer_system_clock            Returns the system clock value    */
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
/*  12-31-2020     Andres Mlinar            Modified comment(s),          */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
ULONG  _tx_time_get(VOID)
{

TX_INTERRUPT_SAVE_AREA

#ifdef TX_ENABLE_EVENT_TRACE
ULONG   another_temp_time =  ((ULONG) 0);
#endif
ULONG   temp_time;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the system clock time.  */
    temp_time =  _tx_timer_system_clock;

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIME_GET, TX_ULONG_TO_POINTER_CONVERT(temp_time), TX_POINTER_TO_ULONG_CONVERT(&another_temp_time), 0, 0, TX_TRACE_TIME_EVENTS)

    /* Log this kernel call.  */
    TX_EL_TIME_GET_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return the time.  */
    return(temp_time);
}

