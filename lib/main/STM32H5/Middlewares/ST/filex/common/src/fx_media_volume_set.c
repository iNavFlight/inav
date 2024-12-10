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
/**   Media                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_media.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"
#endif /* FX_ENABLE_EXFAT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_volume_set                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the volume name to the name supplied by the      */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    volume_name                           New volume name               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_read              Read a directory entry        */
/*    _fx_utility_logical_sector_read       Read directory sector         */
/*    _fx_utility_logical_sector_write      Write directory sector        */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),updated   */
/*                                            the initialization of       */
/*                                            dir_entry for exFAT format, */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_volume_set(FX_MEDIA *media_ptr, CHAR *volume_name)
{

ULONG        i, j;
FX_DIR_ENTRY dir_entry, dir_entry1;
UINT         status, offset;
UCHAR       *work_ptr;
CHAR         alpha;


    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    dir_entry.fx_dir_entry_log_sector = 0;
    dir_entry.fx_dir_entry_byte_offset = 0;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_VOLUME_SET, media_ptr, volume_name, 0, 0, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* Setup pointer to media name buffer.  */
        dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer;

        /* Clear the short name string.  */
        dir_entry.fx_dir_entry_short_name[0] =  0;

        /* Attempt to find the volume name in the root directory.  */
        i =  0;
        do
        {

            /* Read an entry from the root directory.  */
            status =  _fx_directory_entry_read(media_ptr, FX_NULL, &i, &dir_entry);

            /* Check for error status.  */
            if (status != FX_SUCCESS)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return to caller.  */
                return(status);
            }

            /* Check for a volume name.  */
            if (dir_entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_VOLUME_LABEL)
            {

                /* Yes, we have found a previously set volume name.  */
                break;
            }

            /* Move to next directory entry.  */
            i++;
        } while (i < media_ptr -> fx_media_root_directory_entries);

        /* Determine if a volume entry has been found.  */
        if (i == media_ptr -> fx_media_root_directory_entries)
        {

            /* Volume entry not found, attempt to find a free entry in the root directory.  */
            i =  0;
            do
            {

                /* Read an entry from the root directory.  */
                status =  _fx_directory_entry_read(media_ptr, FX_NULL, &i, &dir_entry);

                /* Check for error status.  */
                if (status != FX_SUCCESS)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return to caller.  */
                    return(status);
                }

                /* Check for a volume name.  */
                if (dir_entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_FREE)
                {

                    /* Yes, we have found a free directory entry.  */
                    break;
                }

                /* Move to next directory entry.  */
                i++;
            } while (i < media_ptr -> fx_media_root_directory_entries);

            /* Determine if a free entry has been found.  */
            if (i == media_ptr -> fx_media_root_directory_entries)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* No existing volume name was found, return an error.  */
                return(FX_NOT_FOUND);
            }
        }

        /* Read the logical directory sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) dir_entry.fx_dir_entry_log_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return error code.  */
            return(status);
        }

        /* Offset to volume label field.  */
        offset = dir_entry.fx_dir_entry_byte_offset + FX_EXFAT_VOLUME_LABEL;

        /* Loop to store the volume name.  */
        for (i = 0; volume_name[i]; i++)
        {

            /* Have we reached the end?  */
            if (i == 11)
            {

                break;
            }

            /* Pickup volume name byte.  */
            alpha =  volume_name[i];

            /* Store a byte of the volume name.  */
            _fx_utility_16_unsigned_write(&media_ptr -> fx_media_memory_buffer[offset], (UINT)alpha);

            /* Move to next character.  */
            offset += 2;
        }

        /* Offset to character count field.  */
        offset = dir_entry.fx_dir_entry_byte_offset;

        /* Store volume label entry type.  */
        media_ptr -> fx_media_memory_buffer[offset] = FX_EXFAT_DIR_ENTRY_TYPE_VOLUME_LABEL;

        /* Store the character count.  */
        media_ptr -> fx_media_memory_buffer[offset + FX_EXFAT_CHAR_COUNT] = (UCHAR)i;

        /* Write the directory sector to the media.  */
        status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) dir_entry.fx_dir_entry_log_sector,
                                                   media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        /* First, check for an invalid volume name.  */
        if (volume_name[0] == 0)
        {

            /* Yes, volume name is invalid.  Return an error.  */
            return(FX_INVALID_NAME);
        }

        /* Read the logical directory sector 0 - we just do this to get a memory_buffer pointer */
        status =  _fx_utility_logical_sector_read(media_ptr, ((ULONG64) 0),
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

        /* Check the read status.  */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the error status.  */
            return(status);
        }

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver read boot sector requests.  */
        media_ptr -> fx_media_driver_boot_read_requests++;
