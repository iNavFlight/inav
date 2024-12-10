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
/*    _fx_fault_tolerant_add_dir_log                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts directory entry write into log entry and add */
/*    it into log file.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    logical_sector                        Original sector to write to   */
/*    offset                                Offset in original sector     */
/*    data                                  Data of log                   */
/*    data_size                             Size of data                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    _fx_utility_32_unsigned_write         Write a ULONG from memory     */
/*    _fx_utility_64_unsigned_write         Write a ULONG64 from memory   */
/*    memcpy                                Memory Copy                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_entry_write                                           */
/*    _fx_directory_exFAT_unicode_entry_write                             */
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
UINT _fx_fault_tolerant_add_dir_log(FX_MEDIA *media_ptr, ULONG64 logical_sector, ULONG offset,
                                    UCHAR *data, ULONG data_size)
{
ULONG                      file_size;
FX_FAULT_TOLERANT_DIR_LOG *dir_log;

    /* Increment the size of the log file. */
    file_size = media_ptr -> fx_media_fault_tolerant_file_size + data_size + FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE;

    /* Check whether log file exceeds the buffer. */
    if (file_size > media_ptr -> fx_media_fault_tolerant_memory_buffer_size)
    {

        /*  Log file exceeds the size of the log buffer.  This is a failure. */
        return(FX_NO_MORE_SPACE);
    }

    /* Any data to write? */
    if (data_size == 0)
    {

        /* No. Just return. */
        return FX_SUCCESS;
    }

    /* Set log pointer. */
    dir_log = (FX_FAULT_TOLERANT_DIR_LOG *)(media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                            media_ptr -> fx_media_fault_tolerant_file_size);

    /* Set log type. */
    _fx_utility_16_unsigned_write((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_type,
                                  FX_FAULT_TOLERANT_DIR_LOG_TYPE);

    /* Size of log. */
    _fx_utility_16_unsigned_write((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_size,
                                  data_size + FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE);

    /* Set sector and offset. */
    _fx_utility_64_unsigned_write((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_sector, logical_sector);
    _fx_utility_32_unsigned_write((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_offset, offset);

    memcpy(media_ptr -> fx_media_fault_tolerant_memory_buffer +  /* Use case of memcpy is verified. */
           media_ptr -> fx_media_fault_tolerant_file_size + FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE,
           data, data_size);


    /* Update log information. */
    media_ptr -> fx_media_fault_tolerant_file_size = (USHORT)file_size;
    media_ptr -> fx_media_fault_tolerant_total_logs += 1;

    return(FX_SUCCESS);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

