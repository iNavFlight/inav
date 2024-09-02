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
#include "fx_system.h"


FX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_media_open                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the media open call.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    media_name                            Pointer to media name string  */
/*    media_driver                          Media driver entry function   */
/*    driver_info_ptr                       Optional information pointer  */
/*                                            supplied to media driver    */
/*    memory_ptr                            Pointer to memory used by the */
/*                                            FileX for this media.       */
/*    memory_size                           Size of media memory - must   */
/*                                            at least 512 bytes and      */
/*                                            one sector size.            */
/*    media_control_block_size              Size of FX_MEDIA structure    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_PTR_ERROR                          One or more input parameters  */
/*                                            are NULL                    */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_identify                    Get current thread            */
/*    tx_thread_preemption_change           Disable/restore preemption    */
/*    _fx_media_open                        Actual media open service     */
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
/*                                                                        */
/**************************************************************************/
UINT  _fxe_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                      VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                      VOID *memory_ptr, ULONG memory_size, UINT media_control_block_size)
{

UINT       status;
ULONG      temp;
FX_MEDIA  *current_media;
ULONG      open_count;

#ifndef FX_SINGLE_THREAD
TX_THREAD *current_thread;
UINT       old_threshold;
#endif


    /* Check for invalid input pointers.  */
    if ((media_ptr == FX_NULL) || (media_driver == FX_NULL) || (memory_ptr == FX_NULL) || (media_control_block_size != sizeof(FX_MEDIA)))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Check for proper size of the logical sector cache.  */
    temp =  _fx_system_media_max_sector_cache;

    /* Isolate the lowest set bit.  */
    temp =  (temp & ((~temp) + ((ULONG) 1)));

    /* If FX_MAX_SECTOR_CACHE is a power of 2, the value of temp should be unchanged.  */
    if ((temp == 1) || (temp != _fx_system_media_max_sector_cache))
    {

        /* Not a power of 2, return an error.  */
        return(FX_MEDIA_INVALID);
    }

    /* Check for proper size of the FAT cache.  */
    temp =  _fx_system_media_max_fat_cache;

    /* Isolate the lowest set bit.  */
    temp =  (temp & ((~temp) + ((ULONG) 1)));

    /* If FX_MAX_FAT_CACHE is a power of 2, the value of temp should be unchanged.  */
    if ((temp == 1) || (temp != _fx_system_media_max_fat_cache))
    {

        /* Not a power of 2, return an error.  */
        return(FX_MEDIA_INVALID);
    }

#ifndef FX_SINGLE_THREAD

    /* Pickup current thread pointer. At this point we know the current thread pointer is non-null since 
       it was checked by code in FX_CALLER_CHECKING_CODE macro.  */
    current_thread =  tx_thread_identify();

    /* Disable preemption temporarily.  */
    tx_thread_preemption_change(current_thread, 0, &old_threshold);
#endif

    /* Loop to check for the media already opened.  */
    current_media =  _fx_system_media_opened_ptr;
    open_count =     _fx_system_media_opened_count;
    while (open_count--)
    {

        /* Is the new media pointer already open?  */
        if (media_ptr == current_media)
        {

#ifndef FX_SINGLE_THREAD

            /* Restore preemption.  */
            tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
#endif

            /* Duplicate media open, return an error!  */
            return(FX_PTR_ERROR);
        }

        /* Move to next entry.  */
        current_media =  current_media -> fx_media_opened_next;
    }

#ifndef FX_SINGLE_THREAD

    /* Restore preemption.  */
    tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
#endif

    /* Call actual media open service.  */
    status =  _fx_media_open(media_ptr, media_name, media_driver, driver_info_ptr,
                             memory_ptr, memory_size);

    /* Return status.  */
    return(status);
}

