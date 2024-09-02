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
/*    _fx_fault_tolerant_write_log_file                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes data of one sector of fault tolerant data      */
/*    from memory to log file in file system.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    relative_sector                       Relative sector number of the */
/*                                          log file to write to          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_logical_sector_write      Write a logical sector        */
/*    _fx_utility_logical_sector_flush      Flush written logical sectors */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_fault_tolerant_cleanup_FAT_chain                                */
/*    _fx_fault_tolerant_reset_log_file                                   */
/*    _fx_fault_tolerant_set_FAT_chain                                    */
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
UINT _fx_fault_tolerant_write_log_file(FX_MEDIA *media_ptr, ULONG relative_sector)
{
UINT  status;
ULONG start_sector;

    /* Calculate the start sector of log file. */
    start_sector = (media_ptr -> fx_media_fault_tolerant_start_cluster - FX_FAT_ENTRY_START) *
                   media_ptr -> fx_media_sectors_per_cluster +
                   media_ptr -> fx_media_data_sector_start;


    /* Write sector directly. */
    status =  _fx_utility_logical_sector_write(media_ptr, (ULONG64) (start_sector + relative_sector),
                                               media_ptr -> fx_media_fault_tolerant_memory_buffer +
                                               media_ptr -> fx_media_bytes_per_sector * relative_sector,
                                               ((ULONG) 1), FX_DATA_SECTOR);

    /* Check for a bad status.  */
    if (status != FX_SUCCESS)
    {

        /* Return the bad status.  */
        return(status);
    }

    /* Flush the internal logical sector cache.  */
    status =  _fx_utility_logical_sector_flush(media_ptr, (ULONG64) start_sector, ((ULONG64) 1), FX_FALSE);

    FX_FAULT_TOLERANT_WRITE_LOG_FILE_EXTENSION

    /* Return the status.  */
    return(status);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

