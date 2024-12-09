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
/**   Directory                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"


#ifdef FX_ENABLE_EXFAT
#include "fx_utility.h"
#include "fx_directory_exFAT.h"
#include "fx_directory.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_exFAT_entry_read                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the supplied directory entry from the supplied  */
/*    source directory.  If the supplied directory entry is NULL, then    */
/*    the root directory is assumed.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    source_dir                            Source directory entry        */
/*    entry_ptr                             Directory entry number        */
/*    destination_ptr                       Pointer to destination for    */
/*                                            the directory entry         */
/*    hash                                  Hash value of the file name   */
/*    skip                                  Skip reading file name        */
/*    unicode_name                          Destination for unicode name  */
/*    unicode_length                        Destination for unicode name  */
/*                                            length                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*    *entry_ptr should point to the 8:3 entry if it is a long name       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_logical_sector_read       Read directory sector         */
/*    _fx_utility_16_unsigned_read          Read a USHORT from memory     */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
/*    _fx_utility_64_unsigned_read          Read a ULONG64 from memory    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_entry_read                                            */
/*    _fx_directory_free_search                                           */
/*    _fx_directory_next_full_entry_find                                  */
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
UINT  _fx_directory_exFAT_entry_read(FX_MEDIA *media_ptr, FX_DIR_ENTRY *source_dir,
                                     ULONG *entry_ptr, FX_DIR_ENTRY *destination_ptr,
                                     UINT hash, UINT skip, UCHAR *unicode_name, UINT *unicode_length)
{

UINT    status = FX_SUCCESS;
UCHAR   i;
UCHAR   j;
ULONG   cluster = 0, next_cluster = 0;
ULONG64 next_logical_sector;
UINT    relative_cluster;
UINT    relative_sector = 0;
ULONG64 logical_sector;
ULONG   byte_offset;
ULONG   bytes_per_cluster;
UCHAR  *read_ptr;
UCHAR   secondary_count;
UCHAR   name_length = 0;
UCHAR   name_pos = 0;
UCHAR   copy_size = 0;
USHORT  checksum = 0;
USHORT  file_checksum;
ULONG   date_time;
UCHAR   dont_use_fat;
ULONG   clusters_count = 0;


    /* Calculate the byte offset of this directory entry.  */
    byte_offset =  (*entry_ptr) * FX_DIR_ENTRY_SIZE;

    /* Calculate the number of bytes per cluster.  */
    bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    /* Now determine the relative cluster in the sub-directory file.  */
    relative_cluster =   (UINT)(byte_offset / bytes_per_cluster);

    /* Calculate the byte offset within the cluster.  */
    byte_offset =  byte_offset % bytes_per_cluster;

    /* Now figure out the relative sector within the cluster.  */
    relative_sector =    (UINT)(byte_offset / ((ULONG)media_ptr -> fx_media_bytes_per_sector));

    /* Read the directory sector into the internal memory buffer.  */

    /* Determine if there is a sub-directory.  */
    if (source_dir)
    {

        /* Yes, setup the starting cluster to that of the sub-directory.  */
        cluster =  source_dir -> fx_dir_entry_cluster;

        /* Save the cluster count for the directory entry.  */
        clusters_count = (ULONG)((source_dir -> fx_dir_entry_file_size + bytes_per_cluster - 1) / bytes_per_cluster - 1);
    }
    else
    {

        /* No, setup the starting cluster to the exFAT root directory cluster.  */
        cluster =  media_ptr -> fx_media_root_cluster_32;
    }

    /* Loop to position to the appropriate cluster.  */
    for (i = 0;; i++)
    {

        /* Check the value of the new cluster - it must be a valid cluster number
           or something is really wrong!  */
        if ((cluster < FX_FAT_ENTRY_START) || (cluster > FX_RESERVED_1_exFAT))
        {

            /* Send error message back to caller.  */
            return(FX_FILE_CORRUPT);
        }

        /* Read the next cluster. First check if FAT is used.  */
        if ((source_dir) && (source_dir -> fx_dir_entry_dont_use_fat & 1))
        {

            /* FAT is not used. Check for file size range.  */
            if (i >= clusters_count)
            {

                /* We are at the last cluster, set next cluster to LAST CLUSTER.  */
                next_cluster =  FX_LAST_CLUSTER_exFAT;
            }
            else
            {

                /* The next cluster is just after the current cluster.  */
                next_cluster =  cluster + 1;
            }
        }
        else
        {

            /* FAT is used, call FAT read function to get the next cluster.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

            /* Check for I/O error.  */
            if (status != FX_SUCCESS)
            {

                /* Return error code.  */
                return(status);
            }
        }

        /* Are we positioned to the appropriate cluster?  */
        if (i < relative_cluster)
        {
            cluster = next_cluster;
        }
        else
        {
            break;
        }
    }

    /* At this point, the directory data sector needs to be read.  */
    logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
        (((ULONG64)cluster - FX_FAT_ENTRY_START) *
         ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
        relative_sector;

    /* Determine if next sector is in next cluster or in next sector.  */
    if ((bytes_per_cluster - byte_offset) >= (ULONG)media_ptr -> fx_media_bytes_per_sector)
    {

        /* Move to the next logical sector.   */
        next_logical_sector = logical_sector + 1;
    }
    else
    {
        /* No, calculate the next logical sector.  */
        if ((next_cluster >= FX_FAT_ENTRY_START) && (next_cluster <= FX_RESERVED_1_exFAT))
        {
            next_logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
                (((ULONG64)next_cluster - FX_FAT_ENTRY_START) * ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
        }
        else
        {
            next_logical_sector = 0; /* No such sector possible.  */
        }
    }

    /* Read the logical directory sector.  */
    status =  _fx_utility_logical_sector_read(media_ptr, ((ULONG64) logical_sector),
                                              media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

    /* Determine if an error occurred.  */
    if (status != FX_SUCCESS)
    {

        /* Return error code.  */
        return(status);
    }

    /* Calculate the byte offset within this sector.  */
    byte_offset =  byte_offset % media_ptr -> fx_media_bytes_per_sector;


    /* Setup a pointer into the buffer.  */
    read_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

    /* Save the logical sector and byte offset in the returned directory entry.  */
    destination_ptr -> fx_dir_entry_log_sector =       logical_sector;
    destination_ptr -> fx_dir_entry_next_log_sector =  next_logical_sector;
    destination_ptr -> fx_dir_entry_byte_offset =      byte_offset;

    /* Clear the short file name information.  */
    destination_ptr -> fx_dir_entry_long_name_shorted =  0;
    destination_ptr -> fx_dir_entry_short_name[0]     =  0;

    /* Check if the directory entry type has InUse bit set, or if it is an end marker.  */
    if ((*read_ptr & FX_EXFAT_ENTRY_TYPE_IN_USE_MASK) || (*read_ptr == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER))
    {

        /* Set the directory entry type.  */
        destination_ptr -> fx_dir_entry_type = *read_ptr;
    }
    else
    {

        /* This is a free entry.  */
        destination_ptr -> fx_dir_entry_type = FX_EXFAT_DIR_ENTRY_TYPE_FREE;
    }

    /* Check if the entry is a file directory entry.  */
    if (destination_ptr -> fx_dir_entry_type != FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY)
    {

        /* The entry is not directory entry, check if we know the type.  */
        if ((destination_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_ALLOCATION_BITMAP) ||
            (destination_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_UP_CASE_TABLE)     ||
            (destination_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_VOLUME_LABEL)      ||
            (destination_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_FREE)              ||
            (destination_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER))
        {

            /* Known critical primary or free, just skip it. */
            return(FX_SUCCESS);
        }

        /* Check if the entry is unknown critical primary directory entry.  */
        if ((destination_ptr -> fx_dir_entry_type & (FX_EXFAT_ENTRY_TYPE_IMPORTANCE_MASK | FX_EXFAT_ENTRY_TYPE_CATEGORY_MASK)) == 0)
        {

            /* Yes, it is an unknown critical primary entry. Return error.  */
            return(FX_MEDIA_INVALID);
        }

        /* Just ignore.  */
        return(FX_SUCCESS);
    }

    /* Calculate checksum for file entry.  */
    for (j = 0; j < FX_DIR_ENTRY_SIZE; ++j)
    {

        /* Skip the checksum field.  */
        if ((j == 2) || (j == 3))
        {
            continue;
        }

        /* Calculate the checksum using the algorithm described in the specification.  */
        /* Right rotate the checksum by one bit position and add the data.  */
        checksum = (USHORT)(((checksum >> 1) | (checksum << 15)) + read_ptr[j]);
    }

    /* Read secondary count field.  */
    read_ptr++;
    secondary_count = *read_ptr;

    /* Validate the secondary count field.  */
    if (secondary_count < 2)
    {

        /* Something wrong, file entry should have at least 2 entries: stream and filename.  */
        return(FX_FILE_CORRUPT);
    }

    /* Save secondary count.  */
    destination_ptr -> fx_dir_entry_secondary_count =  secondary_count;

    /* exFAT has no short/long name concept, set long name present for compatibility.  */
    destination_ptr -> fx_dir_entry_long_name_present =  1;

    /* Advance the pointer to the checksum field.  */
    read_ptr++;

    /* Read checksum field.  */
    file_checksum = (USHORT)_fx_utility_16_unsigned_read(read_ptr);
    read_ptr += 2;

    /* Read FileAttributes field.  */
    destination_ptr -> fx_dir_entry_attributes = (UCHAR)_fx_utility_16_unsigned_read(read_ptr); /* The same order as FAT32 but more unused bits.  */

    /* Skip Reserved1.  */
    read_ptr += 4;

    /* Read CreatedTimestamp field.  */
    date_time = _fx_utility_32_unsigned_read(read_ptr);
    destination_ptr -> fx_dir_entry_created_time = date_time & 0xffff;
    destination_ptr -> fx_dir_entry_created_date = date_time >> 16;
    read_ptr += 4;

    /* Read LastModifiedTimestamp.  */
    date_time = _fx_utility_32_unsigned_read(read_ptr);
    destination_ptr -> fx_dir_entry_time = date_time & 0xffff;
    destination_ptr -> fx_dir_entry_date = date_time >> 16;
    read_ptr += 4;

    /* Read LastAccessedTimestamp.  */
    date_time = _fx_utility_32_unsigned_read(read_ptr);
    destination_ptr -> fx_dir_entry_last_accessed_date = date_time >> 16;
    read_ptr += 4;

    /* Read Create10msIncrement.  */
    destination_ptr -> fx_dir_entry_created_time_ms = *read_ptr;
    read_ptr++;

    /* LastModified10msIncrement field is currently ignored.  */
    read_ptr++;

    /* CreateUtcOffset field is currently ignored.  */
    read_ptr++;

    /* LastModifiedUtcOffset field is currently ignored.  */
    read_ptr++;

    /* LastAccessedUtcOffset field is currently ignored.  */

    /* Process all the secondary directory entries.  */
    for (i = 0; i < secondary_count; ++i)
    {

        /* Determine if a new sector needs to be read.  */
        if (byte_offset + FX_DIR_ENTRY_SIZE >= media_ptr -> fx_media_bytes_per_sector)
        {

            /* Determine the next sector of the directory entry.  */
            if (relative_sector < (media_ptr -> fx_media_sectors_per_cluster - 1))
            {

                /* More sectors in this cluster.  */

                /* Simply increment the logical sector.  */
                logical_sector++;

                /* Increment the relative sector.  */
                relative_sector++;
            }
            else
            {

                /* We need to move to the next cluster.  */

                /* Read the next cluster. First check if FAT is used.  */
                if ((source_dir) && (source_dir -> fx_dir_entry_dont_use_fat & 1))
                {

                    /* FAT is not used. Check for file size range.  */
                    if (relative_cluster >= clusters_count)
                    {

                        /* We are at the last cluster, set next cluster to LAST CLUSTER.  */
                        next_cluster =  FX_LAST_CLUSTER_exFAT;
                    }
                    else
                    {

                        /* The next cluster is just after the current cluster.  */
                        next_cluster =  cluster + 1;
                    }
                }
                else
                {

                    /* FAT is used, call FAT read function to get the next cluster.  */
                    status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

                    /* Check for I/O error.  */
                    if (status != FX_SUCCESS)
                    {

                        /* Return error code.  */
                        return(status);
                    }
                }

                /* Copy next cluster to the current cluster.  */
                cluster =  next_cluster;

                /* Check the value of the new cluster - it must be a valid cluster number
                   or something is really wrong!  */
                if ((cluster < FX_FAT_ENTRY_START) || (cluster > FX_RESERVED_1_exFAT))
                {

                    /* Send error message back to caller.  */
                    return(FX_FILE_CORRUPT);
                }

                /* Now increment the relative cluster.  */
                relative_cluster++;

                /* Setup the relative sector (this is zero for subsequent cluster.  */
                relative_sector =  0;

                /* Calculate the next logical sector.  */
                logical_sector =   ((ULONG)media_ptr -> fx_media_data_sector_start) +
                    (((ULONG64)cluster - FX_FAT_ENTRY_START) *
                     ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
            }

            /* Read the new sector.  */
            status =  _fx_utility_logical_sector_read(media_ptr, ((ULONG64) logical_sector),
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

            /* Check I/O status.  */
            if (status != FX_SUCCESS)
            {
                return(status);
            }

            /* Set the byte offset to 0 for new sector.  */
            byte_offset = 0;
        }
        else
        {

            /* Calculate the new byte offset.  */
            byte_offset += FX_DIR_ENTRY_SIZE;
        }

        /* Read sub entry.  */
        read_ptr = (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

        /* Calculate checksum for sub entry.  */
        for (j = 0; j < FX_DIR_ENTRY_SIZE; ++j)
        {

            /* Right rotate the checksum by one bit position and add the data.  */
            checksum = (USHORT)(((checksum >> 1) | (checksum << 15)) + read_ptr[j]);
        }

        /* Add the entry number to the next entry.  */
        (*entry_ptr)++;

        /* Check if we are processing the first secondary directory entry.  */
        if (i == 0)
        {

            /* Make sure the directory entry type is stream extension.  */
            if (*read_ptr != FX_EXFAT_DIR_ENTRY_TYPE_STREAM_EXTENSION)
            {

                /* Something wrong, stream entry should be next to file entry.  */
                return(FX_FILE_CORRUPT);
            }

            /* Read stream entry.  */

            /* Advance the pointer to general secondary flags field.  */
            read_ptr++;

            /* Read NoFatChain field.  */
            dont_use_fat = ((*read_ptr) >> 1) & 1; /* 0 bit - current */

            /* Check if we have a parent directory.  */
            if (source_dir)
            {

                /* Save the dont_use_fat flag for parent directory.  */
                dont_use_fat = (UCHAR)(dont_use_fat | ((source_dir -> fx_dir_entry_dont_use_fat & 1) << 1)); /* 1st bit parent */
            }

            /* Put the dont_use_fat flag to the destination.  */
            destination_ptr -> fx_dir_entry_dont_use_fat =  (CHAR)dont_use_fat;

            /* Skip the reserved1 field.  */
            read_ptr += 2;

            /* Read the name length field.  */
            name_length = *read_ptr;

            /* Check if name_length is invalid.  */
            if (name_length == 0)
            {

                /* Name length should be at least 1. Return error.  */
                return(FX_FILE_CORRUPT);
            }

            /* Advance the pointer to the name hash field.  */
            read_ptr++;

            /* Hash search */
            if (skip || (hash && (hash != _fx_utility_16_unsigned_read(read_ptr))))
            {

                /* Wrong hash, skip filename.  */
                destination_ptr -> fx_dir_entry_type =  FX_EXFAT_DIR_ENTRY_TYPE_STREAM_EXTENSION;
                name_length =  0;
                (*entry_ptr) +=  (ULONG)(secondary_count - 1);

                /* Return success.  */
                return(FX_SUCCESS);
            }

            /* Skip name hash and reserved2.  */
            read_ptr += 4;

            /* Read Valid Data Len. */
            destination_ptr -> fx_dir_entry_file_size = _fx_utility_64_unsigned_read(read_ptr);

            /* Skip ValidDataLen and reserved3.  */
            read_ptr += 12;
            destination_ptr -> fx_dir_entry_cluster = _fx_utility_32_unsigned_read(read_ptr);
            read_ptr += 4;

            /* Read Data Len.  */
            destination_ptr -> fx_dir_entry_available_file_size = _fx_utility_64_unsigned_read(read_ptr);

            /* Checks for corruption */
            if (((destination_ptr -> fx_dir_entry_available_file_size == 0)   &&
                 (destination_ptr -> fx_dir_entry_cluster       != 0))  ||
                ((destination_ptr -> fx_dir_entry_available_file_size != 0)   &&
                 (destination_ptr -> fx_dir_entry_cluster       == 0)))
            {

                /* Return file corrupt error.  */
                return(FX_FILE_CORRUPT);
            }

            /* File System Specification Ver 3.00 */
            if (destination_ptr -> fx_dir_entry_cluster == 0)
            {

                /* Don't use FAT by default.  */
                destination_ptr -> fx_dir_entry_dont_use_fat |=  1;
            }

            /* Check for directory restrictions.  */
            if ((destination_ptr -> fx_dir_entry_attributes & FX_DIRECTORY) &&
                (destination_ptr -> fx_dir_entry_available_file_size != destination_ptr -> fx_dir_entry_file_size))
            {

                /* Return file corrupt error.  */
                return(FX_FILE_CORRUPT);
            }

            /* Check if the directory size exceed file system limits.  */
            if ((destination_ptr -> fx_dir_entry_attributes & FX_DIRECTORY) &&
                (destination_ptr -> fx_dir_entry_available_file_size > FX_EXFAT_MAX_DIRECTORY_SIZE))
            {

                /* Return the no more space error.  */
                return(FX_NO_MORE_SPACE);
            }
        }
        else
        {

            /* Check if the entry type is file name directory entry.  */
            if (*read_ptr == FX_EXFAT_DIR_ENTRY_TYPE_FILE_NAME)
            {

                /* Skip the entry type and flags field.  */
                read_ptr += 2;

                /* There are 15 characters in one file name directory entry.  */
                if (name_length > 15)
                {

                    /* Set copy size to the max size of one entry.  */
                    copy_size = 15;
                }
                else
                {

                    /* Set copy size to the acutal remaining length.  */
                    copy_size = name_length;
                }

                /* Check if we are requested to return the unicode name.  */
                if (unicode_name)
                {

                    /* Loop to copy the unicode file name. from the file name directory entry.  */
                    for (j = 0; j < copy_size * 2; ++j)
                    {

                        /* Copy the unicode file name from the file name directory entry.  */
                        unicode_name[name_pos * 2 + j] = *read_ptr++;
                    }

                    /* Revert the read pointer for later use.  */
                    read_ptr -= copy_size * 2;
                }

                /* Loop to copy the non-unicode file name.  */
                for (j = 0; j < copy_size; ++j)
                {

                    /* Copy and convert the file name from the file name directory entry.  */
                    destination_ptr -> fx_dir_entry_name[name_pos++] = (CHAR)_fx_utility_16_unsigned_read(read_ptr);
                    read_ptr += 2;
                }

                /* Modify the name_length to indicate the remaining length.  */
                name_length = (UCHAR)(name_length - copy_size);
            }
            else if (((*read_ptr & (FX_EXFAT_ENTRY_TYPE_IN_USE_MASK | FX_EXFAT_ENTRY_TYPE_IMPORTANCE_MASK | FX_EXFAT_ENTRY_TYPE_CATEGORY_MASK)) ==
                      (FX_EXFAT_ENTRY_TYPE_IN_USE_MASK | FX_EXFAT_ENTRY_TYPE_CATEGORY_MASK)))
            {

                /* Unknown critical secondary, we can't work with this directory.  */
                return(FX_FILE_CORRUPT);
            }
        }
    }

    /* Terminate the entry name string.  */
    destination_ptr -> fx_dir_entry_name[name_pos] = 0;

    /* Check if we are requested to return the unicode name.  */
    if (unicode_name)
    {

        /* Terminate the unicode name string.  */
        unicode_name[name_pos * 2] = 0;
        unicode_name[name_pos * 2 + 1] = 0;

        /* Check if we are requested to return the unicode name length.  */
        if (unicode_length)
        {

            /* Return the unicode name length.  */
            *unicode_length = name_pos;
        }
    }

    /* Verify checksum.  */
    if (checksum != file_checksum)
    {

        /* Return corrupted file error.  */
        return(FX_FILE_CORRUPT);
    }

    /* Check if we have found all the file name directory entries.  */
    if (name_length != 0)
    {

        /* Return file corrupted error.  */
        return(FX_FILE_CORRUPT);
    }

    /* Return success.  */
    return(FX_SUCCESS);
}

#endif

