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
/*    _fx_fault_tolerant_read_log_file                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the entire log file from file system into the   */
/*    fault tolerant cache.                                               */
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
/*    _fx_utility_logical_sector_read       Read a logical sector         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_fault_tolerant_enable                                           */
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
UINT _fx_fault_tolerant_read_log_file(FX_MEDIA *media_ptr)
{
UINT  status;
ULONG sectors;
ULONG start_sector;

    /* Calculate count of sectors to read. */
    sectors = media_ptr -> fx_media_fault_tolerant_memory_buffer_size / media_ptr -> fx_media_bytes_per_sector;

    /* Calculate the start sector of log file. */
    start_sector = (media_ptr -> fx_media_fault_tolerant_start_cluster - FX_FAT_ENTRY_START) *
                   media_ptr -> fx_media_sectors_per_cluster +
                   media_ptr -> fx_media_data_sector_start;

    status =  _fx_utility_logical_sector_read(media_ptr, (ULONG64) start_sector,
                                              media_ptr -> fx_media_fault_tolerant_memory_buffer,
                                              sectors, FX_DATA_SECTOR);

    /* Return the status.  */
    return(status);
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

