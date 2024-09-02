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
#include "fx_media.h"
#include "fx_utility.h"


/* Define global variables necessary for formatting.  */

/* Define OEM Name. This name must be 8 characters long and be blank padded.
   The default may be changed by modifying this file or calling the
   fx_media_format_oem_name_set utility prior to calling fx_media_format.  */

UCHAR   _fx_media_format_oem_name[8] = "EL FILEX";


/* Define the default media type.  This default may be changed by modifying
   this file or calling the fx_media_format_type_set utility prior to calling
   fx_media_format.  */

UCHAR _fx_media_format_media_type =  0xF8;


/* Define the default volume ID.  This default may be changed by modifying
   this file or calling the fx_media_format_volume_id_set utility prior to calling
   fx_media_format.  */

ULONG _fx_media_format_volume_id =  1;


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_format                                    PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a FAT12/16/32 format with raw calls to the    */
/*    I/O driver. It can and must be called before the fx_media_open      */
/*    and is designed to utilize the same underlying FileX driver.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media control block*/
/*                                            (does not need to be opened)*/
/*    driver                                Pointer to FileX driver (must */
/*                                            be able to field requests   */
/*                                            prior to opening)           */
/*    driver_info_ptr                       Optional information pointer  */
/*    memory_ptr                            Pointer to memory used by the */
/*                                            FileX for this media.       */
/*    memory_size                           Size of media memory - must   */
/*                                            at least 512 bytes and      */
/*                                            one sector size.            */
/*    volume_name                           Name of the volume            */
/*    number_of_fats                        Number of FAT tables          */
/*    directory_entries                     Number of directory entries   */
/*    hidden_sectors                        Number of hidden sectors      */
/*    total_sectors                         Total number of sectors       */
/*    bytes_per_sector                      Number of bytes per sector    */
/*    sectors_per_cluster                   Number of sectors per cluster */
/*    heads                                 Number of heads               */
/*    sectors_per_track                     Number of sectors per track   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Media driver                                                        */
/*    _fx_utility_16_unsigned_write         Write 16-bit unsigned         */
/*    _fx_utility_32_unsigned_write         Write 32-bit unsigned         */
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
/*                                            resulting in version 6.1    */
/*  03-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.5  */
/*  08-02-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            updated boot write logic,   */
/*                                            resulting in version 6.1.8  */
/*  04-25-2022     Bhupendra Naphade        Modified comment(s), and      */
/*                                            updated reserved FAT entry  */
/*                                            value,                      */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                       CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors,
                       ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster,
                       UINT heads, UINT sectors_per_track)
{

UCHAR *byte_ptr;
UINT   reserved_sectors, i, j, root_sectors, total_clusters, bytes_needed;
UINT   sectors_per_fat, f, s;


    /* Create & write bootrecord from drive geometry information.  */

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_FORMAT, media_ptr, directory_entries, total_sectors, sectors_per_cluster, FX_TRACE_MEDIA_EVENTS, 0, 0)

    /* Validate bytes per sector value: greater than zero and no more than 4096.  */
    if((bytes_per_sector == 0) || (bytes_per_sector > 4096))
        return(FX_SECTOR_INVALID);

    /* Validate sectors per cluster value: greater than zero and no more than 128.  */
    if((sectors_per_cluster == 0) || (sectors_per_cluster > 128))
        return(FX_SECTOR_INVALID);

    /* Setup driver pointer and memory information.  */
    media_ptr -> fx_media_driver_entry =                driver;
    media_ptr -> fx_media_memory_buffer =               (UCHAR *)memory_ptr;
    media_ptr -> fx_media_memory_size =                 memory_size;

    /* Store geometry information in media record - driver needs this.  */
    media_ptr -> fx_media_bytes_per_sector =            bytes_per_sector;
    media_ptr -> fx_media_sectors_per_track =           sectors_per_track;
    media_ptr -> fx_media_heads =                       heads;
    media_ptr -> fx_media_hidden_sectors =              hidden_sectors;

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

    /* Setup driver buffer memory.  */
    media_ptr -> fx_media_driver_buffer =  memory_ptr;

    /* Move the buffer pointer into a local copy.  */
    byte_ptr =  media_ptr -> fx_media_driver_buffer;

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    /* Clear the buffer record out, assuming it is large enough for one sector.   */
    for (i = 0; i < bytes_per_sector; i++)
    {

        /* Clear each byte of the boot record.  */
        byte_ptr[i] =  (UCHAR)0;
    }
