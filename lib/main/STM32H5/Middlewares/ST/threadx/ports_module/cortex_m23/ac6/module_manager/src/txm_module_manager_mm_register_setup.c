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
/*    _txm_module_manager_mm_register_setup              Cortex-M23       */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up the Cortex-M23 MPU register definitions based */
/*    on the module's memory characteristics.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Pointer to module instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    MPU settings for the module in module_instance                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    none                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _txm_module_manager_thread_create                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-02-2021      Scott Larson            Initial Version 6.1.6         */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_mm_register_setup(TXM_MODULE_INSTANCE *module_instance)
{

ULONG   data_size;
ULONG   start_stop_stack_size;
ULONG   callback_stack_size;

    /* Setup MPU region for kernel mode entry.  */
    /* Set base address register to user mode entry function address, which is guaranteed to be at least 32-byte aligned.
       Mask address to proper range, inner shareable, read only. */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MPU_KERNEL_ENTRY_INDEX].txm_module_mpu_region_base_address = ((ULONG) _txm_module_manager_user_mode_entry & 0xFFFFFFE0) | TXM_MODULE_ATTRIBUTE_INNER_SHAREABLE | TXM_MODULE_ATTRIBUTE_READ_ONLY;
    /* Set the limit address, attribute index, and enable bit.  */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MPU_KERNEL_ENTRY_INDEX].txm_module_mpu_region_limit_address = ((ULONG) _txm_module_manager_user_mode_entry & 0xFFFFFFE0) | TXM_MODULE_ATTRIBUTE_INDEX | TXM_MODULE_ATTRIBUTE_REGION_ENABLE;
    /* End of kernel mode entry setup.  */


    /* Setup MPU region for module code protection.  */
    /* Set base address register to module code address, which should be at least 32-byte aligned.
       Mask address to proper range, inner shareable, read only. */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MPU_CODE_INDEX].txm_module_mpu_region_base_address = ((ULONG) module_instance -> txm_module_instance_code_start & 0xFFFFFFE0) | TXM_MODULE_ATTRIBUTE_INNER_SHAREABLE | TXM_MODULE_ATTRIBUTE_READ_ONLY;
    /* Set the limit address (code start + code size-1), attribute index, and enable bit.  */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MPU_CODE_INDEX].txm_module_mpu_region_limit_address = (((ULONG) module_instance -> txm_module_instance_code_start + module_instance -> txm_module_instance_preamble_ptr -> txm_module_preamble_code_size - 1) & 0xFFFFFFE0) | TXM_MODULE_ATTRIBUTE_INDEX | TXM_MODULE_ATTRIBUTE_REGION_ENABLE;
    /* End of module code protection.  */


    /* Setup MPU region for module data protection.  */
    /* Set base address register to module data address, which should be at least 32-byte aligned.
       Mask address to proper range, inner shareable, read write, execute never. */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MPU_DATA_INDEX].txm_module_mpu_region_base_address = ((ULONG) module_instance -> txm_module_instance_data_start & 0xFFFFFFE0) | TXM_MODULE_ATTRIBUTE_INNER_SHAREABLE | TXM_MODULE_ATTRIBUTE_READ_WRITE | TXM_MODULE_ATTRIBUTE_EXECUTE_NEVER;
    
    /* Adjust the size of the module elements to be aligned to the default alignment. We do this
       so that when we partition the allocated memory, we can simply place these regions right beside
       each other without having to align their pointers. Note this only works when they all have
       the same alignment.  */
    
    data_size =             module_instance -> txm_module_instance_preamble_ptr -> txm_module_preamble_data_size;
    start_stop_stack_size = module_instance -> txm_module_instance_preamble_ptr -> txm_module_preamble_start_stop_stack_size;
    callback_stack_size =   module_instance -> txm_module_instance_preamble_ptr -> txm_module_preamble_callback_stack_size;
    
    data_size =              ((data_size + TXM_MODULE_DATA_ALIGNMENT - 1)/TXM_MODULE_DATA_ALIGNMENT) * TXM_MODULE_DATA_ALIGNMENT;
    start_stop_stack_size =  ((start_stop_stack_size + TXM_MODULE_DATA_ALIGNMENT - 1)/TXM_MODULE_DATA_ALIGNMENT) * TXM_MODULE_DATA_ALIGNMENT;
    callback_stack_size =    ((callback_stack_size + TXM_MODULE_DATA_ALIGNMENT - 1)/TXM_MODULE_DATA_ALIGNMENT) * TXM_MODULE_DATA_ALIGNMENT;

    /* Update the data size to include thread stacks.  */
    data_size = data_size + start_stop_stack_size + callback_stack_size;
    
    /* Set the limit address (data start + data size-1), attribute index, and enable bit.  */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MPU_DATA_INDEX].txm_module_mpu_region_limit_address = (((ULONG) module_instance -> txm_module_instance_data_start + data_size - 1) & 0xFFFFFFE0) | TXM_MODULE_ATTRIBUTE_INDEX | TXM_MODULE_ATTRIBUTE_REGION_ENABLE;
    /* End of module data protection.  */
    
    /* Remaining MPU entries are disabled for now and can be used for shared memory. */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_inside_data_check              Cortex-M23       */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if the specified object is inside shared       */
/*    memory.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Pointer to module instance        */
/*    obj_ptr                           Pointer to the object             */
/*    obj_size                          Size of the object                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Whether the object is inside the shared memory region.              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Module dispatch check functions                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-02-2021      Scott Larson            Initial Version 6.1.6         */
/*                                                                        */
/**************************************************************************/
UINT _txm_module_manager_inside_data_check(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE obj_ptr, UINT obj_size)
{

UINT shared_memory_index;
UINT num_shared_memory_mpu_entries;
ALIGN_TYPE shared_memory_address_start;
ALIGN_TYPE shared_memory_address_end;

    /* Check for overflow. */
    if ((obj_ptr) > ((obj_ptr) + (obj_size)))
    {
        return(TX_FALSE);
    }
    
    /* Check if the object is inside the module data.  */
    if ((obj_ptr >= (ALIGN_TYPE) module_instance -> txm_module_instance_data_start) &&
        ((obj_ptr + obj_size) <= ((ALIGN_TYPE) module_instance -> txm_module_instance_data_end + 1)))
    {
        return(TX_TRUE);
    }

    /* Check if the object is inside the shared memory.  */
    num_shared_memory_mpu_entries = module_instance -> txm_module_instance_shared_memory_count;
    for (shared_memory_index = 0; shared_memory_index < num_shared_memory_mpu_entries; shared_memory_index++)
    {

        shared_memory_address_start = (ALIGN_TYPE) module_instance -> txm_module_instance_shared_memory_address[shared_memory_index];
        shared_memory_address_end = shared_memory_address_start + module_instance -> txm_module_instance_shared_memory_length[shared_memory_index];

        if ((obj_ptr >= (ALIGN_TYPE) shared_memory_address_start) &&
            ((obj_ptr + obj_size) <= (ALIGN_TYPE) shared_memory_address_end))
        {
            return(TX_TRUE);
        }
    }

    return(TX_FALSE);
}
