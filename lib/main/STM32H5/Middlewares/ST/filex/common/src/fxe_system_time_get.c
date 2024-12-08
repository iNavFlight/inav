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
/*    _fxe_system_time_get                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks of errors in the get system time call.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hour                                  Pointer to hour               */
/*    minute                                Pointer to minute             */
/*    second                                Pointer to second             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_PTR_ERROR                          Invalid destination pointer   */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_system_time_get                   Actual get time service       */
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
UINT  _fxe_system_time_get(UINT *hour, UINT *minute, UINT *second)
{

UINT status;


    /* Check for an invalid destination pointer.  */
    if ((hour == FX_NULL) || (minute == FX_NULL) || (second == FX_NULL))
    {
        return(FX_PTR_ERROR);
    }

    /* Call actual get time service.  */
    status =  _fx_system_time_get(hour, minute, second);

    /* Return status.  */
    return(status);
}

