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
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_entry_write                           PORTABLE C      */
/*                                                           6.1.5        */
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
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a new FAT entry          */
/*    _fx_utility_logical_sector_read       Read directory sector         */
/*    _fx_utility_logical_sector_write      Write directory sector        */
/*    _fx_utility_16_unsigned_write         Write a UINT from memory      */
/*    _fx_utility_32_unsigned_write         Write a ULONG from memory     */
/*    _fx_fault_tolerant_add_dir_log        Add directory redo log        */
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
/*  03-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_entry_write(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr)
{

UCHAR *work_ptr, *sector_base_ptr;
UINT   status, temp, entry, delete_flag;
UINT   i, j, k, l, card, len, dotfound, dotpos, match;
UCHAR  checksum, eof_marker;
CHAR   alpha;
CHAR   shortname[FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE + 1];
ULONG  logical_sector, relative_sector;
ULONG  byte_offset;
ULONG  cluster, next_cluster;


#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR *changed_ptr;
UINT   changed_size;
ULONG  changed_offset;
#endif /* FX_ENABLE_FAULT_TOLERANT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of directory entry write requests.  */
    media_ptr -> fx_media_directory_entry_writes++;
#endif

    /* Extended port-specific processing macro, which is by default defined to white space.  */
    FX_DIRECTORY_ENTRY_WRITE_EXTENSION

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_DIR_ENTRY_WRITE, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Determine if this is entry is being deleted.  */
    if (((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_FREE) &&
        ((UCHAR)entry_ptr -> fx_dir_entry_short_name[0] == (UCHAR)FX_DIR_ENTRY_FREE))
    {

        /* Yes, this is a request to delete the entry. Set the flag to remember this.  */
        delete_flag =  FX_TRUE;

        /* Null the short file name.  */
        entry_ptr -> fx_dir_entry_short_name[0] =  0;
    }
    else
    {

        /* Not a deleted entry. Set the flag to false.  */
        delete_flag =  FX_FALSE;
    }

    /* Pickup the byte offset of the entry.  */
    byte_offset = entry_ptr -> fx_dir_entry_byte_offset;

    /* Pickup the logical sector of the entry.  */
    logical_sector = (ULONG)entry_ptr -> fx_dir_entry_log_sector;

    /* Figure out where what cluster we are in.  */
    if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
    {

        /* Calculate the cluster that this logical sector is in.  */
        cluster =  (logical_sector - media_ptr -> fx_media_data_sector_start) / (media_ptr -> fx_media_sectors_per_cluster) + FX_FAT_ENTRY_START;

        /* Calculate the relative cluster.  */
        relative_sector =  logical_sector -  (((ULONG)media_ptr -> fx_media_data_sector_start) +
                                              (((ULONG)cluster - FX_FAT_ENTRY_START) *
                                               ((ULONG)media_ptr -> fx_media_sectors_per_cluster)));
    }
    else
    {

        /* Clear the cluster and the relative sector.  */
        cluster =  0;
        relative_sector =  0;
    }

    /* Read the logical directory sector.  */
    status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) entry_ptr -> fx_dir_entry_log_sector,
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

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Initialize data for fault tolerant. */
    changed_ptr = work_ptr;
    changed_size = 0;
    changed_offset = entry_ptr -> fx_dir_entry_byte_offset;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Determine if a long file name is present.  */
    if (entry_ptr -> fx_dir_entry_long_name_present)
    {

        /* Yes, long name is present - prepare short name and write out this name.  */
        for (len = 0, i = 0, dotpos = 0, dotfound = 0; entry_ptr -> fx_dir_entry_name[len]; len++)
        {

            /* Check for a dot.  */
            if (entry_ptr -> fx_dir_entry_name[len] == '.')
            {

                /* Check for leading dot. */
                if (len == 0)
                {
                    /* Yes, this is a leading dot. */
                    continue;
                }

                /* Yes, a dot is present.  From this position the extension will
                   be written.  */
                dotfound = i;
                dotpos   = len + 1;
                continue;
            }

            /* Check for non-space and within the short file name length.  */
            if ((entry_ptr -> fx_dir_entry_name[len] != ' ') && (i < 8))
            {

                /* Copy characters into the short file name area.  */
                shortname[i] = entry_ptr -> fx_dir_entry_name[len];
                i++;
            }
        }

        /* Fill remaining short file name with spaces.  */
        for (j = i; j < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; j++)
        {
            shortname[j] =  ' ';
        }

        /* Determine if a dot was encountered.  */
        if (dotpos)
        {

            /* Process relative to the dot position.  */
            if (entry_ptr -> fx_dir_entry_name[dotpos])
            {
                shortname[8] = entry_ptr -> fx_dir_entry_name[dotpos++];
            }
            if (entry_ptr -> fx_dir_entry_name[dotpos])
            {
                shortname[9] = entry_ptr -> fx_dir_entry_name[dotpos++];
            }
            if (entry_ptr -> fx_dir_entry_name[dotpos])
            {
                shortname[10] = entry_ptr -> fx_dir_entry_name[dotpos++];
            }

            /* Determine if additional spaces are needed.  */
            i = dotfound;

            for (; dotfound <= 7; dotfound++)
            {
                /* Add space...  */
                shortname[dotfound] = ' ';
            }
        }

        /* Each entry contains 13 unicode entries.  Calculate the remainder.  */
        if (len % 13 == 0)
        {
            card =  len / 13;
        }
        else
        {
            card =  len / 13 + 1;
        }

        /* Default the name match to true.  */
        match =  FX_TRUE;

        /* Loop through the newly derived short name and the original name and look
           for a non-matching character.  */
        l =  0;
        k =  0;
        while (k < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE)
        {

            /* Determine if a space is detected in the short name. If so,
               advance to the extension index.  */
            if (shortname[k] == ' ')
            {

                /* The first pad space was detected. First, check for a name
                   without an extension.  */
                if (entry_ptr -> fx_dir_entry_name[l] == FX_NULL)
                {

                    /* All is okay, get out of the loop!  */
                    break;
                }

                /* Now check for a period in the long name... if not, there is a non-match!  */
                if (entry_ptr -> fx_dir_entry_name[l] != '.')
                {

                    /* Set the match flag to false and exit the loop.  */
                    match =  FX_FALSE;
                    break;
                }

                /* Otherwise move short file name index to the extension area and
                   increment the long file name index.  */
                k =  8;
                l++;

                /* Restart the loop at the top.  */
                continue;
            }

            /* Check for the dot for the 8.3 match... it is no longer in the
               shortname but possibly still present in the long name.  */
            if ((k == 8) && (entry_ptr -> fx_dir_entry_name[l] == '.'))
            {

                /* Yes, handle the implicit dot in the shortname by
                   positioning past it in the long name.  */
                l++;
            }

            /* Do the names match?  */
            if (shortname[k] != entry_ptr -> fx_dir_entry_name[l])
            {

                /* No, the names do not match, set the match flag to false and
                   exit the loop.  */
                match =  FX_FALSE;
                break;
            }

            /* Move the indices forward.  */
            k++;
            l++;
        }

        /* Check if there is a dot in the name, but no extension in the short name.  In this case, 
           we should create a mangled short name.  */
        if ((dotpos) && (shortname[8] == ' '))
        {
        
            /* Something left.. the names do not match!  */
            match =  FX_FALSE;
        }

        /* One final check to make sure there is nothing left on the long file name.  */
        if (entry_ptr -> fx_dir_entry_name[l])
        {

            /* Something left.. the names do not match!  */
            match =  FX_FALSE;
        }

        /* Determine if the derived short name matches exactly the long file name. If so
           we don't need to mangle the name with a numeric value based on its entry.  */
        if (match == FX_FALSE)
        {

            /* Name does not match, create a mangled name.  */

            /* Generate short file name from LFN.  */
            entry = entry_ptr -> fx_dir_entry_number;

            /* Name suffice is between 000 and FFFF in hex, calculate this short file
               name's numeric component.  */
            entry = entry % 0x10000;

            /* Build short name of the format xxx~NNNN.ext.  */
            if (i > 3)
            {
                i = 3;
            }
            shortname[i++] = '~';

            /* Loop to build the numeric part of the name.  */
            for (l = 0; l < 4; l++)
            {
                
                /* Shift down the entry number based on the numeric position.  */
                if (l == 0)
                {
                    temp =  ((entry >> 12) & 0xf);
                }
                else if (l == 1)
                {
                     temp = ((entry >> 8) & 0xf);
                }
                else if (l == 2)
                {
                     temp = ((entry >> 4) & 0xf);
                }
                else
                {
                     temp = ((entry) & 0xf);
                }
                
                /* Now build hex value.  */
                if (temp > 9)
                    shortname[i++] =  (CHAR)('A' + (temp - 10));
                else
                    shortname[i++] =  (CHAR)('0' + temp);
            }
        }

        /* Set end of short string to NULL.   */
        shortname[11] = 0;

        /* Determine if the first character of the short file name is the directory free
           value. If so, it must be changed.  */
        if (((UCHAR)shortname[0] == (UCHAR)FX_DIR_ENTRY_FREE) && (delete_flag == FX_FALSE))
        {

            /* Change to 0x8F to be compatible with what DOS does.  */
            shortname[0] =  (CHAR)0x8F;
        }

        /* Loop to convert the new short file name to upper case.  */
        for (i = 0; i < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); i++)
        {

            /* Pickup shortname character.  */
            alpha = shortname[i];

            /* Determine if character is lower case.  */
            if ((alpha >= 'a') && (alpha <= 'z'))
            {

                /* Store the character - converted to upper case.  */
                alpha =  (CHAR)(alpha - ((CHAR)0x20));
            }

            /* Now store the short name character.  */
            shortname[i] =  alpha;
        }

        /* Determine if there already is a short name and we are not deleting the entry.  */
        if (entry_ptr -> fx_dir_entry_short_name[0] != 0)
        {

            /* Yes, override the calculated shortname with the original 8.3 name.  */

            /* Clear the short file name area.  */
            for (i = 0; i < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; i++)
            {
                shortname[i] = ' ';
            }

            /* Loop to copy the original short file name.  */
            for (i = 0, j = 0; j < FX_DIR_NAME_SIZE; i++, j++)
            {

                /* Check for end of copy conditions.  */
                if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] == '.')
                {
                    break;
                }
                if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] == 0)
                {
                    break;
                }

                /* Pickup the character.  */
                alpha =  entry_ptr -> fx_dir_entry_short_name[i];

                /* Copy file name character.  */
                shortname[j] =  alpha;
            }

            /* Determine if there is anything left in the short file name.  */
            if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] != 0)
            {

                /* Pickup remaining characters.  */
                for (i++, j = FX_DIR_NAME_SIZE; j < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; i++, j++)
                {

                    /* If NULL is encountered, stop the copying.  */
                    if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] == 0)
                    {
                        break;
                    }

                    /* Pickup the character.  */
                    alpha =  entry_ptr -> fx_dir_entry_short_name[i];

                    /* Copy file name character.  */
                    shortname[j] =  alpha;
                }
            }

            /* Loop to make sure the short name is upper case.  */
            for (j = 0; j < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); j++)
            {

                /* Pickup the character.  */
                alpha =  shortname[j];

                /* Determine if character is lower case.  */
                if ((alpha >= 'a') && (alpha <= 'z'))
                {

                    /* Store the character - converted to upper case.  */
                    alpha =  (CHAR)(alpha - ((CHAR)0x20));
                }

                /* Copy file name character.  */
                shortname[j] =  alpha;
            }

            /* Determine if the first character of the short file name is the directory free
               value. If so, it must be changed.  */
            if (((UCHAR)shortname[0]) == ((UCHAR)FX_DIR_ENTRY_FREE))
            {

                /* Change to 0x8F to be compatible with what DOS does.  */
                shortname[0] =  (CHAR)0x8F;
            }
        }

        /* Loop to calculate the checksum.  */
        for (i = checksum = 0; i < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; i++)
        {

            /* Calculate the checksum.  */
            checksum = (UCHAR)((UCHAR)(((checksum & 1) << 7) | ((checksum & (UCHAR)0xfe) >> 1)) + shortname[i]);
        }

        /* Set the last entry mark.  */
        work_ptr[0] =  (UCHAR)(0x40 | card);

        /* Loop to process remainder of long file name entry.  */
        while (card > 0)
        {

            /* Clear eof marker.  */
            eof_marker = 0;

            /* Determine if the entry is free.  */
            if ((UCHAR)shortname[0] == (UCHAR)FX_DIR_ENTRY_FREE)
            {
                /* Yes, place delete marker.  */
                work_ptr[0] =  (UCHAR)FX_DIR_ENTRY_FREE;
            }

            /* Setup various long file name fields.  */
            work_ptr[11] = FX_LONG_NAME;
            work_ptr[12] = 0;
            work_ptr[13] = checksum;
            work_ptr[26] = 0;
            work_ptr[27] = 0;

            /* Loop through file name fields.  */
            for (i = 1, j = 13 * (card - 1); i < FX_DIR_ENTRY_SIZE; i += 2)
            {

                /* Process relative to specific fields.  */
                if ((i == 11) || (i == 26))
                {
                    continue;
                }

                if (i == 13)
                {
                    i = 12;
                    continue;
                }

                /* Determine if the EOF marker is present.  */
                if (eof_marker)
                {

                    work_ptr[i] = eof_marker;
                    work_ptr[i + 1] = eof_marker;
                }
                else
                {
                    work_ptr[i] = (UCHAR)entry_ptr -> fx_dir_entry_name[j];
                    work_ptr[i + 1] = 0;
                }

                if (entry_ptr -> fx_dir_entry_name[j] == 0)
                {

                    /* end of name, pad with 0xff.  */
                    eof_marker =  (UCHAR)0xff;
                }

                j++;
            }

            /* Move to the next directory entry.  */
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
                if (media_ptr -> fx_media_fault_tolerant_enabled)
                {

                    /* Redirect this request to log file. */
                    status = _fx_fault_tolerant_add_dir_log(media_ptr, logical_sector, changed_offset, changed_ptr, changed_size);
                }
                else
                {
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Write current logical sector out.  */
                    status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                               sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
#ifdef FX_ENABLE_FAULT_TOLERANT
                }
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Determine if we are in the root directory.  */
                if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
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
                        if ((cluster < FX_FAT_ENTRY_START) || (cluster >= media_ptr -> fx_media_fat_reserved))
                        {

                            /* Send error message back to caller.  */
                            return(FX_FILE_CORRUPT);
                        }

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

                    /* Increment the logical sector.  */
                    logical_sector++;

                    /* Determine if the logical sector is valid.  */
                    if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
                    {

                        /* We have exceeded the root directory.  */

                        /* Send error message back to caller.  */
                        return(FX_FILE_CORRUPT);
                    }
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

            /* Decrement loop control.  */
            card--;
            work_ptr[0] = (UCHAR)card;
        }

        /* Determine if there is a short name.  */
        if (entry_ptr -> fx_dir_entry_short_name[0] == 0)
        {

            /* Loop to copy the new short file name.  */
            for (i = 0; i < (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE); i++)
            {

                /* Pickup shortname character.  */
                alpha = shortname[i];

                /* Now store the short name character.  */
                *work_ptr++ =  (UCHAR)alpha;
            }
        }
        else
        {

            /* Clear the short file name area.  */
            for (i = 0; i < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; i++)
            {
                work_ptr[i] = ' ';
            }

            /* Loop to copy the old short file name.  */
            for (i = 0, j = 0; j < FX_DIR_NAME_SIZE; i++, j++)
            {

                /* Check for end of copy conditions.  */
                if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] == '.')
                {
                    break;
                }
                if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] == 0)
                {
                    break;
                }

                /* Copy file name character.  */
                work_ptr[j] =  (UCHAR)entry_ptr -> fx_dir_entry_short_name[i];
            }

            /* Determine if there is anything left in the short file name.  */
            if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] != 0)
            {

                /* Pickup remaining characters.  */
                for (i++, j = FX_DIR_NAME_SIZE; j < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; i++, j++)
                {

                    /* If NULL is encountered, stop the copying.  */
                    if ((UCHAR)entry_ptr -> fx_dir_entry_short_name[i] == 0)
                    {
                        break;
                    }

                    /* Copy file name character.  */
                    work_ptr[j] =  (UCHAR)entry_ptr -> fx_dir_entry_short_name[i];
                }
            }

            /* Adjust the work pointer accordingly.  */
            work_ptr += (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE);
        }
    }
    else
    {

        /* Determine if long name was shorted.  */
        if (entry_ptr -> fx_dir_entry_long_name_shorted > 0)
        {

            /* Check for a valid short name.  */
            if ((UCHAR)(0x40 | entry_ptr -> fx_dir_entry_long_name_shorted) == (UCHAR)(*work_ptr))
            {

                /* Loop through the file name.  */
                for (j = 0; j < entry_ptr -> fx_dir_entry_long_name_shorted; j++)
                {

                    /* Check for a free entry to be written.  */
                    if ((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_FREE)
                    {
                        /* Delete long parts.  */
                        work_ptr[0] =  (UCHAR)FX_DIR_ENTRY_FREE;
                    }

                    /* Setup pointers for the name write.  */
                    work_ptr += FX_DIR_ENTRY_SIZE;
                    byte_offset += FX_DIR_ENTRY_SIZE;

#ifdef FX_ENABLE_FAULT_TOLERANT
                    /* Update changed_size. */
                    changed_size += FX_DIR_ENTRY_SIZE;
#endif /* FX_ENABLE_FAULT_TOLERANT */

                    /* Determine if the write is within the current sector.   */
                    if (byte_offset >= media_ptr -> fx_media_bytes_per_sector)
                    {
#ifdef FX_ENABLE_FAULT_TOLERANT
                        if (media_ptr -> fx_media_fault_tolerant_enabled)
                        {

                            /* Redirect this request to log file. */
                            status = _fx_fault_tolerant_add_dir_log(media_ptr, (ULONG64) logical_sector, changed_offset, changed_ptr, changed_size);
                        }
                        else
                        {
#endif /* FX_ENABLE_FAULT_TOLERANT */

                            /* Write the current sector out.  */
                            status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                                       sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
#ifdef FX_ENABLE_FAULT_TOLERANT
                        }
#endif /* FX_ENABLE_FAULT_TOLERANT */

                        /* Determine if an error occurred.  */
                        if (status != FX_SUCCESS)
                        {

                            /* Return the error status.  */
                            return(status);
                        }

                        /* Determine if we are in the root directory.  */
                        if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
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
                                if ((cluster < FX_FAT_ENTRY_START) || (cluster >= media_ptr -> fx_media_fat_reserved))
                                {

                                    /* Send error message back to caller.  */
                                    return(FX_FILE_CORRUPT);
                                }

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

                            /* Increment the logical sector.  */
                            logical_sector++;

                            /* Determine if the logical sector is valid.  */
                            if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
                            {

                                /* We have exceeded the root directory.  */

                                /* Send error message back to caller.  */
                                return(FX_FILE_CORRUPT);
                            }
                        }

                        /* Read the next logical sector.  */
                        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

                        /* Determine if an error occurred.  */
                        if (status != FX_SUCCESS)
                        {

                            /* Return the error status.  */
                            return(status);
                        }

                        /* Move to the next sector buffer.  */
                        sector_base_ptr = media_ptr -> fx_media_memory_buffer;

                        /* Setup new buffer pointers.  */
                        byte_offset =  0;
                        work_ptr = sector_base_ptr;

#ifdef FX_ENABLE_FAULT_TOLERANT
                        /* Initialize data for fault tolerant. */
                        changed_ptr = work_ptr;
                        changed_size = 0;
                        changed_offset = 0;
#endif /* FX_ENABLE_FAULT_TOLERANT */
                    }
                }
            }
        }

        /* This is an 8.3 name.  First clear the directory name.  */
        for (j = 0; j < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; j++)
        {
            work_ptr[j] = ' ';
        }

        /* Copy leading dots in case of first two entries of a directory.  */
        for (i = 0; (UCHAR)entry_ptr -> fx_dir_entry_name[i] == '.'; i++)
        {
            work_ptr[i] = '.';
        }

        /* Determine if there are more characters to copy.  */
        if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] != 0)
        {

            /* Copy directory name.  */
            for (i = 0, j = 0; j < FX_DIR_NAME_SIZE; i++, j++)
            {

                /* Check for end of copy conditions.  */
                if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] == '.')
                {
                    break;
                }
                if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] == 0)
                {
                    break;
                }

                /* Pickup shortname character.  */
                alpha = entry_ptr -> fx_dir_entry_name[i];

                /* Determine if character is lower case.  */
                if ((alpha >= 'a') && (alpha <= 'z'))
                {

                    /* Store the character - converted to upper case.  */
                    alpha =  (CHAR)(alpha - ((CHAR)0x20));
                }

                /* Copy a name character.  */
                work_ptr[j] =  (UCHAR)alpha;
            }
        }

        /* Determine if there are more characters in the name.  */
        if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] != 0)
        {

            /* Loop to copy the remainder of the name.  */
            for (i++, j = FX_DIR_NAME_SIZE; j < FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE; i++, j++)
            {

                /* Check for end of copy conditions.  */
                if ((UCHAR)entry_ptr -> fx_dir_entry_name[i] == 0)
                {
                    break;
                }

                /* Pickup shortname character.  */
                alpha = entry_ptr -> fx_dir_entry_name[i];

                /* Determine if character is lower case.  */
                if ((alpha >= 'a') && (alpha <= 'z'))
                {

                    /* Store the character - converted to upper case.  */
                    alpha =  (CHAR)(alpha - ((CHAR)0x20));
                }

                /* Copy a name character.  */
                work_ptr[j] =  (UCHAR)alpha;
            }
        }

        /* Move to the next entry.  */
        work_ptr += (FX_DIR_NAME_SIZE + FX_DIR_EXT_SIZE);
    }

    /* Write out the 8.3 part of the name. */

    /* Copy the attribute into the destination.  */
    *work_ptr++ =  entry_ptr -> fx_dir_entry_attributes;

    /* Copy the reserved byte.  */
    *work_ptr++ =  entry_ptr -> fx_dir_entry_reserved;

    /* Copy the created time in milliseconds.  */
    *work_ptr++ =  entry_ptr -> fx_dir_entry_created_time_ms;

    /* Copy the created time.  */
    _fx_utility_16_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_created_time);
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Copy the created date.  */
    _fx_utility_16_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_created_date);
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Copy the last accessed date.  */
    _fx_utility_16_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_last_accessed_date);
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Determine if a FAT32 entry is present.  */
    if (media_ptr -> fx_media_32_bit_FAT)
    {

        /* Yes, FAT32 is present, store upper half of cluster.  */
        temp = (entry_ptr -> fx_dir_entry_cluster >> 16);
        _fx_utility_16_unsigned_write(work_ptr, temp);
    }
    else
    {

        /* No, FAT16 or FAT12 is present, just write a 0 for
           the upper half of the cluster.  */
        _fx_utility_16_unsigned_write(work_ptr, 0);
    }

    /* Advance the entry pointer.  */
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Copy the time into the destination.  */
    _fx_utility_16_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_time);
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Copy the date into the destination.  */
    _fx_utility_16_unsigned_write(work_ptr, entry_ptr -> fx_dir_entry_date);
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Copy the starting cluster into the destination.  */
    _fx_utility_16_unsigned_write(work_ptr, (UINT)entry_ptr -> fx_dir_entry_cluster);
    work_ptr =  work_ptr + 2;  /* Always 2 bytes  */

    /* Copy the file size into the destination.  */
    _fx_utility_32_unsigned_write(work_ptr, (ULONG)entry_ptr -> fx_dir_entry_file_size);

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Update changed_size. */
    changed_size += FX_DIR_ENTRY_SIZE;

    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
    {

        /* Redirect this request to log file. */
        status = _fx_fault_tolerant_add_dir_log(media_ptr, (ULONG64) logical_sector, changed_offset, changed_ptr, changed_size);
    }
    else
    {
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Write the directory sector to the media.  */
        status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                   sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
#ifdef FX_ENABLE_FAULT_TOLERANT
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Determine if an error occurred.  */
    if (status != FX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
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
        }
    }
#endif

    /* Return success to the caller.  */
    return(FX_SUCCESS);
}

