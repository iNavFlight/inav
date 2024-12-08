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
/**   File                                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_file.h"


FX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_file_date_time_set                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the file date/time set service.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_name                             File name pointer             */
/*    year                                  Year                          */
/*    month                                 Month                         */
/*    day                                   Day                           */
/*    hour                                  Hour                          */
/*    minute                                Minute                        */
/*    second                                Second                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_date_time_set                Actual file date/time set     */
/*                                            service                     */
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
UINT  _fxe_file_date_time_set(FX_MEDIA *media_ptr, CHAR *file_name,
                              UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second)
{

UINT status;


    /* Check for a NULL media or file name pointer.  */
    if ((media_ptr == FX_NULL) || (file_name == FX_NULL))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for invalid year.  */
    if ((year < FX_BASE_YEAR) || (year > FX_MAXIMUM_YEAR))
    {
        return(FX_INVALID_YEAR);
    }

    /* Check for invalid day.  */
    if (day < 1)
    {
        return(FX_INVALID_DAY);
    }

    /* Check for invalid day.  */
    switch (month)
    {

    case 1:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 2:
    {

        /* Check for leap year.  */
        if ((year % 4) == 0)
        {

            /* Leap year, February has 29 days.  */
            if (day > 29)
            {
                return(FX_INVALID_DAY);
            }
        }
        else
        {

            /* Otherwise, non-leap year.  February has
               28 days.  */
            if (day > 28)
            {
                return(FX_INVALID_DAY);
            }
        }
        break;
    }

    case 3:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 4:
    {

        /* Check for 30 days.  */
        if (day > 30)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 5:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 6:
    {

        /* Check for 30 days.  */
        if (day > 30)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 7:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 8:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 9:
    {

        /* Check for 30 days.  */
        if (day > 30)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 10:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 11:
    {

        /* Check for 30 days.  */
        if (day > 30)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }

    case 12:
    {

        /* Check for 31 days.  */
        if (day > 31)
        {
            return(FX_INVALID_DAY);
        }
        break;
    }
    
    default:
    
        /* Invalid month.  */
        return(FX_INVALID_MONTH);
    }

    /* Check for invalid hour.  */
    if (hour > FX_MAXIMUM_HOUR)
    {
        return(FX_INVALID_HOUR);
    }

    /* Check for invalid minute.  */
    if (minute > FX_MAXIMUM_MINUTE)
    {
        return(FX_INVALID_MINUTE);
    }

    /* Check for invalid second.  */
    if (second > FX_MAXIMUM_SECOND)
    {
        return(FX_INVALID_SECOND);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Call actual file date/time set service.  */
    status =  _fx_file_date_time_set(media_ptr, file_name,
                                     year, month, day,
                                     hour, minute, second);

    /* Return status to the caller.  */
    return(status);
}

