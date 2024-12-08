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
/*    _nx_rarp_disable                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disables the RARP management component for the        */
/*    specified IP instance.                                              */
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
UINT  _nx_rarp_disable(NX_IP *ip_ptr)
{

#ifndef NX_DISABLE_IPV4
TX_INTERRUPT_SAVE_AREA

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_RARP_DISABLE, ip_ptr, 0, 0, 0, NX_TRACE_RARP_EVENTS, 0, 0);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Check to see if RARP is enabled.  */
    if (!ip_ptr -> nx_ip_rarp_periodic_update)
    {

        /* Error, IP instance already has RARP disabled.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return to caller.  */
        return(NX_NOT_ENABLED);
    }

    /* Clear the RARP periodic update routine.  */
    ip_ptr -> nx_ip_rarp_periodic_update =  NX_NULL;

    /* Clear the RARP queue process routine.  */
    ip_ptr -> nx_ip_rarp_queue_process =  NX_NULL;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

