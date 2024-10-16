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
/*    _fx_utility_FAT_entry_multiple_sectors_check        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Tiejun Zhou, Microsoft Corporation                                  */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether the FAT entry spans multiple sectors.  */
/*    It is only possible when the file system is FAT12.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_TRUE                               FAT entry spans two sectors   */
/*    FX_FALSE                              FAT entry in one sector       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_fault_tolerant_cleanup_FAT_chain                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Tiejun Zhou              Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
static UINT _fx_utility_FAT_entry_multiple_sectors_check(FX_MEDIA *media_ptr, ULONG cluster)
{
ULONG byte_offset;
ULONG FAT_sector;

    /* Check FAT format.  */
    if (!media_ptr -> fx_media_12_bit_FAT)
    {

        /* Not FAT12. The FAT entry will be in one sector.  */
        return(FX_FALSE);
    }

    /* File system is FAT12.  */
    /* Calculate the byte offset to the cluster entry.  */
    byte_offset =  (((ULONG)cluster << 1) + cluster) >> 1;

    /* Calculate the FAT sector the requested FAT entry resides in.  */
    FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
        (ULONG)media_ptr -> fx_media_reserved_sectors;

    /* Now calculate the byte offset into this FAT sector.  */
    byte_offset =  byte_offset -
        ((FAT_sector - (ULONG)media_ptr -> fx_media_reserved_sectors) *
            media_ptr -> fx_media_bytes_per_sector);

    /* Determine if we are now past the end of the FAT buffer in memory.  */
    if (byte_offset == (ULONG)(media_ptr -> fx_media_bytes_per_sector - 1))
    {

        /* Yes, we are past the end of the FAT buffer.  */
        return(FX_TRUE);
    }
    else
    {

        /* No, we are not past the end of the FAT buffer.  */
        return(FX_FALSE);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_cleanup_FAT_chain                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up FAT chain. In order to prevent failures     */
/*    during the link-list walk, and to improve efficiency, this function */
/*    frees the FAT entry chain in sections.  For each section, it frees  */
/*    FAT entry from the last entry in the chain and works backwards.     */
/*    Using this method, if the system fails in the middle of the free    */
/*    operation, the head is preserved, and fault tolerant module can     */
/*    "redo" this operation next time it starts up.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    operation                             Recover a failure operation or*/
/*                                            cleanup old FAT chain       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_exFAT_cluster_state_get   Get state of exFAT cluster    */
/*    _fx_utility_exFAT_cluster_state_set   Set state of exFAT cluster    */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_FAT_entry_write           Write a FAT entry             */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    _fx_utility_32_unsigned_write         Write a ULONG from memory     */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
/*    _fx_utility_FAT_flush                 Flush written FAT entries     */
/*    _fx_fault_tolerant_calculate_checksum Compute Checksum of data      */
/*    _fx_fault_tolerant_write_log_file     Write log file                */
/*    _fx_utility_FAT_entry_multiple_sectors_check                        */
/*                                          Check sectors for FAT entry   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_fault_tolerant_apply_logs                                       */
/*    _fx_fault_tolerant_recover                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Tiejun Zhou              Modified comment(s),          */
/*                                            fixed FAT entry span two    */
/*                                            sectors for FAT12,          */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT    _fx_fault_tolerant_cleanup_FAT_chain(FX_MEDIA *media_ptr, UINT operation)
{
UINT                         status;
ULONG                        current_cluster;
ULONG                        next_cluster = 0;
ULONG                        head_cluster;
ULONG                        tail_cluster;
ULONG                        cache_count;
ULONG                        cache_max = FX_FAULT_TOLERANT_CACHE_SIZE >> 2;
ULONG                       *cache_ptr = media_ptr -> fx_media_fault_tolerant_cache;
ULONG                        next_session;
USHORT                       checksum;
ULONG                        i;
FX_FAULT_TOLERANT_FAT_CHAIN *FAT_chain;
ULONG                        FAT_sector;
ULONG                        last_FAT_sector;

    /* Set FAT chain pointer. */
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(media_ptr -> fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);

    /* Get head and tail cluster. */
    if (operation == FX_FAULT_TOLERANT_FAT_CHAIN_RECOVER)
    {

        /* Recover phase. Cleanup (remove) new FAT chain. */
        head_cluster = _fx_utility_32_unsigned_read((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_new);
    }
    else
    {

        /* Cleanup phase. Cleanup (remove) old FAT chain. */
        head_cluster = _fx_utility_32_unsigned_read((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_original);
    }

    /* At this point, the head_cluster points to the cluster chain that is to be removed. */

    /* Tail cluster points to the back of the origianal FAT chain where the new chain is attached to.
       The remove process terminates once this cluster is encountered. */
    tail_cluster = _fx_utility_32_unsigned_read((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_insertion_back);

    /* Are there any clusters to free? */
    if ((head_cluster < FX_FAT_ENTRY_START) || (head_cluster >= media_ptr -> fx_media_fat_reserved))
    {
        return(FX_SUCCESS);
    }

    /* To optimize the deletion process of a long FAT chain, the deletion is done in sessions.  During one session,
       a set of FAT entries are removed.  If one session is interrupted, after restart the deletion process resumes and
       picks up from where it left.

       During one deletion session, all FAT entries are between head_cluster(inclusive) and next_session (exclusive).
     */
    next_session = _fx_utility_32_unsigned_read((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_next_deletion);

    /* Loop to cleanup all FAT entries. */
    while ((head_cluster >= FX_FAT_ENTRY_START) && (head_cluster < media_ptr -> fx_media_fat_reserved))
    {

        /* Initialize. */
        current_cluster = head_cluster;
        cache_count = 0;

        /* Loop from head cluster to find entries to cleanup and store this information into cache. */
        do
        {

            /* Get the next FAT entry. */
            status = _fx_utility_FAT_entry_read(media_ptr, current_cluster, &next_cluster);
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

            /* Whether or not the cluster is occupied. */
            if (next_cluster == FX_FREE_CLUSTER)
            {

                /* Current cluster has already been released. */
                break;
            }

            /* Cache the FAT list. */
            cache_ptr[cache_count++] = current_cluster;

            /* Check whether FAT entry spans multiple sectors.  */
            if (_fx_utility_FAT_entry_multiple_sectors_check(media_ptr, current_cluster))
            {
                if (head_cluster == next_session || next_session == FX_FREE_CLUSTER)
                {
                    next_session = next_cluster;
                }
                break;
            }

            /* Move to next cluster. */
            current_cluster = next_cluster;
        } while ((next_cluster >= FX_FAT_ENTRY_START) &&
                 (next_cluster < media_ptr -> fx_media_fat_reserved) &&
                 (next_cluster != tail_cluster) &&
                 (cache_count < cache_max));

        /* Get next session. */
        if (cache_count == cache_max)
        {
            next_session = next_cluster;
        }

        /* Update head cluster and next session into log file. */
        _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_next_deletion, next_session);
        if (operation == FX_FAULT_TOLERANT_FAT_CHAIN_RECOVER)
        {
            _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_new, head_cluster);
        }
        else
        {
            _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_original, head_cluster);
        }

        /* Update checksum. */
        _fx_utility_16_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_checksumm, 0);
        checksum = _fx_fault_tolerant_calculate_checksum((UCHAR *)FAT_chain, FX_FAULT_TOLERANT_FAT_CHAIN_SIZE);
        _fx_utility_16_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_checksumm, checksum);

        /* Flush the fault tolerant log into the file system.  */
        _fx_fault_tolerant_write_log_file(media_ptr, 0);

        /* Loop to free FAT in cache from back to front. */
        if (cache_count > 0)
        {

            /* Get sector of last FAT entry. */
            last_FAT_sector = _fx_utility_FAT_sector_get(media_ptr, cache_ptr[cache_count - 1]);
            i = cache_count;
            while (i > 0)
            {
                i--;

                /* Get sector of current FAT entry. */
                FAT_sector = _fx_utility_FAT_sector_get(media_ptr, cache_ptr[i]);
                if (FAT_sector != last_FAT_sector)
                {

                    /* Current FAT entry is located in different sector. Force flush. */
                    /* Flush the cached individual FAT entries */
                    _fx_utility_FAT_flush(media_ptr);
#ifdef FX_ENABLE_EXFAT
                    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
                    {

                        /* Flush exFAT bitmap.  */
                        _fx_utility_exFAT_bitmap_flush(media_ptr);
                    }
#endif /* FX_ENABLE_EXFAT */

                    /* Update sector of current FAT entry. */
                    last_FAT_sector = FAT_sector;
                }

                /* Free current_cluster. */
                status = _fx_utility_FAT_entry_write(media_ptr, cache_ptr[i], FX_FREE_CLUSTER);
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

#ifdef FX_ENABLE_EXFAT
                if (media_ptr -> fx_media_FAT_type == FX_exFAT)
                {

                    /* Free bitmap. */
                    status = _fx_utility_exFAT_cluster_state_set(media_ptr, cache_ptr[i], FX_EXFAT_BITMAP_CLUSTER_FREE);
                    if (status != FX_SUCCESS)
                    {

                        /* Return the error status.  */
                        return(status);
                    }
                }
#endif /* FX_ENABLE_EXFAT */
            }

            /* Increase the available clusters in the media control block. */
            media_ptr -> fx_media_available_clusters  += cache_count;
        }

        /* Flush the cached individual FAT entries */
        _fx_utility_FAT_flush(media_ptr);
#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {

            /* Flush exFAT bitmap.  */
            _fx_utility_exFAT_bitmap_flush(media_ptr);
        }
#endif /* FX_ENABLE_EXFAT */


        /* Get head cluster. */
        if (head_cluster == next_session)
        {
            break;
        }
        head_cluster = next_session;
    }

    return(FX_SUCCESS);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */