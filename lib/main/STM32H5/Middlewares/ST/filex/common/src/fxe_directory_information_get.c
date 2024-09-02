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
/**   Directory                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_directory.h"

FX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_directory_information_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the directory information        */
/*    get call.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Directory name pointer        */
/*    attributes                            Pointer to attributes         */
/*    size                                  Pointer to size               */
/*    year                                  Pointer to year               */
/*    month                                 Pointer to month              */
/*    day                                   Pointer to day                */
/*    hour                                  Pointer to hour               */
/*    minute                                Pointer to minute             */
/*    second                                Pointer to second             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_information_get         Actual directory information  */
/*                                            get service                 */
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
UINT  _fxe_directory_information_get(FX_MEDIA *media_ptr, CHAR *directory_name,
                                     UINT *attributes, ULONG *size,
                                     UINT *year, UINT *month, UINT *day,
                                     UINT *hour, UINT *minute, UINT *second)
{

UINT status;


    /* Check for a null media pointer or all null return parameter pointers.  */
    if ((media_ptr == FX_NULL) ||
        ((attributes == FX_NULL) && (size == FX_NULL) && (year == FX_NULL) && (month == FX_NULL) &&
         (day == FX_NULL) && (hour == FX_NULL) && (minute == FX_NULL) && (second == FX_NULL)))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Call actual directory information get service.  */
    status =  _fx_directory_information_get(media_ptr, directory_name, attributes, size,
                                            year, month, day, hour, minute, second);

    /* Directory information get is complete, return status.  */
    return(status);
}

