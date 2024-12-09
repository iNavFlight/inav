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
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_FAT_entry_read                          PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the supplied FAT entry from the first FAT of    */
/*    the media.  12-bit, 16-bit, and 32-bit FAT reading is supported.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*    entry_ptr                             Pointer to destination for    */
/*                                            the FAT entry               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_read          Read a UINT from FAT buffer   */
/*    _fx_utility_32_unsigned_read          Read a ULONG form FAT buffer  */
/*    _fx_utility_FAT_flush                 Flush FAT entry cache         */
/*    _fx_utility_logical_sector_read       Read FAT sector into memory   */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable fat entry refresh,  */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Tiejun Zhou              Modified comment(s), and      */
/*                                            fixed compiler warning,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_FAT_entry_read(FX_MEDIA *media_ptr, ULONG cluster, ULONG *entry_ptr)
{

ULONG               FAT_sector;
ULONG               byte_offset, entry32;
UCHAR              *FAT_ptr;
UINT                entry, index;
UINT                status;
FX_FAT_CACHE_ENTRY *cache_entry_ptr;
#ifndef FX_DISABLE_FAT_ENTRY_REFRESH
FX_FAT_CACHE_ENTRY  temp_cache_entry;
#endif /* FX_DISABLE_FAT_ENTRY_REFRESH */


