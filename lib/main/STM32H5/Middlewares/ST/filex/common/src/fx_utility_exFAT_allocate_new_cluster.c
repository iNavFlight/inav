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
#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_allocate_new_cluster              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates new cluster media for a free                */
/*    directory entry.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_ptr                         Pointer to directory to       */
/*                                            search in                   */
/*    last_cluster                          Last cluster of entry         */
/*    cluster                               Allocated cluster             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_exFAT_entry_read        Read exFAT entries            */
/*    _fx_utility_exFAT_bitmap_flush        Flush exFAT allocation bitmap */
/*    _fx_utility_exFAT_bitmap_free_cluster_find                          */
/*                                          Find free cluster             */
/*    _fx_utility_exFAT_cluster_state_get   Get cluster state             */
/*    _fx_utility_FAT_entry_read            Read FAT entry                */
/*    _fx_utility_FAT_entry_write           Write FAT entry               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_exFAT_free_search                                     */
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
UINT  _fx_utility_exFAT_allocate_new_cluster(FX_MEDIA *media_ptr, FX_DIR_ENTRY *directory_ptr,
                                             ULONG *last_cluster, ULONG *cluster)
{

UINT  status = FX_SUCCESS;
ULONG cluster_count;
ULONG bytes_per_cluster;
ULONG i;
ULONG FAT_value = 0;
UCHAR cluster_state = FX_EXFAT_BITMAP_CLUSTER_OCCUPIED;


    /* Clear the last cluster number.  */
    *last_cluster = 0;

    /* Check if we have any already allocated clusters.  */
    if (!directory_ptr || (directory_ptr -> fx_dir_entry_available_file_size > 0))
    {

        /* Calculate bytes per cluster.  */
        bytes_per_cluster = ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
                             ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

        /* Check if FAT table is not used.  */
        if (directory_ptr && (directory_ptr -> fx_dir_entry_dont_use_fat & 1))
        {

            /* Calculate the cluster count associated with the directory.  */
            cluster_count = (ULONG)((directory_ptr -> fx_dir_entry_available_file_size + bytes_per_cluster - 1) /
                                    bytes_per_cluster);

            /* Calculate the next cluster and the last cluster.  */
            *cluster = directory_ptr -> fx_dir_entry_cluster + cluster_count;
            *last_cluster = *cluster - 1;

            /* Make sure the next cluster number don't exceed the total cluster number.  */
            if (*cluster < media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
            {

                /* Get cluster state.  */
                status = _fx_utility_exFAT_cluster_state_get(media_ptr, *cluster, &cluster_state);

                if (status != FX_SUCCESS)
                {

                    /* Return the bad status.  */
                    return(status);
                }
            }

            /* Is the next cluster free?  */
            if (cluster_state == FX_EXFAT_BITMAP_CLUSTER_FREE)
            {

                /* Yes, so we still don't need to use FAT.  */
                return(FX_SUCCESS);
            }

            /* Now we should use FAT. Clear the bit 0.  */
            directory_ptr -> fx_dir_entry_dont_use_fat &= (CHAR)0xfe;

            /* Loop to build FAT chain.  */
            for (i = directory_ptr -> fx_dir_entry_cluster; i < *last_cluster; ++i)
            {

                /* Write the FAT entry.  */
                status = _fx_utility_FAT_entry_write(media_ptr, i, i + 1);
                if (status != FX_SUCCESS)
                {

                    /* Return the bad status.  */
                    return(status);
                }
            }

            /* Write last cluster entry.  */
            status = _fx_utility_FAT_entry_write(media_ptr, *last_cluster, FX_LAST_CLUSTER_exFAT);

            if (status != FX_SUCCESS)
            {

                /* Return the bad status.  */
                return(status);
            }
        }
        else
        {

            /* Find last cluster by walk through the FAT chain.  */
            /* See if we are in sub directory or the root directory. And set the start cluster.  */
            if (directory_ptr)
            {
                *cluster = directory_ptr -> fx_dir_entry_cluster;
            }
            else
            {
                *cluster = media_ptr -> fx_media_root_cluster_32;
            }

            /* Initialize loop variables.  */
            *last_cluster =  0;
            i =  0;

            /* Follow the link of FAT entries.  */
            while ((*cluster >= FX_FAT_ENTRY_START) && (*cluster < FX_RESERVED_1_exFAT))
            {

                /* Read the current cluster entry from the FAT.  */
                status =  _fx_utility_FAT_entry_read(media_ptr, *cluster, &FAT_value);
                i++;

                /* Check the return value.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Determine if the FAT read was invalid.  */
                if ((*cluster == FAT_value) || (i > media_ptr -> fx_media_total_clusters))
                {

                    /* Return the bad status.  */
                    return(FX_FAT_READ_ERROR);
                }

                /* Save the last valid cluster.  */
                *last_cluster = *cluster;

                /* Setup for the next cluster.  */
                *cluster =  FAT_value;
            }
        }
    }

    /* Find free cluster from exFAT media.  */
    status =  _fx_utility_exFAT_bitmap_free_cluster_find(media_ptr,
                                                         media_ptr -> fx_media_cluster_search_start,
                                                         cluster);

    return(status);
}


#endif /* FX_ENABLE_EXFAT */

