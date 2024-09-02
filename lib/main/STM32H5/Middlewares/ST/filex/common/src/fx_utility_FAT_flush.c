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
/*    _fx_utility_FAT_flush                               PORTABLE C      */
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function flushes the contents of the FAT cache to the media.   */
/*    12-bit, 16-bit and 32-bit FAT writing is supported.                 */
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
/*    _fx_utility_16_unsigned_write         Write a UINT into buffer      */
/*    _fx_utility_32_unsigned_read          Read a ULONG from buffer      */
/*    _fx_utility_32_unsigned_write         Write a ULONG into buffer     */
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
/*  11-09-2020     William E. Lamie         Modified comment(s),          */
/*                                            updated logic for           */
/*                                            FAT secondary update map,   */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_FAT_flush(FX_MEDIA *media_ptr)
{

ULONG  FAT_sector;
ULONG  byte_offset;
UCHAR *FAT_ptr;
UINT   temp, i;
UINT   status, index, ind;
ULONG  cluster, next_cluster;
UCHAR  sectors_per_bit;
INT    multi_sector_entry;
ULONG  sector;

#ifndef FX_MEDIA_STATISTICS_DISABLE
    /* Increment the number of cache flush requests.  */
    media_ptr -> fx_media_fat_cache_flushes++;
#endif

    /* Loop through the media's FAT cache and flush out dirty entries.  */
    for (index = 0; index < FX_MAX_FAT_CACHE; index++)
    {

        /* Determine if the entry is dirty.  */
        if ((media_ptr -> fx_media_fat_cache[index].fx_fat_cache_entry_dirty) == 0)
        {

            /* No, just advance to the next entry.  */
            continue;
        }

        /* Otherwise, the entry is indeed dirty and must be flushed out.  Process
           relative to the type of FAT that is being used.  */

        /* Pickup the contents of the FAT cache entry.  */
        cluster =       media_ptr -> fx_media_fat_cache[index].fx_fat_cache_entry_cluster;

        /* Determine which type of FAT is present.  */
#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_FAT_type == FX_FAT12)
#else
        if (media_ptr -> fx_media_12_bit_FAT)
#endif /* FX_ENABLE_EXFAT */
        {

            /* Calculate the byte offset to the cluster entry.  */
            byte_offset =  (((ULONG)cluster << 1) + cluster) >> 1;

            /* Calculate the FAT sector the requested FAT entry resides in.  */
            FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
                (ULONG)media_ptr -> fx_media_reserved_sectors;

            /* Initialize as not written.  */
            multi_sector_entry = -1;

            for (;;)
            {

                /* Pickup the FAT sector.  */
                status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                          media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Determine if a mulit-sector FAT update is present.  */
                if (multi_sector_entry != -1)
                {

                    /* Yes, store the remaining portion of the new FAT entry in the
                       next FAT sector.  */

                    /* Setup a pointer into the buffer.  */
                    FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer;

                    /* Pickup the cluster and next cluster.  */
                    cluster = (media_ptr -> fx_media_fat_cache[multi_sector_entry].fx_fat_cache_entry_cluster);
                    next_cluster = media_ptr -> fx_media_fat_cache[multi_sector_entry].fx_fat_cache_entry_value;

                    /* Determine if the cluster entry is odd or even.  */
                    if (cluster & 1)
                    {

                        /* Store the upper 8 bits of the FAT entry.  */
                        *FAT_ptr =  (UCHAR)((next_cluster >> 4) & 0xFF);
                    }
                    else
                    {

                        /* Store the upper 4 bits of the FAT entry.  */
                        temp =  ((UINT)*FAT_ptr) & 0xF0;
                        *FAT_ptr =  (UCHAR)(temp | ((next_cluster >> 8) & 0xF));
                    }

                    /* Clear the multi-sector flag.  */
                    multi_sector_entry = -1;
                }

                /* Loop through the remainder of the cache to check for multiple entries
                   within the same FAT sector being written out.  */
                for (i = index; i < FX_MAX_FAT_CACHE; i++)
                {

                    /* Is the cache entry dirty?  */
                    if ((media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty) == 0)
                    {

                        /* Not dirty, does not need to be flushed.  */
                        continue;
                    }

                    /* Isolate the cluster.  */
                    cluster = (media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_cluster);

                    /* Calculate the byte offset to the cluster entry.  */
                    byte_offset =  (((ULONG)cluster << 1) + cluster) >> 1;

                    /* Pickup the sector.  */
                    sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
                        (ULONG)media_ptr -> fx_media_reserved_sectors;

                    /* Is it the current FAT sector?  */
                    if (sector != FAT_sector)
                    {

                        /* Different FAT sector - not in this pass of the loop.  */
                        continue;
                    }

                    /* Pickup new value for this FAT entry.  */
                    next_cluster =  media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_value;

                    /* Now calculate the byte offset into this FAT sector.  */
                    byte_offset =  byte_offset -
                        ((FAT_sector - (ULONG)media_ptr -> fx_media_reserved_sectors) *
                         media_ptr -> fx_media_bytes_per_sector);

                    /* Determine if we are now past the end of the FAT buffer in memory.  */
                    if (byte_offset == (ULONG)(media_ptr -> fx_media_bytes_per_sector - 1))
                    {

                        /* Yes, we need to read the next sector */
                        multi_sector_entry = (INT)i;
                    }

                    /* Setup a pointer into the buffer.  */
                    FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

                    /* Clear the dirty flag.  */
                    media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty = 0;

                    /* Determine if the cluster entry is odd or even.  */
                    if (cluster & 1)
                    {

                        /* Odd cluster number.  */

                        /* Pickup the upper nibble of the FAT entry.  */

                        /* First, set the lower nibble of the FAT entry.  */
                        temp =      (((UINT)*FAT_ptr) & 0x0F);
                        *FAT_ptr =  (UCHAR)(temp | ((next_cluster << 4) & 0xF0));

                        /* Determine if this is a mulit-sector entry.  */
                        if ((multi_sector_entry) == (INT)i)
                        {

                            /* Yes, requires multiple sector - will write rest of the part later.  */
                            continue;
                        }

                        /* Move to the next byte of the FAT entry.  */
                        FAT_ptr++;

                        /* Store the upper 8 bits of the FAT entry.  */
                        *FAT_ptr =  (UCHAR)((next_cluster >> 4) & 0xFF);
                    }
                    else
                    {

                        /* Even cluster number.  */

                        /* Store the lower byte of the FAT entry.  */
                        *FAT_ptr =  (UCHAR)(next_cluster & 0xFF);

                        /* Determine if this is a mulit-sector entry.  */
                        if ((multi_sector_entry) == (INT)i)
                        {

                            /* Yes, requires multiple sector - will write rest of the part later.  */
                            continue;
                        }

                        /* Move to the next nibble of the FAT entry.  */
                        FAT_ptr++;

                        /* Store the upper 4 bits of the FAT entry.  */
                        temp =  ((UINT)*FAT_ptr) & 0xF0;
                        *FAT_ptr =  (UCHAR)(temp | ((next_cluster >> 8) & 0xF));
                    }
                }

                /* First, write out the current sector. */
                status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) FAT_sector,
                                                           media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);
                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Mark the FAT sector update bit map to indicate this sector has been written.  */
                if (media_ptr -> fx_media_sectors_per_FAT % (FX_FAT_MAP_SIZE << 3) == 0)
                {
                    sectors_per_bit =  (UCHAR)((UINT)media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3));
                }
                else
                {
                    sectors_per_bit =  (UCHAR)((UINT)media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3) + 1);
                }

                /* Check for invalid value.  */
                if (sectors_per_bit == 0)
                {

                    /* Invalid media, return error.  */
                    return(FX_MEDIA_INVALID);
                }

                ind = ((FAT_sector - media_ptr -> fx_media_reserved_sectors) / sectors_per_bit) >> 3;
                media_ptr -> fx_media_fat_secondary_update_map[ind] = 
                    (UCHAR)((INT)media_ptr -> fx_media_fat_secondary_update_map[ind]
                    | (1 <<(((FAT_sector - media_ptr -> fx_media_reserved_sectors) / sectors_per_bit) & 7)));

                /* Determine if the multi-sector flag is set.  */
                if (multi_sector_entry != -1)
                {

                    /* Yes, position to the next sector and read it in.  */
                    FAT_sector++;
                }
                else
                {

                    /* No, we are finished with this loop.   */
                    break;
                }
            }
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

            /* Calculate the FAT sector the requested FAT entry resides in.  */
            FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
                (ULONG)media_ptr -> fx_media_reserved_sectors;

            /* Read the FAT sector.  */
            status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

            /* Loop through the remainder of the cache to check for multiple entries
               within the same FAT sector being written out.  */
            for (i = index; i < FX_MAX_FAT_CACHE; i++)
            {

                /* Determine if the entry is dirty.  */
                if (media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty == 0)
                {

                    /* Not dirty, does not need to be flushed.  */
                    continue;
                }

                /* Isolate the cluster.  */
                cluster = (media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_cluster);

                /* Calculate the byte offset to the cluster entry.  */
                byte_offset =  (((ULONG)cluster) * 2);

                /* Pickup the sector.  */
                sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
                    (ULONG)media_ptr -> fx_media_reserved_sectors;

                /* Is it the current FAT sector?  */
                if (sector != FAT_sector)
                {

                    /* Different FAT sector - not in this pass of the loop.  */
                    continue;
                }

                /* Now calculate the byte offset into this FAT sector.  */
                byte_offset =  byte_offset -
                    ((FAT_sector - (ULONG)media_ptr -> fx_media_reserved_sectors) *
                     media_ptr -> fx_media_bytes_per_sector);

                /* Setup a pointer into the buffer.  */
                FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

                /* Pickup new value for this FAT entry.  */
                next_cluster =  media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_value;

                /* Store the FAT entry.  */
                _fx_utility_16_unsigned_write(FAT_ptr, (UINT)next_cluster);

                /* Clear the dirty flag.  */
                media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty = 0;
            }

            /* Write the last written FAT sector out.  */
            status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) FAT_sector,
                                                       media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {
                /* Return the error status.  */
                return(status);
            }

            /* Mark the FAT sector update bit map to indicate this sector has been
               written.  */
            if (media_ptr -> fx_media_sectors_per_FAT % (FX_FAT_MAP_SIZE << 3) == 0)
            {
                sectors_per_bit =  (UCHAR)(media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3));
            }
            else
            {
                sectors_per_bit =  (UCHAR)((media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3)) + 1);
            }
            ind = ((FAT_sector - media_ptr -> fx_media_reserved_sectors) / sectors_per_bit) >> 3;
            media_ptr -> fx_media_fat_secondary_update_map[ind] = 
                (UCHAR)((INT)media_ptr -> fx_media_fat_secondary_update_map[ind]
                | (1 <<(((FAT_sector - media_ptr -> fx_media_reserved_sectors) / sectors_per_bit) & 7)));
        }
        else
        {

            /* 32-bit FAT or exFAT are present.  */

            /* Calculate the byte offset to the cluster entry.  */
            byte_offset =  (((ULONG)cluster) * 4);

            /* Calculate the FAT sector the requested FAT entry resides in.  */
            FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
                (ULONG)media_ptr -> fx_media_reserved_sectors;

            /* Read the FAT sector.  */
            status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                      media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

            /* Loop through the remainder of the cache to check for multiple entries
               within the same FAT sector being written out.  */
            for (i = index; i < FX_MAX_FAT_CACHE; i++)
            {

                /* Determine if the entry is dirty.  */
                if (media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty == 0)
                {

                    /* Not dirty, does not need to be flushed.  */
                    continue;
                }

                /* Isolate the cluster.  */
                cluster = (media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_cluster);

                /* Calculate the byte offset to the cluster entry.  */
                byte_offset =  (((ULONG)cluster) * 4);

                /* Pickup the sector.  */
                sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
                    (ULONG)media_ptr -> fx_media_reserved_sectors;

                /* Is it the current FAT sector?  */
                if (sector != FAT_sector)
                {

                    /* Different FAT sector - not in this pass of the loop.  */
                    continue;
                }

                /* Now calculate the byte offset into this FAT sector.  */
                byte_offset =  byte_offset -
                    ((FAT_sector - (ULONG)media_ptr -> fx_media_reserved_sectors) *
                     media_ptr -> fx_media_bytes_per_sector);

                /* Setup a pointer into the buffer.  */
                FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

                /* Pickup new value for this FAT entry.  */
                next_cluster =  media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_value;

                /* Store the FAT entry.  */
                _fx_utility_32_unsigned_write(FAT_ptr, next_cluster);

                /* Clear the dirty flag.  */
                media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty = 0;
            }

            /* Write the last written FAT sector out.  */
            status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) FAT_sector,
                                                       media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

            /* Determine if an error occurred.  */
            if (status != FX_SUCCESS)
            {

                /* Return the error status.  */
                return(status);
            }

