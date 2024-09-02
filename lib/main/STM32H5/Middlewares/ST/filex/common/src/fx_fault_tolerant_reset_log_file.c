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
#include "fx_directory.h"
#include "fx_fault_tolerant.h"


#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_reset_log_file                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes all log entries from the log file, and        */
/*    it to its initial state.                                            */
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
/*    _fx_utility_32_unsigned_write         Write a ULONG from memory     */
/*    _fx_fault_tolerant_calculate_checksum Compute Checksum of data      */
/*    _fx_fault_tolerant_write_log_file     Write log file                */
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
/*    _fx_fault_tolerant_enable                                           */
/*    _fx_fault_tolerant_transaction_end                                  */
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

UINT _fx_fault_tolerant_reset_log_file(FX_MEDIA *media_ptr)
{
UINT                           status;
USHORT                         checksum;
FX_FAULT_TOLERANT_LOG_HEADER  *log_header;
FX_FAULT_TOLERANT_FAT_CHAIN   *FAT_chain;
FX_FAULT_TOLERANT_LOG_CONTENT *log_content;

    /* Set log header, FAT chain and log content pointer. */
    log_header = (FX_FAULT_TOLERANT_LOG_HEADER *)media_ptr -> fx_media_fault_tolerant_memory_buffer;
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                                FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);
    log_content = (FX_FAULT_TOLERANT_LOG_CONTENT *)(media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                                    FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET);

    /* Reset the log file header. */
    _fx_utility_32_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_id, FX_FAULT_TOLERANT_ID);
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_total_size,
                                  (FX_FAULT_TOLERANT_LOG_HEADER_SIZE + FX_FAULT_TOLERANT_FAT_CHAIN_SIZE));
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_checksum, 0);
    log_header -> fx_fault_tolerant_log_header_version_major = FX_FAULT_TOLERANT_VERSION_MAJOR;
    log_header -> fx_fault_tolerant_log_header_version_minor = FX_FAULT_TOLERANT_VERSION_MINOR;
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_reserved, 0);
    checksum = _fx_fault_tolerant_calculate_checksum((UCHAR *)log_header, FX_FAULT_TOLERANT_LOG_HEADER_SIZE);
    _fx_utility_16_unsigned_write((UCHAR *)&log_header -> fx_fault_tolerant_log_header_checksum, checksum);

    /* Reset the undo log section. */
    _fx_utility_16_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_checksumm, 0xFFFF);
    FAT_chain -> fx_fault_tolerant_FAT_chain_flag = 0;
    FAT_chain -> fx_fault_tolerant_FAT_chain_reserved = 0;
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_insertion_front, 0);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_new, 0);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_original, 0);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_insertion_back, 0);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_next_deletion, 0);

    /* Reset log content header. */
    _fx_utility_16_unsigned_write((UCHAR *)&log_content -> fx_fault_tolerant_log_content_checksum, 0xFFFF);
    _fx_utility_16_unsigned_write((UCHAR *)&log_content -> fx_fault_tolerant_log_content_count, 0xFFFF);

    /* No matter success or fail, close this transaction. */
    media_ptr -> fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_IDLE;

    /* Write the log header and FAT chain.  */
    status =  _fx_fault_tolerant_write_log_file(media_ptr, 0);

    /* Return the status.  */
    return(status);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

