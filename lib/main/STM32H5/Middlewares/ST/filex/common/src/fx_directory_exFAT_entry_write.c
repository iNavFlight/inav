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

#ifdef FX_ENABLE_EXFAT
#include "fx_directory.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_exFAT_entry_write                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes the supplied directory entry to the specified  */
/*    logical sector and offset.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    entry_ptr                             Pointer to directory entry    */
/*    update_level                          Update level for entry write  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_exFAT_unicode_entry_write                             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    fx_directory_attributes_set                                         */
/*    fx_directory_create                                                 */
/*    fx_directory_delete                                                 */
/*    fx_directory_exFAT_unicode_entry_write                              */
/*    fx_directory_rename                                                 */
/*    fx_file_allocate                                                    */
/*    fx_file_attributes_set                                              */
/*    fx_file_best_effort_allocate                                        */
/*    fx_file_close                                                       */
/*    fx_file_create                                                      */
/*    fx_file_date_time_set                                               */
/*    fx_file_delete                                                      */
/*    fx_file_rename                                                      */
/*    fx_file_truncate_release                                            */
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
UINT  _fx_directory_exFAT_entry_write(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr, UCHAR update_level)
{

UINT status;


    /* Call the unicode director entry write function.  */
    status =  _fx_directory_exFAT_unicode_entry_write(media_ptr, entry_ptr, update_level, NULL, 0);

    /* Return completion status.  */
    return(status);
}

#endif

