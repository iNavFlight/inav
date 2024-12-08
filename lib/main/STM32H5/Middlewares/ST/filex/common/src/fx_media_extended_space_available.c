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
#include "fx_system.h"
#include "fx_media.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_extended_space_available                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the number of bytes available in the          */
/*    specified media.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    available_bytes_ptr                   Pointer to available bytes    */
/*                                            destination                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _fx_media_extended_space_available(FX_MEDIA *media_ptr, ULONG64 *available_bytes_ptr)
{

ULONG64 available_bytes;
ULONG   bytes_per_cluster;
ULONG   available_clusters;


    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_SPACE_AVAILABLE, media_ptr, available_bytes_ptr, media_ptr -> fx_media_available_clusters, 0, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Pickup the number of free clusters.  */
    available_clusters =  media_ptr -> fx_media_available_clusters;

    /* Derive the bytes per cluster.  */
    bytes_per_cluster =  media_ptr -> fx_media_bytes_per_sector * media_ptr -> fx_media_sectors_per_cluster;

    /* Calculate the free space.  */
    available_bytes =  ((ULONG64)available_clusters) * ((ULONG64)bytes_per_cluster);

    /* Return the value.  */
    *available_bytes_ptr =  available_bytes;

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return a successful status to the caller.  */
    return(FX_SUCCESS);
}