#else
    _fx_utility_memory_set(byte_ptr, 0, bytes_per_sector);
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

    /* Set jump instruction at the beginning of the sector.  */
    byte_ptr[0] =  (UCHAR)0xEB;
    byte_ptr[1] =  (UCHAR)0x34;
    byte_ptr[2] =  (UCHAR)0x90;

    /* Set the OEM name in the boot record.  */
    for (i = 0; i < 8; i++)
    {

        /* Copy a character from the OEM name.  */
        byte_ptr[i + 3] =  _fx_media_format_oem_name[i];
    }

    /* Set the media type in the boot record.  */
    byte_ptr[FX_MEDIA_TYPE] =  _fx_media_format_media_type;

    /* Set the number of bytes per sector.  */
    _fx_utility_16_unsigned_write(&byte_ptr[FX_BYTES_SECTOR], bytes_per_sector);

    /* Set the number of sectors per track.  */
    _fx_utility_16_unsigned_write(&byte_ptr[FX_SECTORS_PER_TRK], sectors_per_track);

    /* Set the number of heads.  */
    _fx_utility_16_unsigned_write(&byte_ptr[FX_HEADS], heads);

#ifdef FX_FORCE_512_BYTE_BOOT_SECTOR

    /* Calculate the number of reserved sectors. If sector size is smaller than 512 bytes, there will be
       reserved sectors, otherwise assumed that only the sector containing bootrecord is reserved.  */
    if (bytes_per_sector < 512)
    {
        reserved_sectors =  512 / bytes_per_sector;
    }
    else
    {
        reserved_sectors =  1;
    }
#else

    /* The boot sector is the only reserved sector.  */
    reserved_sectors =  1;
