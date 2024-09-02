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
/*    _fx_system_date_get                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the current contents of the system date in    */
/*    the fields specified by the caller.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    year                                  Pointer to year               */
/*    month                                 Pointer to month              */
/*    day                                   Pointer to day                */
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
/*  06-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            checked null pointer,       */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_system_date_get(UINT *year, UINT *month, UINT *day)
{

UINT date;


    /* Get a copy of the current date.  */
    date =  _fx_system_date;

    /* Check to see if the year is required.  */
    if (year)
    {

        /* Pickup the year.  */
        *year =  ((date >> FX_YEAR_SHIFT) & FX_YEAR_MASK) + FX_BASE_YEAR;
    }

    /* Check to see if the month is required.  */
    if (month)
    {

        /* Pickup the month.  */
        *month =  (date >> FX_MONTH_SHIFT) & FX_MONTH_MASK;
    }

    /* Check to see if the day is required.  */
    if (day)
    {

        /* Pickup the day.  */
        *day =  date & FX_DAY_MASK;
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    if (year && month && day)
    {
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_SYSTEM_DATE_GET, *year, *month, *day, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)
    }

    /* Return successful status.  */
    return(FX_SUCCESS);
}

