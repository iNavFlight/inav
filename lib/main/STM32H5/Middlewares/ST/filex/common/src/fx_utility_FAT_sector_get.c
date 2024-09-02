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
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_FAT_sector_get                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the sector of supplied FAT entry.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return sector of FAT entry                                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
ULONG  _fx_utility_FAT_sector_get(FX_MEDIA *media_ptr, ULONG cluster)
{

ULONG  FAT_sector;
ULONG  byte_offset;

    /* Determine which type of FAT is present.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_FAT12)
#else
    if (media_ptr -> fx_media_12_bit_FAT)
#endif /* FX_ENABLE_EXFAT */
    {

        /* 12-bit FAT is present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster << 1) + cluster) >> 1;

    }
#ifdef FX_ENABLE_EXFAT
    else if (media_ptr -> fx_media_FAT_type == FX_FAT16)
#else
    else if (!media_ptr -> fx_media_32_bit_FAT)
#endif /* FX_ENABLE_EXFAT */
    {

        /* 16-bit FAT is present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster) << 1);
    }
    else
    {

        /* 32-bit FAT or exFAT are present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster) * 4);
    }

    /* Calculate the FAT sector the requested FAT entry resides in.  */
    FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
        (ULONG)media_ptr -> fx_media_reserved_sectors;

    /* Return successful status.  */
    return(FAT_sector);
}

