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
/*    _fxe_system_date_set                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the set system date call.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    year                                  Year (1980-2107)              */
/*    month                                 Month (1-12)                  */
/*    day                                   Day (1-28/29/30/31)           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_INVALID_YEAR                       Supplied year is invalid      */
/*    FX_INVALID_MONTH                      Supplied month is invalid     */
/*    FX_INVALID_DAY                        Supplied day is invalid       */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_system_date_set                   Actual system date set call   */
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
UINT  _fxe_system_date_set(UINT year, UINT month, UINT day)
{

UINT  status;


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

    /* Call actual system date set service.  */
    status =  _fx_system_date_set(year, month, day);

    /* Return status.  */
    return(status);
}

