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
/**   Fault Tolerant                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE

#include "fx_api.h"
#include "fx_utility.h"
#include "fx_fault_tolerant.h"


#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_read_directory_sector            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles directory sector read requests. During file or*/
/*    directory operations with Fault Tolerant protection, intermediate   */
/*    operations are written to the fault tolerant log files.  Therefore, */
/*    sector read should search for fault tolerant log before the request */
/*    can be passed to normal directory entry read routine.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Logical sector number         */
/*    buffer_ptr                            Pointer of receiving buffer   */
/*    sectors                               Number of sectors to read     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_read          Read a USHORT from memory     */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
/*    memcpy                                Memory Copy                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_utility_logical_sector_read                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s), verified */
/*                                            memcpy usage,               */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT    _fx_fault_tolerant_read_directory_sector(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                                 VOID *buffer_ptr, ULONG64 sectors)
{
ULONG                      logs_remaining;
ULONG                      copy_offset;
ULONG                      copy_size;
UCHAR                     *current_ptr;
UCHAR                     *current_buffer_ptr;
ULONG                      size;
USHORT                     type;
ULONG                      log_len;
ULONG64                    log_sector;
FX_FAULT_TOLERANT_DIR_LOG *dir_log;

    /* Get fault tolerant data. */
    logs_remaining = media_ptr -> fx_media_fault_tolerant_total_logs;

    /* Any redo logs? */
    if (logs_remaining == 0)
    {

        /* No. Just return. */
        return(FX_SUCCESS);
    }

    /* Get size of all logs. */
    size = media_ptr -> fx_media_fault_tolerant_file_size - FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET;

    /* Get the first log pointer. */
    current_ptr = media_ptr -> fx_media_fault_tolerant_memory_buffer +
                  FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET +
                  FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;

    /* Loop throught all FAT logs. */
    while (logs_remaining)
    {

        /* Check log type. */
        type = (USHORT)_fx_utility_16_unsigned_read(current_ptr);

        /* Get log length. */
        log_len = _fx_utility_16_unsigned_read(current_ptr + 2);

        /* Validate the size value. */
        if (log_len > size)
        {

            /* Log file is corrupted.  Nothing can be done.  Return.*/
            return(FX_FILE_CORRUPT);
        }
        size -= log_len;

        /* Check log type. */
        if (type == FX_FAULT_TOLERANT_DIR_LOG_TYPE)
        {

            /* Get the log pointer. */
            dir_log = (FX_FAULT_TOLERANT_DIR_LOG *)current_ptr;

            /* Get the sector of log. */
            log_sector = _fx_utility_64_unsigned_read((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_sector);

            /* Is this log sector in the range of sectors to read? */
            if ((log_sector >= logical_sector) && (log_sector < logical_sector + sectors))
            {

                /* Yes. Update the content in this sector. */
                current_buffer_ptr = ((UCHAR *)buffer_ptr) + (log_sector - logical_sector) *
                    media_ptr -> fx_media_bytes_per_sector;

                /* Set copy information. */
                copy_offset = _fx_utility_32_unsigned_read((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_offset);

                copy_size = log_len - FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE;

                if ((copy_offset + copy_size) > media_ptr -> fx_media_bytes_per_sector)
                {
                    return(FX_FILE_CORRUPT);
                }

                /* Copy data into destination sector. */
                memcpy(current_buffer_ptr + copy_offset,  /* Use case of memcpy is verified. */
                       current_ptr + FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE, copy_size);
            }
        }

        /* Decrease the logs_remaining counter. */
        logs_remaining--;

        /* Move to start position of next log entry. */
        current_ptr += log_len;
    }

    /* Return success. */
    return(FX_SUCCESS);
}

#endif /* FX_ENABLE_FAULT_TOLERANT */

