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
#include "fx_utility.h"
#include "fx_fault_tolerant.h"


#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_recover                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called after the FAT chain is been updated but     */
/*    the rest of the write operation fails.  At this point, there is     */
/*    need to undo the FAT chain operation, which includes:               */
/*    (1) Remove newly allocated FAT entries;                             */
/*    (2) Restore the origianl FAT chain removed during the FAT chain     */
/*        update;                                                         */
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
/*    _fx_fault_tolerant_cleanup_FAT_chain  Cleanup FAT chain             */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
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
/*    _fx_fault_tolerant_enable                                           */
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
UINT _fx_fault_tolerant_recover(FX_MEDIA *media_ptr)
{
UINT                         status;
ULONG                        insertion_front;
ULONG                        origianl_head_cluster;
FX_FAULT_TOLERANT_FAT_CHAIN *FAT_chain;


    /* Set fault tolerant state to IDLE. */
    media_ptr -> fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_IDLE;

    /* Set FAT chain pointer. */
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(media_ptr -> fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);

    /* Whether or not the supplied FAT chain is valid. */
    if (!(FAT_chain -> fx_fault_tolerant_FAT_chain_flag & FX_FAULT_TOLERANT_FLAG_FAT_CHAIN_VALID))
    {

        /* Invalid, which indiates the FAT chain has been cleaned up already.  In this case, just return. */
        return(FX_SUCCESS);
    }

    /* Set FAT chain pointer. */
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(media_ptr -> fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);

    /* Recover FAT chain. */
    status = _fx_fault_tolerant_cleanup_FAT_chain(media_ptr, FX_FAULT_TOLERANT_FAT_CHAIN_RECOVER);
    if (status != FX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Now, link the front of the insertion point back to the origianl FAT chain. */
    insertion_front = _fx_utility_32_unsigned_read((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_insertion_front);
    origianl_head_cluster = _fx_utility_32_unsigned_read((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_original);

    if (insertion_front != FX_FREE_CLUSTER)
    {

        /* Front of the insertion point exists. Link the origianl chain back to the front of the insertion point. */
        status = _fx_utility_FAT_entry_write(media_ptr, insertion_front, origianl_head_cluster);
        if (status != FX_SUCCESS)
        {

            /* Return the error status.  */
            return(status);
        }
    }

    /* New FAT chain is always linked by FAT entries. */

    /* Flush FAT table. */
#ifdef FX_FAULT_TOLERANT
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Flush exFAT bitmap.  */
        _fx_utility_exFAT_bitmap_flush(media_ptr);
    }
#endif /* FX_ENABLE_EXFAT */

    /* Ensure the new FAT chain is properly written to the media.  */

    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);
#endif

    /* Return success. */
    return(FX_SUCCESS);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