#endif


    /* Calculate the maximum clusters.... This is actually greater than the actual since the FAT
       sectors have yet to be accounted for.  */
    total_clusters =  (total_sectors - reserved_sectors - ((directory_entries * FX_DIR_ENTRY_SIZE) + (bytes_per_sector - 1)) / bytes_per_sector) / sectors_per_cluster;

    /* Calculate the maximum number of FAT sectors necessary for FAT12.  */
    if (total_clusters % 2)
    {
        bytes_needed = (total_clusters + total_clusters / 2) + 1;
    }
    else
    {
        bytes_needed = (total_clusters + total_clusters / 2);
    }
    sectors_per_fat =  bytes_needed / bytes_per_sector;
    if (bytes_needed % bytes_per_sector)
    {
        sectors_per_fat++;
    }

    /* Now adjust the total clusters by the number of sectors per FAT.  */
    total_clusters =  total_clusters - ((sectors_per_fat * number_of_fats) + (sectors_per_cluster - 1)) / sectors_per_cluster;

    /* Is the total cluster count greater than the FAT12 maximum?  */
    if (total_clusters >= FX_12_BIT_FAT_SIZE)
    {

        /* Yes, too big for FAT12, we need to evaluate for FAT16.  */

        /* Reset the maximum clusters.... This is actually greater than the actual since the FAT
           sectors have yet to be accounted for.  */
        total_clusters =  (total_sectors - reserved_sectors -  ((directory_entries * FX_DIR_ENTRY_SIZE) + (bytes_per_sector - 1)) / bytes_per_sector) / sectors_per_cluster;

        /* Calculate 16-bit FAT is present. Each cluster requires a 2 byte entry in the FAT table.  */
        sectors_per_fat =  (total_clusters * 2) / bytes_per_sector;
        if ((total_clusters * 2) % bytes_per_sector)
        {
            sectors_per_fat++;
        }

        /* Now adjust the total clusters by the number of sectors per FAT.  */
        total_clusters =  total_clusters - ((sectors_per_fat * number_of_fats) + (sectors_per_cluster - 1)) / sectors_per_cluster;

        /* Is the total cluster count greater than the FAT16 maximum?  */
        if (total_clusters >= FX_16_BIT_FAT_SIZE)
        {

            /* Yes, FAT32 is present.  */

            /* Allocate room for the FAT32 additional information sector. This contains useful information
               such as the number of available clusters between successive mounting of the media.  */
            if (bytes_per_sector == 512)
            {

                /* Write sector number 1 to the additional information sector.  */
                _fx_utility_16_unsigned_write(&byte_ptr[48], 1);

                /* Increment the reserved sectors count, since this will count as a reserved sector.  */
                reserved_sectors++;
            }
            else
            {

                /* Write value to indicate there is no additional information sector.  */
                _fx_utility_16_unsigned_write(&byte_ptr[48], 0xFFFF);
            }

            /* Allocate the first cluster to the root directory.  */
            _fx_utility_32_unsigned_write(&byte_ptr[FX_ROOT_CLUSTER_32], FX_FAT_ENTRY_START);

            /* Determine if the number of root directory entries should be modified.  */
            directory_entries =  (sectors_per_cluster * bytes_per_sector) / FX_DIR_ENTRY_SIZE;

            /* Reset the total_clusters for the FAT32 calculation.  */
            total_clusters =  (total_sectors - reserved_sectors) / sectors_per_cluster;

            /* 32-bit FAT is present. Each cluster requires a 4 byte entry in the FAT table.  */
            sectors_per_fat =  (total_clusters * 4) / bytes_per_sector;
            if ((total_clusters * 4) % bytes_per_sector)
            {
                sectors_per_fat++;
            }

            /* Now adjust the total clusters by the number of sectors per FAT.  */
            total_clusters =  total_clusters - ((sectors_per_fat * number_of_fats) + (sectors_per_cluster - 1)) / sectors_per_cluster;
        }
    }

    /* Set sectors per FAT type.  */
    if (total_clusters < FX_16_BIT_FAT_SIZE)
    {

        /* Set the number of sectors per FAT12/16.  */
        _fx_utility_16_unsigned_write(&byte_ptr[FX_SECTORS_PER_FAT], sectors_per_fat);

        /* Set the signature.  */
        byte_ptr[FX_BOOT_SIG] =  0x29;

        /* Setup the volume ID.  */
        _fx_utility_32_unsigned_write(&byte_ptr[FX_VOLUME_ID], _fx_media_format_volume_id);
    }
    else
    {

        /* Set the number of sectors per FAT32.  */
        _fx_utility_32_unsigned_write(&byte_ptr[FX_SECTORS_PER_FAT_32], sectors_per_fat);

        /* Set the signature.  */
        byte_ptr[FX_BOOT_SIG_32] =  0x29;

        /* Setup the volume ID.  */
        _fx_utility_32_unsigned_write(&byte_ptr[FX_VOLUME_ID_32], _fx_media_format_volume_id);
    }

    /* Set the total number of sectors.  */
    if (total_sectors < (ULONG)0xFFFF)
    {

        /* Write the 16-bit total sector field.  */
        _fx_utility_16_unsigned_write(&byte_ptr[FX_SECTORS], (UINT)(total_sectors));

        /* Set the number of huge sectors.  */
        _fx_utility_32_unsigned_write(&byte_ptr[FX_HUGE_SECTORS], 0);
    }
    else
    {

        /* Write the 16-bit total sector field as 0.  */
        _fx_utility_16_unsigned_write(&byte_ptr[FX_SECTORS], (UINT)0);

        /* Set the number of huge sectors.  */
        _fx_utility_32_unsigned_write(&byte_ptr[FX_HUGE_SECTORS], total_sectors);
    }

    /* Set the number of reserved sectors.  */
    _fx_utility_16_unsigned_write(&byte_ptr[FX_RESERVED_SECTORS], reserved_sectors);

    /* Set the number of sectors per cluster */
    byte_ptr[FX_SECTORS_CLUSTER] =  (UCHAR)sectors_per_cluster;

    /* Set the number of FATs.  */
    byte_ptr[FX_NUMBER_OF_FATS] =  (UCHAR)number_of_fats;

    /* Set the number of hidden sectors.  */
    _fx_utility_32_unsigned_write(&byte_ptr[FX_HIDDEN_SECTORS], hidden_sectors);

    /* Determine if a FAT12 or FAT16 is present.  If FAT32 is present, these fields are left alone!   */
    if (total_clusters < FX_16_BIT_FAT_SIZE)
    {

        /* Yes, set the number of root directory entries.  */
        _fx_utility_16_unsigned_write(&byte_ptr[FX_ROOT_DIR_ENTRIES], directory_entries);
    }

    /* Now setup the volume label. */
    if (total_clusters < FX_16_BIT_FAT_SIZE)
    {

        /* FAT12/16 volume label offset.  */
        j =  FX_VOLUME_LABEL;
    }
    else
    {

        /* FAT32 volume label offset.  */
        j =  FX_VOLUME_LABEL_32;
    }

    i = 0;
    while (i < 11)
    {

        /* Determine if it is NULL.  */
        if (volume_name[i] == 0)
        {

            /* Yes, the copying is finished.  */
            break;
        }

        /* Otherwise, copy byte of volume name into boot record.  */
        byte_ptr[j + i] =  (UCHAR)volume_name[i];

        /* Increment byte position.  */
        i++;
    }

    /* Now blank-pad the remainder of the volume name.  */
#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    while (i < 11)
    {

        byte_ptr[j + i] =  (UCHAR)' ';
        i++;
    }
#else
    _fx_utility_memory_set(&byte_ptr[j + i], ' ', (11 - i));
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */


