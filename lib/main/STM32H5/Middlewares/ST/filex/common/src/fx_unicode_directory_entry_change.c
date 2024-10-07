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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_directory_entry_change                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function changes the unicode name of a directory entry.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    entry_ptr                             Directory entry               */
/*    unicode_name                          Destination unicode name      */
/*    unicode_name_length                   Unicode name size             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*    _fx_utility_logical_sector_read       Read a logical sector         */
/*    _fx_utility_logical_sector_write      Write a logical sector        */
/*    _fx_fault_tolerant_add_dir_log        Add directory redo log        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Unicode Utilities                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_unicode_directory_entry_change(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr, UCHAR *unicode_name, ULONG unicode_name_length)
{

UCHAR *work_ptr, *sector_base_ptr;
UINT   status;
UINT   i, j, k, u, card, lfn_entries;
UCHAR  eof_marker;
ULONG  logical_sector, relative_sector;
ULONG  byte_offset;
ULONG  cluster, next_cluster;

#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR *changed_ptr;
UINT   changed_size;
ULONG  changed_offset;
#endif /* FX_ENABLE_FAULT_TOLERANT */


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of directory entry write requests.  */
    media_ptr -> fx_media_directory_entry_writes++;
#endif

    /* Pickup the byte offset of the entry.  */
    byte_offset = entry_ptr -> fx_dir_entry_byte_offset;

    /* Pickup the logical sector of the entry.  */
    logical_sector = (ULONG)entry_ptr -> fx_dir_entry_log_sector;

    /* Figure out where what cluster we are in.  */
    if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
    {

        /* Calculate the cluster that this logical sector is in.  */
        cluster =  (logical_sector - media_ptr -> fx_media_data_sector_start) / (media_ptr -> fx_media_sectors_per_cluster) + FX_FAT_ENTRY_START;

        /* Calculate the relative cluster.  */
        relative_sector =  logical_sector -  (((ULONG)media_ptr -> fx_media_data_sector_start) +
                                              (((ULONG)cluster - FX_FAT_ENTRY_START) *
                                               ((ULONG)media_ptr -> fx_media_sectors_per_cluster)));
    }
    else
    {

        /* Clear the cluster and the relative sector.  */
        cluster =  0;
        relative_sector =  0;
    }

    /* Read the logical directory sector.  */
    status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) entry_ptr -> fx_dir_entry_log_sector,
                                              media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

    /* Determine if an error occurred.  */
    if (status != FX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Setup a pointer into the buffer.  */
    sector_base_ptr = (UCHAR *)media_ptr -> fx_media_memory_buffer;
    work_ptr =  sector_base_ptr + (UINT)entry_ptr -> fx_dir_entry_byte_offset;

    /* Initialize the unicode index.  */
    u =  0;

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Initialize data for fault tolerant. */
    changed_ptr = work_ptr;
    changed_size = 0;
    changed_offset = entry_ptr -> fx_dir_entry_byte_offset;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Check for a valid long name.  */
    if ((0x40 & (*work_ptr)))
    {

        /* Get the lower 5 bit containing the cardinality.  */
        card = (*work_ptr & (UCHAR)0x1f);
        lfn_entries =  card;

        /* Loop through the file name.  */
        for (j = 0; j < lfn_entries; j++)
        {

            /* Clear the eof marker.  */
            eof_marker =  0;

            /* Loop through file name fields.  */
            for (i = 1, k = (26 * (card - 1)) & 0xFFFFFFFF; i < FX_DIR_ENTRY_SIZE; i += 2)
            {

                /* Process relative to specific fields.  */
                if ((i == 11) || (i == 26))
                {
                    continue;
                }

                if (i == 13)
                {
                    i = 12;
                    continue;
                }

                /* Determine if the EOF marker is present.  */
                if (eof_marker)
                {

                    work_ptr[i] = eof_marker;
                    work_ptr[i + 1] = eof_marker;
                }
                else if (k < (unicode_name_length * 2))
                {

                    work_ptr[i] = unicode_name[k];
                    work_ptr[i + 1] = unicode_name[k + 1];
                    u =  u + 2;
                }
                else if (k == (unicode_name_length * 2))
                {

                    work_ptr[i] = 0;
                    work_ptr[i + 1] =  0;

                    /* end of name, pad with 0xff.  */
                    eof_marker =  (UCHAR)0xff;
                }

                k =  k + 2;
            }

            /* Decrement the card.  */
            card--;

            /* Setup pointers for the name write.  */
            work_ptr += FX_DIR_ENTRY_SIZE;
            byte_offset += FX_DIR_ENTRY_SIZE;

#ifdef FX_ENABLE_FAULT_TOLERANT
            /* Update changed_size. */
            changed_size += FX_DIR_ENTRY_SIZE;
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Determine if the write is within the current sector.   */
            if (byte_offset >= media_ptr -> fx_media_bytes_per_sector)
            {
#ifdef FX_ENABLE_FAULT_TOLERANT
                if (media_ptr -> fx_media_fault_tolerant_enabled)
                {

                    /* Redirect this request to log file. */
                    status = _fx_fault_tolerant_add_dir_log(media_ptr, (ULONG64) logical_sector, changed_offset, changed_ptr, changed_size);
                }
                else
                {

                    /* Write the current sector out.  */
                    status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                               sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
                }


                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }
#else /* FX_ENABLE_FAULT_TOLERANT */

                /* Write the current sector out.  */
                /* Note: Since the sector is in the cache after a sector read, therefore _fx_utility_logical_sector_write
                   always returns success when FX_ENABLE_FAULT_TOLERANT is not defined.  In other words, the checking
                   on the return value is needed only when FX_ENABLE_FAULT_TOLERANT is defined. */
                _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                 sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
#endif /* FX_ENABLE_FAULT_TOLERANT */

                /* Determine if we are in the root directory.  */
                if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
                {

                    /* Determine the next sector of the directory entry.  */
                    if (relative_sector < (media_ptr -> fx_media_sectors_per_cluster - 1))
                    {

                        /* More sectors in this cluster.  */

                        /* Simply increment the logical sector.  */
                        logical_sector++;

                        /* Increment the relative sector.  */
                        relative_sector++;
                    }
                    else
                    {

                        /* We need to move to the next cluster.  */

                        /* Pickup the next cluster.  */
                        status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

                        /* Check for I/O error.  */
                        if (status != FX_SUCCESS)
                        {

                            /* Return error code.  */
                            return(status);
                        }

                        /* Copy next cluster to the current cluster.  */
                        cluster =  next_cluster;

                        /* Check the value of the new cluster - it must be a valid cluster number
                           or something is really wrong!  */
                        if ((cluster < FX_FAT_ENTRY_START) || (cluster > media_ptr -> fx_media_fat_reserved))
                        {

                            /* Send error message back to caller.  */
                            return(FX_FILE_CORRUPT);
                        }

                        /* Setup the relative sector (this is zero for subsequent cluster.  */
                        relative_sector =  0;

                        /* Calculate the next logical sector.  */
                        logical_sector =   ((ULONG)media_ptr -> fx_media_data_sector_start) +
                            (((ULONG)cluster - FX_FAT_ENTRY_START) *
                             ((ULONG)media_ptr -> fx_media_sectors_per_cluster));
                    }
                }
                else
                {

                    /* Increment the logical sector.  */
                    logical_sector++;

                    /* Determine if the logical sector is valid.  */
                    if (logical_sector >= (ULONG)(media_ptr -> fx_media_data_sector_start))
                    {

                        /* We have exceeded the root directory.  */

                        /* Send error message back to caller.  */
                        return(FX_FILE_CORRUPT);
                    }
                }

                /* Read the next logical sector.  */
                status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) logical_sector,
                                                          media_ptr -> fx_media_memory_buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);

                /* Determine if an error occurred.  */
                if (status != FX_SUCCESS)
                {

                    /* Return the error status.  */
                    return(status);
                }

                /* Move to the next sector buffer.  */
                sector_base_ptr = media_ptr -> fx_media_memory_buffer;

                /* Setup new buffer pointers.  */
                byte_offset =  0;
                work_ptr = sector_base_ptr;

#ifdef FX_ENABLE_FAULT_TOLERANT
                /* Initialize data for fault tolerant. */
                changed_ptr = work_ptr;
                changed_size = 0;
                changed_offset = 0;
#endif /* FX_ENABLE_FAULT_TOLERANT */
            }
        }
    }

    /* Check for an error!  */
    if (u != (unicode_name_length * 2))
    {

        /* Return an error!  */
        return(FX_FILE_CORRUPT);
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {

        /* Redirect this request to log file. */
        status = _fx_fault_tolerant_add_dir_log(media_ptr, (ULONG64) logical_sector, changed_offset, changed_ptr, changed_size);
    }
    else
    {
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Write the directory sector to the media.  */
        status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) logical_sector,
                                                   sector_base_ptr, ((ULONG) 1), FX_DIRECTORY_SECTOR);
#ifdef FX_ENABLE_FAULT_TOLERANT
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */
    
    return(status);
}

