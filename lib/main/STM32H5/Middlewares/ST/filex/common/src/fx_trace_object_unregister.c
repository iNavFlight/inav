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
/** FileX Component                                                       */
/**                                                                       */
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef TX_SOURCE_CODE
#define TX_SOURCE_CODE
#endif

#ifndef FX_SOURCE_CODE
#define FX_SOURCE_CODE
#endif

#include "fx_api.h"

#ifdef TX_ENABLE_EVENT_TRACE


/* Include necessary system files.  */

#include "tx_trace.h"



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_trace_object_unregister                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unregisters a FileX object in the trace registry.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_ptr                            Address of system object      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_trace_object_unregister           Actual unregister function    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX delete functions                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _fx_trace_object_unregister(VOID *object_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Call actual object unregister function.  */
    _tx_trace_object_unregister(object_ptr);

    /* Restore interrupts.  */
    TX_RESTORE
}
#endif

