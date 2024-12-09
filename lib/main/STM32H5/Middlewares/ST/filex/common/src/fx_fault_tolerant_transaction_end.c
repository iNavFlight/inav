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
#include "fx_fault_tolerant.h"
#include "fx_utility.h"

#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_transaction_end                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function completes a file system update operation protected    */
/*    by fault tolerant.  This function first computes the checksum       */
/*    for the log entries, flush the log file to the file system (a       */
/*    necessary step to record vital information for the file system to   */
/*    recover, in case the update operation fails midway).  After the     */
/*    log entries are written to the physical medium, the actualy file    */
/*    system changes (FAT table, directory entries) are applied.          */
/*                                                                        */
/*    If the file system changes are successfully applied, the log        */
/*    entries can be removed.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    _fx_fault_tolerant_apply_logs         Apply logs into file system   */
/*    _fx_fault_tolerant_calculate_checksum Compute Checksum of data      */
/*    _fx_fault_tolerant_write_log_file     Write log file                */
/*    _fx_fault_tolerant_reset_log_file     Reset the log file            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_attributes_set                                        */
/*    _fx_directory_create                                                */
/*    _fx_directory_delete                                                */
/*    _fx_directory_rename                                                */
/*    _fx_file_allocate                                                   */
/*    _fx_file_attributes_set                                             */
/*    _fx_file_best_effort_allocate                                       */
/*    _fx_file_create                                                     */
/*    _fx_file_delete                                                     */
/*    _fx_file_rename                                                     */
/*    _fx_file_truncate                                                   */
/*    _fx_file_truncate_release                                           */
/*    _fx_file_write                                                      */
/*    _fx_unicode_directory_create                                        */
/*    _fx_unicode_file_create                                             */
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
UINT    _fx_fault_tolerant_transaction_end(FX_MEDIA *media_ptr)
{
ULONG                          relative_sector;
UINT                           status;
USHORT                         checksum;
UINT                           offset;
FX_FAULT_TOLERANT_LOG_HEADER  *log_header;
FX_FAULT_TOLERANT_LOG_CONTENT *log_content;

    /* Is fault tolerant enabled? */
    if (media_ptr -> fx_media_fault_tolerant_enabled == FX_FALSE)
    {
        return(FX_SUCCESS);
    }

    /* Decrease the transaction. */
    media_ptr -> fx_media_fault_tolerant_transaction_count--;

    /* Is transaction finished? */
    if (media_ptr -> fx_media_fault_tolerant_transaction_count != 0)
    {
        return(FX_SUCCESS);
    }

    /* Close this transaction. */
    media_ptr -> fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_IDLE;

    /* Set log header and FAT chain pointer. */
    log_header = (FX_FAULT_TOLERANT_LOG_HEADER *)media_ptr -> fx_media_fault_tolerant_memory_buffer;
    log_content = (FX_FAULT_TOLERANT_LOG_CONTENT *)(media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                                    FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET);

    /* Reset checksum field and update log entry counter field. */
    _fx_utility_16_unsigned_write((UCHAR *)&log_content -> fx_fault_tolerant_log_content_checksum, 0);
    _fx_utility_16_unsigned_write((UCHAR *)&log_content -> fx_fault_tolerant_log_content_count,
                                  media_ptr -> fx_media_fault_tolerant_total_logs);

    /* Now calculate the checksum of log content. */
    checksum = _fx_fault_tolerant_calculate_checksum((media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                                      FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET),
                                                     (media_ptr -> fx_media_fault_tolerant_file_size -
                                                      FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET));

    /* Record checksum field of log content. */
    _fx_utility_16_unsigned_write((UCHAR *)&log_content -> fx_fault_tolerant_log_content_checksum, checksum);

    /* Record log header. */
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_total_size,
                                  media_ptr -> fx_media_fault_tolerant_file_size);
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_checksum, 0);

    /* Calculate checksum of the header. */
    checksum = _fx_fault_tolerant_calculate_checksum((UCHAR *)log_header, FX_FAULT_TOLERANT_LOG_HEADER_SIZE);

    /* Record checksum field of the header. */
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_checksum, checksum);

    /* Flush log content and flush first sector at last. The first sector contains size of total log file.
     * The log file will contain wrong value of size when it is interrupted after first sector flushed to file system. */
    relative_sector = 1;
    offset = media_ptr -> fx_media_bytes_per_sector;
    while (offset < media_ptr -> fx_media_fault_tolerant_file_size)
    {

        /* Write back the log.  */
        status =  _fx_fault_tolerant_write_log_file(media_ptr, relative_sector);

        /* Check for good completion status.  */
        if (status !=  FX_SUCCESS)
        {

            /* Error.  */
            return(status);
        }

        /* Increase relative sector and offset. */
        relative_sector++;
        offset += media_ptr -> fx_media_bytes_per_sector;
    }

    /* Flush first sector.  */
    status =  _fx_fault_tolerant_write_log_file(media_ptr, 0);

    /* Check for good completion status.  */
    if (status !=  FX_SUCCESS)
    {

        /* Error.  */
        return(status);
    }

    /* At this pointer, the vital information has been flushed to the physical medium.
       Update the file system (FAT table, directory entry) using information recorded in the
       log file.  */
    status = _fx_fault_tolerant_apply_logs(media_ptr);

    /* Check for a bad status.  */
    if (status != FX_SUCCESS)
    {

        /* Return the bad status.  */
        return(status);
    }

    /* The file system has been updated successfully.  Remove and reset the fault tolerant
       log file. */
    status = _fx_fault_tolerant_reset_log_file(media_ptr);

    return(status);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

