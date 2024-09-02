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
#ifndef TXM_MUTEX_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_mutex_info_get                                 PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the mutex information get        */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*    name                              Destination for the mutex name    */
/*    count                             Destination for the owner count   */
/*    owner                             Destination for the owner's       */
/*                                        thread control block pointer    */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on the mutex   */
/*    suspended_count                   Destination for suspended count   */
/*    next_mutex                        Destination for pointer to next   */
/*                                        mutex on the created list       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_MUTEX_ERROR                    Invalid mutex pointer             */
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
UINT _txe_mutex_info_get(TX_MUTEX *mutex_ptr, CHAR **name, ULONG *count, TX_THREAD **owner, TX_THREAD **first_suspended, ULONG *suspended_count, TX_MUTEX **next_mutex)
{

UINT return_value;
ALIGN_TYPE extra_parameters[5];

    extra_parameters[0] = (ALIGN_TYPE) count;
    extra_parameters[1] = (ALIGN_TYPE) owner;
    extra_parameters[2] = (ALIGN_TYPE) first_suspended;
    extra_parameters[3] = (ALIGN_TYPE) suspended_count;
    extra_parameters[4] = (ALIGN_TYPE) next_mutex;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_MUTEX_INFO_GET_CALL, (ALIGN_TYPE) mutex_ptr, (ALIGN_TYPE) name, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