#endif

        /* Build a driver request to read the boot record.  */
        media_ptr -> fx_media_driver_request =      FX_DRIVER_BOOT_READ;
        media_ptr -> fx_media_driver_status =       FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =       media_ptr -> fx_media_memory_buffer;
        media_ptr -> fx_media_driver_sectors =      1;
        media_ptr -> fx_media_driver_sector_type =  FX_BOOT_SECTOR;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_BOOT_READ, media_ptr, media_ptr -> fx_media_memory_buffer, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to read the boot sector.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Determine if the request is successful.  */
        if (media_ptr -> fx_media_driver_status)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* An error occurred in the driver.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* Calculate the offset based on the FAT present.  */
        if (media_ptr -> fx_media_32_bit_FAT)
        {

            /* FAT32 is present.  */
            offset =  FX_VOLUME_LABEL_32;
        }
        else
        {

            /* FAT12/16 is present.  */
            offset =  FX_VOLUME_LABEL;
        }

        /* Loop to store the volume name.  */
        for (i = 0; volume_name[i]; i++)
        {

            /* Have we reached the end?  */
            if (i == 11)
            {

                break;
            }

            /* Pickup volume name byte.  */
            alpha =  volume_name[i];

            /* Determine if alpha needs to be converted to upper case.  */
            if ((alpha >= 'a') && (alpha <= 'z'))
            {

                /* Convert alpha to upper case.  */
                alpha =  (CHAR)((INT)alpha - 0x20);
            }

            /* Store a byte of the volume name.  */
            media_ptr -> fx_media_memory_buffer[offset + i] =  (UCHAR)alpha;
        }

        /* Now pad with spaces.  */
        for (; i < 11; i++)
        {

            /* Append space character to volume name.  */
            media_ptr -> fx_media_memory_buffer[offset + i] =  0x20;
        }

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver write boot sector requests.  */
        media_ptr -> fx_media_driver_boot_write_requests++;
