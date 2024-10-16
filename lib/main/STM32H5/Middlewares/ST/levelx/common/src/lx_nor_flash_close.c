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
/** LevelX Component                                                      */ 
/**                                                                       */
/**   NOR Flash                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define LX_SOURCE_CODE


/* Disable ThreadX error checking.  */

#ifndef LX_DISABLE_ERROR_CHECKING
#define LX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include "lx_api.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _lx_nor_flash_close                                 PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function closes a NOR flash instance.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_delete                       Delete thread-safe mutex      */ 
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nor_flash_close(LX_NOR_FLASH *nor_flash)
{

LX_INTERRUPT_SAVE_AREA


    /* Lockout interrupts for NOR flash close.  */
    LX_DISABLE

    /* See if the media is the only one on the media opened list.  */
    if ((_lx_nor_flash_opened_ptr == nor_flash) &&
        (_lx_nor_flash_opened_ptr == nor_flash -> lx_nor_flash_open_next) &&
        (_lx_nor_flash_opened_ptr == nor_flash -> lx_nor_flash_open_previous))
    {

        /* Only opened NOR flash, just set the opened list to NULL.  */
        _lx_nor_flash_opened_ptr =  LX_NULL;
    }
    else
    {

        /* Otherwise, not the only opened NOR flash, link-up the neighbors.  */
        (nor_flash -> lx_nor_flash_open_next) -> lx_nor_flash_open_previous =
                                            nor_flash -> lx_nor_flash_open_previous;
        (nor_flash -> lx_nor_flash_open_previous) -> lx_nor_flash_open_next =
                                            nor_flash -> lx_nor_flash_open_next;

        /* See if we have to update the opened list head pointer.  */
        if (_lx_nor_flash_opened_ptr == nor_flash)
        {
                    
            /* Yes, move the head pointer to the next opened NOR flash. */
            _lx_nor_flash_opened_ptr =  nor_flash -> lx_nor_flash_open_next; 
        }
    }

    /* Decrement the opened NOR flash counter.  */
    _lx_nor_flash_opened_count--;

    /* Finally, indicate that this NOR flash is closed.  */
    nor_flash -> lx_nor_flash_state =  LX_NOR_FLASH_CLOSED;
    
    /* Restore interrupt posture.  */
    LX_RESTORE

#ifdef LX_THREAD_SAFE_ENABLE

    /* Delete the thread safe mutex.  */
    tx_mutex_delete(&nor_flash -> lx_nor_flash_mutex);
#endif
    /* Return success.  */
    return(LX_SUCCESS);
}


