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
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_open                                      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function opens the specified media with the supplied device    */
/*    driver.  The specified media must conform to the FAT compatible     */
/*    file format, which is verified during the media open process.  In   */
/*    addition, the supplied FileX device driver must also conform to     */
/*    the FileX device driver specification.                              */
/*                                                                        */
/*    The FAT boot sector (512 bytes) that is verified by this            */
/*    function must look like the following:                              */
/*                                                                        */
/*          Byte Offset         Meaning             Size                  */
/*                                                                        */
/*            0x000         Jump Instructions        3                    */
/*            0x003         OEM Name                 8                    */
/*            0x00B        *Bytes per Sector         2                    */
/*            0x00D        *Sectors per Cluster      1                    */
/*            0x00E        *Reserved Sectors         2                    */
/*            0x010        *Number of FATs           1                    */
/*            0x011        *Max Root Dir Entries     2                    */
/*            0x013        *Number of Sectors        2                    */
/*            0x015         Media Type               1                    */
/*            0x016        *Sectors per FAT          2                    */
/*            0x018        *Sectors per Track        2                    */
/*            0x01A        *Number of Heads          2                    */
/*            0x01C        *Hidden Sectors           4                    */
/*            0x020        *Huge Sectors             4                    */
/*            0x024         Drive Number             1                    */
/*            0x025         Reserved                 1                    */
/*            0x026         Boot Signature           1                    */
/*            0x027         Volume ID                4                    */
/*            0x02B         Volume Label             11                   */
/*            0x036         File System Type         8                    */
/*             ...              ...                 ...                   */
/*            0x1FE       **Signature (0x55aa)       2                    */
/*                                                                        */
/*            * Denotes which elements of the boot record                 */
/*              FileX uses.                                               */
/*                                                                        */
/*            **Denotes the element is checked by the I/O                 */
/*              driver.  This eliminates the need for a minimum           */
/*              512-byte buffer for FileX.                                */
/*                                                                        */
/*  Note: All values above are in little endian format, i.e. the LSB is   */
/*        in the lowest address.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    media_name                            Pointer to media name string  */
/*    media_driver                          Media driver entry function   */
/*    driver_info_ptr                       Optional information pointer  */
/*                                            supplied to media driver    */
/*    memory_ptr                            Pointer to memory used by the */
/*                                            FileX for this media.       */
/*    memory_size                           Size of media memory - must   */
/*                                            at least 512 bytes and      */
/*                                            one sector size.            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    I/O Driver                                                          */
/*    _fx_utility_exFAT_bitmap_initialize   Initialize exFAT bitmap       */
/*    _fx_utility_16_unsigned_read          Read 16-bit unsigned value    */
/*    _fx_utility_32_unsigned_read          Read 32-bit unsigned value    */
/*    _fx_utility_logical_sector_flush      Invalidate log sector cache   */
/*    _fx_media_boot_info_extract           Extract media information     */
/*    _fx_utility_FAT_entry_read            Pickup FAT entry contents     */
/*    tx_mutex_create                       Create protection mutex       */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable force memset,       */
/*                                            build options and cache,    */
/*                                            resulting in version 6.1    */
/*  01-31-2022     William E. Lamie         Modified comment(s), fixed    */
/*                                            errors without cache,       */
/*                                            resulting in version 6.1.10 */
/*  10-31-2022     Tiejun Zhou              Modified comment(s),          */
/*                                            fixed memory buffer when    */
/*                                            cache is disabled,          */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                     VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                     VOID *memory_ptr, ULONG memory_size)
{

FX_MEDIA_PTR      tail_ptr;
ULONG             cluster_number;
ULONG             FAT_entry, FAT_sector, FAT_read_sectors;
ULONG             i, j;
#ifndef FX_DISABLE_CACHE
FX_CACHED_SECTOR *cache_entry_ptr;
#endif /* FX_DISABLE_CACHE */
UINT              status;
UINT              additional_info_sector;
UCHAR            *original_memory_ptr;
ULONG             bytes_in_buffer;
FX_INT_SAVE_AREA


#ifndef FX_DISABLE_BUILD_OPTIONS
    /* Reference the version ID and option words to ensure they are linked in.  */
    if ((_fx_system_build_options_1 | _fx_system_build_options_2 | _fx_system_build_options_3) == 0 ||
        _fx_version_id[0] == 0)
    {

        /* We should never get here!  */
        return(FX_NOT_IMPLEMENTED);
    }
#endif /* FX_DISABLE_BUILD_OPTIONS */

#ifdef FX_DISABLE_FORCE_MEMORY_OPERATION
    _fx_utility_memory_set((UCHAR *)media_ptr, 0, sizeof(FX_MEDIA));
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */
#ifdef FX_DISABLE_CACHE
    media_ptr -> fx_media_memory_buffer_sector = (ULONG64)-1;
#endif /* FX_DISABLE_CACHE */

    /* Save the basic information in the media control block.  */
    media_ptr -> fx_media_name =                        media_name;
    media_ptr -> fx_media_driver_entry =                media_driver;
    media_ptr -> fx_media_memory_buffer =               (UCHAR *)memory_ptr;
    media_ptr -> fx_media_memory_size =                 memory_size;
#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    media_ptr -> fx_media_disable_burst_cache =         FX_FALSE;
    media_ptr -> fx_media_FAT_type =                    0;
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

    /* Save the original memory pointer.  */
    original_memory_ptr =  (UCHAR *)memory_ptr;

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Clear the optional media statistics.  */
    media_ptr -> fx_media_directory_attributes_reads =  0;
    media_ptr -> fx_media_directory_attributes_sets =  0;
    media_ptr -> fx_media_directory_creates =  0;
    media_ptr -> fx_media_directory_default_gets =  0;
    media_ptr -> fx_media_directory_default_sets =  0;
    media_ptr -> fx_media_directory_deletes =  0;
    media_ptr -> fx_media_directory_first_entry_finds =  0;
    media_ptr -> fx_media_directory_first_full_entry_finds =  0;
    media_ptr -> fx_media_directory_information_gets =  0;
    media_ptr -> fx_media_directory_local_path_clears =  0;
    media_ptr -> fx_media_directory_local_path_gets =  0;
    media_ptr -> fx_media_directory_local_path_restores =  0;
    media_ptr -> fx_media_directory_local_path_sets =  0;
    media_ptr -> fx_media_directory_name_tests =  0;
    media_ptr -> fx_media_directory_next_entry_finds =  0;
    media_ptr -> fx_media_directory_next_full_entry_finds =  0;
    media_ptr -> fx_media_directory_renames =  0;
    media_ptr -> fx_media_file_allocates =  0;
    media_ptr -> fx_media_file_attributes_reads =  0;
    media_ptr -> fx_media_file_attributes_sets =  0;
    media_ptr -> fx_media_file_best_effort_allocates =  0;
    media_ptr -> fx_media_file_closes =  0;
    media_ptr -> fx_media_file_creates =  0;
    media_ptr -> fx_media_file_deletes =  0;
    media_ptr -> fx_media_file_opens =  0;
    media_ptr -> fx_media_file_reads =  0;
    media_ptr -> fx_media_file_relative_seeks =  0;
    media_ptr -> fx_media_file_renames =  0;
    media_ptr -> fx_media_file_seeks =  0;
    media_ptr -> fx_media_file_truncates =  0;
    media_ptr -> fx_media_file_truncate_releases =  0;
    media_ptr -> fx_media_file_writes =  0;
    media_ptr -> fx_media_aborts =  0;
    media_ptr -> fx_media_flushes =  0;
    media_ptr -> fx_media_reads =  0;
    media_ptr -> fx_media_writes =  0;
    media_ptr -> fx_media_directory_entry_reads =  0;
    media_ptr -> fx_media_directory_entry_writes =  0;
    media_ptr -> fx_media_directory_searches =  0;
#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE
    media_ptr -> fx_media_directory_search_cache_hits =  0;
#endif
    media_ptr -> fx_media_directory_free_searches =  0;
    media_ptr -> fx_media_fat_entry_reads =  0;
    media_ptr -> fx_media_fat_entry_writes =  0;
    media_ptr -> fx_media_fat_entry_cache_read_hits =  0;
    media_ptr -> fx_media_fat_entry_cache_read_misses =  0;
    media_ptr -> fx_media_fat_entry_cache_write_hits =  0;
    media_ptr -> fx_media_fat_entry_cache_write_misses =  0;
    media_ptr -> fx_media_fat_cache_flushes =  0;
    media_ptr -> fx_media_fat_sector_reads =  0;
    media_ptr -> fx_media_fat_sector_writes =  0;
    media_ptr -> fx_media_logical_sector_reads =  0;
    media_ptr -> fx_media_logical_sector_writes =  0;
    media_ptr -> fx_media_logical_sector_cache_read_hits =  0;
    media_ptr -> fx_media_logical_sector_cache_read_misses =  0;
    media_ptr -> fx_media_driver_read_requests =  0;
    media_ptr -> fx_media_driver_write_requests =  0;
    media_ptr -> fx_media_driver_boot_read_requests =  0;
    media_ptr -> fx_media_driver_boot_write_requests =  0;
    media_ptr -> fx_media_driver_release_sectors_requests =  0;
    media_ptr -> fx_media_driver_flush_requests =  0;
#endif
#ifdef FX_ENABLE_FAULT_TOLERANT
    media_ptr -> fx_media_fault_tolerant_enabled = FX_FALSE;
    media_ptr -> fx_media_fault_tolerant_state = 0;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_OPEN, media_ptr, media_driver, memory_ptr, memory_size, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Initialize the supplied media I/O driver.  First, build the
       initialize driver request.  */
    media_ptr -> fx_media_driver_request =              FX_DRIVER_INIT;
    media_ptr -> fx_media_driver_status =               FX_IO_ERROR;
    media_ptr -> fx_media_driver_info =                 driver_info_ptr;
    media_ptr -> fx_media_driver_write_protect =        FX_FALSE;
    media_ptr -> fx_media_driver_free_sector_update =   FX_FALSE;
    media_ptr -> fx_media_driver_data_sector_read =     FX_FALSE;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_INIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Call the specified I/O driver with the initialize request.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Determine if the I/O driver initialized successfully.  */
    if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
    {

        /* Return the driver error status.  */
        return(FX_IO_ERROR);
    }

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of driver boot read requests.  */
    media_ptr -> fx_media_driver_boot_read_requests++;
#endif

    /* Read the boot sector from the device.  Build the read boot sector
       command.  */
    media_ptr -> fx_media_driver_request =          FX_DRIVER_BOOT_READ;
    media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
    media_ptr -> fx_media_driver_buffer =           memory_ptr;
    media_ptr -> fx_media_driver_sectors =          1;
    media_ptr -> fx_media_driver_sector_type =      FX_BOOT_SECTOR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_BOOT_READ, media_ptr, memory_ptr, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Invoke the driver to read the boot sector.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Determine if the boot sector was read correctly. */
    if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
    {

        /* Build the "uninitialize" I/O driver request.  */
        media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
        media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Call the specified I/O driver with the uninitialize request.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Return the boot sector error status.  */
        return(FX_BOOT_ERROR);
    }

    /* Extract and validate the media parameters from the boot sector.  */
    if (_fx_media_boot_info_extract(media_ptr) != FX_SUCCESS)
    {

        /* Build the "uninitialize" I/O driver request.  */
        media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
        media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Call the specified I/O driver with the uninitialize request.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Return the invalid media error status.  */
        return(FX_MEDIA_INVALID);
    }

    /* Pickup the additional info sector number. This will only be used in FAT32 situations.  */
    additional_info_sector =  _fx_utility_16_unsigned_read(&media_ptr -> fx_media_driver_buffer[48]);

    /* Is there at least one?  */
    if (memory_size < media_ptr -> fx_media_bytes_per_sector)
    {

        /* Build the "uninitialize" I/O driver request.  */
        media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
        media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Call the specified I/O driver with the uninitialize request.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Error in the buffer size supplied by user.  */
        return(FX_BUFFER_ERROR);
    }

#ifndef FX_DISABLE_CACHE
    /* Determine how many logical sectors can be cached with user's supplied
       buffer area - there must be at least enough for one sector!  */
    media_ptr -> fx_media_sector_cache_size =  memory_size / media_ptr -> fx_media_bytes_per_sector;

    /* If trace is enabled, register this object.  */
    FX_TRACE_OBJECT_REGISTER(FX_TRACE_OBJECT_TYPE_MEDIA, media_ptr, media_name, FX_MAX_FAT_CACHE, media_ptr -> fx_media_sector_cache_size)
    
    /* Adjust the internal cache to fit the fixed number of sector cache control blocks
       built into the media control block.  */
    if (media_ptr -> fx_media_sector_cache_size > FX_MAX_SECTOR_CACHE)
    {

        /* Adjust the number of cache sectors downward.  If this is insufficient,
           the FX_MAX_SECTOR_CACHE constant in FX_API.H must be changed and the FileX
           library must be rebuilt.  */
        media_ptr -> fx_media_sector_cache_size =  FX_MAX_SECTOR_CACHE;
    }

    /* Otherwise, everything is okay.  Initialize the data structures for managing the
       logical sector cache.  */
    i =  (UINT)media_ptr -> fx_media_sector_cache_size;
    cache_entry_ptr =  media_ptr -> fx_media_sector_cache;
    while (i--)
    {

        /* Initialize each of the cache entries.  */
        cache_entry_ptr -> fx_cached_sector_memory_buffer =  (UCHAR *)memory_ptr;
        cache_entry_ptr -> fx_cached_sector =                (~(ULONG64)0);
        cache_entry_ptr -> fx_cached_sector_buffer_dirty =   FX_FALSE;
        cache_entry_ptr -> fx_cached_sector_valid =          FX_FALSE;
        cache_entry_ptr -> fx_cached_sector_next_used =      cache_entry_ptr + 1;

        /* Move to the next cache sector entry.  */
        cache_entry_ptr++;

        /* Update the memory pointer to the next buffer slot.  */
        memory_ptr =  (VOID *)(((UCHAR *)memory_ptr) + media_ptr -> fx_media_bytes_per_sector);
    }

    /* Backup to the last cache entry to set its next pointer to NULL.  */
    cache_entry_ptr--;
    cache_entry_ptr -> fx_cached_sector_next_used =  FX_NULL;

    /* Remember the last memory address used by the caching logic.  */
    media_ptr -> fx_media_sector_cache_end =  ((UCHAR *)memory_ptr) - 1;

    /* Setup the head pointer of the list.  */
    media_ptr -> fx_media_sector_cache_list_ptr =  media_ptr -> fx_media_sector_cache;

    /* Setup the bit map that keeps track of the valid hashed cache logical sectors.  */
    media_ptr -> fx_media_sector_cache_hashed_sector_valid =  0;

    /* Clear the counter of the number of outstanding dirty sectors.  */
    media_ptr -> fx_media_sector_cache_dirty_count =  0;

    /* Determine if the logical sector cache should be managed by the hash function
       instead of the linear search. The cache must be a power of 2 that is between the
       minimum and maximum cache size.  */
    if ((media_ptr -> fx_media_sector_cache_size >= FX_SECTOR_CACHE_HASH_ENABLE) &&
        ((media_ptr -> fx_media_sector_cache_size ^ (media_ptr -> fx_media_sector_cache_size - 1)) ==
         (media_ptr -> fx_media_sector_cache_size | (media_ptr -> fx_media_sector_cache_size - 1))))
    {


        /* Set the logical sector cache hash flag. When this flag is set, the logical
           sector cache is accessed with a hash function instead of a linear search.  */
        media_ptr -> fx_media_sector_cache_hashed =  FX_TRUE;
        media_ptr -> fx_media_sector_cache_hash_mask =
            ((media_ptr -> fx_media_sector_cache_size / FX_SECTOR_CACHE_DEPTH) - 1);
    }
    else
    {

        /* Clear the logical sector cache flag.  */
        media_ptr -> fx_media_sector_cache_hashed =  FX_FALSE;
    }
#else
    media_ptr -> fx_media_memory_buffer = memory_ptr;
#endif /* FX_DISABLE_CACHE */

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    /* Initialize the FAT cache entry array.  */
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {

        /* Clear entry in the FAT cache.  */
        media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_cluster =   0;
        media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_value   =   0;
        media_ptr -> fx_media_fat_cache[i].fx_fat_cache_entry_dirty   =   0;
    }

    /* Initialize the secondary FAT update map.  */
    for (i = 0; i < FX_FAT_MAP_SIZE; i++)
    {

        /* Clear bit map entry for secondary FAT update.  */
        media_ptr -> fx_media_fat_secondary_update_map[i] =  0;
    }
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type != FX_exFAT)
    {
#endif /* FX_ENABLE_EXFAT */

        /* Root_sector_start has been computed */
        media_ptr -> fx_media_root_sector_start =  media_ptr -> fx_media_reserved_sectors +
            (media_ptr -> fx_media_number_of_FATs *
             media_ptr -> fx_media_sectors_per_FAT);

        /* Calculate the number of directory sectors.  */
        media_ptr -> fx_media_root_sectors =
            ((media_ptr -> fx_media_root_directory_entries * FX_DIR_ENTRY_SIZE) +
             media_ptr -> fx_media_bytes_per_sector - 1) /
            media_ptr -> fx_media_bytes_per_sector;

        /* Calculate the starting data sector.  */
        media_ptr -> fx_media_data_sector_start =  media_ptr -> fx_media_root_sector_start +
            media_ptr -> fx_media_root_sectors;

        /* Calculate the total number of clusters.  */
        media_ptr -> fx_media_total_clusters =  (ULONG)((media_ptr -> fx_media_total_sectors - media_ptr -> fx_media_data_sector_start) /
                                                            media_ptr -> fx_media_sectors_per_cluster);

        /* Determine if a 12-bit FAT is in use.  */
        if (media_ptr -> fx_media_total_clusters < FX_12_BIT_FAT_SIZE)
        {

            /* Yes, 12-bit FAT is present.  Set flag accordingly.  */
            media_ptr -> fx_media_12_bit_FAT = FX_TRUE;
            media_ptr -> fx_media_32_bit_FAT = FX_FALSE;
#ifdef FX_ENABLE_EXFAT
            media_ptr -> fx_media_FAT_type = FX_FAT12;
#endif /* FX_ENABLE_EXFAT */

            /* No additional information sector in FAT12.  */
            media_ptr -> fx_media_FAT32_additional_info_sector =  0;

            /* Set FAT last and FAT reserved. */
            media_ptr -> fx_media_fat_reserved = FX_RESERVED_1;
            media_ptr -> fx_media_fat_last = FX_LAST_CLUSTER_2;
        }
        else if (media_ptr -> fx_media_total_clusters < FX_16_BIT_FAT_SIZE)
        {

            /* A 16-bit FAT is present.  Set flag accordingly.  */
            media_ptr -> fx_media_12_bit_FAT =  FX_FALSE;
            media_ptr -> fx_media_32_bit_FAT =  FX_FALSE;
#ifdef FX_ENABLE_EXFAT
            media_ptr -> fx_media_FAT_type = FX_FAT16;
#endif /* FX_ENABLE_EXFAT */

            /* No additional information sector in FAT16.  */
            media_ptr -> fx_media_FAT32_additional_info_sector =  0;

            /* Set FAT last and FAT reserved. */
            media_ptr -> fx_media_fat_reserved = FX_RESERVED_1;
            media_ptr -> fx_media_fat_last = FX_LAST_CLUSTER_2;
        }
        else
        {

            /* Yes, a 32-bit FAT is present.  */
            media_ptr -> fx_media_12_bit_FAT =  FX_FALSE;
            media_ptr -> fx_media_32_bit_FAT =  FX_TRUE;
#ifdef FX_ENABLE_EXFAT
            media_ptr -> fx_media_FAT_type = FX_FAT32;
#endif /* FX_ENABLE_EXFAT */

            /* Save the additional information sector FAT32. This was read from the boot
               sector earlier in this routine. */
            media_ptr -> fx_media_FAT32_additional_info_sector =  additional_info_sector;

            /* Set FAT last and FAT reserved. */
            media_ptr -> fx_media_fat_reserved = FX_RESERVED_1_32;
            media_ptr -> fx_media_fat_last = FX_LAST_CLUSTER_2_32;
        }
#ifdef FX_ENABLE_EXFAT
    }
    else
    {

        /* Set FAT last and FAT reserved. */
        media_ptr -> fx_media_fat_reserved = FX_RESERVED_1_exFAT;
        media_ptr -> fx_media_fat_last = FX_LAST_CLUSTER_exFAT;
    }