#endif

        /* Write the boot sector with the new volume name.  */
        media_ptr -> fx_media_driver_request =      FX_DRIVER_BOOT_WRITE;
        media_ptr -> fx_media_driver_status =       FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =       media_ptr -> fx_media_memory_buffer;
        media_ptr -> fx_media_driver_sectors =      1;
        media_ptr -> fx_media_driver_sector_type =  FX_BOOT_SECTOR;

        /* Set the system write flag since we are writing the boot sector.  */
        media_ptr -> fx_media_driver_system_write =  FX_TRUE;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_BOOT_WRITE, media_ptr, media_ptr -> fx_media_memory_buffer, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to write the boot sector.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Clear the system write flag.  */
        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

        /* Determine if the request is successful.  */
        if (media_ptr -> fx_media_driver_status)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* An error occurred in the driver.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* Setup pointer to media name buffer.  */
        dir_entry1.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer;

        /* Clear the short name string.  */
        dir_entry1.fx_dir_entry_short_name[0] =  0;

        /* Now we need to find the copy of the volume name in the root directory.  */
        i =  0;
        j =  media_ptr -> fx_media_root_directory_entries + 1;
        do
        {

            /* Read an entry from the root directory.  */
            status =  _fx_directory_entry_read(media_ptr, FX_NULL, &i, &dir_entry1);

            /* Check for error status.  */
            if (status != FX_SUCCESS)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return to caller.  */
                return(status);
            }

            /* Determine if this is an empty entry.  */
            if ((dir_entry1.fx_dir_entry_name[0] == (CHAR)FX_DIR_ENTRY_FREE) && (dir_entry1.fx_dir_entry_short_name[0] == 0))
            {

                /* Yes, this is free entry.  Is it the first?  */
                if (i < j)
                {

                    /* Yes, first free entry - remember it.  */
                    dir_entry =  dir_entry1;
                    j =  i;
                }
            }
            /* Determine if the directory entries are exhausted.  */
            else if (dir_entry1.fx_dir_entry_name[0] == FX_DIR_ENTRY_DONE)
            {

                /* Yes, this we are at the end of the directory.  Have there
                been any other free entries?  */
                if (i < j)
                {

                    /* No, we need to remember this as the free entry.  */
                    dir_entry =  dir_entry1;
                    j =  i;
                }
                break;
            }
            /* Check for a volume name.  */
            else if (dir_entry1.fx_dir_entry_attributes & FX_VOLUME)
            {

                /* Yes, we have found a previously set volume name - use this entry.  */
                dir_entry =  dir_entry1;
                j =  i;
                break;
            }

            /* Move to next directory entry.  */
            i++;
        } while (i < media_ptr -> fx_media_root_directory_entries);

        /* Determine if a volume entry was not found and there are no more empty slots.  */
        if (i == media_ptr -> fx_media_root_directory_entries)
        {

            /* Determine if there was a free or previous volume name.  */
            if (j == (media_ptr -> fx_media_root_directory_entries + 1))
            {
        
                /* No, nothing was available in the root directory.  */

                /* Release media protection.  */
                FX_UNPROTECT

                /* No, existing volume name or space in the root directly was found, return an error.  */
                return(FX_MEDIA_INVALID);
            }
        }

        /* Now set the volume name and attribute.  */

        /* Read the logical directory sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) dir_entry.fx_dir_entry_log_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_BOOT_SECTOR);

        /* Check the status of reading the directory entry. */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the error status.  */
            return(status);
        }

        /* Calculate pointer into directory buffer.  */
        work_ptr =  media_ptr -> fx_media_memory_buffer +
            (UINT)dir_entry.fx_dir_entry_byte_offset;

        /* Copy the volume name into the directory entry.  */
        for (i = 0; volume_name[i]; i++)
        {

            /* Have we reached the end?  */
            if (i == 11)
            {

                break;
            }

            /* Pickup volume name byte.  */
            alpha =  volume_name[i];

            /* Determine if alpha needs to be converted to upper case.  */
            if ((alpha >= 'a') && (alpha <= 'z'))
            {

                /* Convert alpha to upper case.  */
                alpha =  (CHAR)((INT)alpha - 0x20);
            }

            /* Store volume name.  */
            work_ptr[i] =  (UCHAR)alpha;
        }

        /* Pad with space characters.  */
        for (; i < 11; i++)
        {
            work_ptr[i] =  0x20;
        }

        /* Set the appropriate attributes.  */
        work_ptr[11] =  FX_VOLUME | FX_ARCHIVE;

        /* Set the other parts of the volume entry.  */

        /* Clear the hi word of cluster.  */
        work_ptr[20] =  0;
        work_ptr[21] =  0;

        /* Clear the low word of cluster.  */
        work_ptr[26] =  0;
        work_ptr[27] =  0;

        /* Clear the file size.  */
        work_ptr[28] =  0;
        work_ptr[29] =  0;
        work_ptr[30] =  0;
        work_ptr[31] =  0;

        /* Write the directory sector to the media.  */
        status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) dir_entry.fx_dir_entry_log_sector,
                                                   media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return the status.  */
    return(status);
}

