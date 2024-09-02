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
#include "fx_media.h"
#include "fx_file.h"
#include "fx_directory.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_close                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function examines the list of open files for this media and    */
/*    closes each file.  If a file has been written to, the file's        */
/*    directory information is also written out to the media.  After      */
/*    the files have been closed, the internal logical sector is          */
/*    flushed and a flush command is sent to the attached driver.         */
/*    Finally, this media control block is removed from the list of       */
/*    opened media control blocks and is marked as closed.                */
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
/*    _fx_directory_entry_write             Write the directory entry     */
/*    _fx_media_abort                       Abort the media on error      */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
/*    _fx_utility_FAT_flush                 Flush cached FAT entries      */
/*    _fx_utility_FAT_map_flush             Flush primary FAT changes to  */
/*                                            secondary FAT(s)            */
/*    _fx_utility_logical_sector_flush      Flush logical sector cache    */
/*    _fx_utility_16_unsigned_read          Read a 16-bit value           */
/*    _fx_utility_32_unsigned_read          Read a 32-bit value           */
/*    _fx_utility_32_unsigned_write         Write a 32-bit value          */
/*    tx_mutex_delete                       Delete protection mutex       */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable file close          */
/*                                            and cache,                  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_close(FX_MEDIA  *media_ptr)
{

FX_INT_SAVE_AREA

#ifndef FX_DISABLE_FILE_CLOSE
ULONG    open_count;
FX_FILE *file_ptr;
#endif /* FX_DISABLE_FILE_CLOSE */
UINT     status;


    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_CLOSE, media_ptr, 0, 0, 0, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* If trace is enabled, unregister this object.  */
    FX_TRACE_OBJECT_UNREGISTER(media_ptr)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifndef FX_DISABLE_FILE_CLOSE
    /* Loop through the media's open files.  */
    open_count =  media_ptr -> fx_media_opened_file_count;
    file_ptr =    media_ptr -> fx_media_opened_file_list;
    while (open_count)
    {

        /* Look at each opened file to see if the same file is opened
           for writing and has been written to.  */
        if ((file_ptr -> fx_file_open_mode == FX_OPEN_FOR_WRITE) &&
            (file_ptr -> fx_file_modified))
        {

            /* Lockout interrupts for time/date access.  */
            FX_DISABLE_INTS

            /* Set the new time and date.  */
            file_ptr -> fx_file_dir_entry.fx_dir_entry_time =  _fx_system_time;
            file_ptr -> fx_file_dir_entry.fx_dir_entry_date =  _fx_system_date;

            /* Restore interrupt posture.  */
            FX_RESTORE_INTS

            /* Copy the new file size into the directory entry.  */
            file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =
                file_ptr -> fx_file_current_file_size;

            /* Write the directory entry to the media.  */
#ifdef FX_ENABLE_EXFAT
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {

                status = _fx_directory_exFAT_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry), UPDATE_STREAM);
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */
                status = _fx_directory_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry));
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Determine if the status was unsuccessful. */
            if (status != FX_SUCCESS)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* Call the media abort routine.  */
                _fx_media_abort(media_ptr);

                /* Return the error status.  */
                return(FX_IO_ERROR);
            }

            /* Clear the file modified flag.  */
            file_ptr -> fx_file_modified =  FX_FALSE;
        }

        /* Mark the file as closed.  */
        file_ptr -> fx_file_id =  FX_FILE_CLOSED_ID;

        /* Adjust the pointer and decrement the opened count.  */
        file_ptr =  file_ptr -> fx_file_opened_next;
        open_count--;
    }
#endif /* FX_DISABLE_FILE_CLOSE */

    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);

    /* Flush changed sector(s) in the primary FAT to secondary FATs.  */
    _fx_utility_FAT_map_flush(media_ptr);