#endif /* FX_ENABLE_EXFAT */

    /* Determine if a 32-bit FAT is present. If so, calculate the size of the root directory (since
       it is variable in FAT32.  */
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_32_bit_FAT == FX_TRUE || 
        (media_ptr -> fx_media_FAT_type == FX_exFAT))
#else
    if (media_ptr -> fx_media_32_bit_FAT == FX_TRUE)
#endif /* FX_ENABLE_EXFAT */
    {
#ifdef FX_ENABLE_EXFAT
        if (media_ptr -> fx_media_32_bit_FAT == FX_TRUE)
        {
#endif /* FX_ENABLE_EXFAT */

            /* Root First cluster starts from at least cluster 2, or higher. */
            if (media_ptr -> fx_media_root_cluster_32 < FX_FAT_ENTRY_START)
            {
                return(FX_MEDIA_INVALID);
            }

            /* Calculate logical number of root dir sector.  */
            media_ptr -> fx_media_root_sector_start = media_ptr -> fx_media_data_sector_start +
                (media_ptr -> fx_media_root_cluster_32 - FX_FAT_ENTRY_START) *
                media_ptr -> fx_media_sectors_per_cluster;
#ifdef FX_ENABLE_EXFAT
        }
#endif /* FX_ENABLE_EXFAT */

        /* Calculate maximum possible value for fx_media_root_directory_entries */
        i = 0;
        for (cluster_number = media_ptr -> fx_media_root_cluster_32;;)
        {

            status =  _fx_utility_FAT_entry_read(media_ptr, cluster_number, &FAT_entry);
            i++;
            /* Determine if the read was successful.  */
            if (status != FX_SUCCESS)
            {

                /* Build the "uninitialize" I/O driver request.  */
                media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
                media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Call the specified I/O driver with the uninitialize request.  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

                return(FX_FAT_READ_ERROR);
            }

            if ((cluster_number == FAT_entry) || (i > media_ptr -> fx_media_total_clusters))
            {

                /* Build the "uninitialize" I/O driver request.  */
                media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
                media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Call the specified I/O driver with the uninitialize request.  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

                return(FX_FAT_READ_ERROR);
            }
            if (FAT_entry >= FX_RESERVED_1_32)
            {
                break;
            }
            cluster_number = FAT_entry;
        }

        /* Calculate the number of directory entries.  */
        media_ptr -> fx_media_root_directory_entries =  (i * media_ptr -> fx_media_sectors_per_cluster *
                                                         media_ptr -> fx_media_bytes_per_sector) / FX_DIR_ENTRY_SIZE;
    }

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    /* Calculate the number of available clusters.  */
    media_ptr -> fx_media_available_clusters =  0;

    /* Set the cluster search start to an invalid value.  */
    media_ptr -> fx_media_cluster_search_start =  0;
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

    /* Determine if there is 32-bit FAT additional information sector. */
    if (media_ptr -> fx_media_FAT32_additional_info_sector)
    {

    UCHAR *buffer_ptr;
    ULONG  signature;


        /* Yes, read the FAT32 additional information sector to get the available cluster count and
           the hint for the first available cluster.  */

#ifndef FX_DISABLE_CACHE
        /* Setup a pointer to the first cached entry's buffer.  */
        buffer_ptr =  (media_ptr -> fx_media_sector_cache_list_ptr) -> fx_cached_sector_memory_buffer;

        /* Invalidate this cache entry.  */
        (media_ptr -> fx_media_sector_cache_list_ptr) -> fx_cached_sector =  (~((ULONG64) 0));
        (media_ptr -> fx_media_sector_cache_list_ptr) -> fx_cached_sector_valid =  FX_FALSE;
#else
        buffer_ptr =  media_ptr -> fx_media_memory_buffer;
        media_ptr -> fx_media_memory_buffer_sector = (ULONG64)-1;
#endif /* FX_DISABLE_CACHE */

        /* Read the FAT32 additional information sector from the device.  */
        media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
        media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
        media_ptr -> fx_media_driver_buffer =           buffer_ptr;
        media_ptr -> fx_media_driver_logical_sector =   media_ptr -> fx_media_FAT32_additional_info_sector;
        media_ptr -> fx_media_driver_sectors =          1;
        media_ptr -> fx_media_driver_sector_type =      FX_DIRECTORY_SECTOR;

#ifndef FX_MEDIA_STATISTICS_DISABLE

        /* Increment the number of driver read sector(s) requests.  */
        media_ptr -> fx_media_driver_read_requests++;
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_READ, media_ptr, media_ptr -> fx_media_FAT32_additional_info_sector, 1, buffer_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Invoke the driver to read the FAT32 additional information sector.  */
        (media_ptr -> fx_media_driver_entry) (media_ptr);

        /* Determine if the FAT32 sector was read correctly. */
        if (media_ptr -> fx_media_driver_status == FX_SUCCESS)
        {

            /* Yes, setup a pointer into the FAT32 additional information sector.  */
            buffer_ptr =  media_ptr -> fx_media_driver_buffer;

            /* Pickup the first signature long word.  */
            signature =  _fx_utility_32_unsigned_read(&buffer_ptr[0]);

            /* Determine if the signature is correct.  */
            if (signature == 0x41615252)
            {

                /* Yes, the first signature is correct, now pickup the next signature.  */
                signature =  _fx_utility_32_unsigned_read(&buffer_ptr[484]);

                /* Determine if this signature is correct.  */
                if (signature == 0x61417272)
                {

                    /* Yes, we have a good FAT32 additional information sector.  */

                    /* Pickup the current available cluster count on the media.  */
                    media_ptr -> fx_media_available_clusters =  _fx_utility_32_unsigned_read(&buffer_ptr[488]);

                    /* Initialize the last reported available cluster count to the same value.  */
                    media_ptr -> fx_media_FAT32_additional_info_last_available =  media_ptr -> fx_media_available_clusters;

                    /* Pickup the hint for the starting free cluster search.  */
                    media_ptr -> fx_media_cluster_search_start =  _fx_utility_32_unsigned_read(&buffer_ptr[492]);

                    /* Perform a quick sanity check on the available cluster count and the starting free
                       cluster search.  */
                    if ((media_ptr -> fx_media_available_clusters > media_ptr -> fx_media_total_clusters) ||
                        (media_ptr -> fx_media_cluster_search_start > media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START) ||
                        (media_ptr -> fx_media_cluster_search_start < FX_FAT_ENTRY_START))
                    {

                        /* Something is wrong, clear the available cluster count and search so the regular processing
                           is used.  */
                        media_ptr -> fx_media_available_clusters =    0;
                        media_ptr -> fx_media_cluster_search_start =  0;

                        /* We don't invalidate the additional info sector here because only the data is bad.  */
                    }
                }
                else
                {

                    /* Signature is bad, invalidate the additional info sector.  */
                    media_ptr -> fx_media_FAT32_additional_info_sector =  0;
                }
            }
            else
            {

                /* Signature is bad, invalidate the additional info sector.  */
                media_ptr -> fx_media_FAT32_additional_info_sector =  0;
            }
        }
        else
        {

            /* IO error trying to read additional information sector, invalidate the additional info sector.  */
            media_ptr -> fx_media_FAT32_additional_info_sector =  0;
        }
    }

    /* Search the media to find the first available cluster as well as the total
       available clusters.  */

    /* Determine what type of FAT is present.  */
    if (media_ptr -> fx_media_12_bit_FAT)
    {

        /* A 12-bit FAT is present.  Utilize the FAT entry read utility to pickup
           each FAT entry's contents.  */

        /* Loop to read each cluster entry in the first FAT.  */
        for (cluster_number =  FX_FAT_ENTRY_START;
             cluster_number < (media_ptr -> fx_media_total_clusters) + FX_FAT_ENTRY_START;
             cluster_number++)
        {

            /* Read a FAT entry.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster_number, &FAT_entry);

            /* Determine if the read was successful.  */
            if (status != FX_SUCCESS)
            {

                /* Build the "uninitialize" I/O driver request.  */
                media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
                media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Call the specified I/O driver with the uninitialize request.  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

                return(FX_FAT_READ_ERROR);
            }

            /* Now determine if the FAT entry is available.  */
            if (FAT_entry == FX_FREE_CLUSTER)
            {

                /* Increment the number of available clusters.  */
                media_ptr -> fx_media_available_clusters++;

                /* Determine if the starting free cluster has been found yet.  */
                if (media_ptr -> fx_media_cluster_search_start == 0)
                {

                    /* Remember the first free cluster to start further searches from.  */
                    media_ptr -> fx_media_cluster_search_start =  cluster_number;
                }
            }
        }
    }
