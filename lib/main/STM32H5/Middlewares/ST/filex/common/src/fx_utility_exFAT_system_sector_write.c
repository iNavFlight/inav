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
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE

#include "fx_api.h"
#include "fx_utility.h"


#ifdef FX_ENABLE_EXFAT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_system_sector_write               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes a sector for exFAT format.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media control block*/
/*    data_buffer                           Pointer to sector data buffer */
/*    logical_sector                        Sector number to write        */
/*    sector_count                          Number of sectors to write    */
/*    sector_type                           Sector type of the sector to  */
/*                                            be written                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Media driver                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_media_exFAT_format                                              */
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
UINT  _fx_utility_exFAT_system_sector_write(FX_MEDIA *media_ptr, UCHAR *data_buffer,
                                            ULONG64 logical_sector, ULONG sector_count,
                                            ULONG sector_type)
{


    /* Build sector write command.  */
#ifdef FX_DRIVER_USE_64BIT_LBA
    media_ptr -> fx_media_driver_logical_sector =   logical_sector;
#else
    media_ptr -> fx_media_driver_logical_sector =   (ULONG)logical_sector;
#endif
    media_ptr -> fx_media_driver_request =          FX_DRIVER_WRITE;
    media_ptr -> fx_media_driver_sectors =          sector_count;
    media_ptr -> fx_media_driver_system_write =     FX_TRUE;
    media_ptr -> fx_media_driver_sector_type =      sector_type;
    media_ptr -> fx_media_driver_buffer =           data_buffer;
    media_ptr -> fx_media_driver_status =           FX_IO_ERROR;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_INTERNAL_IO_DRIVER_WRITE, media_ptr, media_ptr -> fx_media_driver_logical_sector, 1, sector_type, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Write out the sector.  */
    (media_ptr -> fx_media_driver_entry)(media_ptr);

    /* Return status.  */
    return(media_ptr -> fx_media_driver_status);
}

#endif

