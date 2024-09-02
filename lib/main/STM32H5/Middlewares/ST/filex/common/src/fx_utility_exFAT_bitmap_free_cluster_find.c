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
/*    _fx_utility_exFAT_bitmap_free_cluster_find          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches for a free cluster.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    search_start_cluster                  Cluster number to begin search*/
/*    free_cluster                          ULONG pointer to store cluster*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UINT  _fx_utility_exFAT_bitmap_free_cluster_find(FX_MEDIA *media_ptr, ULONG search_start_cluster, ULONG *free_cluster)
{

UINT  status;
UCHAR cluster_state;
ULONG cluster = search_start_cluster;


    /* Search for a free cluster.  */
    while (cluster < media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
    {

        /* Get the cluster state.  */
        status = _fx_utility_exFAT_cluster_state_get(media_ptr, cluster, &cluster_state);

        /* Check the status of the state get.  */
        if (status != FX_SUCCESS)
        {

            /* Media error or out of total clusters number - stop searching.  */
            return(status);
        }

        /* Is this cluster free?  */
        if (cluster_state == FX_EXFAT_BITMAP_CLUSTER_FREE)
        {

            /* Yes, finished the search.  */

            /* Return the cluster.  */
            *free_cluster = cluster;

            /* Return success.  */
            return(FX_SUCCESS);
        }

        /* Move to next cluster.  */
        cluster++;
    }

    /* See if there is anything to search in the beginning.  */
    if (search_start_cluster > FX_FAT_ENTRY_START)
    {

        /* Start at the beginning.  */
        cluster = FX_FAT_ENTRY_START;

        /* Loop to search clusters.  */
        while (cluster < search_start_cluster)
        {

            /* Get the cluster state.  */
            status = _fx_utility_exFAT_cluster_state_get(media_ptr, cluster, &cluster_state);
            if (status != FX_SUCCESS)
            {

                /* Media error or out of total clusters number - stop searching.  */
                return(status);
            }

            /* Is this cluster free?  */
            if (cluster_state == FX_EXFAT_BITMAP_CLUSTER_FREE)
            {

                /* Yes, finished the search.  */

                /* Return the cluster.  */
                *free_cluster = cluster;

                /* Return success.  */
                return(FX_SUCCESS);
            }

            /* Move to the next cluster.  */
            cluster++;
        }
    }

    /* No more free clusters, return error.  */
    return(FX_NO_MORE_SPACE);
}

#endif /* FX_ENABLE_EXFAT */