#ifdef FX_ENABLE_EXFAT
    else if ((media_ptr -> fx_media_available_clusters == 0)
             && (media_ptr -> fx_media_FAT_type != FX_exFAT))
#else
    else if (media_ptr -> fx_media_available_clusters == 0)
#endif /* FX_ENABLE_EXFAT */
    {

        /* A 16 or 32-bit FAT is present. Read directly into the logical sector
           cache memory to optimize I/O on larger devices. Since we are looking for
           values of zero, endian issues are not important.  */

        /* Invalidate the current logical sector cache.  */
        _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64) (media_ptr -> fx_media_total_sectors), FX_TRUE);

        /* Reset the memory pointer.  */
        media_ptr -> fx_media_memory_buffer =  original_memory_ptr;

        /* Loop through all FAT sectors in the primary FAT.  The first two entries are
           examined in this loop, but they are always unavailable.  */
        cluster_number =  0;
#ifndef FX_DISABLE_CACHE
        for (i = 0; i < media_ptr -> fx_media_sectors_per_FAT; i = i + media_ptr -> fx_media_sector_cache_size)
        {

            /* Calculate the starting next FAT sector.  */
            FAT_sector =  media_ptr -> fx_media_reserved_sectors + i;

            /* Calculate how many sectors to read.  */
            FAT_read_sectors =  media_ptr -> fx_media_sectors_per_FAT - i;

            /* Determine if there is not enough memory to read the remaining FAT sectors.  */
            if (FAT_read_sectors > media_ptr -> fx_media_sector_cache_size)
            {
                FAT_read_sectors =  media_ptr -> fx_media_sector_cache_size;
            }
#else
        for (i = 0; i < media_ptr -> fx_media_sectors_per_FAT; i++)
        {

            /* Calculate the starting next FAT sector.  */
            FAT_sector =  media_ptr -> fx_media_reserved_sectors + i;

            /* Calculate how many sectors to read.  */
            FAT_read_sectors =  1;
#endif /* FX_DISABLE_CACHE */

            /* Read the FAT sectors directly from the driver.  */
            media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
            media_ptr -> fx_media_driver_status =           FX_IO_ERROR;
            media_ptr -> fx_media_driver_buffer =           media_ptr -> fx_media_memory_buffer;
            media_ptr -> fx_media_driver_logical_sector =   FAT_sector;
            media_ptr -> fx_media_driver_sectors =          FAT_read_sectors;
            media_ptr -> fx_media_driver_sector_type =      FX_FAT_SECTOR;

            /* If trace is enabled, insert this event into the trace buffer.  */
            FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_READ, media_ptr, FAT_sector, FAT_read_sectors, media_ptr -> fx_media_memory_buffer, FX_TRACE_INTERNAL_EVENTS, 0, 0)

            /* Invoke the driver to read the FAT sectors.  */
            (media_ptr -> fx_media_driver_entry) (media_ptr);

            /* Determine if the read was successful.  */
            if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
            {

                /* Build the "uninitialize" I/O driver request.  */
                media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
                media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

                /* If trace is enabled, insert this event into the trace buffer.  */
                FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

                /* Call the specified I/O driver with the uninitialize request.  */
                (media_ptr -> fx_media_driver_entry) (media_ptr);

                return(FX_FAT_READ_ERROR);
            }

            /* Calculate the number of bytes in the buffer.  */
            bytes_in_buffer =  (media_ptr -> fx_media_bytes_per_sector * FAT_read_sectors);

            /* Walk through the sector cache memory to search for available clusters and the first
               available if not already found.  */
            for (j = 0; j < bytes_in_buffer;)
            {

                /* Check for a 32-bit FAT.  */
                if (media_ptr -> fx_media_32_bit_FAT)
                {

                    /* Pickup 32-bit FAT entry.  */
                    FAT_entry =  *((ULONG *)&(media_ptr -> fx_media_memory_buffer[j]));

                    /* Advance to next FAT entry.  */
                    j = j + 4;
                }
                else
                {

                    /* Process a 16-bit FAT entry.  */
                    FAT_entry =  (((ULONG)(media_ptr -> fx_media_memory_buffer[j])) & 0xFF) |
                        ((((ULONG)(media_ptr -> fx_media_memory_buffer[j + 1])) & 0xFF) << 8);

                    /* Advance to next FAT entry.  */
                    j =  j + 2;
                }

                /* Determine if the FAT entry is free.  */
                if (FAT_entry == FX_FREE_CLUSTER)
                {

                    /* Entry is free, increment available clusters.  */
                    media_ptr -> fx_media_available_clusters++;

                    /* Determine if the starting free cluster has been found yet.  */
                    if (media_ptr -> fx_media_cluster_search_start == 0)
                    {

                        /* Remember the first free cluster to start further searches from.  */
                        media_ptr -> fx_media_cluster_search_start =  cluster_number;
                    }
                }

                /* Increment the cluster number.  */
                cluster_number++;

                /* Determine if we have reviewed all FAT entries.  */
                if (cluster_number >= (media_ptr -> fx_media_total_clusters + FX_FAT_ENTRY_START))
                {

                    /* Yes, we have looked at all the FAT entries.  */

                    /* Ensure that the outer loop terminates as well.  */
                    i = media_ptr -> fx_media_sectors_per_FAT;
                    break;
                }
            }
        }
    }
