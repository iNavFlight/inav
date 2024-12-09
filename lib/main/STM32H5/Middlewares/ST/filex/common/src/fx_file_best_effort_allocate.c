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
/*    _fx_file_best_effort_allocate                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to allocate the number of consecutive        */
/*    clusters required to satisfy the user's request.  If there are not  */
/*    enough clusters, the largest set of clusters are allocated and      */
/*    linked to the file.  If there are no free clusters, an error        */
/*    code is returned to the caller.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    size                                  Number of bytes to allocate   */
/*    actual_size_allocated                 Number of bytes allocated     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_file_extended_best_effort_allocate                              */
/*                                          Allocate the largest set of   */
/*                                            clusters                    */
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
UINT  _fx_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated)
{

UINT    status;
ULONG64 temp_actual_size_allocated;

    /* Call actual best effort file allocate service.  */
    status = _fx_file_extended_best_effort_allocate(file_ptr, (ULONG64)size, &temp_actual_size_allocated);

    /* Check status.  */
    if (status == FX_SUCCESS)
    {
        *actual_size_allocated = (ULONG)temp_actual_size_allocated;
    }

    /* Return status to the caller.  */
    return(status);
}

