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
/*    _txm_module_manager_external_memory_enable         Cortex-M33       */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an entry in the MPU table for a shared        */
/*    memory space. The start_address must be 32-byte aligned.            */
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
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021      Scott Larson            Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_external_memory_enable(TXM_MODULE_INSTANCE *module_instance,
                                                 VOID *start_address,
                                                 ULONG length,
                                                 UINT attributes)
{

ULONG   address;
ULONG   shared_index;

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
    
    /* Start address must adhere to Cortex-M33 MPU alignment.  */
    address = (ULONG) start_address;
    if(address != (address & ~(TXM_MODULE_MPU_ALIGNMENT - 1)))
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);
        
        /* Return alignment error.  */
        return(TXM_MODULE_ALIGNMENT_ERROR);
    }
    
    /* At this point, we have a valid address. Set up MPU registers.  */
    
    /* Pick up index into shared memory entries.  */
    shared_index = TXM_MODULE_MPU_SHARED_INDEX + module_instance -> txm_module_instance_shared_memory_count;
    
    /* Set base address register with start address, sanitized attributes and execute never.  */
    module_instance -> txm_module_instance_mpu_registers[shared_index].txm_module_mpu_region_base_address = address | (attributes & TXM_MODULE_ATTRIBUTE_MASK) | TXM_MODULE_ATTRIBUTE_EXECUTE_NEVER;
    
    /* Set the limit address (data start + length-1), attribute index, and enable bit.  */
    module_instance -> txm_module_instance_mpu_registers[shared_index].txm_module_mpu_region_limit_address = (address + length-1) | TXM_MODULE_ATTRIBUTE_INDEX | TXM_MODULE_ATTRIBUTE_REGION_ENABLE;
    
    /* Keep track of shared memory address and length in module instance.  */
    module_instance -> txm_module_instance_shared_memory_address[module_instance -> txm_module_instance_shared_memory_count] = address;
    module_instance -> txm_module_instance_shared_memory_length[module_instance -> txm_module_instance_shared_memory_count] = length;
    
    /* Increment counter.  */
    module_instance -> txm_module_instance_shared_memory_count++;
    
    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);
    
    /* Return success.  */
    return(TX_SUCCESS);
}
