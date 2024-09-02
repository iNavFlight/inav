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
/*    _nx_ip_fragment_enable                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the IP fragment processing by setting up the  */
/*    function pointers responsible for fragmenting and unfragmenting IP  */
/*    packets.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
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
/*    Application                                                         */
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
UINT  _nx_ip_fragment_enable(NX_IP *ip_ptr)
{
#ifndef NX_DISABLE_FRAGMENTATION
TX_INTERRUPT_SAVE_AREA

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_FRAGMENT_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Disable interrupts temporarily.  */
    TX_DISABLE

    /* Setup the IP fragment processing routine pointer.  */
    ip_ptr -> nx_ip_fragment_processing =  _nx_ip_fragment_packet;

    /* Setup the IP fragment assembly routine pointer.  */
    ip_ptr -> nx_ip_fragment_assembly =  _nx_ip_fragment_assembly;

    /* Setup the IP fragment timeout routine pointer.  */
    ip_ptr -> nx_ip_fragment_timeout_check =  _nx_ip_fragment_timeout_check;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return success to the caller.  */
    return(NX_SUCCESS);

#else /* NX_DISABLE_FRAGMENTATION */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_ENABLED);

#endif /* NX_DISABLE_FRAGMENTATION */
}

