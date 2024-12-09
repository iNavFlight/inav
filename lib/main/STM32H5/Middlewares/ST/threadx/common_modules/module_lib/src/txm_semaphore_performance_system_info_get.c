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
#ifndef TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_semaphore_performance_system_info_get           PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves system semaphore performance information.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    puts                              Destination for total number of   */
/*                                        semaphore puts                  */
/*    gets                              Destination for total number of   */
/*                                        semaphore gets                  */
/*    suspensions                       Destination for total number of   */
/*                                        semaphore suspensions           */
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
UINT _tx_semaphore_performance_system_info_get(ULONG *puts, ULONG *gets, ULONG *suspensions, ULONG *timeouts)
{

UINT return_value;
ALIGN_TYPE extra_parameters[2];

    extra_parameters[0] = (ALIGN_TYPE) suspensions;
    extra_parameters[1] = (ALIGN_TYPE) timeouts;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL, (ALIGN_TYPE) puts, (ALIGN_TYPE) gets, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
