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
#include "tx_mutex.h"
#include "tx_queue.h"
#include "tx_thread.h"
#include "txm_module.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_external_memory_enable          Cortex-M7       */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an entry in the MPU table for a shared        */
/*    memory space.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance pointer           */
/*    start_address                     Start address of memory           */
/*    length                            Length of external memory         */
/*    attributes                        Memory attributes (r/w)           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_mutex_get                     Get protection mutex              */
/*    _tx_mutex_put                     Release protection mutex          */
/*    _txm_power_of_two_block_size      Round length to power of two      */
/*    _txm_module_manager_mm_register_setup    Reconfigure MPU registers  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_external_memory_enable(TXM_MODULE_INSTANCE *module_instance,
                                                 VOID *start_address,
                                                 ULONG length,
                                                 UINT attributes)
{
#ifdef TXM_MODULE_MANAGER_16_MPU
ULONG   block_size;
ULONG   region_size;
ULONG   srd_bits;
ULONG   size_register;
ULONG   address;
ULONG   shared_index;
ULONG   attributes_check = 0;

    /* Determine if the module manager has not been initialized yet.  */
    if (_txm_module_manager_ready != TX_TRUE)
    {
        /* Module manager has not been initialized.  */
        return(TX_NOT_AVAILABLE);
    }

    /* Determine if the module is valid.  */
    if (module_instance == TX_NULL)
    {
        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Get module manager protection mutex.  */
    _tx_mutex_get(&_txm_module_manager_mutex, TX_WAIT_FOREVER);

    /* Determine if the module instance is valid.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }
    
    /* Determine if the module instance is in the loaded state.  */
    if (module_instance -> txm_module_instance_state != TXM_MODULE_LOADED)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_START_ERROR);
    }
    
    /* Determine if there are shared memory entries available.  */
    if(module_instance -> txm_module_instance_shared_memory_count >= TXM_MODULE_MPU_SHARED_ENTRIES)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);
        
        /* No more entries available.  */
        return(TX_NO_MEMORY);
    }
    
    /* Start address and length must adhere to Cortex-M7 MPU.
       The address must align with the block size.  */
    
    block_size = _txm_power_of_two_block_size(length);
    address = (ULONG) start_address;
    if(address != (address & ~(block_size - 1)))
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);
        
        /* Return alignment error.  */
        return(TXM_MODULE_ALIGNMENT_ERROR);
    }
    
    /* At this point, we have a valid address and block size.
       Set up MPU registers.  */
    
    /* Pick up index into shared memory entries.  */
    shared_index = TXM_MODULE_MPU_SHARED_INDEX + module_instance -> txm_module_instance_shared_memory_count;
    
    /* Save address register with address, MPU region, set Valid bit.  */
    module_instance -> txm_module_instance_mpu_registers[shared_index].txm_module_mpu_region_address = address | shared_index | 0x10;
    
    /* Calculate the region size.  */
    region_size = (_txm_module_manager_region_size_get(block_size) << 1);
    
    /* Calculate the subregion bits.  */
    srd_bits = _txm_module_manager_calculate_srd_bits(block_size, length);
    
    /* Generate SRD, size, and enable attributes.  */
    size_register = srd_bits | region_size | TXM_ENABLE_REGION | TXM_MODULE_MPU_SHARED_ACCESS_CONTROL;
    
    /* Check for optional write attribute.  */
    if(attributes & TXM_MODULE_MANAGER_SHARED_ATTRIBUTE_WRITE)
    {
        attributes_check = TXM_MODULE_MANAGER_ATTRIBUTE_WRITE_MPU_BIT;
    }
    
    /* Save attribute-size register. */
    module_instance -> txm_module_instance_mpu_registers[shared_index].txm_module_mpu_region_attribute_size = attributes_check | size_register;
    
    /* Keep track of shared memory address and length in module instance.  */
    module_instance -> txm_module_instance_shared_memory_address[module_instance -> txm_module_instance_shared_memory_count] = address;
    module_instance -> txm_module_instance_shared_memory_length[module_instance -> txm_module_instance_shared_memory_count] = length;
    
    /* Increment counter.  */
    module_instance -> txm_module_instance_shared_memory_count++;
    
    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);
    
    /* Return success.  */
    return(TX_SUCCESS);
    
#else

ULONG   block_size;
ULONG   region_size;
ULONG   subregion_bits;
ULONG   address;
UINT    attributes_check = 0;
TXM_MODULE_PREAMBLE     *module_preamble;

    /* Determine if the module manager has not been initialized yet.  */
    if (_txm_module_manager_ready != TX_TRUE)
    {
        /* Module manager has not been initialized.  */
        return(TX_NOT_AVAILABLE);
    }

    /* Determine if the module is valid.  */
    if (module_instance == TX_NULL)
    {
        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Get module manager protection mutex.  */
    _tx_mutex_get(&_txm_module_manager_mutex, TX_WAIT_FOREVER);

    /* Determine if the module instance is valid.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }
    
    /* Determine if the module instance is in the loaded state.  */
    if (module_instance -> txm_module_instance_state != TXM_MODULE_LOADED)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_START_ERROR);
    }
    
    /* Check if preamble shared mem and mem protection property bits are set.  */
    module_preamble = module_instance -> txm_module_instance_preamble_ptr;
    if((module_preamble -> txm_module_preamble_property_flags & (TXM_MODULE_MEMORY_PROTECTION | TXM_MODULE_SHARED_EXTERNAL_MEMORY_ACCESS))
         != (TXM_MODULE_MEMORY_PROTECTION | TXM_MODULE_SHARED_EXTERNAL_MEMORY_ACCESS))
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if bit not set.  */
        return(TXM_MODULE_INVALID_PROPERTIES);
    }
    
    /* Start address and length must adhere to Cortex-M MPU.
       The address must align with the block size.  */
    
    block_size = _txm_power_of_two_block_size(length);
    address = (ULONG) start_address;
    if(address != (address & ~(block_size - 1)))
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);
        
        /* Return alignment error.  */
        return(TXM_MODULE_ALIGNMENT_ERROR);
    }
    
    /* At this point, we have a valid address and block size.
       Set up MPU registers.  */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MANAGER_SHARED_MPU_INDEX] = address | TXM_MODULE_MANAGER_SHARED_MPU_REGION | 0x10;
    
    /* Calculate the region size.  */
    region_size = (_txm_module_manager_region_size_get(block_size) << 1);
    /* Calculate the subregion bits.  */
    subregion_bits = _txm_module_manager_calculate_srd_bits(block_size, length);
    
    /* Check for valid attributes.  */
    if(attributes & TXM_MODULE_MANAGER_SHARED_ATTRIBUTE_WRITE)
    {
        attributes_check = TXM_MODULE_MANAGER_ATTRIBUTE_WRITE_MPU_BIT;
    }
    
    /* Build register with attributes. */
    module_instance -> txm_module_instance_mpu_registers[TXM_MODULE_MANAGER_SHARED_MPU_INDEX+1] = region_size | subregion_bits | attributes_check | 0x12070001;
    
    /* Keep track of shared memory address and length in module instance.  */
    module_instance -> txm_module_instance_shared_memory_address = address;
    module_instance -> txm_module_instance_shared_memory_length = length;
    
    /* Recalculate MPU settings.  */
    _txm_module_manager_mm_register_setup(module_instance);
    
    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);
    
    /* Return success.  */
    return(TX_SUCCESS);

#endif
}
