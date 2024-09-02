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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_cluster_state_get                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the cluster state of exFAT volume.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster number                */
/*    cluster_state                         UCHAR pointer to store state  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_exFAT_bitmap_cache_prepare                              */
/*                                          Read bitmap to cache          */
/*    _fx_fault_tolerant_read_FAT           Read FAT entry from log file  */
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
UINT  _fx_utility_exFAT_cluster_state_get(FX_MEDIA *media_ptr, ULONG cluster, UCHAR *cluster_state)
{

UINT  status;
UINT  bitmap_offset;
UCHAR cluster_shift;
UCHAR eight_clusters_block;
#ifdef FX_ENABLE_FAULT_TOLERANT
ULONG value;

    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
    {


        /* Redirect this request to the log file.
           During file or directory operations with Fault Tolerant
           protection, intermediate operations are written to the fault
           tolerant log file.  Therefore, FAT-entry read should search for
           fault tolerant log before the request can be passed to normal FAT
           entry read routine.
         */
        status = _fx_fault_tolerant_read_FAT(media_ptr, cluster, &value, FX_FAULT_TOLERANT_BITMAP_LOG_TYPE);

        /* Return on success. */
        if (status != FX_READ_CONTINUE)
        {
            *cluster_state = (UCHAR)value;
            return(status);
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Prepare bitmap cache.  */
    status = _fx_utility_exFAT_bitmap_cache_prepare(media_ptr, cluster);

    /* Was it successful?  */
    if (status == FX_SUCCESS)
    {

        /* Calculate the bitmap offset.  */
        bitmap_offset = (UINT)(cluster - media_ptr -> fx_media_exfat_bitmap_cache_start_cluster) >> BITS_PER_BYTE_SHIFT;

        /* Pickup the 8 cluster block.  */
        eight_clusters_block = *(media_ptr -> fx_media_exfat_bitmap_cache + bitmap_offset);

        /* Check all 8 bits for 0x00 (all clusters in the block are free).  */
        if (eight_clusters_block == 0x00)
        {

            /* Cluster is free, return state.  */
            *cluster_state = FX_EXFAT_BITMAP_CLUSTER_FREE;
        }
        /* Check all 8 bits for 0xFF (all 8 clusters are occupied).  */
        else if (0xFF == eight_clusters_block)
        {

            /* Cluster is not free, return state.  */
            *cluster_state = FX_EXFAT_BITMAP_CLUSTER_OCCUPIED;
        }
        else
        {

            /* Check bit respondent for cluster state.  */
            cluster_shift = (UCHAR)((cluster - FX_FAT_ENTRY_START) % BITS_PER_BYTE);

            *cluster_state = (eight_clusters_block >> cluster_shift) & 0x1;
        }
    }

    /* Return status.  */
    return(status);
}

#endif /* FX_ENABLE_EXFAT */