#ifdef FX_ENABLE_EXFAT
    if ((media_ptr -> fx_media_FAT_type == FX_exFAT) &&
        (FX_TRUE == media_ptr -> fx_media_exfat_bitmap_cache_dirty))
    {

        /* Flush bitmap.  */
        _fx_utility_exFAT_bitmap_flush(media_ptr);
    }
#endif /* FX_ENABLE_EXFAT */

    /* Flush the internal logical sector cache.  */
    status =  _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64) (media_ptr -> fx_media_total_sectors), FX_FALSE);

    /* Determine if the flush was unsuccessful. */
    if (status != FX_SUCCESS)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Call the media abort routine.  */
        _fx_media_abort(media_ptr);

        /* Return the error status.  */
        return(FX_IO_ERROR);
    }

    /* Determine if the media needs to have the additional information sector updated. This will
       only be the case for 32-bit FATs. The logic here only needs to be done if the last reported
       available cluster count is different that the currently available clusters.  */
    if ((media_ptr -> fx_media_FAT32_additional_info_sector) &&
        (media_ptr -> fx_media_FAT32_additional_info_last_available != media_ptr -> fx_media_available_clusters) &&
        (media_ptr -> fx_media_driver_write_protect == FX_FALSE))
    {

    UCHAR *buffer_ptr;
    ULONG  signature;


#ifndef FX_DISABLE_CACHE
        /* Setup a pointer to the first cached entry's buffer.  */
        buffer_ptr =  (media_ptr -> fx_media_sector_cache_list_ptr) -> fx_cached_sector_memory_buffer;

        /* Invalidate this cache entry.  */
        (media_ptr -> fx_media_sector_cache_list_ptr) -> fx_cached_sector =  (~(ULONG64)0);
        (media_ptr -> fx_media_sector_cache_list_ptr) -> fx_cached_sector_valid =  FX_FALSE;
#else
        buffer_ptr =  media_ptr -> fx_media_memory_buffer;
#endif /* FX_DISABLE_CACHE */

        /* Read the FAT32 additional information sector from the device.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           buffer_ptr;
        media_ptr -> fx_media_driver_logical_sector =   media_ptr -> fx_media_FAT32_additional_info_sector;
        media_ptr -> fx_media_driver_sectors =          1;
        media_ptr -> fx_media_driver_sector_type =      FX_DIRECTORY_SECTOR;

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver read sector(s) requests.  */
        media_ptr -> fx_media_driver_read_requests++;
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_READ, media_ptr, media_ptr -> fx_media_FAT32_additional_info_sector, 1, buffer_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to read the FAT32 additional information sector.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Determine if the FAT32 sector was read correctly. */
        if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Call the media abort routine.  */
            _fx_media_abort(media_ptr);

            /* Return the error status.  */
            return(FX_IO_ERROR);
        }

        /* Setup a pointer into the FAT32 additional information sector.  */
        buffer_ptr =  media_ptr -> fx_media_driver_buffer;

        /* Pickup the first signature long word.  */
        signature =  _fx_utility_32_unsigned_read(&buffer_ptr[0]);

        /* Determine if the signature is correct.  */
        if (signature == 0x41615252)
        {

            /* Yes, the first signature is correct, now pickup the next signature.  */
            signature =  _fx_utility_32_unsigned_read(&buffer_ptr[484]);

            /* Determine if this signature is correct.  */
            if (signature == 0x61417272)
            {

                /* Yes, we have a good FAT32 additional information sector.  */

                /* Set the free cluster count to the available clusters in the media control block.  */
                _fx_utility_32_unsigned_write(&buffer_ptr[488], media_ptr -> fx_media_available_clusters);

                /* Set the next free cluster number hint to starting search cluster in the media control block.  */
                _fx_utility_32_unsigned_write(&buffer_ptr[492], media_ptr -> fx_media_cluster_search_start);

                /* Now write the sector back out to the media.  */
                media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
                media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
                media_ptr -> fx_media_driver_buffer =           buffer_ptr;
                media_ptr -> fx_media_driver_logical_sector =   media_ptr -> fx_media_FAT32_additional_info_sector;
                media_ptr -> fx_media_driver_sectors =          1;
                media_ptr -> fx_media_driver_sector_type =      FX_DIRECTORY_SECTOR;

                /* Set the system write flag since we are writing a directory sector.  */
                media_ptr -> fx_media_driver_system_write =  FX_TRUE;

#ifndef FX_MEDIA_STATISTICS_DISABLE

                /* Increment the number of driver write sector(s) requests.  */
                media_ptr -> fx_media_driver_write_requests++;
#endif

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, media_ptr -> fx_media_FAT32_additional_info_sector, 1, buffer_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Invoke the driver to write the FAT32 additional information sector.  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

                /* Clear the system write flag.  */
                media_ptr -> fx_media_driver_system_write =  FX_FALSE;

                /* Determine if the FAT32 sector was written correctly. */
                if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Call the media abort routine.  */
                    _fx_media_abort(media_ptr);

                    /* Return the sector IO error status.  */
                    return(FX_IO_ERROR);
                }

                /* Successful update of the FAT32 additional information sector. Update the
                   last written available cluster count.  */
                media_ptr -> fx_media_FAT32_additional_info_last_available =  media_ptr -> fx_media_available_clusters;
            }
        }
    }

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of driver flush requests.  */
    media_ptr -> fx_media_driver_flush_requests++;
