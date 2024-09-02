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
/*    _fxe_file_open                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the file open call.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    file_ptr                              File control block pointer    */
/*    file_name                             Name pointer                  */
/*    open_type                             Type of open requested        */
/*    file_control_block_size               Size of FX_FILE structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_open                         Actual open file service      */
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
UINT  _fxe_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name, UINT open_type, UINT file_control_block_size)
{

UINT     status;
FX_FILE *current_file;
ULONG    open_count;


    /* Check for a null media or file pointer.  */
    if ((media_ptr == FX_NULL) || (media_ptr -> fx_media_id != FX_MEDIA_ID) || (file_ptr == FX_NULL) || (file_control_block_size != sizeof(FX_FILE)))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for an invalid open type.  */
    if ((open_type != FX_OPEN_FOR_READ) && (open_type != FX_OPEN_FOR_READ_FAST) && (open_type != FX_OPEN_FOR_WRITE))
    {
        return(FX_ACCESS_ERROR);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Get protection.  */
    FX_PROTECT

    /* Check for a duplicate file open.  */

    /* Loop to search the list for the same file handle.  */
    current_file =  media_ptr -> fx_media_opened_file_list;
    open_count =    media_ptr -> fx_media_opened_file_count;

    while (open_count--)
    {

        /* See if a match exists.  */
        if (file_ptr == current_file)
        {

            /* Release protection.  */
            FX_UNPROTECT

            /* Return error.  */
            return(FX_PTR_ERROR);
        }

        /* Move to the next opened file.  */
        current_file =  current_file -> fx_file_opened_next;
    }

    /* Release protection.  */
    FX_UNPROTECT

    /* Call actual file open service.  */
    status =  _fx_file_open(media_ptr, file_ptr, file_name, open_type);

    /* Open is complete, return status.  */
    return(status);
}

