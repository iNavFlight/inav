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
/**   File                                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_file.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_file_extended_seek                              PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function positions the internal file pointers to the specified */
/*    byte offset such that the next read or write operation will be      */
/*    performed there.  If the byte offset is greater than the size, the  */
/*    file pointers will be positioned to the end of the file.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*    byte_offset                           Byte offset into the file     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s), fixed    */
/*                                            relative cluster logic,     */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_file_extended_seek(FX_FILE *file_ptr, ULONG64 byte_offset)
{

UINT      status;
ULONG     cluster;
ULONG     contents = 0;
ULONG     bytes_per_cluster;
ULONG     last_cluster;
ULONG     cluster_count;
ULONG64   bytes_remaining;
FX_MEDIA *media_ptr;


    /* First, determine if the file is still open.  */
    if (file_ptr -> fx_file_id != FX_FILE_ID)
    {

        /* Return the file not open error status.  */
        return(FX_NOT_OPEN);
    }

#ifndef FX_MEDIA_STATISTICS_DISABLE
    /* Setup pointer to media structure.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_file_seeks++;
#endif

    /* Setup pointer to associated media control block.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_SEEK, file_ptr, byte_offset, file_ptr -> fx_file_current_file_offset, 0, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Check if we actually have to do anything.  */
    if (byte_offset == file_ptr -> fx_file_current_file_offset)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Seek is complete, return successful status.  */
        return(FX_SUCCESS);
    }

    /* Calculate the number of bytes per cluster.  */
    bytes_per_cluster =  ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    /* Check for invalid value.  */
    if (bytes_per_cluster == 0)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Invalid media, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* See if we need to adjust the byte offset.  */
    if (byte_offset > file_ptr -> fx_file_current_file_size)
    {

        /* Adjust the byte offset down to the file size. */
        byte_offset =  file_ptr -> fx_file_current_file_size;
    }

    /* Check if the desired position within the leading consecutive clusters.  */
    if (byte_offset >= (ULONG64)file_ptr -> fx_file_consecutive_cluster * (ULONG64)bytes_per_cluster)
    {
#ifdef FX_ENABLE_EXFAT
        if (file_ptr -> fx_file_dir_entry.fx_dir_entry_dont_use_fat & 1)
        {
            if (byte_offset == (ULONG64)file_ptr -> fx_file_consecutive_cluster * (ULONG64)bytes_per_cluster)
            {
                /* If the file bytes exactly fits the cluster size */
                bytes_remaining = bytes_per_cluster;

                file_ptr -> fx_file_current_relative_cluster = (ULONG)(byte_offset / bytes_per_cluster - 1);

                file_ptr -> fx_file_current_physical_cluster =
                    file_ptr -> fx_file_first_physical_cluster + file_ptr -> fx_file_current_relative_cluster;
            }
            else
            {

                /* We shouldn't be here if don't using FAT!  */
                FX_UNPROTECT

                return(FX_FILE_CORRUPT);
            }
        }
        else
        {
#endif /* FX_ENABLE_EXFAT */

            /* At this point, we are ready to walk list of clusters to setup the
               seek position of this file.  */

            /* check if byte_offset is greater than where we were left off earlier */
            if ((ULONG64)file_ptr -> fx_file_current_relative_cluster * (ULONG64)bytes_per_cluster < byte_offset)
            {

                cluster =    file_ptr -> fx_file_current_physical_cluster;

                bytes_remaining =   byte_offset -
                    file_ptr -> fx_file_current_relative_cluster * bytes_per_cluster;

                cluster_count = file_ptr -> fx_file_current_relative_cluster;
            }
            else
            {

                cluster =    file_ptr -> fx_file_first_physical_cluster +
                    (file_ptr -> fx_file_consecutive_cluster - 1);
                bytes_remaining =   byte_offset -
                    (file_ptr -> fx_file_consecutive_cluster - 1) * bytes_per_cluster;
                cluster_count =     (file_ptr -> fx_file_consecutive_cluster - 1);
            }


            /* Follow the link of FAT entries.  */
            while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
            {

                /* Increment the number of clusters.  */
                cluster_count++;

                /* Read the current cluster entry from the FAT.  */
                status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &contents);

                /* Check the return value.  */
                if (status != FX_SUCCESS)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error status.  */
                    return(status);
                }

                /* Save the last valid cluster.  */
                last_cluster =  cluster;

                /* Setup for the next cluster.  */
                cluster =  contents;

                /* Determine if this is the last written cluster.  */
                if (bytes_remaining > bytes_per_cluster)
                {

                    /* Still more seeking, just decrement the working byte offset.  */
                    bytes_remaining =  bytes_remaining - bytes_per_cluster;
                }
                else
                {

                    /* Remember this cluster number.  */
                    file_ptr -> fx_file_current_physical_cluster =  last_cluster;

                    /* Remember the relative cluster.  */
                    file_ptr -> fx_file_current_relative_cluster =  cluster_count - 1;

                    /* If the remaining bytes exactly fits the cluster size, check for
                       a possible adjustment to the next cluster.  */
                    if ((bytes_remaining == bytes_per_cluster) &&
                        (cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
                    {

                        /* We need to position to next allocated cluster.  */
                        file_ptr -> fx_file_current_physical_cluster =  cluster;
                        file_ptr -> fx_file_current_relative_cluster++;

                        /* Clear the remaining bytes.  */
                        bytes_remaining =  0;
                    }

                    /* This is the cluster that contains the seek position.  */
                    break;
                }
            }
        
            /* Check for errors in traversal of the FAT chain.  */
            if (byte_offset > (((ULONG64) bytes_per_cluster) * ((ULONG64) cluster_count)))
            {
    
                /* Release media protection.  */
                FX_UNPROTECT

                /* This is an error that suggests a corrupt file.  */
                return(FX_FILE_CORRUPT);
            }
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */
    }
    else
    {

        /* we should directly access the desired cluster */
        file_ptr -> fx_file_current_relative_cluster = (ULONG)(byte_offset / bytes_per_cluster);

        file_ptr -> fx_file_current_physical_cluster =
            file_ptr -> fx_file_first_physical_cluster + file_ptr -> fx_file_current_relative_cluster;

        bytes_remaining =  byte_offset % bytes_per_cluster;
    }


    /* Determine if the remaining bytes fit exactly into the cluster size.  */
    if (bytes_remaining == bytes_per_cluster)
    {

        /* Position to the end of the cluster.  */
        file_ptr -> fx_file_current_logical_sector = (ULONG)(((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                             (((ULONG64)file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START) *
                                                              ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                                                             ((bytes_remaining - 1) / (ULONG)media_ptr -> fx_media_bytes_per_sector));
        file_ptr -> fx_file_current_relative_sector =   (UINT)(((bytes_remaining - 1) / (ULONG)media_ptr -> fx_media_bytes_per_sector));
        file_ptr -> fx_file_current_file_offset =       byte_offset;
        file_ptr -> fx_file_current_logical_offset =    media_ptr -> fx_media_bytes_per_sector;
    }
    else
    {

        /* Position the pointers to the new offset.  */
        file_ptr -> fx_file_current_logical_sector = (ULONG)(((ULONG)media_ptr -> fx_media_data_sector_start) +
                                                             (((ULONG64)file_ptr -> fx_file_current_physical_cluster - FX_FAT_ENTRY_START) *
                                                              ((ULONG)media_ptr -> fx_media_sectors_per_cluster)) +
                                                             (bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector));
        file_ptr -> fx_file_current_relative_sector =   (UINT)((bytes_remaining / (ULONG)media_ptr -> fx_media_bytes_per_sector));
        file_ptr -> fx_file_current_file_offset =       byte_offset;
        file_ptr -> fx_file_current_logical_offset =    (ULONG)(bytes_remaining % ((ULONG)media_ptr -> fx_media_bytes_per_sector));
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Seek is complete, return successful status.  */
    return(FX_SUCCESS);
}

