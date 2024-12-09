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
#include "fx_directory_exFAT.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_cluster_free                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deallocates clusters deleted unknown type             */
/*    directory entry.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    work_ptr                              Pointer to directory to       */
/*                                            unknown directory entry     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_32_unsigned_read          Read 32-bit value             */
/*    _fx_utility_64_unsigned_read          Read 64-bit value             */
/*    _fx_utility_FAT_entry_read            Read FAT entry                */
/*    _fx_utility_FAT_entry_write           Write FAT entry               */
/*    _fx_utility_exFAT_cluster_state_set   Set cluster state             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_exFAT_unicode_entry_write                             */
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
UINT  _fx_utility_exFAT_cluster_free(FX_MEDIA *media_ptr, UCHAR *work_ptr)
{

ULONG   cluster;
ULONG64 data_length;
ULONG   clusters_released;
ULONG   clusters_count;
ULONG   bytes_per_cluster;
ULONG   contents = 0;
UCHAR   dont_use_fat;
UINT    status;


    /* Pickup the cluster to release. */
    cluster =       _fx_utility_32_unsigned_read(&(work_ptr[FX_EXFAT_FIRST_CLUSTER]));

    /* Pickup the data length.  */
    data_length =   _fx_utility_64_unsigned_read(&(work_ptr[FX_EXFAT_DATA_LENGTH]));

    /* Pickup the flag that determines if the FAT should be used.  */
    dont_use_fat =  (work_ptr[FX_EXFAT_SECOND_FLAG] >> 1) & 1;

    /* Default status to success.  */
    status =  FX_SUCCESS;

    /* Is the allocation possible flag cleared? Is there any allocated cluster for this entry?  */
    if (((work_ptr[FX_EXFAT_SECOND_FLAG] & FX_EXFAT_SECOND_FLAG_ALLOCATION_POSSIBLE_MASK) == 0) ||
        (cluster == 0) ||
        (data_length == 0))
    {

        /* No cluster allocated to this entry, nothing to deallocate.  */
        return(FX_SUCCESS);
    }

    /* Calculate the number of bytes per cluster.  */
    bytes_per_cluster = ((ULONG)media_ptr -> fx_media_bytes_per_sector) *
        ((ULONG)media_ptr -> fx_media_sectors_per_cluster);

    /* Determine how many clusters.  */
    clusters_count =  (ULONG)((data_length + bytes_per_cluster - 1) / bytes_per_cluster - 1);

    /* Initialize released cluster count to 0.  */
    clusters_released = 0;

    /* Follow the link of FAT entries.  */
    while ((cluster >= FX_FAT_ENTRY_START) && (cluster < FX_RESERVED_1_exFAT))
    {

        /* Increment the number of clusters released.  */
        clusters_released++;

        /* Determine if the FAT chain is to be used.  */
        if (dont_use_fat)
        {

            /* Yes, don't use the FAT chain.  */

            /* Check for file size range.  */
            if (clusters_released - 1 >= clusters_count)
            {

                /* Set the next cluster to LAST to indicate it is the last cluster.  */
                contents =  FX_LAST_CLUSTER_exFAT;
            }
            else
            {

                /* The next cluster is just after the current cluster.  */
                contents =  cluster + 1;
            }
        }
        else
        {

            /* Read the current cluster entry from the FAT.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &contents);

            /* Check the return value.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }
        }

        /* Check for data corruption.  */
        if ((cluster == contents) || (clusters_released > media_ptr -> fx_media_total_clusters))
        {

            /* Return the bad status.  */
            return(FX_FAT_READ_ERROR);
        }

        /* Make the current cluster available.  */
        if (!dont_use_fat)
        {

            /* Write the FAT to free the cluster.  */
            status = _fx_utility_FAT_entry_write(media_ptr, cluster, FX_FREE_CLUSTER);

            /* Check the return value.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }
        }

        /* Set cluster state in the bitmap.  */
        status = _fx_utility_exFAT_cluster_state_set(media_ptr, cluster, FX_EXFAT_BITMAP_CLUSTER_FREE);

        /* Check the return status.  */
        if (status != FX_SUCCESS)
        {

            /* Return the bad status.  */
            return(status);
        }

        /* Setup for the next cluster.  */
        cluster =  contents;
    }

    /* Update the free clusters in the media control block.  */
    media_ptr -> fx_media_available_clusters =  media_ptr -> fx_media_available_clusters + clusters_released;

    /* Return success.  */
    return(FX_SUCCESS);
}

#endif /* FX_ENABLE_EXFAT */

