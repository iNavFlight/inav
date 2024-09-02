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
#include "fx_directory.h"
#include "fx_fault_tolerant.h"


#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_enable                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the FileX Fault Tolerant feature. It first    */
/*    searches for a valid log file.  A valid log file indicates the      */
/*    previous write operation failed, and appropriate action now must    */
/*    be taken to restore the integrity of the file system.  Once the     */
/*    recovery effort is completed, the file system is properly restored. */
/*    An empty log file indicates the previous write operation was        */
/*    successfully completed and no action needs to be taken at this      */
/*    point.  If the file system does not have a log file, or the         */
/*    checksum is not valid, it is an indication either the file system   */
/*    is not under the protection of FileX Fault Tolerant.  A new log     */
/*    file is created.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    memory_buffer                         Pointer to memory buffer.     */
/*    memory_size                           Size of memory buffer.        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_fault_tolerant_calculate_checksum Compute Checksum of data      */
/*    _fx_fault_tolerant_apply_logs         Apply logs into file system   */
/*    _fx_fault_tolerant_recover            Recover FAT chain             */
/*    _fx_fault_tolerant_reset_log_file     Reset the log file            */
/*    _fx_fault_tolerant_read_log_file      Read log file to cache        */
/*    _fx_utility_exFAT_cluster_state_get   Get state of exFAT cluster    */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_16_unsigned_read          Read a USHORT from memory     */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
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
/*  10-31-2022     Tiejun Zhou              Modified comment(s),          */
/*                                            fixed memory buffer when    */
/*                                            cache is disabled,          */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_fault_tolerant_enable(FX_MEDIA *media_ptr, VOID *memory_buffer, UINT memory_size)
{
ULONG                         start_cluster;
ULONG                         FAT_value;
UINT                          status;
ULONG                         checksum;
ULONG                         total_size;
FX_FAULT_TOLERANT_LOG_HEADER *log_header;
FX_FAULT_TOLERANT_FAT_CHAIN  *FAT_chain;
ULONG                         cluster_number;
ULONG                         i, j;
ULONG                         FAT_entry, FAT_sector, FAT_read_sectors;
ULONG                         bytes_in_buffer;
ULONG                         clusters;
ULONG                         bytes_per_sector; 
ULONG                         bytes_per_cluster; 

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Calculate clusters needed for fault tolerant log. */
    bytes_per_sector = media_ptr -> fx_media_bytes_per_sector;
    bytes_per_cluster = bytes_per_sector * media_ptr -> fx_media_sectors_per_cluster;

    if (bytes_per_cluster == 0)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        return(FX_MEDIA_INVALID);
    }

    clusters = (FX_FAULT_TOLERANT_MAXIMUM_LOG_FILE_SIZE + bytes_per_cluster - 1) / bytes_per_cluster;
    media_ptr -> fx_media_fault_tolerant_clusters = clusters;

    /* Check buffer size requirement. */
    if (memory_size < bytes_per_sector)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        return(FX_NOT_ENOUGH_MEMORY);
    }


    if (media_ptr -> fx_media_FAT32_additional_info_sector)
    {

        /* A 32-bit FAT is present. Read directly into the logical sector
           cache memory to optimize I/O on larger devices. Since we are looking for
           values of zero, endian issues are not important.  */
        /* Force update the available cluster. */

#ifndef FX_DISABLE_CACHE
        /* Invalidate the current logical sector cache.  */
        _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64) (media_ptr -> fx_media_total_sectors), FX_TRUE);

        /* Reset the memory pointer.  */
        media_ptr -> fx_media_memory_buffer = media_ptr -> fx_media_sector_cache[0].fx_cached_sector_memory_buffer;
#endif /* FX_DISABLE_CACHE */

        /* Reset the available cluster. */
        media_ptr -> fx_media_available_clusters = 0;

        /* Loop through all FAT sectors in the primary FAT.  The first two entries are
           examined in this loop, but they are always unavailable.  */
        cluster_number =  0;
