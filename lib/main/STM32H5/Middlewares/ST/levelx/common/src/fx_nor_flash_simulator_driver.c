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
/**   FileX NOR FLASH Simulator Driver                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#include "fx_api.h"
#include "lx_api.h"


/* Create a NOR flash control block.  */

LX_NOR_FLASH       nor_flash;


/* The simulated NOR driver relies on the fx_media_format call to be made prior to
   the fx_media_open call.   

        fx_media_format(&ram_disk, 
                            _fx_nor_sim_driver,     // Driver entry
                            FX_NULL,                // Unused
                            media_memory,           // Media buffer pointer
                            sizeof(media_memory),   // Media buffer size 
                            "MY_NOR_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            120,                    // Total sectors 
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

*/


/* Define prototypes.  */

UINT  _lx_nor_flash_simulator_initialize(LX_NOR_FLASH *nor_flash);
VOID  _fx_nor_flash_simulator_driver(FX_MEDIA *media_ptr);


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _fx_nor_flash_simulator_driver                      PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the entry point to the generic NOR simulated       */ 
/*    disk driver that is delivered with the flash wear leveling product  */ 
/*    LevelX.                                                             */ 
/*                                                                        */ 
/*    This driver also serves as a template for developing other LevelX   */ 
/*    NOR flash drivers for actual flash devices. Simply replace the      */ 
/*    read/write sector logic with calls to read/write from the           */ 
/*    appropriate physical device access functions.                       */ 
/*                                                                        */ 
/*    FileX NOR FLASH structures look like the following:                 */ 
/*                                                                        */ 
/*          Logical Sector                  Contents                      */ 
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
/*    lx_nor_flash_close                    Close NOR flash manager       */ 
/*    lx_nor_flash_open                     Open NOR flash manager        */ 
/*    lx_nor_flash_sector_read              Read a NOR sector             */ 
/*    lx_nor_flash_sector_release           Release a NOR sector          */ 
/*    lx_nor_flash_sector_write             Write a NOR sector            */ 
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
VOID  _fx_nor_flash_simulator_driver(FX_MEDIA *media_ptr)
{

UCHAR       *source_buffer;
UCHAR       *destination_buffer;
ULONG       logical_sector;
ULONG       i;
UINT        status;


    /* There are several useful/important pieces of information contained in the media
       structure, some of which are supplied by FileX and others are for the driver to
       setup. The following is a summary of the necessary FX_MEDIA structure members:
       
            FX_MEDIA Member                              Meaning
                                        
        fx_media_driver_request             FileX request type. Valid requests from FileX are 
                                            as follows:

                                                    FX_DRIVER_READ
                                                    FX_DRIVER_WRITE                 
                                                    FX_DRIVER_FLUSH
                                                    FX_DRIVER_ABORT
                                                    FX_DRIVER_INIT
                                                    FX_DRIVER_BOOT_READ
                                                    FX_DRIVER_RELEASE_SECTORS
                                                    FX_DRIVER_BOOT_WRITE
                                                    FX_DRIVER_UNINIT

        fx_media_driver_status              This value is RETURNED by the driver. If the 
                                            operation is successful, this field should be
                                            set to FX_SUCCESS for before returning. Otherwise,
                                            if an error occurred, this field should be set
                                            to FX_IO_ERROR. 

        fx_media_driver_buffer              Pointer to buffer to read or write sector data.
                                            This is supplied by FileX.

        fx_media_driver_logical_sector      Logical sector FileX is requesting.

        fx_media_driver_sectors             Number of sectors FileX is requesting.


       The following is a summary of the optional FX_MEDIA structure members:
       
            FX_MEDIA Member                              Meaning
                                        
        fx_media_driver_info                Pointer to any additional information or memory.
                                            This is optional for the driver use and is setup
                                            from the fx_media_open call. The RAM disk uses
                                            this pointer for the RAM disk memory itself.

        fx_media_driver_write_protect       The DRIVER sets this to FX_TRUE when media is write 
                                            protected. This is typically done in initialization, 
                                            but can be done anytime.

        fx_media_driver_free_sector_update  The DRIVER sets this to FX_TRUE when it needs to 
                                            know when clusters are released. This is important
                                            for FLASH wear-leveling drivers.

        fx_media_driver_system_write        FileX sets this flag to FX_TRUE if the sector being
                                            written is a system sector, e.g., a boot, FAT, or 
                                            directory sector. The driver may choose to use this
                                            to initiate error recovery logic for greater fault
                                            tolerance.

        fx_media_driver_data_sector_read    FileX sets this flag to FX_TRUE if the sector(s) being
                                            read are file data sectors, i.e., NOT system sectors.

        fx_media_driver_sector_type         FileX sets this variable to the specific type of 
                                            sector being read or written. The following sector
                                            types are identified:

                                                    FX_UNKNOWN_SECTOR 
                                                    FX_BOOT_SECTOR
                                                    FX_FAT_SECTOR
                                                    FX_DIRECTORY_SECTOR
                                                    FX_DATA_SECTOR  
    */

    /* Process the driver request specified in the media control block.  */
    switch(media_ptr -> fx_media_driver_request)
    {

        case FX_DRIVER_READ:
        {

            /* Setup the destination buffer and logical sector.  */
            logical_sector =      media_ptr -> fx_media_driver_logical_sector;
            destination_buffer =  (UCHAR *) media_ptr -> fx_media_driver_buffer;

            /* Loop to read sectors from flash.  */
            for (i = 0; i < media_ptr -> fx_media_driver_sectors; i++)
            {
            
                /* Read a sector from NOR flash.  */
                status =  lx_nor_flash_sector_read(&nor_flash, logical_sector, destination_buffer);

                /* Determine if the read was successful.  */
                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                    return;
                } 

                /* Move to the next entries.  */
                logical_sector++;
                destination_buffer =  destination_buffer + 512;
            }

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }
        
        case FX_DRIVER_WRITE:
        {

            /* Setup the source buffer and logical sector.  */
            logical_sector =      media_ptr -> fx_media_driver_logical_sector;
            source_buffer =       (UCHAR *) media_ptr -> fx_media_driver_buffer;

            /* Loop to write sectors to flash.  */
            for (i = 0; i < media_ptr -> fx_media_driver_sectors; i++)
            {
            
                /* Write a sector to NOR flash.  */
                status =  lx_nor_flash_sector_write(&nor_flash, logical_sector, source_buffer);

                /* Determine if the write was successful.  */
                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                    return;
                } 

                /* Move to the next entries.  */
                logical_sector++;
                source_buffer =  source_buffer + 512;
            }

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_RELEASE_SECTORS:
        {
        
            /* Setup the logical sector.  */
            logical_sector =  media_ptr -> fx_media_driver_logical_sector;

            /* Release sectors.  */
            for (i = 0; i < media_ptr -> fx_media_driver_sectors; i++)
            {
            
                /* Release NOR flash sector.  */
                status =  lx_nor_flash_sector_release(&nor_flash, logical_sector);

                /* Determine if the sector release was successful.  */
                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                    return;
                } 

                /* Move to the next entries.  */
                logical_sector++;
            }

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

            /* With flash wear leveling, FileX should tell wear leveling when sectors
               are no longer in use.  */
            media_ptr -> fx_media_driver_free_sector_update =  FX_TRUE;

            /* Open the NOR flash simulation.  */
            status =  lx_nor_flash_open(&nor_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

            /* Determine if the flash open was successful.  */
            if (status != LX_SUCCESS)
            {
                
                /* Return an I/O error to FileX.  */
                media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                return;
            } 

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_UNINIT:
        {

            /* There is nothing to do in this case for the RAM driver.  For actual
               devices some shutdown processing may be necessary.  */

            /* Close the NOR flash simulation.  */
            status =  lx_nor_flash_close(&nor_flash);

            /* Determine if the flash close was successful.  */
            if (status != LX_SUCCESS)
            {
                
                /* Return an I/O error to FileX.  */
                media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                return;
            } 

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }
        
        case FX_DRIVER_BOOT_READ:
        {

            /* Read the boot record and return to the caller.  */

            /* Setup the destination buffer.  */
            destination_buffer =  (UCHAR *) media_ptr -> fx_media_driver_buffer;

            /* Read boot sector from NOR flash.  */
            status =  lx_nor_flash_sector_read(&nor_flash, 0, destination_buffer);

            /* For NOR driver, determine if the boot record is valid.  */
            if ((destination_buffer[0] != (UCHAR) 0xEB) ||
                (destination_buffer[1] != (UCHAR) 0x34) ||
                (destination_buffer[2] != (UCHAR) 0x90))
            {

                /* Invalid boot record, return an error!  */
                media_ptr -> fx_media_driver_status =  FX_MEDIA_INVALID;
                return;
            }

            /* Determine if the boot read was successful.  */
            if (status != LX_SUCCESS)
            {
                
                /* Return an I/O error to FileX.  */
                media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                return;
            } 

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_WRITE:
        {

            /* Write the boot record and return to the caller.  */

            /* Setup the source buffer.  */
            source_buffer =       (UCHAR *) media_ptr -> fx_media_driver_buffer;

            /* Write boot sector to NOR flash.  */
            status =  lx_nor_flash_sector_write(&nor_flash, 0, source_buffer);

            /* Determine if the boot write was successful.  */
            if (status != LX_SUCCESS)
            {
                
                /* Return an I/O error to FileX.  */
                media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                return;
            } 

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break ;
        }

        default:
        {

            /* Invalid driver request.  */
            media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
            break;
        }
    }
}

