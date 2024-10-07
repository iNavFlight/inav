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
/*    _fx_system_timer_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is FileX system timer function.  It is called at the  */
/*    rate specified by FX_UPDATE_RATE_IN_SECONDS and is responsible for  */
/*    maintaining both the system date and time.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                                    Not used                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Initialization                                          */
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
VOID    _fx_system_timer_entry(ULONG id)
{

UINT second;
UINT minute;
UINT hour;
UINT day;
UINT month;
UINT year;


    /* Determine if the ID is valid.  */
    if (id == FX_TIMER_ID)
    {

        /* Break the current date time into separate fields for easier work!  */
        second =  (_fx_system_time & FX_SECOND_MASK) * 2;
        minute =  (_fx_system_time >> FX_MINUTE_SHIFT) & FX_MINUTE_MASK;
        hour =    (_fx_system_time >> FX_HOUR_SHIFT) & FX_HOUR_MASK;
        day =     _fx_system_date & FX_DAY_MASK;
        month =   (_fx_system_date >> FX_MONTH_SHIFT) & FX_MONTH_MASK;
        year =    ((_fx_system_date >> FX_YEAR_SHIFT) & FX_YEAR_MASK) + FX_BASE_YEAR;

        /* Now apply the "second" update.  */
        second =  second + FX_UPDATE_RATE_IN_SECONDS;

        /* Determine if we need to adjust the minute field.  */
        if (second > FX_MAXIMUM_SECOND)
        {

            /* Yes, we need to adjust the minute field.  */
            minute =  minute + second / 60;
            second =  second % 60;

            /* Determine if we need to adjust the hour field.  */
            if (minute > FX_MAXIMUM_MINUTE)
            {

                /* Yes, we need to adjust the hour field.  */
                hour =    hour + minute / 60;
                minute =  minute % 60;

                /* Determine if we need to adjust the day field.  */
                if (hour > FX_MAXIMUM_HOUR)
                {

                    /* Yes, we need to adjust the day field.  */
                    hour =  0;
                    day++;

                    /* Determine if we need to adjust the month field.  */
                    switch (month)
                    {

                    case 1:                 /* January  */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 2:                 /* February  */
                    {

                        /* Check for leap year.  We don't need to check for leap
                           century her (century years divisible by 400) since 2000
                           is and this FAT format only supports years to 2107. */
                        if ((year % 4) == 0)
                        {

                            /* Leap year in February... check for 29 days
                               instead of 28.  */
                            if (day > 29)
                            {

                                /* Adjust the month.  */
                                day =  1;
                                month++;
                            }
                        }
                        else
                        {

                            if (day > 28)
                            {

                                /* Adjust the month.  */
                                day = 1;
                                month++;
                            }
                        }
                        break;
                    }

                    case 3:                 /* March  */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 4:                 /* April  */
                    {

                        /* Check for end of the month.  */
                        if (day > 30)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 5:                 /* May  */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 6:                 /* June */
                    {

                        /* Check for end of the month.  */
                        if (day > 30)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 7:                 /* July */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 8:                 /* August */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 9:                 /* September */
                    {

                        /* Check for end of the month.  */
                        if (day > 30)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 10:                /* October */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 11:                /* November */
                    {

                        /* Check for end of the month.  */
                        if (day > 30)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month++;
                        }
                        break;
                    }

                    case 12:                /* December */
                    {

                        /* Check for end of the month.  */
                        if (day > 31)
                        {

                            /* Move to next month.  */
                            day = 1;
                            month = 1;

                            /* Also move to next year.  */
                            year++;

                            /* Check for a year that exceeds the representation
                               in this format.  */
                            if (year > FX_MAXIMUM_YEAR)
                            {
                                return;
                            }
                        }
                        break;
                    }

                    default:                /* Invalid month!  */

                        return;             /* Skip updating date/time!  */
                    }
                }
            }
        }

        /* Now apply the new setting to the internal representation.  */

        /* Set the system date.  */
        _fx_system_date =  ((year - FX_BASE_YEAR) << FX_YEAR_SHIFT) |
                            (month << FX_MONTH_SHIFT) | day;

        /* Set the new system time.  */
        _fx_system_time  =  (hour << FX_HOUR_SHIFT) |
                            (minute << FX_MINUTE_SHIFT) | (second / 2);
    }
}

