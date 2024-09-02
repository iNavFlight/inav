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
#include "fx_media.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_check_FAT_chain_check                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function walks the supplied FAT chain looking for cross links  */
/*    (FAT entries already used in another FAT chain) and abnormal FAT    */
/*    entries and invalid chain termination.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to a previously       */
/*                                            opened media                */
/*    starting_cluster                      Starting cluster of FAT chain */
/*    last_valid_cluster                    Last valid cluster of chain   */
/*    total_valid_clusters                  Total number of valid clusters*/
/*    logical_fat                           Pointer to the logical FAT    */
/*                                            bit map                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    error                                 Error code                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_check_media                       Check media function          */
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
ULONG  _fx_media_check_FAT_chain_check(FX_MEDIA *media_ptr, ULONG starting_cluster, ULONG *last_valid_cluster, ULONG *total_valid_clusters, UCHAR *logical_fat)
{

ULONG prev_cluster, cluster, next_clust = 0;
ULONG cluster_number;
ULONG count, error;
UINT  status;


    /* Initialize the error code.  */
    error =  0;

    /* Setup at the first cluster.  */
    cluster =  starting_cluster;

    /* Initialize the previous cluster.  */
    prev_cluster =  0;

    /* Initialize the count to 0. */
    count =  0;

    /* Loop to walk through the FAT chain.  */
    while ((cluster >= (ULONG)FX_FAT_ENTRY_START) &&
           (cluster < (FX_FAT_ENTRY_START + media_ptr -> fx_media_total_clusters)))
    {

        cluster_number = cluster;

#ifdef FX_ENABLE_EXFAT

        /* For the index of the first cluster in exFAT is 2, adjust the number of clusters to fit Allocation Bitmap Table. */
        /* We will compare logical_fat with Aollcation Bitmap table later to find out lost clusters. */
        if (media_ptr -> fx_media_FAT_type == FX_exFAT)
        {
            cluster_number = cluster - FX_FAT_ENTRY_START;
        }
#endif /* FX_ENABLE_EXFAT */

        /* Determine if this cluster is already in the logical FAT bit map.  */
        if (logical_fat[cluster_number >> 3] & (1 << (cluster_number & 7)))
        {

            /* Yes, the cluster is already being used by another file or
               sub-directory.  */
            error =  FX_FAT_CHAIN_ERROR;
            break;
        }

        /* Now read the contents of the cluster.  */
        status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_clust);

        /* Check the return status.  */
        if (status != FX_SUCCESS)
        {

            /* Yes, the cluster is already being used by another file or
               sub-directory.  */
            error =  FX_IO_ERROR;
            break;
        }

        /* Determine if the link is circular or the count is greater than the
           total clusters.  */
        if ((cluster == next_clust) ||
            (next_clust < (ULONG)FX_FAT_ENTRY_START) ||
            ((next_clust >= (FX_FAT_ENTRY_START + media_ptr -> fx_media_total_clusters)) &&
             (next_clust != media_ptr -> fx_media_fat_last)))
        {

            error =  FX_FAT_CHAIN_ERROR;
            break;
        }

        /* Everything is good with the chain at this point.  Mark it as valid.  */
        logical_fat[cluster_number >> 3] = (UCHAR)(logical_fat[cluster_number >> 3] | (1 << (cluster_number & 7)));

        /* Move the cluster variable forward.  */
        prev_cluster =  cluster;
        cluster =       next_clust;

        /* Increment the number of valid clusters.  */
        count++;
    }

    /* Return the last valid cluster and the total valid cluster count.  */
    *last_valid_cluster =   prev_cluster;
    *total_valid_clusters = count;

    /* Return error code.  */
    return(error);
}

