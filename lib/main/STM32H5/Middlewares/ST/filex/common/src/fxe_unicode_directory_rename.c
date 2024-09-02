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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"


FX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_unicode_directory_rename                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the Unicode directory rename     */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    old_unicode_name                      Pointer to old unicode name   */
/*    old_unicode_length                    Old unicode name length       */
/*    new_unicode_name                      Pointer to new unicode name   */
/*    new_unicode_length                    Old unicode name length       */
/*    new_short_name                        Pointer to new short name     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_directory_rename          Actual Unicode directory      */
/*                                            rename service              */
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
UINT  _fxe_unicode_directory_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                                    UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name)
{

UINT status, i;


    /* Check for a NULL media or name pointers.  */
    if ((media_ptr == FX_NULL) || (old_unicode_name == FX_NULL) || (old_unicode_length == 0) ||
        (new_unicode_name == FX_NULL) || (new_unicode_length == 0) || (new_short_name == FX_NULL))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Check unicode zero in old_unicode_name */
    for (i = 0; i < (old_unicode_length << 1); i += 2)
    {
        if ((old_unicode_name[i] == 0) && (old_unicode_name[i + 1] == 0))
        {
            return(FX_INVALID_NAME);
        }
    }

    /* Check unicode zero in new_unicode_name */
    for (i = 0; i < (new_unicode_length << 1); i += 2)
    {
        if ((new_unicode_name[i] == 0) && (new_unicode_name[i + 1] == 0))
        {
            return(FX_INVALID_NAME);
        }
    }

    /* Call actual Unicode directory rename service.  */
    status =  _fx_unicode_directory_rename(media_ptr, old_unicode_name, old_unicode_length, new_unicode_name, new_unicode_length, new_short_name);

    /* Return status to the caller.  */
    return(status);
}

