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
/*    _fx_utility_exFAT_geometry_check                    PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks exFAT geometry.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    sector_buffer                         Pointer to sector buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_exFAT_system_area_checksum_verify Verify SystemArea checksum    */
/*    _fx_utility_64_unsigned_read          Read ULONG64 from memory      */
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
/*  01-31-2022     Bhupendra Naphade        Modified comment(s), replaced */
/*                                            sector size constant,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_exFAT_geometry_check(FX_MEDIA *media_ptr, UCHAR *sector_buffer)
{

ULONG  temp;
ULONG  counter;
ULONG  calculated_checksum         = 0;
ULONG  main_system_area_checksum   = 0;
ULONG  backup_system_area_checksum      = 0;
ULONG  bitmap_start_sector       = 0;
ULONG  bitmap_size_in_bytes      = 0;
ULONG  upcase_table_start_sector  = 0;
ULONG  upcase_table_size_in_bytes = 0;
ULONG  upcase_table_checksum    = 0;
UCHAR *dir_entry_ptr           = NULL;
ULONG  dir_entries_found        = 0;
UINT   status;


    /* Check FAT Size.  */
    temp = (media_ptr -> fx_media_sectors_per_FAT <<
            media_ptr -> fx_media_exfat_bytes_per_sector_shift) >>
        FX_EXFAT_SIZE_OF_FAT_ELEMENT_SHIFT;

    /* Is the FAT capable of holding the total number of clusters?  */
    if (temp < media_ptr -> fx_media_total_clusters)
    {

        /* No, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Check Cluster Heap Offset.  */
    temp = media_ptr -> fx_media_reserved_sectors +
        media_ptr -> fx_media_sectors_per_FAT;

    /* Is the cluster heap placed after the FAT table?  */
    if (temp > media_ptr -> fx_media_data_sector_start)
    {

        /* No, the media is invalid.  */
        return(FX_MEDIA_INVALID);
    }

    /* Check System Area checksum.  */
    status = _fx_utility_exFAT_system_area_checksum_verify(media_ptr,
                                                           sector_buffer,
                                                           FX_EXFAT_FAT_MAIN_BOOT_SECTOR_OFFSET,
                                                           &main_system_area_checksum);

    /* Is the checksum correct?  */
    if (FX_SUCCESS != status)
    {

        /* No, return error.  */
        return(status);
    }

    /* Check System Area checksum in backup region.  */
    status = _fx_utility_exFAT_system_area_checksum_verify(media_ptr,
                                                           sector_buffer,
                                                           FX_EXFAT_FAT_BACKUP_BOOT_SECTOR_OFFSET,
                                                           &backup_system_area_checksum);

    /* Is the checksum correct?  */
    if (FX_SUCCESS != status)
    {

        /* No, return error.  */
        return(status);
    }

    /* Is the main system area checksum equals to the backup system area checksum?  */
    if (main_system_area_checksum != backup_system_area_checksum)
    {

        /* No, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Get Bit Map and Up Case table parameters.  */
    media_ptr -> fx_media_driver_request = FX_DRIVER_READ;
    media_ptr -> fx_media_driver_buffer  = sector_buffer;
    media_ptr -> fx_media_driver_sectors = 1;

    counter = 0;

    /* Go through the first cluster of exFAT root directory.
       This cluster should contain BitMap allocation Dir Entry
       and Up Case table Dir Entry.  */
    while ((counter < media_ptr -> fx_media_sectors_per_cluster) &&
           (EXFAT_NUM_OF_DIR_ENTRIES != dir_entries_found))
    {

        /* Build the read sector command.  */
        media_ptr -> fx_media_driver_logical_sector = media_ptr -> fx_media_root_sector_start + counter;
        media_ptr -> fx_media_driver_status = FX_IO_ERROR;

        /* Invoke the driver to read the directory entry sector.  */
        (media_ptr -> fx_media_driver_entry)(media_ptr);

        /* Determine if the read was successful. */
        if (FX_SUCCESS != media_ptr -> fx_media_driver_status)
        {

            /* Return the error status.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* Move to next sector.  */
        counter++;

        /* Setup directory entry pointer to the sector buffer.  */
        dir_entry_ptr = sector_buffer;

        /* Go through the read buffer and try to find BitMap table and UpCase table dir entries.  */
        while (dir_entry_ptr < (media_ptr -> fx_media_memory_buffer +
                                media_ptr -> fx_media_bytes_per_sector) &&
               (EXFAT_NUM_OF_DIR_ENTRIES != dir_entries_found))
        {

            /* Check if we found the bitmap table.  */
            if (FX_EXFAT_DIR_ENTRY_TYPE_ALLOCATION_BITMAP == dir_entry_ptr[FX_EXFAT_ENTRY_TYPE])
            {

                /* Calculate BitMap table start sector.  */
                bitmap_start_sector  = media_ptr -> fx_media_data_sector_start +
                    ((_fx_utility_32_unsigned_read(&dir_entry_ptr[FX_EXFAT_FIRST_CLUSTER]) -
                      FX_FAT_ENTRY_START) << media_ptr -> fx_media_exfat_sector_per_clusters_shift);

                bitmap_size_in_bytes = (ULONG)_fx_utility_64_unsigned_read(&dir_entry_ptr[FX_EXFAT_DATA_LENGTH]);

                dir_entries_found++;
            }
            /* Check if we found the upcase table.  */
            else if (FX_EXFAT_DIR_ENTRY_TYPE_UP_CASE_TABLE == dir_entry_ptr[FX_EXFAT_ENTRY_TYPE])
            {

                /* Calculate UpCase table start sector.  */
                upcase_table_start_sector  = media_ptr -> fx_media_data_sector_start +
                    ((_fx_utility_32_unsigned_read(&dir_entry_ptr[FX_EXFAT_FIRST_CLUSTER]) -
                      FX_FAT_ENTRY_START) << media_ptr -> fx_media_exfat_sector_per_clusters_shift);

                upcase_table_checksum    = _fx_utility_32_unsigned_read(&dir_entry_ptr[FX_EXFAT_UP_CASE_TABLE_CHECK_SUM]);
                upcase_table_size_in_bytes = (ULONG)_fx_utility_64_unsigned_read(&dir_entry_ptr[FX_EXFAT_DATA_LENGTH]);

                dir_entries_found++;
            }

            /* Move to next directory entry.  */
            dir_entry_ptr += FX_EXFAT_DIR_ENTRY_SIZE;
        }
    }

    /* Validate the directory entries.  */
    if ((0 == bitmap_start_sector)  ||
        (0 == bitmap_size_in_bytes) ||
        (0 == upcase_table_start_sector) ||
        (0 == upcase_table_checksum) ||
        (0 == upcase_table_size_in_bytes))
    {

        /* Directory entries have invalid value, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Check Bit Map size.  */
    if (media_ptr -> fx_media_total_clusters >
        /* 1 Byte = > map to 8 clusters.  */
        (bitmap_size_in_bytes << BITS_PER_BYTE_SHIFT))
    {

        /* Bitmap size is not correct, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Check Up Case Table checksum.  */
    calculated_checksum = 0;

    /* Build driver request to read the upcase table.  */
    media_ptr -> fx_media_driver_logical_sector = upcase_table_start_sector;

    /* Loop to calculate upcase table checksum.  */
    while (upcase_table_size_in_bytes > 0)
    {

        /* Build driver request.  */
        media_ptr -> fx_media_driver_status = FX_IO_ERROR;

        /* Invoke the driver to read one sector.  */
        (media_ptr -> fx_media_driver_entry)(media_ptr);

        /* Determine if the read was successful. */
        if (FX_SUCCESS != media_ptr -> fx_media_driver_status)
        {

            /* Return the error status.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* Determine the processing size.  */
        if (upcase_table_size_in_bytes >= media_ptr -> fx_media_bytes_per_sector)
        {

            /* Process one sector at a time.  */
            temp = media_ptr -> fx_media_bytes_per_sector;
            upcase_table_size_in_bytes -= media_ptr -> fx_media_bytes_per_sector;
            media_ptr -> fx_media_driver_logical_sector++;
        }
        else
        {

            /* Remaining data is less than one sector, process all the remaining bytes.  */
            temp = upcase_table_size_in_bytes;
            upcase_table_size_in_bytes = 0;
        }

        /* Calculate checksum.  */
        for (counter = 0; counter < temp; counter++)
        {

            /* Calculate the checksum using the algorithm specified in the specification.  */
            /* Right rotate the checksum by one bit position and add the data.  */
            calculated_checksum = ((calculated_checksum >> 1) | (calculated_checksum << 31)) + (ULONG)sector_buffer[counter];
        }
    }

    /* Verify the checksum.  */
    if (calculated_checksum != upcase_table_checksum)
    {

        /* Checksum not correct, return error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Return success.  */
    return(FX_SUCCESS);
}

#endif /* FX_ENABLE_EXFAT */

