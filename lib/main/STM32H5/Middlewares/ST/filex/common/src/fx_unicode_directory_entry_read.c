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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_directory_entry_read                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads a unicode directory entry.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    source_dir                            Pointer to source directory   */
/*    entry_ptr                             Entry index in the directory  */
/*    destination_ptr                       Destination directory         */
/*    unicode_name                          Destination unicode name      */
/*    unicode_size                          Destination unicode name size */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_logical_sector_read       Read a logical sector         */
/*    _fx_utility_16_unsigned_read          Read a 2-byte value           */
/*    _fx_utility_32_unsigned_read          Read a 4-byte value           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Unicode Utilities                                                   */
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
UINT  _fx_unicode_directory_entry_read(FX_MEDIA *media_ptr, FX_DIR_ENTRY *source_dir,
                                       ULONG *entry_ptr, FX_DIR_ENTRY *destination_ptr,
                                       UCHAR *unicode_name, ULONG *unicode_size)
{

UINT   i, j, k, card, dotflag, get_short_name;
UINT   number_of_lfns;
UINT   status;
ULONG  cluster, next_cluster = 0;
UINT   relative_cluster;
UINT   relative_sector;
ULONG  logical_sector;
ULONG  byte_offset;
ULONG  bytes_per_cluster;
UCHAR *read_ptr;
CHAR  *short_name_ptr;
ULONG  entry = *entry_ptr;
UINT   u;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of directory entry read requests.  */
    media_ptr -> fx_media_directory_entry_reads++;
#endif

    /* Calculate the byte offset of this directory entry.  */
    byte_offset =  entry * FX_DIR_ENTRY_SIZE;

    /* Determine if a sub-directory or FAT32 root directory is specified.  */
    if ((source_dir) || (media_ptr -> fx_media_32_bit_FAT))
    {

        /* Yes, a sub-directory is present.  */

        /* Calculate the number of bytes per cluster.  */
        bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
            ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

        /* Check for invalid value.  */
        if (bytes_per_cluster == 0)
        {

            /* Invalid media, return error.  */
            return(FX_MEDIA_INVALID);
        }

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

            /* Determine if this source directory has valid information from the previous call.  */
            if ((source_dir -> fx_dir_entry_last_search_cluster) &&
                (source_dir -> fx_dir_entry_last_search_relative_cluster <= relative_cluster) &&
                (source_dir -> fx_dir_entry_last_search_log_sector == source_dir -> fx_dir_entry_log_sector) &&
                (source_dir -> fx_dir_entry_last_search_byte_offset == source_dir -> fx_dir_entry_byte_offset))
            {

                /* Use the previous information to start the search.  */
                cluster =  source_dir -> fx_dir_entry_last_search_cluster;

                /* Setup the relative cluster index to the saved relative cluster.  */
                i =  source_dir -> fx_dir_entry_last_search_relative_cluster;

                /* Clear the search cluster.  It will be updated prior to successful return.  */
                source_dir -> fx_dir_entry_last_search_cluster =  0;
            }
            else
            {

                /* Nothing from the previous directory read, just setup the starting cluster to the
                   beginning of the sub-directory.  */
                cluster =  source_dir -> fx_dir_entry_cluster;

                /* Setup the relative cluster index to zero.  */
                i =  0;
            }
        }
        else
        {

            /* No, setup the starting cluster to the FAT32 root cluster.  */
            cluster =  media_ptr -> fx_media_root_cluster_32;

            /* Setup the relative cluster index to zero.  */
            i =  0;
        }

        /* Loop to position to the appropriate cluster.  */
        while (i < relative_cluster)
        {

            /* Check the value of the new cluster - it must be a valid cluster number
               or something is really wrong!  */
            if ((cluster < FX_FAT_ENTRY_START) || (cluster > media_ptr -> fx_media_fat_reserved))
            {

                /* Send error message back to caller.  */
                return(FX_FILE_CORRUPT);
            }

            /* Read the next cluster.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

            /* There is a potential for loop, but hardly anything can be done */

            /* Check for I/O error.  */
            if (status != FX_SUCCESS)
            {

                /* Return error code.  */
                return(status);
            }

            /* Setup the actual cluster.  */
            cluster = next_cluster;

            /* Increment the relative cluster number.  */
            i++;
        }

        /* At this point, the directory data sector needs to be read.  */
        logical_sector =    ((ULONG)media_ptr -> fx_media_data_sector_start) +
            (((ULONG)cluster - FX_FAT_ENTRY_START) *
             ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
            relative_sector;

        /* Read the logical directory sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Return error code.  */
            return(status);
        }

        /* Calculate the byte offset within this sector.  */
        byte_offset =  byte_offset % media_ptr -> fx_media_bytes_per_sector;
    }
    else
    {

        /* Read the entry from the root directory.  */

        /* Determine which sector the requested root directory entry is in.  */
        logical_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
            (ULONG)media_ptr -> fx_media_root_sector_start;

        /* Read the logical directory sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Return error code.  */
            return(status);
        }

        /* Set the cluster and relative variables (not used in this case) to avoid any compiler
           warnings.  */
        relative_cluster =  relative_sector =  cluster =  0;

        /* Now calculate the byte offset into this sector.  */
        byte_offset =  byte_offset -
            ((logical_sector - (ULONG)media_ptr -> fx_media_root_sector_start) *
             media_ptr -> fx_media_bytes_per_sector);
    }

    /* Setup a pointer into the buffer.  */
    read_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

    /* Save the logical sector and byte offset in the returned directory entry.  */
    destination_ptr -> fx_dir_entry_log_sector =       logical_sector;
    destination_ptr -> fx_dir_entry_byte_offset =      byte_offset;

    /* Clear the short file name information.  */
    destination_ptr -> fx_dir_entry_long_name_shorted =  0;
    destination_ptr -> fx_dir_entry_short_name[0]     =  0;

    /* Setup short name pointer.  */
    short_name_ptr =  destination_ptr -> fx_dir_entry_name;

    /* Initialize the unicode index.  */
    u =  0;

    /* Check if long file name exists.  */
    get_short_name =  0;
    if ((*(read_ptr + 11) == (UCHAR)FX_LONG_NAME) && (*read_ptr != (UCHAR)FX_DIR_ENTRY_FREE))
    {

        /* Collate the long name. */


        /* Save the number of LFN entries.  */
        number_of_lfns =  (UINT)(*read_ptr & (UCHAR)0x1f);

        if(number_of_lfns == 0)
        {
            /* Number of LFN cannot is 1-based.  Therefore it cannot be zero. */
            return(FX_FILE_CORRUPT);
        }
        else 
        {
            /* Pickup the file name length.  */
            i = (number_of_lfns - 1) * FX_LONG_NAME_ENTRY_LEN;
        }

        /* Check the file name size.  */
        if (i >= (FX_MAX_LONG_NAME_LEN - 1))
        {

            /* Name is too big, shorten it.  */
            get_short_name = 1;
            destination_ptr -> fx_dir_entry_long_name_shorted =  (UINT)(*read_ptr & (UCHAR)0x1f);
        }
        else
        {

            /* Size of name is fine, save pointer to short file name.  */
            short_name_ptr = destination_ptr -> fx_dir_entry_short_name;

            /* Loop to make sure the long file name is NULL terminated.  */
            j = i + FX_LONG_NAME_ENTRY_LEN + 1;
            do
            {
                /* Place a NULL in the long name.  */
                destination_ptr -> fx_dir_entry_name[i] =  0;

                /* Position to the next entry.  */
                i++;
            } while ((i < j) && (i < FX_MAX_LONG_NAME_LEN));
        }

        /* Loop to pickup the rest of the name.  */
        do
        {

            /* Get the lower 5 bit containing the cardinality.  */
            card = (*read_ptr) & 0x1f;

            if(card == 0)
            {

                /* Number of LFN starts from one.  Therefore it cannot be zero. */
                return(FX_FILE_CORRUPT);
            }
            else
            {
                card = card - 1;
            }
            /* For simplicity no checksum or cardinality checking is done */
            if ((get_short_name == 0) || (u))
            {

                /* Loop to pickup name.  */
                for (i = 1, j = 0, k = 0; i < FX_DIR_ENTRY_SIZE; i += 2)
                {

                    if ((i == 11) || (i == 26))
                    {
                        continue;
                    }

                    /* i = 12, 27 is not generated due to +=2 */
                    if (i == 13)
                    {
                        i = 12;
                        continue; /* this time next unicode is byte offset 14*/
                    }

                    /* Determine if there is an actual unicode character present.  */
                    if (read_ptr[i + 1])
                    {

                        /* Extended byte is non-zero, make sure both bytes of the unicode entry are not
                           all ones, since this is a normal case.  */
                        if ((read_ptr[i + 1] != (UCHAR)0xFF) || (read_ptr[i] != (UCHAR)0xFF))
                        {

                            /* Name is an actual unicode name, shorten it.  */
                            get_short_name = 1;

                            /* Save the number of directory entries the LFN has.  This will be
                               used later when updating the 8.3 portion of the LFN.  */
                            destination_ptr -> fx_dir_entry_long_name_shorted =  number_of_lfns;

                            /* Setup short name pointer.  */
                            short_name_ptr =  destination_ptr -> fx_dir_entry_name;
                        }
                    }

                    /* Determine if the character is NULL.  */
                    if (((read_ptr[i] == FX_NULL) && (read_ptr[i + 1] == FX_NULL)) ||
                        ((read_ptr[i] == (UCHAR)0xFF) && (read_ptr[i + 1] == (UCHAR)0xFF)))
                    {
                        continue;
                    }

                    /* Determine if the name is too big.  */
                    if ((card * 13 + j) >= (FX_MAX_LONG_NAME_LEN - 1))
                    {

                        /* Name is actually too big, shorten it.  */
                        get_short_name =  1;

                        /* Save the number of directory entries the LFN has.  This will be
                           used later when updating the 8.3 portion of the LFN.  */
                        destination_ptr -> fx_dir_entry_long_name_shorted =  number_of_lfns;

                        /* Also reposition the short name pointer.  */
                        short_name_ptr =  destination_ptr -> fx_dir_entry_name;

                        break;
                    }

                    /* Each entry contains 13 unicode and first byte ASCII, second byte is extended. */
                    if (get_short_name == 0)
                    {
                        destination_ptr -> fx_dir_entry_name[13 * card + j] = (CHAR)read_ptr[i];
                    }

                    /* Save the potential unicode characters.  */
                    unicode_name[k + 26 * card + j] =    read_ptr[i];
                    unicode_name[k + 26 * card + j + 1] =  read_ptr[i + 1];
                    k++;
                    u++;

                    j++;
                }
            }

            /* Determine if a new sector needs to be read.  */
            if (byte_offset + FX_DIR_ENTRY_SIZE >= media_ptr -> fx_media_bytes_per_sector)
            {

                /* Determine if a sub-directory or FAT32 root directory is specified.  */
                if ((source_dir) || (media_ptr -> fx_media_32_bit_FAT))
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

                        /* Pickup the next cluster.  */
                        status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

                        /* Check for I/O error.  */
                        if (status != FX_SUCCESS)
                        {

                            /* Return error code.  */
                            return(status);
                        }

                        /* Copy next cluster to the current cluster.  */
                        cluster =  next_cluster;

                        /* Check the value of the new cluster - it must be a valid cluster number
                           or something is really wrong!  */
                        if ((cluster < FX_FAT_ENTRY_START) || (cluster > media_ptr -> fx_media_fat_reserved))
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
                            (((ULONG)cluster - FX_FAT_ENTRY_START) *
                             ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
                    }
                }
                else
                {

                    /* Non-FAT 32 root directory.  */

                    /* Advance to the next sector.  */
                    logical_sector++;

                    /* Determine if the logical sector is valid.  */
                    if (logical_sector >= (ULONG)(media_ptr -> fx_media_root_sector_start + media_ptr -> fx_media_root_sectors))
                    {

                        /* Trying to read past root directory - send error message back to caller.  */
                        return(FX_FILE_CORRUPT);
                    }
                }

                /* Read the new sector.  */
                status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
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

            /* Calculate the next read pointer.  */
            read_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

            /* Move to the next entry.  */
            entry++;
        } while (card > 0);

        /* Set flag indicating long file name is present.  */
        destination_ptr -> fx_dir_entry_long_name_present = 1;
    }
    else
    {
        /* No long file name is present.  */
        get_short_name = 1;
    }

    /* Determine if we need to clear the long name flag.  */
    if (get_short_name == 1)
    {

        /* Clear the long name flag.  */
        destination_ptr -> fx_dir_entry_long_name_present =  0;
    }

    /* Store the unicode name size.  */
    *unicode_size =  u;

    /* Pickup the short file name.  */
    short_name_ptr[0] =  0;
    dotflag =  0;
    for (i = 0, j = 0; i < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); i++)
    {

        /* Check for a NULL.  */
        if ((CHAR)read_ptr[i] == 0)
        {
            break;
        }

        /* Check for a dot.  This happens for the first two directory entries, no
           extra dot is needed.  */
        if ((CHAR)read_ptr[i] == '.')
        {
            dotflag =  2;
        }

        /* Check for a space.  */
        if ((CHAR)read_ptr[i] == ' ')
        {
            /* Put a dot if a character comes after space.  */
            if (dotflag == 0)
            {
                dotflag =  1;
            }
            continue;
        }

        /* Check for the main short file name size.  */
        if (i == FX_DIR_NAME_SIZE)
        {
            /* Check to see if we need to insert a dot.  */
            if (dotflag == 0)
            {
                dotflag =  1;
            }
        }

        /* Check to see if we need to add a dot.  */
        if (dotflag == 1)
        {
            /* Add dot to short file name.  */
            short_name_ptr[j++] =  '.';
            dotflag =  2;    /* no more dot for spaces */
        }

        /* Copy a character.  */
        short_name_ptr[j] =  (CHAR)read_ptr[i];

        /* Increment size.  */
        j++;
    }

    /* Determine if a long file name is present and its associated short file
       name is actually free.  */
    if ((destination_ptr -> fx_dir_entry_long_name_present) && (((UCHAR)short_name_ptr[0]) == (UCHAR)FX_DIR_ENTRY_FREE))
    {

        /* Yes, the short file name is really free even though long file name entries directly precede it.
           In this case, simply place the free directory marker at the front of the long file name.  */
        destination_ptr -> fx_dir_entry_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;
        short_name_ptr[0] =  (CHAR)0;
    }

    /* Determine if the short name pointer is NULL while the read pointer is
       non-NULL.  */
    if ((short_name_ptr[0] == 0) && (read_ptr[0] == ' '))
    {

        /* This condition can occur with an all blank volume name.  Simply
           copy the volume name to the short name in this case.  */
        for (j = 0; j < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); j++)
        {

            /* Copy a byte of the volume name.  */
            short_name_ptr[j] =  (CHAR)read_ptr[j];
        }
    }

    /* Set end of string to null.  */
    short_name_ptr[j] = 0;

    /* Load up the destination directory entry.  */
    read_ptr += (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE);

    /* Copy the attribute into the destination.  */
    destination_ptr -> fx_dir_entry_attributes =  *read_ptr++;

    /* Pickup the reserved byte.  */
    destination_ptr -> fx_dir_entry_reserved =  *read_ptr++;

    /* Check for an undocumented NT file name feature for optimizing the storage
       of all lower case file names that otherwise are valid 8.3 file names. The
       following reserved bit definitions are present:

         BIT3 - set if 8.3 is all in lower case and no extended filename.
         BIT4 - set for file, clear for directory entry if no extended filename.

       This is true for all NT systems. Prior to NT follows MSDOS FAT documentation and
       is set to 0x00, all bits cleared. Therefore if BIT3 is set force lowercase.  */
    if ((get_short_name) && (destination_ptr -> fx_dir_entry_reserved & 0x08))
    {

        /* Microsoft undocumented NT file name feature... convert short name to lower
           case.  */
        for (j = 0; j <= (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE) && (short_name_ptr[j] != 0x00); j++)
        {

            /* Determine if an upper case character is present.  */
            if ((short_name_ptr[j] >= 'A') && (short_name_ptr[j] <= 'Z'))
            {

                /* Yes, an upper case character is present. Force it to lower case.  */
                short_name_ptr[j] =  (CHAR)((INT)short_name_ptr[j] + 32);
            }
        }
    }

    /* Pickup the created time in milliseconds.  */
    destination_ptr -> fx_dir_entry_created_time_ms =  *read_ptr++;

    /* Pickup the created time.  */
    destination_ptr -> fx_dir_entry_created_time =  _fx_utility_16_unsigned_read(read_ptr);
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* Pickup the created date.  */
    destination_ptr -> fx_dir_entry_created_date =  _fx_utility_16_unsigned_read(read_ptr);
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* Pickup the last accessed date.  */
    destination_ptr -> fx_dir_entry_last_accessed_date =  _fx_utility_16_unsigned_read(read_ptr);
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* read the upper 2 bytes of starting cluster - required only for 32 bit FAT */
    if (media_ptr -> fx_media_32_bit_FAT)
    {

        /* FAT32 only.  */
        destination_ptr -> fx_dir_entry_cluster =  _fx_utility_16_unsigned_read(read_ptr);
        destination_ptr -> fx_dir_entry_cluster <<= 16;
    }
    else
    {
        /* Not required for non FAT32.  */
        destination_ptr -> fx_dir_entry_cluster =  0;
    }

    /* Advance the read pointer.  */
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* Copy the time into the destination.  */
    destination_ptr -> fx_dir_entry_time =  _fx_utility_16_unsigned_read(read_ptr);
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* Copy the date into the destination.  */
    destination_ptr -> fx_dir_entry_date =  _fx_utility_16_unsigned_read(read_ptr);
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* Copy the starting cluster into the destination.  */
    destination_ptr -> fx_dir_entry_cluster +=  _fx_utility_16_unsigned_read(read_ptr);
    read_ptr =  read_ptr + 2;  /* Always 2 bytes */

    /* Copy the file size into the destination.  */
    destination_ptr -> fx_dir_entry_file_size =  _fx_utility_32_unsigned_read(read_ptr);

    /* Clear the destination search specific fields.  */
    destination_ptr -> fx_dir_entry_last_search_cluster =           0;
    destination_ptr -> fx_dir_entry_last_search_relative_cluster =  0;
    destination_ptr -> fx_dir_entry_last_search_log_sector =        0;
    destination_ptr -> fx_dir_entry_last_search_byte_offset =       0;

    /* Remember the entry number.  */
    destination_ptr -> fx_dir_entry_number =  entry;

    /* Return entry number.  */
    *entry_ptr =  entry;

    /* Determine if we should remember the last cluster and relative cluster.  */
    if (source_dir)
    {

        /* Yes, remember the last cluster and relative cluster for a subsequent call
           to read a directory entry.  */
        source_dir -> fx_dir_entry_last_search_cluster =           cluster;
        source_dir -> fx_dir_entry_last_search_relative_cluster =  relative_cluster;

        /* Also remember several other items that are unique to the directory... just to verify that the
           search information can be used.  */
        source_dir -> fx_dir_entry_last_search_log_sector =        source_dir -> fx_dir_entry_log_sector;
        source_dir -> fx_dir_entry_last_search_byte_offset =       source_dir -> fx_dir_entry_byte_offset;
    }

    /* Return success to the caller.  */
    return(FX_SUCCESS);
}

