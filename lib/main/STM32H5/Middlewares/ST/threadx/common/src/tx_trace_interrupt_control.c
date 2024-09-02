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
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_trace_interrupt_control                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function provides a shell for the tx_interrupt_control         */
/*    function so that a trace event can be logged for its use.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    new_posture                           New interrupt posture         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Previous Interrupt Posture                                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_interrupt_control          Interrupt control service     */
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
UINT  _tx_trace_interrupt_control(UINT new_posture)
{

#ifdef TX_ENABLE_EVENT_TRACE

TX_INTERRUPT_SAVE_AREA
UINT    saved_posture;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_INTERRUPT_CONTROL, TX_ULONG_TO_POINTER_CONVERT(new_posture), TX_POINTER_TO_ULONG_CONVERT(&saved_posture), 0, 0, TX_TRACE_INTERRUPT_CONTROL_EVENT)

    /* Restore interrupts.  */
    TX_RESTORE

    /* Perform the interrupt service.  */
    saved_posture =  _tx_thread_interrupt_control(new_posture);

    /* Return saved posture.  */
    return(saved_posture);
#else

UINT    saved_posture;

    /* Perform the interrupt service.  */
    saved_posture =  _tx_thread_interrupt_control(new_posture);

    /* Return saved posture.  */
    return(saved_posture);
#endif
}

