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
/*    _fx_fault_tolerant_set_FAT_chain                     PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets data of FAT chain.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    use_bitmap                            Whether or not the orignal    */
/*                                            FAT chain uses bitmap       */
/*    insertion_front                       Previous cluster of head      */
/*    new_head_cluster                      Head cluster of new chain     */
/*    origianl_head_cluster                 Head cluster of the original  */
/*                                            chain                       */
/*    insertion_back                        next cluster of chain tail    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    _fx_utility_32_unsigned_write         Write a ULONG from memory     */
/*    _fx_fault_tolerant_calculate_checksum Compute Checksum of data      */
/*    _fx_fault_tolerant_write_log_file     Write log file                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_file_allocate                                                   */
/*    _fx_file_best_effort_allocate                                       */
/*    _fx_file_delete                                                     */
/*    _fx_file_truncate_release                                           */
/*    _fx_file_write                                                      */
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
UINT    _fx_fault_tolerant_set_FAT_chain(FX_MEDIA *media_ptr, UINT use_bitmap, ULONG insertion_front,
                                         ULONG new_head_cluster, ULONG original_head_cluster,
                                         ULONG insertion_back)
{
UINT                         status;
USHORT                       checksum;
UCHAR                        flag = FX_FAULT_TOLERANT_FLAG_FAT_CHAIN_VALID;
FX_FAULT_TOLERANT_FAT_CHAIN *FAT_chain;

    /* Set FAT chain pointer. */
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(media_ptr -> fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);

#ifdef FX_ENABLE_EXFAT
    /* Check flag for bitmap. */
    if (use_bitmap == FX_TRUE)
    {
        flag |= FX_FAULT_TOLERANT_FLAG_BITMAP_USED;
    }
#else
    /* Parameters not used. Avoid compiler warnings. */
    FX_PARAMETER_NOT_USED(use_bitmap);
#endif /* FX_ENABLE_EXFAT */


    /* Reset checksum first. */
    _fx_utility_16_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_checksumm, 0);

    /* Set clusters. */
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_insertion_front, insertion_front);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_new, new_head_cluster);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_original, original_head_cluster);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_insertion_back, insertion_back);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_next_deletion, FX_FREE_CLUSTER);

    /* Set flag and reserved. */
    FAT_chain -> fx_fault_tolerant_FAT_chain_flag = flag;
    FAT_chain -> fx_fault_tolerant_FAT_chain_reserved = (UCHAR)0;

    /* Calculate checksum. */
    checksum = _fx_fault_tolerant_calculate_checksum((UCHAR *)FAT_chain, FX_FAULT_TOLERANT_FAT_CHAIN_SIZE);
    _fx_utility_16_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_checksumm, checksum);

    /* Write the log header and FAT chain.  */
    status =  _fx_fault_tolerant_write_log_file(media_ptr, 0);

    /* Return the status.  */
    return(status);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