#ifdef FX_ENABLE_EXFAT
    else if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        status = _fx_utility_exFAT_bitmap_initialize(media_ptr);

        if ((FX_SUCCESS         != status)  &&
            (FX_NO_MORE_SPACE   != status))
        {
            return(status);
        }
    }
#endif /* FX_ENABLE_EXFAT */

    /* If there were no free clusters, just set the search pointer to the
       first cluster number.  */
    if (media_ptr -> fx_media_cluster_search_start == 0)
    {
        media_ptr -> fx_media_cluster_search_start =  FX_FAT_ENTRY_START;
    }

    /* Setup the current working directory fields to default to the root
       directory.  */
    media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name =
        media_ptr -> fx_media_default_path.fx_path_name_buffer;
    media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_short_name[0] =  0;
    media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name[0] =        0;
    media_ptr -> fx_media_default_path.fx_path_string[0] =                      (CHAR)0;
    media_ptr -> fx_media_default_path.fx_path_string[FX_MAXIMUM_PATH - 1] =      (CHAR)0;
    media_ptr -> fx_media_default_path.fx_path_current_entry =                         0;

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Invalidate the previously found directory entry.  */
    media_ptr -> fx_media_last_found_name[0] =  0;
#endif

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    /* Initialize the opened file linked list and associated counter.  */
    media_ptr -> fx_media_opened_file_list =      FX_NULL;
    media_ptr -> fx_media_opened_file_count =     0;
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

    /* Create the media protection structure if FX_SINGLE_THREAD is not
       defined.  */
