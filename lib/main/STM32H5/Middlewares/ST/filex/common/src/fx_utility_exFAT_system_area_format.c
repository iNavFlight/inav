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
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_system_area_format                PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function formats system area for exFAT.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media control block*/
/*    boundary_unit                         Data area alignment in sectors*/
/*    system_area_checksum_ptr              Pointer system area checksum  */
/*    memory_ptr                            Pointer to work buffer        */
/*    memory_size                           Size of the work buffer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_memory_set                Set memory                    */
/*    _fx_utility_exFAT_system_sector_write Write exFAT format sector     */
/*    _fx_utility_32_unsigned_write         Write 32-bit word             */
/*    _fx_utility_16_unsigned_write         Write 16-bit word             */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_media_exFAT_format                                              */
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
UINT  _fx_utility_exFAT_system_area_format(FX_MEDIA *media_ptr, ULONG boundary_unit,
                                           ULONG *system_area_checksum_ptr,
                                           UCHAR *memory_ptr, UINT memory_size)
{

UINT   counter;
UINT   i;
UINT   sector_offset   = 0;
UINT   status;
UCHAR *system_area_buffer  = NULL;


    /* Is the memory size large enough?  */
    if (memory_size < media_ptr -> fx_media_bytes_per_sector)
    {

        /* No, return an error.  */
        return(FX_PTR_ERROR);
    }

    /* Setup pointer to system area buffer.  */
    system_area_buffer =  memory_ptr;

    /* Align the boundary unit.  */
    boundary_unit = ALIGN_UP(boundary_unit, 128);

    /* Clear work buffer.  */
    _fx_utility_memory_set(system_area_buffer, 0x00, media_ptr -> fx_media_bytes_per_sector);

    /* Add signature to System area sectors without boot, OEM, reserved and checksum sector.  */
    system_area_buffer[media_ptr -> fx_media_bytes_per_sector - 2] =  FX_SIG_BYTE_1;
    system_area_buffer[media_ptr -> fx_media_bytes_per_sector - 1] =  FX_SIG_BYTE_2;

    /* Loop to write all the extended boot sectors.  */
    for (counter = 0; counter < (FX_EXFAT_FAT_OEM_PARAM_OFFSET - FX_EXFAT_FAT_EXT_BOOT_SECTOR_OFFSET); counter++)
    {

        /* Loop to calculate the checksum for System Area  */
        for (i = 0; i < media_ptr -> fx_media_bytes_per_sector; i++)
        {

            /* Calculate the checksum using the algorithm specified in the specification.  */
            /* Right rotate the checksum by one bit position and add the data.  */
            *system_area_checksum_ptr = ((*system_area_checksum_ptr >> 1) | (*system_area_checksum_ptr << 31)) +
                (ULONG)system_area_buffer[i];
        }

        /* Write out the extended boot sector.  */
        status = _fx_utility_exFAT_system_sector_write(media_ptr, system_area_buffer,
                                                       (ULONG64) (FX_EXFAT_FAT_EXT_BOOT_SECTOR_OFFSET + counter),
                                                       ((ULONG) 1), FX_BOOT_SECTOR);


        /* Determine if the write was successful.  */
        if (status != FX_SUCCESS)
        {
            return(status);
        }

        /* Write out the extended boot sector in the backup region.  */
        status = _fx_utility_exFAT_system_sector_write(media_ptr, system_area_buffer,
                                                       (ULONG64) (FX_EXFAT_FAT_EXT_BOOT_SECTOR_OFFSET + counter + FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE),
                                                       ((ULONG) 1), FX_BOOT_SECTOR);

        /* Determine if the write was successful.  */
        if (status != FX_SUCCESS)
        {
            return(status);
        }
    }

    /* Add OEM - Flash parameters.  */
    system_area_buffer[media_ptr -> fx_media_bytes_per_sector - 2] = 0;
    system_area_buffer[media_ptr -> fx_media_bytes_per_sector - 1] = 0;

    /* GUID 0A0C7E46-3399-4021-90C8-FA6D389C4BA2 */
    _fx_utility_32_unsigned_write(&(system_area_buffer[sector_offset]), 0x0A0C7E46);
    _fx_utility_16_unsigned_write(&(system_area_buffer[sector_offset + 4]), 0x3399);
    _fx_utility_16_unsigned_write(&(system_area_buffer[sector_offset + 6]), 0x4021);
    system_area_buffer[sector_offset +  8] = 0x90; system_area_buffer[sector_offset +  9] = 0xC8;
    system_area_buffer[sector_offset + 10] = 0xFA; system_area_buffer[sector_offset + 11] = 0x6D;
    system_area_buffer[sector_offset + 12] = 0x38; system_area_buffer[sector_offset + 13] = 0x9C;
    system_area_buffer[sector_offset + 14] = 0x4B; system_area_buffer[sector_offset + 15] = 0xA2;

    /* Add EraseBlockSize.  */
    _fx_utility_32_unsigned_write(&(system_area_buffer[sector_offset + 16]), boundary_unit / 2);

    /* Write out the OEM parameters sector.  */
    status = _fx_utility_exFAT_system_sector_write(media_ptr, system_area_buffer,
                                                   FX_EXFAT_FAT_OEM_PARAM_OFFSET,
                                                   1, FX_BOOT_SECTOR);

    /* Determine if the write was successful.  */
    if (status != FX_SUCCESS)
    {
        return(status);
    }

    /* Write out the OEM parameters sector in the backup region.  */
    status = _fx_utility_exFAT_system_sector_write(media_ptr, system_area_buffer,
                                                   FX_EXFAT_FAT_OEM_PARAM_OFFSET + FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE,
                                                   1, FX_BOOT_SECTOR);

    /* Determine if the write was successful.  */
    if (status != FX_SUCCESS)
    {
        return(status);
    }

    /* Loop to calculate checksum for System Area.  */
    for (counter = 0; counter < media_ptr -> fx_media_bytes_per_sector; counter++)
    {

        /* Calculate the checksum using the algorithm specified in the specification.  */
        /* Right rotate the checksum by one bit position and add the data.  */
        *system_area_checksum_ptr = ((*system_area_checksum_ptr >> 1) | (*system_area_checksum_ptr << 31)) +
            (ULONG)system_area_buffer[counter];
    }

    /* Clear work buffer.  */
    _fx_utility_memory_set(system_area_buffer, 0x00, media_ptr -> fx_media_bytes_per_sector);

    /* Loop to calculate checksum for System Area.  */
    for (counter = 0; counter < media_ptr -> fx_media_bytes_per_sector; counter++)
    {

        /* Calculate the checksum using the algorithm specified in the specification.  */
        /* Right rotate the checksum by one bit position and add the data.  */
        *system_area_checksum_ptr = ((*system_area_checksum_ptr >> 1) | (*system_area_checksum_ptr << 31)) +
            (ULONG)system_area_buffer[counter];
    }

    /* Write out the boot checksum sector.  */
    status = _fx_utility_exFAT_system_sector_write(media_ptr, system_area_buffer,
                                                   FX_EXFAT_FAT_OEM_PARAM_OFFSET + 1,
                                                   1, FX_BOOT_SECTOR);

    /* Determine if the write was successful.  */
    if (status != FX_SUCCESS)
    {
        return(status);
    }

    /* Write out the boot checksum sector in the backup region.  */
    status = _fx_utility_exFAT_system_sector_write(media_ptr, system_area_buffer,
                                                   FX_EXFAT_FAT_OEM_PARAM_OFFSET + 1 + FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE,
                                                   1, FX_BOOT_SECTOR);

    /* Return completion status.  */
    return(status);
}

#endif

