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
/*    _fx_utility_exFAT_bitmap_cache_prepare              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if the bitmap for specified cluster is in      */
/*    cache. If not, it will read the bitmap portion to cache.            */
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
/*    _fx_utility_exFAT_bitmap_flush        Flush bitmap cache            */
/*    _fx_utility_exFAT_bitmap_cache_update Read bitmap to cache          */
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
UINT  _fx_utility_exFAT_bitmap_cache_prepare(FX_MEDIA *media_ptr, ULONG cluster)
{

UINT status;


    /* Default the status to no more space.  */
    status = FX_NO_MORE_SPACE;

    /* Make sure the cluster does not exceed the total count.  */
    if (cluster < media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START)
    {

        /* Check if the request cluster is already cached.  */
        if ((cluster >= media_ptr -> fx_media_exfat_bitmap_cache_start_cluster) &&
            (cluster <= media_ptr -> fx_media_exfat_bitmap_cache_end_cluster))
        {

            /* Cluster already cached.  */
            status = FX_SUCCESS;
        }
        else
        {

            /* Need to Cache update.  */

            /* Flush previous cached data if required.  */
            if (FX_SUCCESS == (status = _fx_utility_exFAT_bitmap_flush(media_ptr)))
            {

                /* Call utility function to update cache.  */
                status = _fx_utility_exFAT_bitmap_cache_update(media_ptr, cluster);
            }
        }
    }

    /* Return status.  */
    return(status);
}

#endif /* FX_ENABLE_EXFAT */

