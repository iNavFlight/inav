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
/*    _fx_media_volume_get_extended                       PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the volume name stored in the media's boot      */
/*    record or root directory.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    volume_name                           Pointer to destination for    */
/*                                            the volume name (maximum    */
/*                                            11 characters + NULL)       */
/*    volume_name_buffer_length             Buffer length for volume_name */
/*    volume_source                         Source of volume              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_logical_sector_read       Read directory sector         */
/*    _fx_directory_entry_read              Directory entry read          */
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
/*  04-25-2022     Bhupendra Naphade        Modified comment(s), and      */
/*                                            updated check for           */
/*                                            volume name,                */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_volume_get_extended(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_name_buffer_length, UINT volume_source)
{

UINT         status, offset;
ULONG        i;
INT          j;

#ifdef FX_ENABLE_EXFAT
ULONG        character_count;
#endif /* FX_ENABLE_EXFAT */
FX_DIR_ENTRY dir_entry;


    /* Clear the volume name.  */
    volume_name[0] =  FX_NULL;

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_VOLUME_GET, media_ptr, volume_name, volume_source, 0, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        volume_source = FX_DIRECTORY_SECTOR;
    }
#endif /* FX_ENABLE_EXFAT */

    /* Ensure the volume name is NULL initially.  */
    volume_name[0] =  FX_NULL;

    if (volume_source == FX_DIRECTORY_SECTOR)
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
#ifdef FX_ENABLE_EXFAT
            if (((media_ptr -> fx_media_FAT_type == FX_exFAT) && ((dir_entry.fx_dir_entry_attributes & FX_VOLUME) || (dir_entry.fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_VOLUME_LABEL))) 
                    || ((media_ptr -> fx_media_FAT_type != FX_exFAT) && (dir_entry.fx_dir_entry_attributes & FX_VOLUME) && ((UCHAR)dir_entry.fx_dir_entry_name[0] != (UCHAR)FX_DIR_ENTRY_FREE)))
#else
            if ((dir_entry.fx_dir_entry_attributes & FX_VOLUME) && ((UCHAR)dir_entry.fx_dir_entry_name[0] != (UCHAR)FX_DIR_ENTRY_FREE))
#endif /* FX_ENABLE_EXFAT */
            {

                /* Yes, we have found a previously set volume name.  */
                break;
            }

            /* Move to next directory entry.  */
            i++;
        } while (i < media_ptr -> fx_media_root_directory_entries);

        /* Determine if a volume entry has been found.  */
        if (i < media_ptr -> fx_media_root_directory_entries)
        {

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

            /* Offset to volume label entry.  */
            offset = dir_entry.fx_dir_entry_byte_offset;

#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {

                /* Read character count of volume label.  */
                character_count = media_ptr -> fx_media_memory_buffer[offset + FX_EXFAT_CHAR_COUNT];

                /* Check if the buffer is too short for the name.  */
                if (character_count >= volume_name_buffer_length)
                {

                    /* Buffer too short, return error.  */
                    status =  FX_BUFFER_ERROR;

                    /* Set character count to fit for the buffer.  */
                    character_count = volume_name_buffer_length - 1;
                }

                /* Move to volume label field.  */
                offset += FX_EXFAT_VOLUME_LABEL;

                for (i = 0; i < character_count; i++)
                {

                    /* Read one character of volume label.  */
                    volume_name[i] = (CHAR)_fx_utility_16_unsigned_read(&media_ptr -> fx_media_memory_buffer[offset]);

                    /* Move to next character.  */
                    offset += 2;
                }

                /* NULL terminate the volume name.  */
                volume_name[i] =  FX_NULL;
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */

                /* Skip trailing space characters of volume name. */
                for (j = 10; j >= 0; j--)
                {

                    /* Check for space character.  */
                    if (media_ptr -> fx_media_memory_buffer[offset + (UINT)j] != (UCHAR)' ')
                    {

                        /* Last character found. */
                        break;
                    }
                }

                /* Check if the buffer is too short for the name.  */
                if (j >= (INT)volume_name_buffer_length - 1)
                {

                    /* Buffer too short, return error.  */
                    status =  FX_BUFFER_ERROR;

                    /* Set character count to fit for the buffer.  */
                    j = (INT)volume_name_buffer_length - 2;
                }

                /* NULL terminate the volume name.  */
                volume_name[j + 1] =  FX_NULL;

                /* Pickup the remaining characters of the volume name from the boot sector.  */
                for (; j >= 0; j--)
                {

                    /* Pickup byte of volume name.  */
                    volume_name[j] =  (CHAR)media_ptr -> fx_media_memory_buffer[offset + (UINT)j];
                }
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the completion status.  */
            return(status);
        }
    }

    /* Read volume name from boot record.  */
    /* Read the logical directory sector 0 - we just do this to get a memory_buffer pointer */
    status =  _fx_utility_logical_sector_read(media_ptr, ((ULONG64) 0),
                                              media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DATA_SECTOR);

    /* Check the return status.  */
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

    /* Build the driver request to read the boot record.  */
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

    /* Calculate the offset to the volume name based on the type of FAT.  */
    if (media_ptr -> fx_media_32_bit_FAT)
    {

        /* FAT32 offset to volume name.  */
        offset =  FX_VOLUME_LABEL_32;
    }
    else
    {

        /* FAT12/16 offset to volume name.  */
        offset =  FX_VOLUME_LABEL;
    }

    /* Skip trailing space characters of volume name. */
    for (j = 10; j >= 0; j--)
    {

        /* Check for space character.  */
        if (media_ptr -> fx_media_memory_buffer[offset + (UINT)j] != (UCHAR)' ')
        {

            /* Last character found. */
            break;
        }
    }
    
    /* Check if the buffer is too short for the name.  */
    if (j >= (INT)volume_name_buffer_length - 1)
    {

        /* Buffer too short, return error.  */
        status =  FX_BUFFER_ERROR;

        /* Set character count to fit for the buffer.  */
        j = (INT)volume_name_buffer_length - 2;
    }

    /* NULL terminate the volume name.  */
    volume_name[j + 1] =  FX_NULL;

    /* Pickup the remaining characters of the volume name from the boot sector.  */
    for (; j >= 0; j--)
    {

        /* Pickup byte of volume name.  */
        volume_name[j] =  (CHAR)media_ptr -> fx_media_memory_buffer[offset + (UINT)j];
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return the completion status.  */
    return(status);
}

