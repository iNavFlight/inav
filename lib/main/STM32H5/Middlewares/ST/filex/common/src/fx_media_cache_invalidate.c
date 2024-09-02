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
/*    _fx_media_cache_invalidate                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function examines the logical cache, flushing written sectors  */
/*    out to the media and invalidating all sectors in the logical cache. */
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
/*    _fx_utility_FAT_flush                 Flush cached FAT entries      */
/*    _fx_utility_FAT_map_flush             Flush primary FAT changes to  */
/*                                            secondary FAT(s)            */
/*    _fx_utility_logical_sector_flush      Flush logical sector cache    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _fx_media_cache_invalidate(FX_MEDIA *media_ptr)
{

UINT status;
UINT i;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_flushes++;
#endif

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_CACHE_INVALIDATE, media_ptr, 0, 0, 0, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT
    /* Flush the cached individual FAT entries */
    _fx_utility_FAT_flush(media_ptr);

    /* Flush changed sector(s) in the primary FAT to secondary FATs.  */
    _fx_utility_FAT_map_flush(media_ptr);

    /* Clear the FAT cache entry array.  */
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {

        /* Clear entry in the FAT cache.  */
        media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_cluster =   0;
        media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_value   =   0;
    }

    /* Clear the secondary FAT update map.  */
    for (i = 0; i < FX_FAT_MAP_SIZE; i++)
    {

        /* Clear bit map entry for secondary FAT update.  */
        media_ptr -> fx_media_fat_secondary_update_map[i] =  0;
    }

    /* Call the logical sector flush to invalidate the logical sector cache.  */
    status =  _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64) (media_ptr -> fx_media_total_sectors), FX_TRUE);

    /* Release media protection.  */
    FX_UNPROTECT

    /* If we get here, return successful status to the caller.  */
    return(status);
}

