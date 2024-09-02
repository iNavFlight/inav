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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_exFAT_unicode_entry_write             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes the supplied directory entry to the specified  */
/*    logical sector and offset.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    entry_ptr                             Pointer to directory entry    */
/*    update_level                          Update level for entry write  */
/*    unicode_name                          Unicode file name             */
/*    unicode_length                        Length of unicode file name   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_exFAT_cluster_free        Release unused clusters       */
/*    _fx_utility_exFAT_name_hash_get       Get name hash                 */
/*    _fx_utility_FAT_entry_read            Read a new FAT entry          */
/*    _fx_utility_logical_sector_flush      Flush the written log sector  */
/*    _fx_utility_logical_sector_read       Read directory sector         */
/*    _fx_utility_logical_sector_write      Write directory sector        */
/*    _fx_utility_string_length_get         Get string's length           */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    _fx_utility_32_unsigned_write         Write a ULONG from memory     */
/*    _fx_utility_64_unsigned_read          Read a ULONG64 from memory    */
/*    _fx_fault_tolerant_add_dir_log        Add directory redo log        */
/*    _fx_fault_tolerant_add_checksum_log   Add checksum redo log         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX System Functions                                              */
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
UINT  _fx_directory_exFAT_unicode_entry_write(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr,
                                              UCHAR update_level, USHORT *unicode_name, UINT unicode_length)
{

UCHAR  *work_ptr;
UCHAR  *sector_base_ptr;
UINT    status;
UINT    i, j;
ULONG64 logical_sector;
ULONG   relative_sector;
ULONG   byte_offset;
ULONG   cluster, next_cluster;
UINT    total_entries;
UINT    name_length;
UINT    name_pos = 0;
UINT    copy_size;
USHORT  checksum = 0;
ULONG   date_time;
UINT    use_unicode = FX_FALSE;

#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR  *changed_ptr;    /* Points to the directory entry that needs to be updated. */
UINT    changed_size;   /* The amount of bytes changed */
ULONG   changed_offset; /* Offset from the beginning of the sector. */
#endif /* FX_ENABLE_FAULT_TOLERANT */



    /* Check if we have unicode file name available.  */
    if (unicode_name && unicode_length)
    {

        /* Set use unicode flag.  */
        use_unicode = FX_TRUE;

        /* Save the length of the name.  */
        name_length = unicode_length;
    }
    else
    {

        /* Calculate the length of the name from the name string.  */
        name_length = _fx_utility_string_length_get(entry_ptr -> fx_dir_entry_name, FX_MAX_EX_FAT_NAME_LEN);

        /* Check if name is valid.  */
        if (entry_ptr -> fx_dir_entry_name[name_length] != 0)
        {

            /* Invalid name, return error.  */
            return(FX_INVALID_NAME);
        }
    }

    /* Pickup the byte offset of the entry.  */
    byte_offset = entry_ptr -> fx_dir_entry_byte_offset;

    /* Pickup the logical sector of the entry.  */
    logical_sector = entry_ptr -> fx_dir_entry_log_sector;

    /* Calculate the cluster that this logical sector is in.  */
    cluster =  (ULONG)((logical_sector - media_ptr -> fx_media_data_sector_start) / (media_ptr -> fx_media_sectors_per_cluster)) + FX_FAT_ENTRY_START;

    /* Calculate the relative cluster.  */
    relative_sector =  (ULONG)(logical_sector -  (((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                  (((ULONG64)cluster - FX_FAT_ENTRY_START) *
                                                   ((ULONG)media_ptr -> fx_media_sectors_per_cluster))));

    /* Read the sector.  */
    status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                              media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

    /* Determine if an error occurred.  */
    if (status != FX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Setup a pointer into the buffer.  */
    sector_base_ptr = (UCHAR *)media_ptr -> fx_media_memory_buffer;
    work_ptr =  sector_base_ptr + (UINT)entry_ptr -> fx_dir_entry_byte_offset;

    /* Build file entry fields.  */

    /* Check if we need to update EntryType field.  */
    if (update_level == UPDATE_FULL)
    {

        /* Set EntryType field to file directory.  */
        *work_ptr = FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY;
    }
    /* Check if we are request to delete the file.  */
    else if (update_level == UPDATE_DELETE)
    {

        /* Clear InUse bit.  */
        *work_ptr &= 0x7f;
    }

    /* Advance the pointer to the next field: SecondaryCount.  */
    work_ptr++;

    /* Update SecondaryCount field.  */
    if (update_level == UPDATE_FULL)
    {

        /* Calculate the secondary entry count from file name length.  */
        total_entries = 1 + (name_length + 14) / 15;

        /* Save SecondaryCount field.  */
        *work_ptr = (UCHAR)total_entries;
    }
    else
    {

        /* We are not performing a full update, read back the secondary entry count.  */
        total_entries = *work_ptr;
    }

    /* Advance the pointer to the next field.  */
    work_ptr++;

    /* Skip SetChecksum field.  */
    work_ptr += 2;

    /* Check if we need to update FileAttributes field.  */
    if (update_level >= UPDATE_FILE)
    {

        /* Write the file attributes field.  */
        _fx_utility_16_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_attributes);
    }

    /* Advance the pointer to the next field.  */
    work_ptr += 2;

    /* Check if we need to perform full entry update.  */
    if (update_level == UPDATE_FULL)
    {

        /* Clear Reserved1 field.  */
        _fx_utility_16_unsigned_write(work_ptr, 0);
    }

    /* Advance the pointer to the next field.  */
    work_ptr += 2;

    /* Check if we need to update the CreateTimestamp field.  */
    if (update_level >= UPDATE_FILE)
    {

        /* Calculate and update CreateTimestamp field.  */
        date_time = ((ULONG)entry_ptr -> fx_dir_entry_created_date << 16) |
                     (ULONG)entry_ptr -> fx_dir_entry_created_time;
        _fx_utility_32_unsigned_write(work_ptr, date_time);
    }

    /* Advance the pointer to the next field.  */
    work_ptr += 4;

    /* Check if we need to update LastModifiedTimestamp field.  */
    if (update_level >= UPDATE_FILE)
    {

        /* Calculate and update LastModifiedTimestamp field.  */
        date_time = ((ULONG)entry_ptr -> fx_dir_entry_date << 16) |
                     (ULONG)entry_ptr -> fx_dir_entry_time;
        _fx_utility_32_unsigned_write(work_ptr, date_time);
    }

    /* Advance the pointer to the next field.  */
    work_ptr += 4;

    /* Check if we need to update LastAccessedTimestamp field.  */
    if (update_level >= UPDATE_FILE)
    {

        /* Calculate and update LastAccessedTimestamp field.  */
        date_time = ((ULONG)entry_ptr -> fx_dir_entry_date << 16) |
                     (ULONG)entry_ptr -> fx_dir_entry_time; /* use modified as accessed */
        _fx_utility_32_unsigned_write(work_ptr, date_time);
    }

    /* Advance the pointer to the next field.  */
    work_ptr += 4;

    /* Check if we need to update Create10msIncrement field.  */
    if (update_level >= UPDATE_FILE)
    {

        /* Update Create10msIncrement field.  */
        *work_ptr = entry_ptr -> fx_dir_entry_created_time_ms;
    }

    /* Advance the pointer to the next field.  */
    work_ptr++;

    /* Check if we need to perform full entry update.  */
    if (update_level >= UPDATE_FILE)
    {

        /* 10ms increment field is not supported, clear this field.  */
        *work_ptr = 0;
    }

    /* Advance the pointer to the next field.  */
    work_ptr++;

    /* Check if we need to perform full entry update.  */
    if (update_level == UPDATE_FULL)
    {

        /*CreateUtcOffset field is not supported, clear this field.  */
        *work_ptr = 0;
    }

    /* Advance the pointer to the next field.  */
    work_ptr++;

    /* Check if we need to perform full entry update.  */
    if (update_level >= UPDATE_FILE)
    {

        /* LastModifiedUtcOffset field is not supported, clear this field.  */
        *work_ptr = 0;
    }

    /* Advance the pointer to the next field.  */
    work_ptr++;

    /* Check if we need to perform full entry update.  */
    if (update_level == UPDATE_FULL)
    {

        /* LastModifiedUtcOffset field is not supported, clear this field.  */
        *work_ptr = 0; /* Not supported.  */
    }

    /* Advance the pointer to the next field.  */
    work_ptr++;

    /* Clear Reserved2 field.  */
    if (update_level == UPDATE_FULL)
    {

        /* Loop to clear the reserved field.  */
        for (i = 0; i < 7; ++i)
        {

            /* Clear the reserved field.  */
            *work_ptr = 0;
            work_ptr++;
        }
    }
    else
    {

        /* Skip the reserved field.  */
        work_ptr += 7;
    }

    /* Calculate checksum.  */
    work_ptr -= FX_DIR_ENTRY_SIZE;

    /* Loop to calculate the entry checksum.  */
    for (j = 0; j < FX_DIR_ENTRY_SIZE; ++j)
    {

        /* Skip the checksum field.  */
        if ((j == 2) || (j == 3))
        {

            continue;
        }

        /* Calculate the checksum using the algorithm described in the specification.  */
        /* Right rotate the checksum by one bit position and add the data.  */
        checksum = (USHORT)(((checksum >> 1) | (checksum << 15)) + work_ptr[j]);
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Initialize variables for fault tolerant. */
    changed_ptr = work_ptr;
    changed_offset = entry_ptr -> fx_dir_entry_byte_offset;
    changed_size = 0;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Update second entries.  */
    for (i = 0; i < total_entries; ++i)
    {

        /* Advance the pointer and offset to next directory entry.  */
        work_ptr += FX_DIR_ENTRY_SIZE;
        byte_offset += FX_DIR_ENTRY_SIZE;

#ifdef FX_ENABLE_FAULT_TOLERANT
        /* Update changed_size. */
        changed_size += FX_DIR_ENTRY_SIZE;
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Determine if the entry overlaps into the next sector.  */
        if (byte_offset >= media_ptr -> fx_media_bytes_per_sector)
        {
#ifdef FX_ENABLE_FAULT_TOLERANT
            if (media_ptr -> fx_media_fault_tolerant_enabled &&
                (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
            {

                /* Redirect this request to log file. */
                status = _fx_fault_tolerant_add_dir_log(media_ptr, logical_sector, changed_offset, changed_ptr, changed_size);
            }
            else
#endif /* FX_ENABLE_FAULT_TOLERANT */
            {

                /* Write current logical sector out.  */
                status =  _fx_utility_logical_sector_write(media_ptr, logical_sector,
                                                           sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
            }

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

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
                if ((entry_ptr -> fx_dir_entry_dont_use_fat >> 1) & 1) /* Check parent dont_use_fat flag.  */
                {

                    /* FAT is not used, next cluster is after the current cluster.  */
                    next_cluster = cluster + 1;
                }
                else
                {

                    /* FAT is used, read FAT to get the next cluster.  */
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

                /* Setup the relative sector (this is zero for subsequent cluster.  */
                relative_sector =  0;

                /* Calculate the next logical sector.  */
                logical_sector =   ((ULONG)media_ptr -> fx_media_data_sector_start) +
                    (((ULONG64)cluster - FX_FAT_ENTRY_START) *
                     ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
            }

            /* Read the sector.  */
            status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

            /* Setup logical sector.  */
            sector_base_ptr = media_ptr -> fx_media_memory_buffer;

            /* Setup a fresh byte offset.  */
            byte_offset = 0;

            /* Setup a new pointer into the buffer.  */
            work_ptr = sector_base_ptr;

#ifdef FX_ENABLE_FAULT_TOLERANT
            /* Initialize data for fault tolerant. */
            changed_ptr = work_ptr;
            changed_size = 0;
            changed_offset = 0;
#endif /* FX_ENABLE_FAULT_TOLERANT */
        }

        /* Check if we are processing the first secondary entry.  */
        if (i == 0)
        {

            /* First sub entry is stream entry.  */

            /* Check if we need to perform full entry update.  */
            if (update_level == UPDATE_FULL)
            {

                /* Set the EntryType field to stream extension type.  */
                *work_ptr = FX_EXFAT_DIR_ENTRY_TYPE_STREAM_EXTENSION;
            }
            else if (update_level == UPDATE_DELETE)
            {

                /* Clear InUse bit.  */
                *work_ptr &= 0x7f;
            }

            /* Advance the pointer to the next field.  */
            work_ptr++;

            /* Update GeneralSecondaryFlags field.  */
            if (update_level >= UPDATE_STREAM)
            {

                /* Check if FAT is used and the entry has associated cluster.  */
                if ((entry_ptr -> fx_dir_entry_dont_use_fat & 1) && (entry_ptr -> fx_dir_entry_cluster != 0))
                {

                    /* Set the flags to don't use FAT.  */
                    *work_ptr = 3;
                }
                else
                {

                    /* FAT is used.  */
                    *work_ptr = 1;
                }
            }

            /* Advance the pointer to the next field.  */
            work_ptr++;

            /* Check if we need to perform full entry update.  */
            if (update_level == UPDATE_FULL)
            {

                /* Clear Reserved1 field.  */
                *work_ptr = 0;
            }

            /* Advance the pointer to the next field.  */
            work_ptr++;

            /* Check if we need to update NameLength field.  */
            if (update_level >= UPDATE_NAME)
            {

                /* Update NameLength field.  */
                *work_ptr = (UCHAR)name_length;
            }

            /* Advance the pointer to the next field.  */
            work_ptr++;

            /* Check if we need to update NameHash field.  */
            if (update_level >= UPDATE_NAME)
            {
                /* Check if the supplied file name is in unicode format.  */
                if (use_unicode)
                {

                    /* Update NameHash field.  */
                    _fx_utility_16_unsigned_write(work_ptr, _fx_utility_exFAT_unicode_name_hash_get((CHAR *)unicode_name, name_length));
                }
                else
                {

                    /* Update NameHash field.  */
                    _fx_utility_16_unsigned_write(work_ptr, _fx_utility_exFAT_name_hash_get(entry_ptr -> fx_dir_entry_name));
                }
            }

            /* Advance the pointer to the next field.  */
            work_ptr += 2;

            /* Check if we need to perform full entry update.  */
            if (update_level == UPDATE_FULL)
            {

                /* Clear Reserved2 field.  */
                _fx_utility_16_unsigned_write(work_ptr, 0);
            }

            /* Advance the pointer to the next field.  */
            work_ptr += 2;

            /* Check if we need to update ValidDataLength field.  */
            if (update_level >= UPDATE_STREAM)
            {

                /* Update ValidDataLength field.  */
                _fx_utility_64_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_file_size);
            }

            /* Advance the pointer to the next field.  */
            work_ptr += 8;

            /* Check if we need to perform full entry update.  */
            if (update_level == UPDATE_FULL)
            {

                /* Clear Reserved3 field.  */
                _fx_utility_32_unsigned_write(work_ptr, 0);
            }

            /* Advance the pointer to the next field.  */
            work_ptr += 4;

            /* Check if we need to update FirstCluster field.  */
            if (update_level >= UPDATE_STREAM)
            {

                /* Update FirstCluster field.  */
                _fx_utility_32_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_cluster);
            }

            /* Advance the pointer to the next field.  */
            work_ptr += 4;

            /* Update DataLength fields.  */
            if (update_level >= UPDATE_STREAM)
            {

                /* Update DataLength fields.  */
                _fx_utility_64_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_available_file_size);
                _fx_utility_64_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_file_size);
            }

            /* Advance the pointer to the next field.  */
            work_ptr += 8;
        }
        else
        {

            /* Check if we are requested to delete the entries.  */
            if (update_level == UPDATE_DELETE)
            {

                /* Check the directory entry type.  */
                if ((*work_ptr != FX_EXFAT_DIR_ENTRY_TYPE_FILE_NAME) && (*work_ptr != FX_EXFAT_DIR_ENTRY_TYPE_STREAM_EXTENSION))
                {

                    /* Unknown secondary, we should free used clusters if any.  */
                    status = _fx_utility_exFAT_cluster_free(media_ptr, work_ptr);
                    if (status != FX_SUCCESS)
                    {

                        /* Return completion status.  */
                        return(status);
                    }
                }

                /* Clear InUse bit.  */
                *work_ptr &= 0x7f;
                work_ptr += FX_DIR_ENTRY_SIZE;
            }
            /* Check if we need to update the file name.  */
            else if ((update_level == UPDATE_FULL) ||
                     ((update_level == UPDATE_NAME) && (*work_ptr == FX_EXFAT_DIR_ENTRY_TYPE_FILE_NAME)))
            {

                /* Build Name entry.  */

                /* Update EntryType field.  */
                *work_ptr = FX_EXFAT_DIR_ENTRY_TYPE_FILE_NAME;
                work_ptr++;

                /* Update GeneralSecondaryFlags field.  */
                *work_ptr = 0;
                work_ptr++;

                /* Update FileName field.  */

                /* One name entry can hold up to 15 characters. Set how many characters to copy.  */
                if (name_length > 15)
                {
                    copy_size = 15;
                }
                else
                {
                    copy_size = name_length;
                }

                /* Check if the supplied file name is in unicode format.  */
                if (use_unicode)
                {

                    /* Loop to copy the unicode file name.  */
                    for (j = 0; j < copy_size; ++j)
                    {

                        /* Copy unicode file name to the file name entry.  */
                        _fx_utility_16_unsigned_write(work_ptr, (UINT)unicode_name[name_pos++]);
                        work_ptr += 2;
                    }
                }
                else
                {

                    /* Loop to copy non-unicode file name.  */
                    for (j = 0; j < copy_size; ++j)
                    {

                        /* Copy and convert non-unicode file name.  */
                        _fx_utility_16_unsigned_write(work_ptr, (UINT)entry_ptr -> fx_dir_entry_name[name_pos++]);
                        work_ptr += 2;
                    }
                }

                /* Loop to clear remaining bytes.  */
                for (j = copy_size; j < 15; ++j)
                {

                    /* Clear the remaining bytes.  */
                    _fx_utility_16_unsigned_write(work_ptr, 0);
                    work_ptr += 2;
                }

                /* Modify the name_length to indicate the remaining length.  */
                name_length -= copy_size;
            }
            else
            {

                /* The entry does need to be updated, move to next entry.  */
                work_ptr += FX_DIR_ENTRY_SIZE;
            }
        }

        /* Set pointer back for checksum calculation.  */
        work_ptr -= FX_DIR_ENTRY_SIZE;

        /* Loop to calculate the checksum.  */
        for (j = 0; j < FX_DIR_ENTRY_SIZE; ++j)
        {

            /* Calculate the checksum.  */
            /* Right rotate the checksum by one bit position and add the data.  */
            checksum = (USHORT)(((checksum >> 1) | (checksum << 15)) + work_ptr[j]);
        }
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Update changed_size. */
    changed_size += FX_DIR_ENTRY_SIZE;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Go back to file entry.  */
    if (byte_offset < total_entries * FX_DIR_ENTRY_SIZE)
    {
#ifdef FX_ENABLE_FAULT_TOLERANT
        if (media_ptr -> fx_media_fault_tolerant_enabled &&
            (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
        {

            /* Redirect this request to log file. */
            status = _fx_fault_tolerant_add_dir_log(media_ptr, logical_sector, changed_offset, changed_ptr, changed_size);
        }
        else
#endif /* FX_ENABLE_FAULT_TOLERANT */
        {

            /* Write current logical sector out.  */
            status =  _fx_utility_logical_sector_write(media_ptr, logical_sector,
                                                       sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
        }
        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Return the error status.  */
            return(status);
        }

        /* Pickup the byte offset of the entry.  */
        byte_offset = entry_ptr -> fx_dir_entry_byte_offset;

        /* Pickup the logical sector of the entry.  */
        logical_sector = entry_ptr -> fx_dir_entry_log_sector;

        /* Calculate the cluster that this logical sector is in.  */
        cluster =  (ULONG)((logical_sector - media_ptr -> fx_media_data_sector_start) / (media_ptr -> fx_media_sectors_per_cluster)) + FX_FAT_ENTRY_START;

        /* Calculate the relative cluster.  */
        relative_sector =  (ULONG)(logical_sector -  (((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                      (((ULONG64)cluster - FX_FAT_ENTRY_START) *
                                                       ((ULONG)media_ptr -> fx_media_sectors_per_cluster))));

        /* Read the sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Return the error status.  */
            return(status);
        }

        /* Setup logical sector.  */
        sector_base_ptr = media_ptr -> fx_media_memory_buffer;

#ifdef FX_ENABLE_FAULT_TOLERANT
        /* Initialize data for fault tolerant. */
        /* A log for checksum. */
        changed_ptr = sector_base_ptr + byte_offset + 2;
        changed_size = 2;
        changed_offset = byte_offset + 2;
#endif /* FX_ENABLE_FAULT_TOLERANT */
    }
    else
    {

        /* The directory entries are not crossing sector boundary, just adjust the pointer to go back to the first entry.  */
        byte_offset -= total_entries * FX_DIR_ENTRY_SIZE;
    }

    /* Setup work pointer to the directory entry.  */
    work_ptr = sector_base_ptr + byte_offset;

    /* Store SetChecksum field.  */
    work_ptr += 2;
    _fx_utility_16_unsigned_write(work_ptr, checksum);

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
    {

        if (changed_size == 2)
        {

            /* Only checksum. */
            /* Redirect this request to log file. */
            status = _fx_fault_tolerant_add_checksum_log(media_ptr, logical_sector, changed_offset, checksum);
        }
        else
        {

            /* Redirect this request to log file. */
            status = _fx_fault_tolerant_add_dir_log(media_ptr, logical_sector, changed_offset, changed_ptr, changed_size);
        }
    }
    else
#endif /* FX_ENABLE_FAULT_TOLERANT */
    {

        /* Write current logical sector out.  */
        status =  _fx_utility_logical_sector_write(media_ptr, logical_sector,
                                                   sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
    }

    /* Determine if an error occurred.  */
    if (status != FX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Check if there is a default path.  */
    if (media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name[0])
    {

        /* Check default path directory was updated.  */
        if ((media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_log_sector ==
             entry_ptr -> fx_dir_entry_log_sector) &&
            (media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_byte_offset ==
             entry_ptr -> fx_dir_entry_byte_offset))
        {

            /* Update default path.  */
            media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_file_size =
                entry_ptr -> fx_dir_entry_file_size;
            media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_available_file_size =
                entry_ptr -> fx_dir_entry_available_file_size;
            media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_dont_use_fat =
                entry_ptr -> fx_dir_entry_dont_use_fat;
            media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_cluster =
                entry_ptr -> fx_dir_entry_cluster;
        }
    }

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Determine if there is a previously found directory entry in the directory
       search cache.  */
    if (media_ptr -> fx_media_last_found_name[0])
    {

        /* Determine if the cached search directory entry matches the directory entry being
           written.  */
        if ((entry_ptr -> fx_dir_entry_log_sector == media_ptr -> fx_media_last_found_entry.fx_dir_entry_log_sector) &&
            (entry_ptr -> fx_dir_entry_byte_offset == media_ptr -> fx_media_last_found_entry.fx_dir_entry_byte_offset))
        {

            /* Yes, this entry is the same as the one currently in the directory search cache.
               Update various fields in the directory search cache with the information being
               written now.  */
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_cluster =         entry_ptr -> fx_dir_entry_cluster;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_file_size =       entry_ptr -> fx_dir_entry_file_size;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_attributes =      entry_ptr -> fx_dir_entry_attributes;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_time =            entry_ptr -> fx_dir_entry_time;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_date =            entry_ptr -> fx_dir_entry_date;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_reserved =        entry_ptr -> fx_dir_entry_reserved;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_created_time_ms = entry_ptr -> fx_dir_entry_created_time_ms;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_created_time =    entry_ptr -> fx_dir_entry_created_time;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_created_date =    entry_ptr -> fx_dir_entry_created_date;

            media_ptr -> fx_media_last_found_entry.fx_dir_entry_dont_use_fat =    entry_ptr -> fx_dir_entry_dont_use_fat;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_type =            entry_ptr -> fx_dir_entry_type;
            media_ptr -> fx_media_last_found_entry.fx_dir_entry_available_file_size = entry_ptr -> fx_dir_entry_available_file_size;
        }
    }
#endif

    /* Return success to the caller.  */
    return(status);
}

#endif

