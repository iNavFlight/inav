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


#if defined(FX_ENABLE_FAULT_TOLERANT) && defined(FX_ENABLE_EXFAT)
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_add_checksum_log                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts checksum of exFAT directory entry write into */
/*    log entries and add it into log file.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Original sector to write to   */
/*    offset                                Offset in original sector     */
/*    checksum                              Value of checksum             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_read          Read a USHORT from memory     */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
/*    _fx_utility_64_unsigned_read          Read a ULONG64 from memory    */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    memcpy                                Memory Copy                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_exFAT_unicode_entry_write                             */
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
UINT _fx_fault_tolerant_add_checksum_log(FX_MEDIA *media_ptr, ULONG64 logical_sector, ULONG offset, USHORT checksum)
{
ULONG                      logs_remaining;
UCHAR                     *current_ptr;
ULONG                      size;
USHORT                     type;
ULONG                      log_len;
ULONG64                    log_sector;
ULONG                      log_offset;
FX_FAULT_TOLERANT_DIR_LOG *dir_log;

    /* Get fault tolerant data. */
    logs_remaining = media_ptr -> fx_media_fault_tolerant_total_logs;

    /* Get size of all logs. */
    size = media_ptr -> fx_media_fault_tolerant_file_size - FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET;

    /* Get the first log pointer. */
    current_ptr = media_ptr -> fx_media_fault_tolerant_memory_buffer +
                  FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET +
                  FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;

    /* Loop throught all DIR logs. */
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

            /* Set the log pointer. */
            dir_log = (FX_FAULT_TOLERANT_DIR_LOG *)current_ptr;

            /* Get the sector of log. */
            log_sector = _fx_utility_64_unsigned_read((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_sector);

            /* Get the offset of log. */
            log_offset = _fx_utility_32_unsigned_read((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_offset);

            /* Is the checksum for this directory log? */
            if ((log_sector == logical_sector) && (offset == log_offset + 2))
            {

                /* Yes. Update the checksum field. */
                _fx_utility_16_unsigned_write(current_ptr + FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE + 2, checksum);

                /* Return success. */
                return(FX_SUCCESS);
            }
        }

        /* Decrease the logs_remaining counter. */
        logs_remaining--;

        /* Move to start position of next log entry. */
        current_ptr += log_len;
    }

    /* The directory is not found. Log file is corrupted. */
    return(FX_FILE_CORRUPT);
}
#endif /* FX_ENABLE_FAULT_TOLERANT && FX_ENABLE_EXFAT */

