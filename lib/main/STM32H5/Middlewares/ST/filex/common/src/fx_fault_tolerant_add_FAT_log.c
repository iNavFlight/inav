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
/*    _fx_fault_tolerant_add_FAT_log                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts FAT write into log entry and add it into     */
/*    log file.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*    value                                 Next cluster value            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_write         Write a USHORT from memory    */
/*    _fx_utility_32_unsigned_write         Write a UINT from memory      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_utility_FAT_entry_write                                         */
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
UINT _fx_fault_tolerant_add_FAT_log(FX_MEDIA *media_ptr, ULONG cluster, ULONG value)
{
ULONG                      file_size;
FX_FAULT_TOLERANT_FAT_LOG *fat_log;

    /* Increment the size of the log file. */
    file_size = media_ptr -> fx_media_fault_tolerant_file_size + FX_FAULT_TOLERANT_FAT_LOG_ENTRY_SIZE;

    /* Check whether log file exceeds the buffer. */
    if (file_size > media_ptr -> fx_media_fault_tolerant_memory_buffer_size)
    {

        /*  Log file exceeds the size of the log buffer.  This is a failure. */
        return(FX_NO_MORE_SPACE);
    }

    /* Set log pointer. */
    fat_log = (FX_FAULT_TOLERANT_FAT_LOG *)(media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                            media_ptr -> fx_media_fault_tolerant_file_size);

    /* Set log type. */
    _fx_utility_16_unsigned_write((UCHAR *)&fat_log -> fx_fault_tolerant_FAT_log_type,
                                  FX_FAULT_TOLERANT_FAT_LOG_TYPE);

    /* Size of log. */
    _fx_utility_16_unsigned_write((UCHAR *)&fat_log -> fx_fault_tolerant_FAT_log_size,
                                  FX_FAULT_TOLERANT_FAT_LOG_ENTRY_SIZE);

    /* Set cluster and value. */
    _fx_utility_32_unsigned_write((UCHAR *)&fat_log -> fx_fault_tolerant_FAT_log_cluster, cluster);
    _fx_utility_32_unsigned_write((UCHAR *)&fat_log -> fx_fault_tolerant_FAT_log_value, value);

    /* Update log information. */
    media_ptr -> fx_media_fault_tolerant_file_size = (USHORT)file_size;
    media_ptr -> fx_media_fault_tolerant_total_logs += 1;

    return(FX_SUCCESS);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

