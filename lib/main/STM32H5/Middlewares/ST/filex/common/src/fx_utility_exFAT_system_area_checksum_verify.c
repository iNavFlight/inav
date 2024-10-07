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
/*    _fx_utility_exFAT_system_area_checksum_verify       PORTABLE C      */
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
/*    boot_sector_offset                    Offset of boot sector         */
/*    calculated_checksum                   Pointer to checksum value     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Media driver                                                        */
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
UINT  _fx_utility_exFAT_system_area_checksum_verify(FX_MEDIA *media_ptr, UCHAR *sector_buffer,
                                                    ULONG boot_sector_offset, ULONG *calculated_checksum)
{

ULONG temp;
ULONG counter;


    /* Clear the checksum for checksum calculation.  */
    *calculated_checksum =  0;

    /* Prepare driver request.  */
    media_ptr -> fx_media_driver_request =          FX_DRIVER_READ;
    media_ptr -> fx_media_driver_buffer  =          sector_buffer;
    media_ptr -> fx_media_driver_sectors =          1;
    media_ptr -> fx_media_driver_status         =   FX_IO_ERROR;
    media_ptr -> fx_media_driver_logical_sector =   boot_sector_offset;

    /* Call driver.  */
    (media_ptr -> fx_media_driver_entry)(media_ptr);

    /* Check driver status.  */
    if (FX_SUCCESS != media_ptr -> fx_media_driver_status)
    {

        /* Error, return error status.  */
        return(media_ptr -> fx_media_driver_status);
    }

    /* Loop to calculate Boot Sector checksum.  */
    for (temp = 0; temp < media_ptr -> fx_media_bytes_per_sector; temp++)
    {

        /* Check if it is VolumeFlags or PercentInUse.  */
        if ((FX_EF_VOLUME_FLAGS     == temp) ||
            (FX_EF_VOLUME_FLAGS + 1 == temp) ||
            (FX_EF_PERCENT_IN_USE   == temp))
        {

            /* Skip these fields in checksum calculation.  */
            continue;
        }

        /* Calculate the checksum using the algorithm specified in the specification.  */
        /* Right rotate the checksum by one bit position and add the data.  */
        *calculated_checksum = ((*calculated_checksum >> 1) | (*calculated_checksum << 31)) + (ULONG)sector_buffer[temp];
    }

    /* Map to Extended boot Sector.  */
    media_ptr -> fx_media_driver_logical_sector++;

    /* Read System Area from Extended Boot Sector
       and calculate checksum.  */
    for (temp = FX_EXFAT_FAT_EXT_BOOT_SECTOR_OFFSET; FX_EXFAT_FAT_CHECK_SUM_OFFSET > temp; temp++)
    {

        /* Build driver request.  */
        media_ptr -> fx_media_driver_status = FX_IO_ERROR;

        /* Read next sector.  */
        (media_ptr -> fx_media_driver_entry)(media_ptr);

        /* Check status.  */
        if (FX_SUCCESS != media_ptr -> fx_media_driver_status)
        {

            /* Clear checksum.  */
            *calculated_checksum = 0;

            /* Error, return error status.  */
            return(media_ptr -> fx_media_driver_status);
        }

        /* Move to next logical sector.  */
        media_ptr -> fx_media_driver_logical_sector++;

        /* Check Sector Signature.  */
        if (((sector_buffer[media_ptr -> fx_media_bytes_per_sector - 2] != FX_SIG_BYTE_1) ||
             (sector_buffer[media_ptr -> fx_media_bytes_per_sector - 1] != FX_SIG_BYTE_2)) &&
            (FX_EXFAT_FAT_OEM_PARAM_OFFSET > temp))
        {

            /* Clear checksum.  */
            *calculated_checksum =  0;

            /* Error, return error status.  */
            return(FX_MEDIA_INVALID);
        }

        /* Loop to calculate the checksum.  */
        for (counter = 0; counter < media_ptr -> fx_media_bytes_per_sector; counter++)
        {

            /* Calculate the checksum using the algorithm specified in the specification.  */
            /* Right rotate the checksum by one bit position and add the data.  */
            *calculated_checksum = ((*calculated_checksum >> 1) | (*calculated_checksum << 31)) + (ULONG)sector_buffer[counter];
        }
    }

    /* Build driver request.  */
    media_ptr -> fx_media_driver_status = FX_IO_ERROR;

    /* Read stored checksum.  */
    (media_ptr -> fx_media_driver_entry)(media_ptr);

    /* Determine if the read was successful.  */
    if (FX_SUCCESS != media_ptr -> fx_media_driver_status)
    {

        /* Not successful, return error status.  */
        return(media_ptr -> fx_media_driver_status);
    }

    /* Loop to check sector content.  */
    for (counter = 0; counter < media_ptr -> fx_media_bytes_per_sector; counter += sizeof(ULONG))
    {

        /* Read a 32 bit value from sector buffer.  */
        temp = _fx_utility_32_unsigned_read(&sector_buffer[counter]);

        /* Compare the read value with the caclulated checksum.  */
        if (temp != *calculated_checksum)
        {

            /* Not equal, checksum verify failed.  */
            return(FX_MEDIA_INVALID);
        }
    }

    /* Return success.  */
    return(FX_SUCCESS);
}

#endif /* FX_ENABLE_EXFAT */