#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled &&
        (media_ptr -> fx_media_fault_tolerant_state & FX_FAULT_TOLERANT_STATE_STARTED))
    {

        /* Redirect this request to log file. */
        status = _fx_fault_tolerant_read_FAT(media_ptr, cluster, entry_ptr, FX_FAULT_TOLERANT_FAT_LOG_TYPE);

        /* Return on success. */
        if (status != FX_READ_CONTINUE)
        {
            return(status);
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

#ifndef FX_MEDIA_STATISTICS_DISABLE
    /* Increment the number of FAT entry reads and cache hits.  */
    media_ptr -> fx_media_fat_entry_reads++;
    media_ptr -> fx_media_fat_entry_cache_read_hits++;
#endif

    /* Extended port-specific processing macro, which is by default defined to white space.  */
    FX_UTILITY_FAT_ENTRY_READ_EXTENSION

    /* Calculate the area of the cache for this FAT entry.  */
    index =  (cluster & FX_FAT_CACHE_HASH_MASK) * FX_FAT_CACHE_DEPTH;

    /* Build a pointer to the cache entry.  */
    cache_entry_ptr =  &media_ptr -> fx_media_fat_cache[index];

#ifndef FX_DISABLE_FAT_ENTRY_REFRESH
    /* Determine if the FAT entry is in the cache - assuming the depth of the FAT cache is
       4 entries.  */
    if ((cache_entry_ptr -> fx_fat_cache_entry_cluster) == cluster)
    {

        /* Yes, return the cached value.  */
        *entry_ptr =  cache_entry_ptr -> fx_fat_cache_entry_value;

        /* Don't move anything since we found the entry.  */

        /* Return a successful status.  */
        return(FX_SUCCESS);
    }
    else if (((cache_entry_ptr + 1) -> fx_fat_cache_entry_cluster) == cluster)
    {

        /* Yes, return the cached value.  */
        *entry_ptr =  (cache_entry_ptr + 1) -> fx_fat_cache_entry_value;

        /* Just swap the first and second entry.  */
        temp_cache_entry =        *(cache_entry_ptr);
        *(cache_entry_ptr) =      *(cache_entry_ptr + 1);
        *(cache_entry_ptr + 1) =  temp_cache_entry;

        /* Return a successful status.  */
        return(FX_SUCCESS);
    }
    else if (((cache_entry_ptr + 2) -> fx_fat_cache_entry_cluster) == cluster)
    {

        /* Yes, return the cached value.  */
        *entry_ptr =  (cache_entry_ptr + 2) -> fx_fat_cache_entry_value;

        /* Move the third entry to the top and the first two entries down.  */
        temp_cache_entry =        *(cache_entry_ptr);
        *(cache_entry_ptr) =      *(cache_entry_ptr + 2);
        *(cache_entry_ptr + 2) =  *(cache_entry_ptr + 1);
        *(cache_entry_ptr + 1) =  temp_cache_entry;

        /* Return a successful status.  */
        return(FX_SUCCESS);
    }
    else if (((cache_entry_ptr + 3) -> fx_fat_cache_entry_cluster) == cluster)
    {

        /* Yes, return the cached value.  */
        *entry_ptr =  (cache_entry_ptr + 3) -> fx_fat_cache_entry_value;

        /* Move the last entry to the top and the first three entries down.  */
        temp_cache_entry =        *(cache_entry_ptr);
        *(cache_entry_ptr) =      *(cache_entry_ptr + 3);
        *(cache_entry_ptr + 3) =  *(cache_entry_ptr + 2);
        *(cache_entry_ptr + 2) =  *(cache_entry_ptr + 1);
        *(cache_entry_ptr + 1) =  temp_cache_entry;

        /* Return a successful status.  */
        return(FX_SUCCESS);
    }
#else
    for (UINT i = 0; i < 4; i++)
    {
        if (((cache_entry_ptr + i) -> fx_fat_cache_entry_cluster) == cluster)
        {
            *entry_ptr =  (cache_entry_ptr + i) -> fx_fat_cache_entry_value;

            /* Return a successful status.  */
            return(FX_SUCCESS);
        }
    }
#endif /* FX_DISABLE_FAT_ENTRY_REFRESH */

    /* Determine if the oldest entry was modified, i.e. whether or not it is
       dirty.  */
    if (media_ptr -> fx_media_fat_cache[index + 3].fx_fat_cache_entry_dirty)
    {

        /* Yes, the entry is dirty and needs to be flushed out.  */
        status = _fx_utility_FAT_flush(media_ptr);

        /* Check for completion status.  */
        if (status != FX_SUCCESS)
        {

            /* Return error status.  */
            return(status);
        }
    }

    /* If we get here, the entry was not found in the FAT entry cache.  We need to
       actually read the FAT entry.  */

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Decrement the number of cache hits.  */
    media_ptr -> fx_media_fat_entry_cache_read_hits--;

    /* Increment the number of cache misses.  */
    media_ptr -> fx_media_fat_entry_cache_read_misses++;
#endif

    /* Determine which type of FAT is present.  */
    if (media_ptr -> fx_media_12_bit_FAT)
    {

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster << 1) + cluster) >> 1;

        /* Calculate the FAT sector the requested FAT entry resides in.  */
        FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
            (ULONG)media_ptr -> fx_media_reserved_sectors;

        /* Read the sector in.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {
            /* Return the error status.  */
            return(status);
        }

        /* Now calculate the byte offset into this FAT sector.  */
        byte_offset =  byte_offset -
            ((FAT_sector - (ULONG)media_ptr -> fx_media_reserved_sectors) *
             media_ptr -> fx_media_bytes_per_sector);

        /* Setup a pointer into the buffer.  */
        FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

        /* Determine if the cluster entry is odd or even.  */
        if (cluster & 1)
        {

            /* Odd cluster number.  */

            /* Pickup the lower nibble of the FAT entry.  */
            entry =  (((UINT)*FAT_ptr) & 0xF0) >> 4;

            /* Move to the next byte of the FAT entry.  */
            FAT_ptr++;

            /* Determine if we are now past the end of the FAT buffer in memory.  */
            if (byte_offset == (ULONG)(media_ptr -> fx_media_bytes_per_sector - 1))
            {

                /* Yes, we need to read the next sector.  */
                FAT_sector++;
                status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                          media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Setup a pointer into the buffer.  */
                FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer;
            }

            /* Pickup the upper 8 bits of the FAT entry.  */
            entry =  entry | (((UINT)*FAT_ptr) << 4);
        }
        else
        {

            /* Even cluster number.  */

            /* Pickup the lower byte of the FAT entry.  */
            entry =  (UINT)(((UINT)*FAT_ptr) & 0xFF);

            /* Move to the next nibble of the FAT entry.  */
            FAT_ptr++;

            /* Determine if we are now past the end of the FAT buffer in memory.  */
            if (byte_offset == (ULONG)(media_ptr -> fx_media_bytes_per_sector - 1))
            {

                /* Yes, we need to read the next sector.  */
                FAT_sector++;
                status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                          media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {
                    return(status);
                }

                /* Setup a pointer into the buffer.  */
                FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer;
            }

            /* Pickup the upper 4 bits of the FAT entry.  */
            entry =  entry | ((((UINT)*FAT_ptr) & 0x0F) << 8);
        }

        /* Determine if we need to do sign extension on the 12-bit eof value.  */
        if (entry >= FX_MAX_12BIT_CLUST)
        {

            /* Yes, we need to sign extend.  */
            entry =  entry | FX_SIGN_EXTEND;
        }

        *entry_ptr =  entry;
    }

    /* Check for a 16-bit FAT.  */
