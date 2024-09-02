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
#include "fx_system.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_FAT_map_flush                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates mirrors changes in the primary FAT to each of */
/*    secondary FATs in the media.                                        */
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
/*    _fx_utility_logical_sector_read       Read FAT sector into memory   */
/*    _fx_utility_logical_sector_write      Write FAT sector back to disk */
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
UINT  _fx_utility_FAT_map_flush(FX_MEDIA *media_ptr)
{

ULONG FAT_sector, last_sector;
UINT  i, status, FATs;
UCHAR sectors_per_bit;


    /* Determine how many FAT sectors each bit in the bit map represents.  Depending on
       the number of sectors in the primary FAT, each bit in this map may represent one
       or more primary FAT sectors. Because of this, it is possible some FAT sectors that
       were not changed may get flushed out to the secondary FAT.  However, this method
       provides very nice performance benefits during normal operation and is much more
       reasonable than performing a total copy of the primary FAT to each secondary FAT
       on media flush and media close.  */
    if (media_ptr -> fx_media_sectors_per_FAT % (FX_FAT_MAP_SIZE << 3) == 0)
    {
        sectors_per_bit =  (UCHAR)(media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3));
    }
    else
    {
        sectors_per_bit =  (UCHAR)(media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3) + 1);
    }

    /* Loop through the FAT update map to mirror primary FAT sectors to secondary FAT(s).  */
    for (i = 0; i < FX_FAT_MAP_SIZE << 3; i++)
    {

        /* Determine if there are FAT changes specified by this entry.  */
        if ((media_ptr -> fx_media_fat_secondary_update_map[i >> 3] & (1 << (i & 7))) == 0)
        {

            /* No, look at the next bit map entry.  */
            continue;
        }

        /* Setup the parameters for performing the update.  */
        FAT_sector =    i * sectors_per_bit + media_ptr -> fx_media_reserved_sectors;
        last_sector =   FAT_sector + sectors_per_bit;

        /* Make sure the last update sector is within range.  */
        if (last_sector > (media_ptr -> fx_media_sectors_per_FAT + media_ptr -> fx_media_reserved_sectors))
        {
            last_sector =  media_ptr -> fx_media_sectors_per_FAT + media_ptr -> fx_media_reserved_sectors;
        }

        /* Loop to mirror primary FAT sectors to secondary FAT(s).  */
        for (; FAT_sector < last_sector; FAT_sector++)
        {

            /* Read the FAT sector.  */
            status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {
                /* Return the error status.  */
                return(status);
            }

            /* Pickup how many secondary FATs there are.  */
            FATs =  media_ptr -> fx_media_number_of_FATs - 1;

            /* Loop to update additional FAT entries.  */
            while (FATs)
            {

                /* Mirror main FAT sector write into the additional FATs.  */
                status =  _fx_utility_logical_sector_write(media_ptr,
                                                           ((ULONG64) FAT_sector) + ((ULONG64)FATs * (ULONG64)(media_ptr -> fx_media_sectors_per_FAT)),
                                                           media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Decrement the number of FATs.  */
                FATs--;
            }
        }
    }

    /* Clear the bit map that indicates primary FAT updates.  */
    for (i = 0; i < FX_FAT_MAP_SIZE; i++)
    {

        /* Clear each entry in the bit map.  */
        media_ptr -> fx_media_fat_secondary_update_map[i] =  0;
    }

    /* Return a successful completion.  */
    return(FX_SUCCESS);
}

