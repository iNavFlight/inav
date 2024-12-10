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
/**   NAND Flash                                                          */
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
/*    _lx_nand_flash_close                                PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function closes a NAND flash instance.                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
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
UINT  _lx_nand_flash_close(LX_NAND_FLASH *nand_flash)
{

LX_INTERRUPT_SAVE_AREA


    /* Lockout interrupts for NAND flash close.  */
    LX_DISABLE

    /* See if the media is the only one on the media opened list.  */
    if ((_lx_nand_flash_opened_ptr == nand_flash) &&
        (_lx_nand_flash_opened_ptr == nand_flash -> lx_nand_flash_open_next) &&
        (_lx_nand_flash_opened_ptr == nand_flash -> lx_nand_flash_open_previous))
    {

        /* Only opened NAND flash, just set the opened list to NULL.  */
        _lx_nand_flash_opened_ptr =  LX_NULL;
    }
    else
    {

        /* Otherwise, not the only opened NAND flash, link-up the neighbors.  */
        (nand_flash -> lx_nand_flash_open_next) -> lx_nand_flash_open_previous =
                                            nand_flash -> lx_nand_flash_open_previous;
        (nand_flash -> lx_nand_flash_open_previous) -> lx_nand_flash_open_next =
                                            nand_flash -> lx_nand_flash_open_next;

        /* See if we have to update the opened list head pointer.  */
        if (_lx_nand_flash_opened_ptr == nand_flash)
        {
                    
            /* Yes, move the head pointer to the next opened NAND flash. */
            _lx_nand_flash_opened_ptr =  nand_flash -> lx_nand_flash_open_next; 
        }
    }

    /* Decrement the opened NAND flash counter.  */
    _lx_nand_flash_opened_count--;

    /* Finally, indicate that this NAND flash is closed.  */
    nand_flash -> lx_nand_flash_state =  LX_NAND_FLASH_CLOSED;
    
    /* Restore interrupt posture.  */
    LX_RESTORE

#ifdef LX_THREAD_SAFE_ENABLE

    /* Delete the thread safe mutex.  */
    tx_mutex_delete(&nand_flash -> lx_nand_flash_mutex);
#endif
    /* Return success.  */
    return(LX_SUCCESS);
}


