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
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"


#ifdef FX_ENABLE_EXFAT
#include "fx_system.h"
#include "fx_media.h"
#include "fx_utility.h"
#include "fx_directory_exFAT.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_bitmap_start_sector_get           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the start sector of exFAT bitmap.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    bitmap_started_sector                 Pointer to ULONG              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_logical_sector_read       Read logical sector           */
/*    _fx_utility_64_unsigned_read          Read a ULONG64 from memory    */
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
UINT   _fx_utility_exFAT_bitmap_start_sector_get(FX_MEDIA *media_ptr, ULONG *bitmap_started_sector)
{

ULONG  sectors_count;
UCHAR *dir_entry_ptr;


    /* Initialize the sector count.  */
    sectors_count =  0;

    /* Go through the first cluster of exFAT root directory. This cluster should
       contain BitMap allocation DirEntry */
    while (sectors_count < media_ptr -> fx_media_sectors_per_cluster)
    {

        /*  Will use read sector function with cache functionality since first cluster
            will be used by other services. */
        if (FX_SUCCESS != _fx_utility_logical_sector_read(media_ptr,
                                                          (ULONG64) (media_ptr -> fx_media_root_sector_start + sectors_count),
                                                          media_ptr -> fx_media_memory_buffer,
                                                          ((ULONG) 1), FX_DATA_SECTOR))
        {

            /* Error reading the sector, get out of the loop.  */
            break;
        }

        /* Increment the sectors count.  */
        sectors_count++;

        /* Setup pointer to directory entry.  */
        dir_entry_ptr =  media_ptr -> fx_media_memory_buffer;

        /* Go through the read buffer and try to find BitMap table directory entry.  */
        while (dir_entry_ptr < (media_ptr -> fx_media_memory_buffer +
                                media_ptr -> fx_media_bytes_per_sector))
        {

            /* Determine if this entry is the bitmap allocation type.  */
            if (FX_EXFAT_DIR_ENTRY_TYPE_ALLOCATION_BITMAP == dir_entry_ptr[FX_EXFAT_ENTRY_TYPE])
            {

                /* Yes, calculate BitMap table start sector.  */
                *bitmap_started_sector = media_ptr -> fx_media_data_sector_start +
                    (UINT)((dir_entry_ptr[FX_EXFAT_FIRST_CLUSTER] - FX_FAT_ENTRY_START) <<
                                        media_ptr -> fx_media_exfat_sector_per_clusters_shift);

                /* Check FirstCluster and DataLength fields for error.  */
                if ((0 == *bitmap_started_sector) ||
                    (media_ptr -> fx_media_total_clusters >
                     ((ULONG)_fx_utility_64_unsigned_read(&dir_entry_ptr[FX_EXFAT_DATA_LENGTH]) <<
                    BITS_PER_BYTE_SHIFT)))
                {

                    /* Media invalid - return error!  */
                    return(FX_MEDIA_INVALID);
                }

                /* Return success!  */
                return(FX_SUCCESS);
            }
            else
            {

                /* Move to next directory entry.  */
                dir_entry_ptr +=  FX_EXFAT_DIR_ENTRY_SIZE;
            }
        }
    }

    /* Return media invalid error.  */
    return(FX_MEDIA_INVALID);
}

#endif /* FX_ENABLE_EXFAT */

