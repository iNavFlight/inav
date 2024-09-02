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

#define FX_SOURCE_CODE

#include "fx_api.h"
#include "fx_fault_tolerant.h"


#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_transaction_start                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called at the beginning of an update to FileX      */
/*    file, directory entry, or FAT table.  It resets fault tolerant      */
/*    internal state information.                                         */
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
UINT    _fx_fault_tolerant_transaction_start(FX_MEDIA *media_ptr)
{

    /* Is fault tolerant enabled? */
    if (media_ptr -> fx_media_fault_tolerant_enabled == FX_TRUE)
    {

        /* Is this a new transaction? */
        if (media_ptr -> fx_media_fault_tolerant_transaction_count == 0)
        {

            /* Yes. Initialize data. */
            media_ptr -> fx_media_fault_tolerant_file_size = FX_FAULT_TOLERANT_LOG_HEADER_SIZE +
                                                             FX_FAULT_TOLERANT_FAT_CHAIN_SIZE +
                                                             FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;

            /* Reset total logs. */
            media_ptr -> fx_media_fault_tolerant_total_logs = 0;

            /* Set state of fault tolerant. */
            media_ptr -> fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_STARTED;
        }

        /* Increase the transaction. */
        media_ptr -> fx_media_fault_tolerant_transaction_count++;
    }

    return(FX_SUCCESS);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