#ifdef FX_FORCE_512_BYTE_BOOT_SECTOR

    /* Set bootrecord signature.  */
    byte_ptr[510] = 0x55;
    byte_ptr[511] = 0xAA;
#else

    /* Set bootrecord signature.  */
    byte_ptr[bytes_per_sector - 2] = 0x55;
    byte_ptr[bytes_per_sector - 1] = 0xAA;
#endif

    /* Select the boot record write command.  */
    media_ptr -> fx_media_driver_request =       FX_DRIVER_BOOT_WRITE;
    media_ptr -> fx_media_driver_system_write =  FX_TRUE;
    media_ptr -> fx_media_driver_sectors =       1;
    media_ptr -> fx_media_driver_sector_type =   FX_BOOT_SECTOR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_BOOT_WRITE, media_ptr, memory_ptr, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Write out the bootrecord */
    (driver)(media_ptr);

    /* Clear the write flag.  */
    media_ptr -> fx_media_driver_system_write =  FX_FALSE;

    /* Determine if it was successful.  */
    if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
    {
        return(FX_IO_ERROR);
    }

    /* Calculate the number of root sectors.  */
    root_sectors =    ((directory_entries * FX_DIR_ENTRY_SIZE) + bytes_per_sector - 1) / bytes_per_sector;

    /* Determine if FAT32 is present AND if the bytes per sector is large enough to have
       a FSINFO sector.  */
    if ((total_clusters >= FX_16_BIT_FAT_SIZE) && (bytes_per_sector == 512))
    {

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
        /* Clear sector buffer.  */
        for (i = 0; i < bytes_per_sector; i++)
        {
            byte_ptr[i] =  (CHAR)0;
        }
#else
        _fx_utility_memory_set(byte_ptr, 0, bytes_per_sector);
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

        /* Build the FSINFO fields.  */

        /* Build first signature word, used to help verify this is a FSINFO sector.  */
        byte_ptr[0] =  0x52;
        byte_ptr[1] =  0x52;
        byte_ptr[2] =  0x61;
        byte_ptr[3] =  0x41;

        /* Build the next signature word, this too is used to help verify that this is a FSINFO sector.  */
        byte_ptr[484] =  0x72;
        byte_ptr[485] =  0x72;
        byte_ptr[486] =  0x41;
        byte_ptr[487] =  0x61;

        /* Build the final signature word, this too is used to help verify that this is a FSINFO sector.  */
        byte_ptr[508] =  0x55;
        byte_ptr[509] =  0xAA;

        /* Setup the total available clusters on the media. We need to subtract 1 for the FAT32 root directory.  */
        _fx_utility_32_unsigned_write(&byte_ptr[488], (total_clusters - 1));

        /* Setup the starting free cluster to 3, since cluster 2 is reserved for the FAT32 root directory.  */
        _fx_utility_32_unsigned_write(&byte_ptr[492], 3);

        /* Now write the FSINFO sector to the media.  */
        media_ptr -> fx_media_driver_logical_sector =  1;
        media_ptr -> fx_media_driver_request =         FX_DRIVER_WRITE;
        media_ptr -> fx_media_driver_sectors =         1;
        media_ptr -> fx_media_driver_system_write =    FX_TRUE;
        media_ptr -> fx_media_driver_sector_type =     FX_BOOT_SECTOR;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, 1, 1, memory_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Write out the sector.  */
        (driver)(media_ptr);

        /* Clear the system write flag.  */
        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

        /* Determine if it was successful.  */
        if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
        {
            return(FX_IO_ERROR);
        }
    }

    /* At this point we need set up first to FAT entries and clear the remaining FAT sectors area.  */

    /* Loop through number of FATs. The first is the only one used.  */
    for (f = 0; f < number_of_fats; f++)
    {

        /* Loop through all the sectors in this FAT.  */
        for (s = 0; s < sectors_per_fat; s++)
        {

            if (s == 0)
            {

                /* Reserve the first two FAT table entries.  */
                if (total_clusters < FX_12_BIT_FAT_SIZE)
                {

                    /* Reserve the first two FAT-12 entries.  */
                    byte_ptr[0] =  _fx_media_format_media_type;
                    byte_ptr[1] =  (UCHAR)0xFF;
                    byte_ptr[2] =  (UCHAR)0xFF;

                    /* Start clearing at FAT entry 3.  */
                    i =  3;
                }
                else if (total_clusters < FX_16_BIT_FAT_SIZE)
                {

                    /* Reserve the first two FAT-16 entries.  */
                    byte_ptr[0] =  _fx_media_format_media_type;
                    byte_ptr[1] =  (UCHAR)0xFF;
                    byte_ptr[2] =  (UCHAR)0xFF;
                    byte_ptr[3] =  (UCHAR)0xFF;

                    /* Start clearing at FAT entry 3.  */
                    i =  4;
                }
                else
                {

                    /* Reserve the first two FAT-32 entries.   */
                    byte_ptr[0] =  _fx_media_format_media_type;
                    byte_ptr[1] =  (UCHAR)0xFF;
                    byte_ptr[2] =  (UCHAR)0xFF;
                    byte_ptr[3] =  (UCHAR)0x0F;
                    byte_ptr[4] =  (UCHAR)0xFF;
                    byte_ptr[5] =  (UCHAR)0xFF;
                    byte_ptr[6] =  (UCHAR)0xFF;
                    byte_ptr[7] =  (UCHAR)0x0F;

                    /* Preallocate the first cluster for the root directory.  */
                    byte_ptr[8] =   (UCHAR)0xFF;
                    byte_ptr[9] =   (UCHAR)0xFF;
                    byte_ptr[10] =  (UCHAR)0xFF;
                    byte_ptr[11] =  (UCHAR)0x0F;

                    /* Start clearing at FAT entry 3.  */
                    i =  12;
                }
            }
            else
            {
                i = 0;
            }

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
            /* Clear remainder of sector buffer.  */
            for (; i < bytes_per_sector; i++)
            {
                byte_ptr[i] =  (CHAR)0;
            }
#else
            _fx_utility_memory_set(&byte_ptr[i], 0, (bytes_per_sector - i));
#endif  /* FX_DISABLE_FORCE_MEMORY_OPERATION */

            /* Build sector write command.  */
            media_ptr -> fx_media_driver_logical_sector =  reserved_sectors + (f * sectors_per_fat) + s;
            media_ptr -> fx_media_driver_request =         FX_DRIVER_WRITE;
            media_ptr -> fx_media_driver_sectors =         1;
            media_ptr -> fx_media_driver_system_write =    FX_TRUE;
            media_ptr -> fx_media_driver_sector_type =     FX_FAT_SECTOR;

            /* If trace is enabled, insert this event into the trace buffer.  */
            FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, media_ptr -> fx_media_driver_logical_sector, 1, memory_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

            /* Write out the sector.  */
            (driver)(media_ptr);

            /* Clear the system write flag.  */
            media_ptr -> fx_media_driver_system_write =  FX_FALSE;

            /* Determine if it was successful.  */
            if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
            {
                return(FX_IO_ERROR);
            }
        }
    }

