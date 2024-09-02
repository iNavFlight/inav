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
/**   RAM Disk Driver                                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#include "fx_api.h"


/* The RAM driver relies on the fx_media_format call to be made prior to
   the fx_media_open call. The following call will format the default
   32KB RAM drive, with a sector size of 128 bytes per sector.

        fx_media_format(&ram_disk,
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            media_memory,           // Media buffer pointer
                            sizeof(media_memory),   // Media buffer size
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            256,                    // Total sectors
                            128,                    // Sector size
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track

 */


VOID _fx_ram_driver(FX_MEDIA *media_ptr);


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_ram_driver                                      PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the entry point to the generic RAM disk driver     */
/*    that is delivered with all versions of FileX. The format of the     */
/*    RAM disk is easily modified by calling fx_media_format prior        */
/*    to opening the media.                                               */
/*                                                                        */
/*    This driver also serves as a template for developing FileX drivers  */
/*    for actual devices. Simply replace the read/write sector logic with */
/*    calls to read/write from the appropriate physical device            */
/*                                                                        */
/*    FileX RAM/FLASH structures look like the following:                 */
/*                                                                        */
/*          Physical Sector                 Contents                      */
/*                                                                        */
/*              0                       Boot record                       */
/*              1                       FAT Area Start                    */
/*              +FAT Sectors            Root Directory Start              */
/*              +Directory Sectors      Data Sector Start                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_memory_copy               Copy sector memory            */
/*    _fx_utility_16_unsigned_read          Read 16-bit unsigned          */
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
/*  03-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
VOID  _fx_ram_driver(FX_MEDIA *media_ptr)
{

UCHAR *source_buffer;
UCHAR *destination_buffer;
UINT   bytes_per_sector;


    /* There are several useful/important pieces of information contained in 
       the media structure, some of which are supplied by FileX and others 
       are for the driver to setup. The following is a summary of the 
       necessary FX_MEDIA structure members:

            FX_MEDIA Member                    Meaning

        fx_media_driver_request             FileX request type. Valid requests from 
                                            FileX are as follows:

                                                    FX_DRIVER_READ
                                                    FX_DRIVER_WRITE
                                                    FX_DRIVER_FLUSH
                                                    FX_DRIVER_ABORT
                                                    FX_DRIVER_INIT
                                                    FX_DRIVER_BOOT_READ
                                                    FX_DRIVER_RELEASE_SECTORS
                                                    FX_DRIVER_BOOT_WRITE
                                                    FX_DRIVER_UNINIT

        fx_media_driver_status              This value is RETURNED by the driver. 
                                            If the operation is successful, this 
                                            field should be set to FX_SUCCESS for 
                                            before returning. Otherwise, if an 
                                            error occurred, this field should be 
                                            set to FX_IO_ERROR.

        fx_media_driver_buffer              Pointer to buffer to read or write 
                                            sector data. This is supplied by 
                                            FileX.

        fx_media_driver_logical_sector      Logical sector FileX is requesting.

        fx_media_driver_sectors             Number of sectors FileX is requesting.


       The following is a summary of the optional FX_MEDIA structure members:

            FX_MEDIA Member                              Meaning

        fx_media_driver_info                Pointer to any additional information 
                                            or memory. This is optional for the 
                                            driver use and is setup from the 
                                            fx_media_open call. The RAM disk uses
                                            this pointer for the RAM disk memory 
                                            itself.

        fx_media_driver_write_protect       The DRIVER sets this to FX_TRUE when 
                                            media is write protected. This is 
                                            typically done in initialization,
                                            but can be done anytime.

        fx_media_driver_free_sector_update  The DRIVER sets this to FX_TRUE when 
                                            it needs to know when clusters are 
                                            released. This is important for FLASH 
                                            wear-leveling drivers.

        fx_media_driver_system_write        FileX sets this flag to FX_TRUE if the 
                                            sector being written is a system sector, 
                                            e.g., a boot, FAT, or directory sector. 
                                            The driver may choose to use this to 
                                            initiate error recovery logic for greater
                                            fault tolerance.

        fx_media_driver_data_sector_read    FileX sets this flag to FX_TRUE if the 
                                            sector(s) being read are file data sectors, 
                                            i.e., NOT system sectors.

        fx_media_driver_sector_type         FileX sets this variable to the specific 
                                            type of sector being read or written. The 
                                            following sector types are identified:

                                                    FX_UNKNOWN_SECTOR
                                                    FX_BOOT_SECTOR
                                                    FX_FAT_SECTOR
                                                    FX_DIRECTORY_SECTOR
                                                    FX_DATA_SECTOR
     */

    /* Process the driver request specified in the media control block.  */
    switch (media_ptr -> fx_media_driver_request)
    {

    case FX_DRIVER_READ:
    {

        /* Calculate the RAM disk sector offset. Note the RAM disk memory is pointed to by
           the fx_media_driver_info pointer, which is supplied by the application in the
           call to fx_media_open.  */
        source_buffer =  ((UCHAR *)media_ptr -> fx_media_driver_info) +
            ((media_ptr -> fx_media_driver_logical_sector + 
              media_ptr -> fx_media_hidden_sectors) * 
             media_ptr -> fx_media_bytes_per_sector);

        /* Copy the RAM sector into the destination.  */
        _fx_utility_memory_copy(source_buffer, media_ptr -> fx_media_driver_buffer, 
                                media_ptr -> fx_media_driver_sectors * 
                                media_ptr -> fx_media_bytes_per_sector);

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_WRITE:
    {

        /* Calculate the RAM disk sector offset. Note the RAM disk memory is pointed to by
           the fx_media_driver_info pointer, which is supplied by the application in the
           call to fx_media_open.  */
        destination_buffer =  ((UCHAR *)media_ptr -> fx_media_driver_info) +
            ((media_ptr -> fx_media_driver_logical_sector + 
              media_ptr -> fx_media_hidden_sectors) * 
             media_ptr -> fx_media_bytes_per_sector);

        /* Copy the source to the RAM sector.  */
        _fx_utility_memory_copy(media_ptr -> fx_media_driver_buffer, destination_buffer,
                                media_ptr -> fx_media_driver_sectors * 
                                media_ptr -> fx_media_bytes_per_sector);

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_FLUSH:
    {

        /* Return driver success.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_ABORT:
    {

        /* Return driver success.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_INIT:
    {

        /* FLASH drivers are responsible for setting several fields in the
           media structure, as follows:

                media_ptr -> fx_media_driver_free_sector_update
                media_ptr -> fx_media_driver_write_protect

           The fx_media_driver_free_sector_update flag is used to instruct
           FileX to inform the driver whenever sectors are not being used.
           This is especially useful for FLASH managers so they don't have
           maintain mapping for sectors no longer in use.

           The fx_media_driver_write_protect flag can be set anytime by the
           driver to indicate the media is not writable.  Write attempts made
           when this flag is set are returned as errors.  */

        /* Perform basic initialization here... since the boot record is going
           to be read subsequently and again for volume name requests.  */

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_UNINIT:
    {

        /* There is nothing to do in this case for the RAM driver.  For actual
           devices some shutdown processing may be necessary.  */

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_BOOT_READ:
    {

        /* Read the boot record and return to the caller.  */

        /* Calculate the RAM disk boot sector offset, which is at the very beginning of 
           the RAM disk. Note the RAM disk memory is pointed to by the 
           fx_media_driver_info pointer, which is supplied by the application in the 
           call to fx_media_open.  */
        source_buffer =  (UCHAR *)media_ptr -> fx_media_driver_info;

        /* For RAM driver, determine if the boot record is valid.  */
        if ((source_buffer[0] != (UCHAR)0xEB)  ||
            ((source_buffer[1] != (UCHAR)0x34)  &&
             (source_buffer[1] != (UCHAR)0x76)) ||          /* exFAT jump code.  */
            (source_buffer[2] != (UCHAR)0x90))
        {

            /* Invalid boot record, return an error!  */
            media_ptr -> fx_media_driver_status =  FX_MEDIA_INVALID;
            return;
        }

        /* For RAM disk only, pickup the bytes per sector.  */
        bytes_per_sector =  _fx_utility_16_unsigned_read(&source_buffer[FX_BYTES_SECTOR]);

#ifdef FX_ENABLE_EXFAT
        /* if byte per sector is zero, then treat it as exFAT volume.  */
        if (bytes_per_sector == 0 && (source_buffer[1] == (UCHAR)0x76))
        {

            /* Pickup the byte per sector shift, and calculate byte per sector.  */
            bytes_per_sector = (UINT)(1 << source_buffer[FX_EF_BYTE_PER_SECTOR_SHIFT]);
        }
#endif /* FX_ENABLE_EXFAT */

        /* Ensure this is less than the media memory size.  */
        if (bytes_per_sector > media_ptr -> fx_media_memory_size)
        {
            media_ptr -> fx_media_driver_status =  FX_BUFFER_ERROR;
            break;
        }

        /* Copy the RAM boot sector into the destination.  */
        _fx_utility_memory_copy(source_buffer, media_ptr -> fx_media_driver_buffer,
                                bytes_per_sector);

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    case FX_DRIVER_BOOT_WRITE:
    {

        /* Write the boot record and return to the caller.  */

        /* Calculate the RAM disk boot sector offset, which is at the very beginning of the
           RAM disk. Note the RAM disk memory is pointed to by the fx_media_driver_info 
           pointer, which is supplied by the application in the call to fx_media_open.  */
        destination_buffer =  (UCHAR *)media_ptr -> fx_media_driver_info;

        /* Copy the RAM boot sector into the destination.  */
        _fx_utility_memory_copy(media_ptr -> fx_media_driver_buffer, destination_buffer,
                                media_ptr -> fx_media_bytes_per_sector);

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
        break;
    }

    default:
    {

        /* Invalid driver request.  */
        media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
        break;
    }
    }
}