#ifdef FX_ENABLE_EXFAT
            /* We are not using fx_media_fat_secondary_update_map for exFAT.  */
            if (media_ptr -> fx_media_FAT_type == FX_FAT32)
            {
#endif /* FX_ENABLE_EXFAT */

                /* Mark the FAT sector update bit map to indicate this sector has been
                   written.  */
                if (media_ptr -> fx_media_sectors_per_FAT % (FX_FAT_MAP_SIZE << 3) == 0)
                {
                    sectors_per_bit =  (UCHAR)(media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3));
                }
                else
                {
                    sectors_per_bit =  (UCHAR)((media_ptr -> fx_media_sectors_per_FAT / (FX_FAT_MAP_SIZE << 3)) + 1);
                }
                ind = ((FAT_sector - media_ptr -> fx_media_reserved_sectors) / sectors_per_bit) >> 3;
                media_ptr -> fx_media_fat_secondary_update_map[ind] = 
                    (UCHAR)((INT)media_ptr -> fx_media_fat_secondary_update_map[ind]
                    | (1 <<(((FAT_sector - media_ptr -> fx_media_reserved_sectors) / sectors_per_bit) & 7)));
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */
        }
    }

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* While fault_tolerant is enabled, this function will be called before the return of all APIs. */
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {

        /* Delete the record of FAT sector since all FAT entries are flushed. */
        media_ptr -> fx_media_fault_tolerant_cached_FAT_sector = 0;
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Return successful status.  */
    return(FX_SUCCESS);
}