#ifndef FX_DISABLE_FORCE_MEMORY_OPERATION
    /* Clear sector buffer.  */
    for (i = 0; i < bytes_per_sector; i++)
    {
        byte_ptr[i] =  (CHAR)0;
    }
#else
    _fx_utility_memory_set(byte_ptr, 0, bytes_per_sector);
#endif /* FX_DISABLE_FORCE_MEMORY_OPERATION */

    /* Now clear the root directory sectors.  */
    for (s = 0; s < root_sectors; s++)
    {

        /* Build sector write command.  */
        media_ptr -> fx_media_driver_logical_sector =  reserved_sectors + (number_of_fats * sectors_per_fat) + s;
        media_ptr -> fx_media_driver_request =         FX_DRIVER_WRITE;
        media_ptr -> fx_media_driver_sectors =         1;
        media_ptr -> fx_media_driver_system_write =    FX_TRUE;
        media_ptr -> fx_media_driver_sector_type =     FX_DIRECTORY_SECTOR;

        /* If trace is enabled, insert this event into the trace buffer.  */
        FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, media_ptr -> fx_media_driver_logical_sector, 1, memory_ptr, FX_TRACE_INTERNAL_EVENTS, 0, 0)

        /* Write out the sector.  */
        (driver)(media_ptr);

        /* Clear the write flag.  */
        media_ptr -> fx_media_driver_system_write =  FX_FALSE;

        /* Determine if it was successful.  */
        if (media_ptr -> fx_media_driver_status != FX_SUCCESS)
        {
            return(FX_IO_ERROR);
        }
    }

    /* Build the "uninitialize" I/O driver request.  */
    media_ptr -> fx_media_driver_request =      FX_DRIVER_UNINIT;
    media_ptr -> fx_media_driver_status =       FX_IO_ERROR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_UNINIT, media_ptr, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Call the specified I/O driver with the uninitialize request.  */
    (media_ptr -> fx_media_driver_entry) (media_ptr);

    /* Return success!  */
    return(media_ptr -> fx_media_driver_status);
}

