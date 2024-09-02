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
#ifndef TXM_BLOCK_POOL_INFO_GET_CALL_NOT_USED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_block_pool_info_get                            PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the block pool information get   */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to block pool control blk */
/*    name                              Destination for the pool name     */
/*    available_blocks                  Number of free blocks in pool     */
/*    total_blocks                      Total number of blocks in pool    */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on block pool  */
/*    suspended_count                   Destination for suspended count   */
/*    next_pool                         Destination for pointer to next   */
/*                                        block pool on the created list  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_POOL_ERROR                     Invalid block pool pointer        */
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
UINT _txe_block_pool_info_get(TX_BLOCK_POOL *pool_ptr, CHAR **name, ULONG *available_blocks, ULONG *total_blocks, TX_THREAD **first_suspended, ULONG *suspended_count, TX_BLOCK_POOL **next_pool)
{

UINT return_value;
ALIGN_TYPE extra_parameters[5];

    extra_parameters[0] = (ALIGN_TYPE) available_blocks;
    extra_parameters[1] = (ALIGN_TYPE) total_blocks;
    extra_parameters[2] = (ALIGN_TYPE) first_suspended;
    extra_parameters[3] = (ALIGN_TYPE) suspended_count;
    extra_parameters[4] = (ALIGN_TYPE) next_pool;

    /* Call module manager dispatcher.  */
    return_value = (UINT) (_txm_module_kernel_call_dispatcher)(TXM_BLOCK_POOL_INFO_GET_CALL, (ALIGN_TYPE) pool_ptr, (ALIGN_TYPE) name, (ALIGN_TYPE) extra_parameters);

    /* Return value to the caller.  */
    return(return_value);
}
#endif
