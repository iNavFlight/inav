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
/*    _fx_utility_exFAT_system_area_checksum_write        PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes system area checksum.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    sector_buffer                         Pointer to sector buffer      */
/*    system_area_checksum_ptr              Pointer to checksum value     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_exFAT_system_sector_write Write a format sector         */
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
UINT _fx_utility_exFAT_system_area_checksum_write(FX_MEDIA *media_ptr, UCHAR *sector_buffer, ULONG *system_area_checksum_ptr)
{

UINT index;

    /* Fill buffer by check sum.  */
    for (index = 0; index < media_ptr -> fx_media_bytes_per_sector;)
    {
        _fx_utility_32_unsigned_write(&sector_buffer[index],
                                      *system_area_checksum_ptr);

        index += sizeof(*system_area_checksum_ptr);
    }

    /* Write Check Sum for Main System area.  */
    _fx_utility_exFAT_system_sector_write(media_ptr, sector_buffer,
                                          FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE - 1,
                                          1, FX_BOOT_SECTOR);

    if (FX_SUCCESS != media_ptr -> fx_media_driver_status)
    {
        return(media_ptr -> fx_media_driver_status);
    }

    /* Write Check Sum for BackUp System area.  */
    _fx_utility_exFAT_system_sector_write(media_ptr, sector_buffer,
                                          FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE - 1 + FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE,
                                          1, FX_BOOT_SECTOR);

    return(media_ptr -> fx_media_driver_status);
}

#endif /* FX_ENABLE_EXFAT */

