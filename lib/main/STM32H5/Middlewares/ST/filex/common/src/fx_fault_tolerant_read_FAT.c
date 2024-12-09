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
/*    _fx_fault_tolerant_read_FAT                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads value of a FAT entry from log file if it        */
/*    exists.  During file or directory operations with Fault Tolerant    */
/*    protection, intermediate operations are written to the fault        */
/*    tolerant log files.  Therefore, FAT-entry read should search for    */
/*    fault tolerant log before the request can be passed to normal FAT   */
/*    entry read routine.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*    value                                 Pointer to value for the entry*/
/*    log_type                              FAT or bitmap                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_16_unsigned_read          Read a USHORT from memory     */
/*    _fx_utility_32_unsigned_read          Read a ULONG from memory      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_utility_exFAT_cluster_state_get                                 */
/*    _fx_utility_FAT_entry_read                                          */
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
UINT    _fx_fault_tolerant_read_FAT(FX_MEDIA *media_ptr, ULONG cluster, ULONG *value, ULONG log_type)
{
ULONG                         logs_remaining;
UCHAR                        *current_ptr;
UCHAR                         found = FX_FALSE;
ULONG                         size;
USHORT                        type;
ULONG                         log_len;
FX_FAULT_TOLERANT_FAT_LOG    *fat_log;
#ifdef FX_ENABLE_EXFAT
FX_FAULT_TOLERANT_BITMAP_LOG *bitmap_log;
#endif


    /* Get fault tolerant data. */
    logs_remaining = media_ptr -> fx_media_fault_tolerant_total_logs;

    /* Any redo logs? */
    if (logs_remaining == 0)
    {

        /* No. Just return. */
        return(FX_READ_CONTINUE);
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
        if (type == log_type)
        {

            /* This is the log with same type . */
            /* Get the log pointer. */
#ifdef FX_ENABLE_EXFAT
            if (type == FX_FAULT_TOLERANT_BITMAP_LOG_TYPE)
            {
                bitmap_log = (FX_FAULT_TOLERANT_BITMAP_LOG *)current_ptr;

                /* Is this bitmap log entry the one looking for? */
                if (_fx_utility_32_unsigned_read((UCHAR *)&bitmap_log -> fx_fault_tolerant_bitmap_log_cluster) == cluster)
                {

                    /* Yes, it is. */
                    *value = _fx_utility_32_unsigned_read((UCHAR *)&bitmap_log -> fx_fault_tolerant_bitmap_log_value);

                    /* Do not return since there may be more than one log for this cluster. */
                    found = FX_TRUE;
                }
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */
                fat_log = (FX_FAULT_TOLERANT_FAT_LOG *)current_ptr;

                /* Is this FAT log entry the one looking for? */
                if (_fx_utility_32_unsigned_read((UCHAR *)&fat_log -> fx_fault_tolerant_FAT_log_cluster) == cluster)
                {

                    /* Yes, it is. */
                    *value = _fx_utility_32_unsigned_read((UCHAR *)&fat_log -> fx_fault_tolerant_FAT_log_value);

                    /* Do not return since there may be more than one log for this cluster. */
                    found = FX_TRUE;
                }
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */
        }

        /* Decrease the logs_remaining counter. */
        logs_remaining--;

        /* Get next log pointer. */
        current_ptr += log_len;
    }

    if (found != FX_TRUE)
    {

        /* Pass the request to FAT entry read. */
        return(FX_READ_CONTINUE);
    }

    /* Return success. */
    return(FX_SUCCESS);
}

#endif /* FX_ENABLE_FAULT_TOLERANT */