#endif

    /* Build the "flush" I/O driver request.  */
    media_ptr -> fx_media_driver_request =      FX_DRIVER_FLUSH;
    media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_FLUSH, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Call the specified I/O driver with the flush request.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Build the "uninitialize" I/O driver request.  */
    media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
    media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Call the specified I/O driver with the uninitialize request.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Now remove this media from the open list.  */

    /* Lockout interrupts for media removal.  */
    FX_DISABLE_INTS

    /* See if the media is the only one on the media opened list.  */
    if (_fx_system_media_opened_count == ((ULONG) 1))
    {

        /* Only opened media, just set the opened list to NULL.  */
        _fx_system_media_opened_ptr =  FX_NULL;
    }
    else
    {

        /* Otherwise, not the only opened media, link-up the neighbors.  */
        (media_ptr -> fx_media_opened_next) -> fx_media_opened_previous =
            media_ptr -> fx_media_opened_previous;
        (media_ptr -> fx_media_opened_previous) -> fx_media_opened_next =
            media_ptr -> fx_media_opened_next;

        /* See if we have to update the opened list head pointer.  */
        if (_fx_system_media_opened_ptr == media_ptr)
        {

            /* Yes, move the head pointer to the next opened media. */
            _fx_system_media_opened_ptr =  media_ptr -> fx_media_opened_next;
        }
    }

    /* Decrement the opened media counter.  */
    _fx_system_media_opened_count--;

    /* Finally, Indicate that this media is closed.  */
    media_ptr -> fx_media_id =  FX_MEDIA_CLOSED_ID;

    /* Restore interrupt posture.  */
    FX_RESTORE_INTS

    /* Delete the media protection structure if FX_SINGLE_THREAD is not
       defined.  */
#ifndef FX_SINGLE_THREAD

#ifndef FX_DONT_CREATE_MUTEX

    /* Note that the protection is never released. The mutex delete
       service will handle all threads waiting access to this media
       control block.  */
    tx_mutex_delete(& (media_ptr -> fx_media_protect));
#endif
#endif

    /* Invoke media close callback. */
    if (media_ptr -> fx_media_close_notify)
    {
        media_ptr -> fx_media_close_notify(media_ptr);
    }

#ifdef FX_DONT_CREATE_MUTEX

    /* Release media protection.  */
    FX_UNPROTECT
#endif

    /* Return success status to the caller.  */
    return(FX_SUCCESS);
}

