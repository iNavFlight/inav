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
/*    _txm_module_manager_alignment_adjust            Cortex-A7/MMU/IAR   */ 
/*                                                           6.1          */
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
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_alignment_adjust(TXM_MODULE_PREAMBLE *module_preamble, ULONG *code_size, ULONG *code_alignment, ULONG *data_size, ULONG *data_alignment)
{

ULONG   local_code_size;
ULONG   local_code_alignment;
ULONG   local_data_size;
ULONG   local_data_alignment;


    /* Copy the input parameters into local variables for ease of use.  */
    local_code_size =       *code_size;
    local_code_alignment =  TXM_MODULE_MEMORY_ALIGNMENT;
    local_data_size =       *data_size;
    local_data_alignment =  TXM_MODULE_MEMORY_ALIGNMENT;

    /* Return all the information to the caller.  */
    *code_size =        ((local_code_size + TXM_MODULE_MEMORY_ALIGNMENT - 1)/TXM_MODULE_MEMORY_ALIGNMENT) * TXM_MODULE_MEMORY_ALIGNMENT;
    *code_alignment =   local_code_alignment;
    *data_size =        ((local_data_size + TXM_MODULE_MEMORY_ALIGNMENT - 1)/TXM_MODULE_MEMORY_ALIGNMENT) * TXM_MODULE_MEMORY_ALIGNMENT;
    *data_alignment =   local_data_alignment;
}
