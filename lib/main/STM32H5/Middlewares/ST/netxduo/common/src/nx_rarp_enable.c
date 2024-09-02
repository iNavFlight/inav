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
/**   Reverse Address Resolution Protocol (RARP)                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_rarp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_rarp_enable                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the RARP management component for the         */
/*    specified IP instance.  For a multi-homed device, RARP is enabled   */
/*    on all attahced interfaces, as long as the interface IP address     */
/*    is not configured yet.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
UINT  _nx_rarp_enable(NX_IP *ip_ptr)
{

#ifndef NX_DISABLE_IPV4
TX_INTERRUPT_SAVE_AREA

UINT i, rarp_enable;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_RARP_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_RARP_EVENTS, 0, 0);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Initialize the outcome to cancelling RARP enable. */
    rarp_enable = NX_FALSE;

    /* Loop through all the interfaces. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Skip the interfaces that are not initialized or is not Ethernet-like.*/
        if ((ip_ptr -> nx_ip_interface[i].nx_interface_valid == 0) ||
            (ip_ptr -> nx_ip_interface[i].nx_interface_address_mapping_needed == 0))
        {
            continue;
        }


        /* Does this interface need a valid IP address? */
        if (ip_ptr -> nx_ip_interface[i].nx_interface_ip_address == 0)
        {

            /* Yes, reset to the flag to enable RARP. */
            rarp_enable = NX_TRUE;
            break;
        }
    }

    if (rarp_enable == NX_FALSE)
    {
        /* Error, all host interfaces already have a valid IP address (non-zero).  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return to caller.  */
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check to see if RARP has been enabled already.  */
    if (ip_ptr -> nx_ip_rarp_periodic_update)
    {

        /* Error, IP instance already has RARP enabled.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return to caller.  */
        return(NX_ALREADY_ENABLED);
    }

    /* Setup the RARP periodic update routine.  */
    ip_ptr -> nx_ip_rarp_periodic_update =  _nx_rarp_periodic_update;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return successful completion.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

