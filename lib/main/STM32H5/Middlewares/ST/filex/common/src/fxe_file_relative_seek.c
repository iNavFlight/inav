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
/*    _fxe_file_relative_seek                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the file relative seek call.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    byte_offset                           Byte offset of the seek       */
/*    seek_from                             Direction for relative seek,  */
/*                                          legal values are:             */
/*                                                                        */
/*                                              FX_SEEK_BEGIN             */
/*                                              FX_SEEK_END               */
/*                                              FX_SEEK_FORWARD           */
/*                                              FX_SEEK_BACK              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_relative_seek                Actual relative file seek     */
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
UINT  _fxe_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from)
{

UINT status;


    /* Check for a null file pointer.  */
    if (file_ptr == FX_NULL)
    {
        return(FX_PTR_ERROR);
    }

    /* Check for valid seek from option.  */
    if ((seek_from != FX_SEEK_BEGIN) && (seek_from != FX_SEEK_END) &&
        (seek_from != FX_SEEK_FORWARD) && (seek_from != FX_SEEK_BACK))
    {
        return(FX_INVALID_OPTION);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Call actual file relative seek service.  */
    status =  _fx_file_relative_seek(file_ptr, byte_offset, seek_from);

    /* Seek is complete, return status.  */
    return(status);
}

