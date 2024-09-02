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
/*    _fx_file_relative_seek                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function positions the internal file pointers to the specified */
/*    byte relative offset such that the next read or write operation is  */
/*    performed there.                                                    */
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
/*    _fx_file_extended_relative_seek       Seek to specified position    */
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
UINT  _fx_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from)
{

    return(_fx_file_extended_relative_seek(file_ptr, (ULONG64)byte_offset, seek_from));
}
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */

