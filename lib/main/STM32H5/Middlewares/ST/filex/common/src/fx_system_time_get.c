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
/*    _fx_system_time_get                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the current contents of the system time in    */
/*    the fields specified by the caller.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hour                                  Pointer to hour               */
/*    minute                                Pointer to minute             */
/*    second                                Pointer to second             */
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
UINT  _fx_system_time_get(UINT *hour, UINT *minute, UINT *second)
{

UINT time;


    /* Get a copy of the current system time.  */
    time =  _fx_system_time;

    /* Check to see if the hour is required.  */
    if (hour)
    {

        /* Pickup the hour.  */
        *hour =  (time >> FX_HOUR_SHIFT) & FX_HOUR_MASK;
    }

    /* Check to see if the minute is required.  */
    if (minute)
    {

        /* Pickup the minute.  */
        *minute =  (time >> FX_MINUTE_SHIFT) & FX_MINUTE_MASK;
    }

    /* Check to see if the second is required.  */
    if (second)
    {

        /* Pickup the second.  */
        *second =  (time & FX_SECOND_MASK) * 2;
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    if (hour && minute && second)
    {
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_SYSTEM_TIME_GET, *hour, *minute, *second, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)
    }

    /* Return successful status.  */
    return(FX_SUCCESS);
}

