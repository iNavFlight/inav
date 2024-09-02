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
/*    _fx_media_boot_info_extract                         PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts and validates the information from the boot  */
/*    record found in the memory buffer.  If the boot record is invalid,  */
/*    an FX_MEDIA_INVALID status is returned to the caller.               */
/*                                                                        */
/*    The FAT boot sector (512 bytes) that is operated on by this         */
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
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_read          Read a UINT from buffer       */
/*    _fx_utility_32_unsigned_read          Read a ULONG from buffer      */
/*    _fx_utility_64_unsigned_read          Read a ULONG64 from memory    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_media_open                        Media open function           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Bhupendra Naphade        Modified comment(s), added    */
/*                                            check for bimap cache size, */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_boot_info_extract(FX_MEDIA *media_ptr)
{

UCHAR *boot_sector;


    /* Move the buffer pointer into a local copy.  */
    boot_sector =  media_ptr -> fx_media_driver_buffer;

    /* Extract the number of bytes per sector.  */
    media_ptr -> fx_media_bytes_per_sector =    _fx_utility_16_unsigned_read(&boot_sector[FX_BYTES_SECTOR]);
    if (media_ptr -> fx_media_bytes_per_sector == 0)
#ifdef FX_ENABLE_EXFAT
    {
        /* Treat as exFAT volume.  */
        /* Extract the number of bytes per sector.  */
        media_ptr -> fx_media_exfat_bytes_per_sector_shift = boot_sector[FX_EF_BYTE_PER_SECTOR_SHIFT];

        /* exFAT requires minimal value 9 (512 bytes) and maximum value 12(4096 bytes) for bytes_per_sector_shift */
        if((media_ptr -> fx_media_exfat_bytes_per_sector_shift < 9) || (media_ptr -> fx_media_exfat_bytes_per_sector_shift > 12))
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_bytes_per_sector = (UINT)(1 << media_ptr -> fx_media_exfat_bytes_per_sector_shift);

        /* Validate bytes per sector value: no more than bitmap cache size */
        if (media_ptr -> fx_media_bytes_per_sector > sizeof(media_ptr -> fx_media_exfat_bitmap_cache))
        {
            return(FX_NOT_ENOUGH_MEMORY);
        }

        media_ptr -> fx_media_total_sectors = _fx_utility_64_unsigned_read(&boot_sector[FX_EF_VOLUME_LENGTH]);
        if (media_ptr -> fx_media_total_sectors == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_reserved_sectors = _fx_utility_32_unsigned_read(&boot_sector[FX_EF_FAT_OFFSET]);
        if (media_ptr -> fx_media_reserved_sectors == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_sectors_per_FAT = _fx_utility_32_unsigned_read(&boot_sector[FX_EF_FAT_LENGTH]);
        if (media_ptr -> fx_media_sectors_per_FAT == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_data_sector_start = _fx_utility_32_unsigned_read(&boot_sector[FX_EF_CLUSTER_HEAP_OFFSET]);
        if (media_ptr -> fx_media_data_sector_start == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_total_clusters = _fx_utility_32_unsigned_read(&boot_sector[FX_EF_CLUSTER_COUNT]);
        if (media_ptr -> fx_media_total_clusters == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_exfat_sector_per_clusters_shift = boot_sector[FX_EF_SECTOR_PER_CLUSTER_SHIFT];
        if (media_ptr -> fx_media_exfat_sector_per_clusters_shift > 25 - media_ptr -> fx_media_exfat_bytes_per_sector_shift)
        {
            return(FX_MEDIA_INVALID);
        }
        media_ptr -> fx_media_sectors_per_cluster = (UINT)(1 << media_ptr -> fx_media_exfat_sector_per_clusters_shift);

        media_ptr -> fx_media_number_of_FATs = boot_sector[FX_EF_NUMBER_OF_FATS];
        if (media_ptr -> fx_media_number_of_FATs == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        media_ptr -> fx_media_root_cluster_32    = _fx_utility_32_unsigned_read(&boot_sector[FX_EF_FIRST_CLUSTER_OF_ROOT_DIR]);
        /* Root cluster starts from at least FX_FAT_ENTRY_START (2), or higher. */
        if (media_ptr -> fx_media_root_cluster_32 < FX_FAT_ENTRY_START)
        {
            return(FX_MEDIA_INVALID);
        }

        /* Overflow check. */
        if (((ULONG64)media_ptr -> fx_media_data_sector_start +
             (ULONG64)(media_ptr -> fx_media_root_cluster_32 - FX_FAT_ENTRY_START) *
             media_ptr -> fx_media_sectors_per_cluster) > 0xFFFFFFFF)
        {

            /* Return the invalid media error status.  */
            return(FX_MEDIA_INVALID);
        }

        /* Calculate logical number of root dir sector.  */
        media_ptr -> fx_media_root_sector_start = media_ptr -> fx_media_data_sector_start +
            (media_ptr -> fx_media_root_cluster_32 - FX_FAT_ENTRY_START) *
            media_ptr -> fx_media_sectors_per_cluster;

        media_ptr -> fx_media_exfat_volume_serial_number  = _fx_utility_32_unsigned_read(&boot_sector[FX_EF_VOLUME_SERIAL_NUMBER]);

        media_ptr -> fx_media_exfat_file_system_revision  = _fx_utility_16_unsigned_read(&boot_sector[FX_EF_FILE_SYSTEM_REVISION]);

        media_ptr -> fx_media_exfat_volume_flag          = _fx_utility_16_unsigned_read(&boot_sector[FX_EF_VOLUME_FLAGS]);

        media_ptr -> fx_media_number_of_FATs = boot_sector[FX_EF_NUMBER_OF_FATS];

        if (0 == media_ptr -> fx_media_number_of_FATs)
        {
            return(FX_MEDIA_INVALID);
        }

        /* Extract the number of hidden sectors.  */
#ifdef FX_DRIVER_USE_64BIT_LBA
        media_ptr -> fx_media_hidden_sectors =      _fx_utility_64_unsigned_read(&boot_sector[FX_EF_PARTITION_OFFSET]);
#else
        media_ptr -> fx_media_hidden_sectors =      _fx_utility_32_unsigned_read(&boot_sector[FX_EF_PARTITION_OFFSET]);
#endif

        media_ptr -> fx_media_exfat_drive_select       = boot_sector[FX_EF_DRIVE_SELECT];
        media_ptr -> fx_media_exfat_percent_in_use       = boot_sector[FX_EF_PERCENT_IN_USE];

        media_ptr -> fx_media_12_bit_FAT = FX_FALSE;
        media_ptr -> fx_media_32_bit_FAT = FX_FALSE;

        /* Legacy code support:
           We will use fx_media_FAT_type for determine FAT type instead of
           fx_media_12_bit_FAT and fx_media_32_bit_FAT.  */
        media_ptr -> fx_media_12_bit_FAT = FX_FALSE;
        media_ptr -> fx_media_32_bit_FAT = FX_FALSE;

        media_ptr -> fx_media_FAT_type = FX_exFAT;

        return(_fx_utility_exFAT_geometry_check(media_ptr, boot_sector));
    }
    else
    {
#else
        return(FX_MEDIA_INVALID);
#endif /* FX_ENABLE_EXFAT */


        /* FAT12/16/32 volume.  */
        /* Extract the number of sectors per track.  */
        media_ptr -> fx_media_sectors_per_track =   _fx_utility_16_unsigned_read(&boot_sector[FX_SECTORS_PER_TRK]);

        /* Extract the number of heads.  */
        media_ptr -> fx_media_heads =               _fx_utility_16_unsigned_read(&boot_sector[FX_HEADS]);

        /* Extract the total number of sectors.  */
        media_ptr -> fx_media_total_sectors =       _fx_utility_16_unsigned_read(&boot_sector[FX_SECTORS]);
        if (media_ptr -> fx_media_total_sectors == 0)
        {
            media_ptr -> fx_media_total_sectors = _fx_utility_32_unsigned_read(&boot_sector[FX_HUGE_SECTORS]);
        }

        if (media_ptr -> fx_media_total_sectors == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        /* Extract the number of reserved sectors before the first FAT.  */
        media_ptr -> fx_media_reserved_sectors =    _fx_utility_16_unsigned_read(&boot_sector[FX_RESERVED_SECTORS]);
        if (media_ptr -> fx_media_reserved_sectors == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        /* Extract the number of sectors per cluster.  */
        media_ptr -> fx_media_sectors_per_cluster = ((UINT)boot_sector[FX_SECTORS_CLUSTER] & 0xFF);

        /* There should always be at least one reserved sector, representing the boot record itself.  */
        if (media_ptr -> fx_media_sectors_per_cluster == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        /* Extract the number of sectors per FAT.  */
        media_ptr -> fx_media_sectors_per_FAT =     _fx_utility_16_unsigned_read(&boot_sector[FX_SECTORS_PER_FAT]);
        if (media_ptr -> fx_media_sectors_per_FAT == 0)
        {
            media_ptr -> fx_media_sectors_per_FAT = _fx_utility_32_unsigned_read(&boot_sector[FX_SECTORS_PER_FAT_32]);
        }

        if (media_ptr -> fx_media_sectors_per_FAT == 0)
        {
            return(FX_MEDIA_INVALID);
        }

        /* Extract the number of FATs.  */
        media_ptr -> fx_media_number_of_FATs =      ((UINT)boot_sector[FX_NUMBER_OF_FATS] & 0xFF);
        if (media_ptr -> fx_media_number_of_FATs == 0)
        {
            return(FX_BOOT_ERROR);
        }

        /* Extract the number of hidden sectors.  */
#ifdef FX_DRIVER_USE_64BIT_LBA
        media_ptr -> fx_media_hidden_sectors =      _fx_utility_64_unsigned_read(&boot_sector[FX_HIDDEN_SECTORS]);
#else
        media_ptr -> fx_media_hidden_sectors =      _fx_utility_32_unsigned_read(&boot_sector[FX_HIDDEN_SECTORS]);
#endif
        /* Extract the number of root directory entries.  */
        media_ptr -> fx_media_root_directory_entries =  _fx_utility_16_unsigned_read(&boot_sector[FX_ROOT_DIR_ENTRIES]);

        /* Extract root directory starting cluster (32 bit only) and compute start sector */
        media_ptr -> fx_media_root_cluster_32 = _fx_utility_32_unsigned_read(&boot_sector[FX_ROOT_CLUSTER_32]);

#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Return a successful status.  */
    return(FX_SUCCESS);
}

