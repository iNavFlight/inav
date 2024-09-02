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
/*    _fx_utility_exFAT_cluster_state_set                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the cluster state of exFAT volume.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster number                */
/*    new_cluster_state                     Cluster state to set          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_exFAT_cluster_state_get   Get cluster state             */
/*    _fx_fault_tolerant_add_bitmap_log     Add bitmap redo log           */
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
UINT  _fx_utility_exFAT_cluster_state_set(FX_MEDIA  *media_ptr, ULONG cluster, UCHAR new_cluster_state)
{

UINT  status;
UCHAR cluster_state;
UINT  bitmap_offset;
UCHAR cluster_shift;

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED) &&
        !(media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN))
    {

        /* Redirect this request to log file.
           Under fault tolerant protection, the changes to FAT is recorded in the fault tolerant
           log, in case the operation fails, the changes can be reverted.  */
        return(_fx_fault_tolerant_add_bitmap_log(media_ptr, cluster, new_cluster_state));
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Get the state of the cluster?  */
    status =  _fx_utility_exFAT_cluster_state_get(media_ptr, cluster, &cluster_state);

    /* Was the state get successful?  */
    if (status == FX_SUCCESS)
    {

        /* Is it the same state?  */
        if (new_cluster_state != cluster_state)
        {

            /* No, we need to set the state.  */

            /* Calculate the bitmap offset.  */
            bitmap_offset =  (UINT)(cluster - media_ptr -> fx_media_exfat_bitmap_cache_start_cluster) >> BITS_PER_BYTE_SHIFT;

            /* Calculate where the cluster is located.  */
            cluster_shift =  (UCHAR)((cluster - FX_FAT_ENTRY_START) % BITS_PER_BYTE);

            /* Is occupied the new state?  */
            if (FX_EXFAT_BITMAP_CLUSTER_OCCUPIED == new_cluster_state)
            {

                /* Yes, mark this cluster as occupied.  */
                *(media_ptr -> fx_media_exfat_bitmap_cache + bitmap_offset) = (UCHAR)(*(media_ptr -> fx_media_exfat_bitmap_cache + bitmap_offset) | (1 << cluster_shift));
            }
            else
            {

                /* No, mark this cluster as not occupied.  */
                *(media_ptr -> fx_media_exfat_bitmap_cache + bitmap_offset) &=  (UCHAR)~(1 << cluster_shift);
            }

            /* Mark the cache as dirty.  */
            media_ptr -> fx_media_exfat_bitmap_cache_dirty = FX_TRUE;
        }
    }

    /* Return status.  */
    return(status);
}

#endif /* FX_ENABLE_EXFAT */