#ifndef FX_DISABLE_CACHE
        for (i = 0; i < media_ptr -> fx_media_sectors_per_FAT; i = i + media_ptr -> fx_media_sector_cache_size)
        {

            /* Calculate the starting next FAT sector.  */
            FAT_sector =  media_ptr -> fx_media_reserved_sectors + i;

            /* Calculate how many sectors to read.  */
            FAT_read_sectors =  media_ptr -> fx_media_sectors_per_FAT - i;

            /* Determine if there is not enough memory to read the remaining FAT sectors.  */
            if (FAT_read_sectors > media_ptr -> fx_media_sector_cache_size)
            {
                FAT_read_sectors =  media_ptr -> fx_media_sector_cache_size;
            }
#else
        /* Reset the buffer sector.  */
        media_ptr -> fx_media_memory_buffer_sector = (ULONG64)-1;
        for (i = 0; i < media_ptr -> fx_media_sectors_per_FAT; i++)
        {

            /* Calculate the starting next FAT sector.  */
            FAT_sector =  media_ptr -> fx_media_reserved_sectors + i;

            /* Calculate how many sectors to read.  */
            FAT_read_sectors =  1;
#endif /* FX_DISABLE_CACHE */

            /* Read the FAT sectors directly from the driver.  */
            media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
            media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
            media_ptr -> fx_media_driver_buffer =           media_ptr -> fx_media_memory_buffer;
            media_ptr -> fx_media_driver_logical_sector =   FAT_sector;
            media_ptr -> fx_media_driver_sectors =          FAT_read_sectors;
            media_ptr -> fx_media_driver_sector_type =      FX_FAT_SECTOR;

            /* If trace is enabled, insert this event into the trace buffer.  */
            FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_READ, media_ptr, FAT_sector, FAT_read_sectors, media_ptr -> fx_media_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

            /* Invoke the driver to read the FAT sectors.  */
            (media_ptr -> fx_media_driver_entry) (media_ptr);

            /* Determine if the read was successful.  */
            if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                return(FX_FAT_READ_ERROR);
            }

            /* Calculate the number of bytes in the buffer.  */
            bytes_in_buffer =  (media_ptr -> fx_media_bytes_per_sector * FAT_read_sectors);

            /* Walk through the sector cache memory to search for available clusters and the first
               available if not already found.  */
            for (j = 0; j < bytes_in_buffer;)
            {

                /* Pickup 32-bit FAT entry.  */
                FAT_entry =  *((ULONG *)&(media_ptr -> fx_media_memory_buffer[j]));

                /* Advance to next FAT entry.  */
                j = j + 4;

                /* Determine if the FAT entry is free.  */
                if (FAT_entry == FX_FREE_CLUSTER)
                {

                    /* Entry is free, increment available clusters.  */
                    media_ptr -> fx_media_available_clusters++;

                    /* Determine if the starting free cluster has been found yet.  */
                    if (media_ptr -> fx_media_cluster_search_start == 0)
                    {

                        /* Remember the first free cluster to start further searches from.  */
                        media_ptr -> fx_media_cluster_search_start =  cluster_number;
                    }
                }

                /* Increment the cluster number.  */
                cluster_number++;

                /* Determine if we have reviewed all FAT entries.  */
                if (cluster_number >= (media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START))
                {

                    /* Yes, we have looked at all the FAT entries.  */

                    /* Ensure that the outer loop terminates as well.  */
                    i = media_ptr -> fx_media_sectors_per_FAT;
                    break;
                }
            }
        }
    }

    /* Store memory buffer and size. */
    media_ptr -> fx_media_fault_tolerant_memory_buffer = (UCHAR *)memory_buffer;
    if (memory_size > (clusters * bytes_per_cluster))
    {
        media_ptr -> fx_media_fault_tolerant_memory_buffer_size = clusters * bytes_per_cluster;
    }
    else
    {
        media_ptr -> fx_media_fault_tolerant_memory_buffer_size = memory_size / bytes_per_sector * bytes_per_sector;
    }

    /* Read the boot sector from the device.  */
    media_ptr -> fx_media_driver_request =          FX_DRIVER_BOOT_READ;
    media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
    media_ptr -> fx_media_driver_buffer =           (UCHAR *)memory_buffer;
    media_ptr -> fx_media_driver_sectors =          1;
    media_ptr -> fx_media_driver_sector_type =      FX_BOOT_SECTOR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_BOOT_READ, media_ptr, memory_buffer, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Invoke the driver to read the boot sector.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Determine if the boot sector was read correctly. */
    if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the boot sector error status.  */
        return(FX_BOOT_ERROR);
    }

    /* Check whether the boot index is used. */
    start_cluster = _fx_utility_32_unsigned_read((UCHAR *)memory_buffer + FX_FAULT_TOLERANT_BOOT_INDEX);
    if (start_cluster != 0)
    {

        /* The location of the fault tolerant log file is found.  Need to verify the integrity of the log file. */
        if ((start_cluster >= FX_FAT_ENTRY_START) && (start_cluster < media_ptr -> fx_media_fat_reserved))
        {

            /* Check whether this cluster is used. */
#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {
            UCHAR cluste_state;

                for (i = 0; i < clusters; i++)
                {

                    /* Update Bitmap */
                    status = _fx_utility_exFAT_cluster_state_get(media_ptr, start_cluster + i, &cluste_state);

                    /* Check for a bad status.  */
                    if (status != FX_SUCCESS)
                    {

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the bad status.  */
                        return(status);
                    }

                    if (cluste_state != FX_EXFAT_BITMAP_CLUSTER_OCCUPIED)
                    {

                        /* Mark invalid. */
                        start_cluster = 0;
                        break;
                    }
                }
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */
                for (i = 0; i < clusters; i++)
                {

                    /* Read FAT entry.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr, start_cluster + i, &FAT_value);

                    /* Check for a bad status.  */
                    if (status != FX_SUCCESS)
                    {

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the bad status.  */
                        return(status);
                    }

                    if (i < clusters - 1)
                    {
                        if (FAT_value != (start_cluster + i + 1))
                        {

                            /* Mark invalid. */
                            start_cluster = 0;
                            break;
                        }
                    }
                    else if (FAT_value != media_ptr -> fx_media_fat_last)
                    {

                        /* Mark invalid. */
                        start_cluster = 0;
                        break;
                    }
                }
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Is this FAT entry occupied by log file? */
            if (start_cluster)
            {

                /* Set the start cluster. */
                media_ptr -> fx_media_fault_tolerant_start_cluster = start_cluster;

                /* Read log file from file system to memory. */
                status = _fx_fault_tolerant_read_log_file(media_ptr);

                /* Check for good completion status.  */
                if (status !=  FX_SUCCESS)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
                    return(status);
                }

                /* Set log header and FAT chain pointer. */
                log_header = (FX_FAULT_TOLERANT_LOG_HEADER *)media_ptr -> fx_media_fault_tolerant_memory_buffer;
                FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                                            FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);

                /* Verify ID field. */
                if (_fx_utility_32_unsigned_read((UCHAR *)&log_header -> fx_fault_tolerant_log_header_id) == FX_FAULT_TOLERANT_ID)
                {

                    /* Calculate checksum of log header. */
                    checksum = _fx_fault_tolerant_calculate_checksum((UCHAR *)log_header,
                                                                     FX_FAULT_TOLERANT_LOG_HEADER_SIZE);

                    if (checksum == 0)
                    {

                        /* Fault tolerant log file is valid. */
                        /* Initialize file size. */
                        total_size = _fx_utility_16_unsigned_read((UCHAR *)&log_header -> fx_fault_tolerant_log_header_total_size);
                        media_ptr -> fx_media_fault_tolerant_file_size = total_size;


                        /* Verify the checksum of the FAT chain. */
                        checksum = _fx_fault_tolerant_calculate_checksum((UCHAR *)FAT_chain,
                                                                         FX_FAULT_TOLERANT_FAT_CHAIN_SIZE);

                        if (checksum == 0)
                        {

                            /* Checksum of FAT chain is correct. */

                            if (total_size > (FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_HEADER_SIZE))
                            {

                                /* Log content is present. */
                                /* Now verify the checksum of log content. */
                                checksum = _fx_fault_tolerant_calculate_checksum((media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                                                                  FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET),
                                                                                 (total_size - FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET));
                                if (checksum == 0)
                                {

                                    /* Checksum of log content is correct. */

                                    /* Extended port-specific processing macro, which is by default defined to white space.  */
                                    FX_FAULT_TOLERANT_ENABLE_EXTENSION

                                    /* This is the situation where the log file contains log entries.  This is an indication
                                       that previous write operation did not complete successfully.  Need to apply the log entries
                                       to recover the previous write operation, effectively to finish up the previous write operation. */
                                    status = _fx_fault_tolerant_apply_logs(media_ptr);
                                }
                            }
                            else
                            {

                                /* Extended port-specific processing macro, which is by default defined to white space.  */
                                FX_FAULT_TOLERANT_ENABLE_EXTENSION

                                /* The log file does not contain log content but the FAT chain operation information is present.
                                   This is the situation where the FAT chain has been modified but the rest of the content of these
                                   clusters are not updated yet.  In this situation, the previous FAT chain operation needs to be
                                   reverted to restore the file system back to its state prior to the write operation. */
                                status = _fx_fault_tolerant_recover(media_ptr);
                            }

                            if (status !=  FX_SUCCESS)
                            {

                                /* Release media protection.  */
                                FX_UNPROTECT

                                /* Return the error status.  */
                                return(status);
                            }
                        }
                    }
                }
            }
        }
        else
        {

            /* Not a valid cluster number. Set the flag to create a new log file. */
            start_cluster = 0;
        }
    }

    /* Check whether or not to create a log file. */
    if (start_cluster == 0)
    {

        /* Create log file. */
        status = _fx_fault_tolerant_create_log_file(media_ptr);

        if (status !=  FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the error status.  */
            return(status);
        }
    }

    /* Reset log file. */
    status = _fx_fault_tolerant_reset_log_file(media_ptr);

    if (status !=  FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error status.  */
        return(status);
    }

    /* Mark fault tolerant feature is enabled. */
    media_ptr -> fx_media_fault_tolerant_enabled = FX_TRUE;

    /* Reset the transaction count. */
    media_ptr -> fx_media_fault_tolerant_transaction_count = 0;

    /* Initialize the sector number of cached FAT entries. */
    media_ptr -> fx_media_fault_tolerant_cached_FAT_sector = 0;

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return the error status.  */
    return(FX_SUCCESS);
}

#endif /* FX_ENABLE_FAULT_TOLERANT */

