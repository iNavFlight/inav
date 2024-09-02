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
/*    _fx_utility_exFAT_bitmap_initialize                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes bitmap for exFAT volume.                  */
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
/*    Media driver                                                        */
/*    _fx_utility_exFAT_bitmap_free_cluster_find                          */
/*                                          Find free cluster             */
/*    _fx_utility_exFAT_bitmap_cache_update Read bitmap cache             */
/*    _fx_utility_exFAT_bitmap_start_sector_get                           */
/*                                          Get the bitmap starting sector*/
/*    _fx_utility_exFAT_cluster_state_get   Get cluster state             */
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
UINT   _fx_utility_exFAT_bitmap_initialize(FX_MEDIA *media_ptr)
{

UINT  status;
ULONG cluster;
UCHAR cluster_state;
ULONG bitmap_cache_size;
ULONG bitmap_size_in_bytes;
ULONG bitmap_size_in_sectors;
ULONG bitmap_total_size_in_bytes;


    /* Calculate exFAT bitmap size.  */
    bitmap_size_in_bytes =    DIVIDE_TO_CEILING(media_ptr -> fx_media_total_clusters, BITS_PER_BYTE);
    bitmap_size_in_sectors =  DIVIDE_TO_CEILING(bitmap_size_in_bytes, media_ptr -> fx_media_bytes_per_sector);

    /* Align bitmap size by sectors size.  */
    bitmap_total_size_in_bytes =  bitmap_size_in_sectors << media_ptr -> fx_media_exfat_bytes_per_sector_shift;

    /* Set default bitMap cache size.  */
    bitmap_cache_size = media_ptr -> fx_media_bytes_per_sector * FX_EXFAT_BIT_MAP_NUM_OF_CACHED_SECTORS;

    /* Do not use the memory if it not required.  */
    if (bitmap_cache_size > bitmap_total_size_in_bytes)
    {

        /* Adjust the cache size.  */
        bitmap_cache_size =  bitmap_total_size_in_bytes;
    }

    /* Try to find BitMap start sector.  */
    status = _fx_utility_exFAT_bitmap_start_sector_get(media_ptr, &media_ptr -> fx_media_exfat_bitmap_start_sector);

    /* Did we get the starting sector?  */
    if (status == FX_SUCCESS)
    {

        /* Yes, setup the media cache parameters.  */
        media_ptr -> fx_media_exfat_bitmap_cache_size_in_sectors  =
            bitmap_cache_size >> media_ptr -> fx_media_exfat_bytes_per_sector_shift;

        media_ptr -> fx_media_exfat_bitmap_cache_dirty         =  FX_FALSE;
        media_ptr -> fx_media_exfat_bitmap_cache_start_cluster =  FX_FAT_ENTRY_START;

        /* Calculate how many clusters mapped in the one sector.  */
        media_ptr -> fx_media_exfat_bitmap_clusters_per_sector_shift =
            media_ptr -> fx_media_exfat_bytes_per_sector_shift +
            BITS_PER_BYTE_SHIFT;

        /* Start at initial cluster.  */
        cluster =  FX_FAT_ENTRY_START;

        /* Read first portion of BitMap.  */
        status = _fx_utility_exFAT_bitmap_cache_update(media_ptr, cluster);

        /* Was the BitMap read successful?  */
        if (status == FX_SUCCESS)
        {

            /* Find first free cluster.  */
            status = _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr, cluster, &cluster);

            /* Did we find a free cluster?  */
            if (status == FX_SUCCESS)
            {

                /* Save first free cluster number.  */
                media_ptr -> fx_media_cluster_search_start =  cluster;

                /* Calculate number of free clusters from first free cluster.  */
                while (cluster < media_ptr -> fx_media_total_clusters  + FX_FAT_ENTRY_START)
                {

                    /* Get the state of the cluster.  */
                    status = _fx_utility_exFAT_cluster_state_get(media_ptr, cluster, &cluster_state);

                    /* Was the state get successful?  */
                    if (status != FX_SUCCESS)
                    {

                        /* No, get out of the loop.  */
                        break;
                    }

                    /* Is the cluster free?  */
                    if (FX_EXFAT_BITMAP_CLUSTER_FREE == cluster_state)
                    {

                        /* Yes, increment the available clusters.  */
                        media_ptr -> fx_media_available_clusters++;
                    }

                    /* Move to next cluster.  */
                    cluster++;
                }
            }
        }
    }

    /* Return status.  */
    return(status);
}

#endif /* FX_ENABLE_EXFAT */

