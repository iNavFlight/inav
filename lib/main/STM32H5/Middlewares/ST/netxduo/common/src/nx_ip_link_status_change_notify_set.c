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
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_link_status_change_notify_set                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures the link status change notify callback     */
/*    function specified by the application.                              */
/*                                                                        */
/*    If a NULL pointer is supplied, the link status change notify        */
/*    callback feature is disabled.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    link_status_change_notify             Routine to call when link     */
/*                                            staus is changed            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ip_link_status_change_notify_set(NX_IP *ip_ptr,  VOID (*link_status_change_notify)(NX_IP *ip_ptr, UINT interface_index, UINT link_up))
{
TX_INTERRUPT_SAVE_AREA

    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup the link status change callback function pointer.  */
    ip_ptr -> nx_ip_link_status_change_callback = link_status_change_notify;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