#ifdef FX_ENABLE_EXFAT
    else if (FX_FAT16  == media_ptr -> fx_media_FAT_type)
#else
    else if (!media_ptr -> fx_media_32_bit_FAT)
#endif /* FX_ENABLE_EXFAT */
    {

        /* 16-bit FAT is present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster) * 2);

        /* Calculate the FAT sector the requested FAT entry resides in.  */
        FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
            (ULONG)media_ptr -> fx_media_reserved_sectors;

        /* Read the FAT sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Return the error code.  */
            return(status);
        }

        /* Now calculate the byte offset into this FAT sector.  */
        byte_offset =  byte_offset -
            ((FAT_sector - (ULONG)media_ptr -> fx_media_reserved_sectors) *
             media_ptr -> fx_media_bytes_per_sector);

        /* Setup a pointer into the buffer.  */
        FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (UINT)byte_offset;

        /* Pickup the FAT entry.  */
        entry =  _fx_utility_16_unsigned_read(FAT_ptr);

        *entry_ptr =  entry;
    }
#ifdef FX_ENABLE_EXFAT
    else if ((media_ptr -> fx_media_FAT_type == FX_FAT32) ||
             (media_ptr -> fx_media_FAT_type == FX_exFAT))
#else
    else
#endif /* FX_ENABLE_EXFAT */
    {

        /* Otherwise, a 32 bit FAT present.  */
        byte_offset =  (((ULONG)cluster) * 4);

        /* Calculate the FAT sector the requested FAT entry resides in.  */
        FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
            (ULONG)media_ptr -> fx_media_reserved_sectors;

        /* Calculate the byte offset to the FAT entry.  */
        byte_offset = (byte_offset % media_ptr -> fx_media_bytes_per_sector);

        /* Read the appropriate FAT sector.  */
        status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) FAT_sector,
                                                  media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_FAT_SECTOR);

        /* Determine if an error occurred.  */
        if (status != FX_SUCCESS)
        {

            /* Return the error code.  */
            return(status);
        }

        /* Setup a pointer into the buffer.  */
        FAT_ptr =  (UCHAR *)media_ptr -> fx_media_memory_buffer + (ULONG)byte_offset;

        /* Pickup the FAT entry.  */
        entry32 =  _fx_utility_32_unsigned_read(FAT_ptr);

#ifdef FX_ENABLE_EXFAT
        /* FAT32 uses 28 bit cluster addressing but  exFAT uses 32 bit.  */
        if (media_ptr -> fx_media_FAT_type == FX_FAT32)
        {
#endif /* FX_ENABLE_EXFAT */

            /* Clear upper nibble.  */
            entry32 = entry32 & 0x0FFFFFFF;
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */

        *entry_ptr =  entry32;
    }

    /* Move all the cache entries down so the oldest is at the bottom.  */
    *(cache_entry_ptr + 3) =  *(cache_entry_ptr + 2);
    *(cache_entry_ptr + 2) =  *(cache_entry_ptr + 1);
    *(cache_entry_ptr + 1) =  *(cache_entry_ptr);

    /* Setup the new FAT entry in the cache.  */
    cache_entry_ptr -> fx_fat_cache_entry_cluster =  cluster;
    cache_entry_ptr -> fx_fat_cache_entry_value   =  *entry_ptr;

    /* Return success to the caller.  */
    return(FX_SUCCESS);
}

