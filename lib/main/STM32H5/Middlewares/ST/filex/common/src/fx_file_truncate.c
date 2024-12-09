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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_truncate                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the file to the specified size, if smaller than  */
/*    the current file size.  If the new file size is less than the       */
/*    current file read/write position, the internal file pointers will   */
/*    also be modified.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    size                                  New size of the file in bytes */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_extended_truncate            Truncate the file size        */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            Added conditional to        */
/*                                            disable one line function,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT  _fx_file_truncate(FX_FILE *file_ptr, ULONG size)
{

    return(_fx_file_extended_truncate(file_ptr, (ULONG64)size));
}
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
