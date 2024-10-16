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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Module                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TXM_MODULE
#include "txm_module.h"
#ifndef TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_block_pool_performance_system_info_get          PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves block pool performance information.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    allocates                         Destination for the total number  */
/*                                        of block allocations            */
/*    releases                          Destination for the total number  */
/*                                        of blocks released              */
/*    suspensions                       Destination for the total number  */
/*                                        of suspensions                  */
/*    timeouts                          Destination for total number of   */
/*                                        timeouts                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_module_kernel_call_dispatcher                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Module application code                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  01-31-2022      Scott Larson            Modified comments and added   */
/*                                            CALL_NOT_USED option,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _tx_block_pool_performance_system_info_get(ULONG *allocates, ULONG *releases, ULONG *suspensions, ULONG *timeouts)
{

UINT return_value;
ALIGN_TYPE extra_parameters[2];

    extra_parameters[0] = (ALIGN_TYPE) suspensions;
    extra_parameters[1] = (ALIGN_TYPE) timeouts;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL, (ALIGN_TYPE) allocates, (ALIGN_TYPE) releases, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
