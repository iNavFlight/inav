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
/**   Media                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_media.h"


FX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_media_format                                   PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the media format call.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media control block*/
/*                                            (does not need to be opened)*/
/*    driver                                Pointer to FileX driver (must */
/*                                            be able to field requests   */
/*                                            prior to opening)           */
/*    driver_info_ptr                       Optional information pointer  */
/*    memory_ptr                            Pointer to memory used by the */
/*                                            FileX for this media.       */
/*    memory_size                           Size of media memory - must   */
/*                                            at least 512 bytes and      */
/*                                            one sector size.            */
/*    volume_name                           Name of the volume            */
/*    number_of_fats                        Number of FAT tables          */
/*    directory_entries                     Number of directory entries   */
/*    hidden_sectors                        Number of hidden sectors      */
/*    total_sectors                         Total number of sectors       */
/*    bytes_per_sector                      Number of bytes per sector    */
/*    sectors_per_cluster                   Number of sectors per cluster */
/*    heads                                 Number of heads               */
/*    sectors_per_track                     Number of sectors per track   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_media_format                      Actual media format service   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  03-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _fxe_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                        CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors,
                        ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster,
                        UINT heads, UINT sectors_per_track)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((media_ptr == FX_NULL) || (driver == FX_NULL) || (memory_ptr == FX_NULL))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Call actual media format service.  */
    status =  _fx_media_format(media_ptr, driver, driver_info_ptr, memory_ptr, memory_size,
                               volume_name, number_of_fats, directory_entries, hidden_sectors,
                               total_sectors, bytes_per_sector, sectors_per_cluster,
                               heads, sectors_per_track);

    /* Return completion status.  */
    return(status);
}

