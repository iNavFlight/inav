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
/*    _nx_ip_auxiliary_packet_pool_set                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function assigns an auxiliary packet pool to IP instance.      */
/*    Note in order to utilize the auxiliary packet pool feature, the     */
/*    symbol NX_ENABLE_DUAL_PACKET_POOL must be defined when building     */
/*    NetX Duo library.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    auxiliary_pool                        Pointer to auxiliary packet   */
/*                                            pool                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT  _nx_ip_auxiliary_packet_pool_set(NX_IP *ip_ptr, NX_PACKET_POOL *auxiliary_pool)
{

#ifdef NX_ENABLE_DUAL_PACKET_POOL
TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Choose a packet pool with smaller payload size as auxiliary packet pool. */
    if (auxiliary_pool -> nx_packet_pool_payload_size > ip_ptr -> nx_ip_default_packet_pool -> nx_packet_pool_payload_size)
    {
        ip_ptr -> nx_ip_default_packet_pool = auxiliary_pool;
    }
    else
    {
        ip_ptr -> nx_ip_auxiliary_packet_pool = auxiliary_pool;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return success to the caller.  */
    return(NX_SUCCESS);

#else /* !NX_ENABLE_DUAL_PACKET_POOL */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(auxiliary_pool);

    return(NX_NOT_SUPPORTED);

#endif /* NX_ENABLE_DUAL_PACKET_POOL */
}

