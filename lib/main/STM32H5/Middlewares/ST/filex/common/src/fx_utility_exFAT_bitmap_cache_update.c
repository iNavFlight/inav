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
/*    _fx_utility_exFAT_bitmap_cache_update               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads parts of exFAT bitmap to cache.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster number                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Media driver                                                        */
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
UINT   _fx_utility_exFAT_bitmap_cache_update(FX_MEDIA *media_ptr, ULONG cluster)
{

    /* Read exFAT bitmap to cache.  */
    cluster -= FX_FAT_ENTRY_START;
    media_ptr -> fx_media_driver_request        =  FX_DRIVER_READ;
    media_ptr -> fx_media_driver_buffer         =  (UCHAR *)media_ptr -> fx_media_exfat_bitmap_cache;
    media_ptr -> fx_media_driver_logical_sector =  media_ptr -> fx_media_exfat_bitmap_start_sector +
        (cluster >> media_ptr -> fx_media_exfat_bitmap_clusters_per_sector_shift);
    media_ptr -> fx_media_driver_sectors        =  media_ptr -> fx_media_exfat_bitmap_cache_size_in_sectors;
    media_ptr -> fx_media_driver_status         =  FX_IO_ERROR;

    /* Invoke the driver to read the FAT sectors.  */
    (media_ptr -> fx_media_driver_entry)(media_ptr);

    /* Calculate new cached clusters.  */
    media_ptr -> fx_media_exfat_bitmap_cache_start_cluster =
        (ULONG)((media_ptr -> fx_media_driver_logical_sector - media_ptr -> fx_media_exfat_bitmap_start_sector) <<
                media_ptr -> fx_media_exfat_bitmap_clusters_per_sector_shift) +
        FX_FAT_ENTRY_START;

    media_ptr -> fx_media_exfat_bitmap_cache_end_cluster = media_ptr -> fx_media_exfat_bitmap_cache_start_cluster +
        ((media_ptr -> fx_media_exfat_bitmap_cache_size_in_sectors << media_ptr -> fx_media_exfat_bytes_per_sector_shift) <<
         BITS_PER_BYTE_SHIFT) - 1;

    /* Return driver status.  */
    return(media_ptr -> fx_media_driver_status);
}

#endif /* FX_ENABLE_EXFAT */