#ifndef FX_SINGLE_THREAD

#ifndef FX_DONT_CREATE_MUTEX

    /* Create ThreadX mutex for protection.  */
    tx_mutex_create(&(media_ptr -> fx_media_protect), "FileX Media Mutex", TX_NO_INHERIT);
#endif

#endif

#ifdef FX_DONT_CREATE_MUTEX

    /* Load the media ID field in the media control block.  This allows the FX_PROTECT
       call to succeed.  */
    media_ptr -> fx_media_id =  (ULONG)FX_MEDIA_ID;

    /* Protect against other threads accessing the media.  */
    FX_PROTECT
#endif

    /* Lockout interrupts.  */
    FX_DISABLE_INTS

    /* At this point, the media has been opened successfully.  Place the
       media on the linked list of currently opened media.  */

    /* Load the media ID field in the media control block.  */
    media_ptr -> fx_media_id =  (ULONG)FX_MEDIA_ID;

    /* Place the thread on the list of opened media.  First,
       check for an empty list.  */
    if (_fx_system_media_opened_ptr)
    {

        /* Pickup tail pointer.  */
        tail_ptr =  _fx_system_media_opened_ptr -> fx_media_opened_previous;

        /* Place the new media in the list.  */
        _fx_system_media_opened_ptr -> fx_media_opened_previous =  media_ptr;
        tail_ptr -> fx_media_opened_next =  media_ptr;

        /* Setup this media's opened links.  */
        media_ptr -> fx_media_opened_previous =  tail_ptr;
        media_ptr -> fx_media_opened_next =      _fx_system_media_opened_ptr;
    }
    else
    {

        /* The opened media list is empty.  Add the media to empty list.  */
        _fx_system_media_opened_ptr =           media_ptr;
        media_ptr -> fx_media_opened_next =     media_ptr;
        media_ptr -> fx_media_opened_previous = media_ptr;
    }

    /* Increment the opened media counter.  */
    _fx_system_media_opened_count++;

    /* Invoke media open callback. */
    if (media_ptr -> fx_media_open_notify)
    {
        media_ptr -> fx_media_open_notify(media_ptr);
    }

    /* Restore interrupts.  */
    FX_RESTORE_INTS

#ifdef FX_DONT_CREATE_MUTEX

    /* Release media protection.  */
    FX_UNPROTECT
#endif

    /* Return a successful status.  */
    return(FX_SUCCESS);
}

