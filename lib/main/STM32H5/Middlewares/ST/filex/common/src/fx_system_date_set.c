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
/**   System                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_system_date_set                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the system date to the date specified by the     */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    year                                  Year (1980-2107)              */
/*    month                                 Month (1-12)                  */
/*    day                                   Day (1-28/29/30/31)           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_SUCCESS                                                          */
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
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_system_date_set(UINT year, UINT month, UINT day)
{


    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_SYSTEM_DATE_SET, year, month, day, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Set the system date.  */
    _fx_system_date =  ((year - FX_BASE_YEAR) << FX_YEAR_SHIFT) |
        (month << FX_MONTH_SHIFT) | day;

    /* Return successful status.  */
    return(FX_SUCCESS);
}

