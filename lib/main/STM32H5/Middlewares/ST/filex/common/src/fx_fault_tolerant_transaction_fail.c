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
/**   Fault Tolerant                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOUCE_CODE

#include "fx_api.h"
#include "fx_fault_tolerant.h"

#if defined(FX_ENABLE_FAULT_TOLERANT) && defined(FX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION)
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_transaction_fail                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up resources created by fault tolerant when    */
/*    transaction fails.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_attributes_set                                        */
/*    _fx_directory_create                                                */
/*    _fx_directory_delete                                                */
/*    _fx_directory_rename                                                */
/*    _fx_file_allocate                                                   */
/*    _fx_file_attributes_set                                             */
/*    _fx_file_best_effort_allocate                                       */
/*    _fx_file_create                                                     */
/*    _fx_file_delete                                                     */
/*    _fx_file_rename                                                     */
/*    _fx_file_truncate                                                   */
/*    _fx_file_truncate_release                                           */
/*    _fx_file_write                                                      */
/*    _fx_unicode_directory_create                                        */
/*    _fx_unicode_file_create                                             */
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
UINT _fx_fault_tolerant_transaction_fail(FX_MEDIA *media_ptr)
{

    /* Check whether fault tolerant is enabled or not. */
    if (media_ptr -> fx_media_fault_tolerant_enabled == FX_TRUE)
    {

        /* Yes. Decrease the counter for transaction. */
        media_ptr -> fx_media_fault_tolerant_transaction_count--;

        /* Is this the last transaction? */
        if (media_ptr -> fx_media_fault_tolerant_transaction_count == 0)
        {

            /* Yes. Perform recover and reset the log file. */
            _fx_fault_tolerant_recover(media_ptr);
            _fx_fault_tolerant_reset_log_file(media_ptr);
        }
    }

    return(FX_SUCCESS);
}
#endif
