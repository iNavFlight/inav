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
/*    _fx_file_allocate                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to allocate the number of consecutive        */
/*    clusters required to satisfy the user's request.  If there are      */
/*    enough clusters, the clusters are allocated and linked to the file. */
/*    Otherwise, if there are not enough consecutive clusters, an error   */
/*    code is returned to the caller.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    size                                  Number of bytes to allocate   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_extended_allocate            Allocate the clusters         */
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
UINT  _fx_file_allocate(FX_FILE *file_ptr, ULONG size)
{

    return(_fx_file_extended_allocate(file_ptr, (ULONG64)size));
}
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
