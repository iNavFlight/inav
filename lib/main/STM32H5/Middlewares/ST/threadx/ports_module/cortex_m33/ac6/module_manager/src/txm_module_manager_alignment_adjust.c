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
/**   Module Manager                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE

#include "tx_api.h"
#include "txm_module.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_alignment_adjust               Cortex-M33       */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adjusts the alignment and size of the code and data   */
/*    section for a given module implementation.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_preamble                   Pointer to module preamble        */
/*    code_size                         Size of the code area (updated)   */
/*    code_alignment                    Code area alignment (updated)     */
/*    data_size                         Size of data area (updated)       */
/*    data_alignment                    Data area alignment (updated)     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Initial thread stack frame                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021      Scott Larson            Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_alignment_adjust(TXM_MODULE_PREAMBLE *module_preamble,
                                           ULONG *code_size,
                                           ULONG *code_alignment,
                                           ULONG *data_size,
                                           ULONG *data_alignment)
{

    /* Round code and data size UP to TXM_MODULE_MPU_ALIGNMENT bytes. */
    *code_size = (*code_size + TXM_MODULE_MPU_ALIGNMENT - 1) & ~(TXM_MODULE_MPU_ALIGNMENT - 1);
    *data_size = (*data_size + TXM_MODULE_MPU_ALIGNMENT - 1) & ~(TXM_MODULE_MPU_ALIGNMENT - 1);
    
    /* Alignment for code and data is TXM_MODULE_MPU_ALIGNMENT bytes. */
    *code_alignment =   TXM_MODULE_MPU_ALIGNMENT;
    *data_alignment =   TXM_MODULE_MPU_ALIGNMENT;
}
