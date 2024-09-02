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
#include "tx_initialize.h"
#include "tx_mutex.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"
#include "txm_module.h"
#include "txm_module_manager_util.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_absolute_load                   PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function ensures the code-related and data-related parts of    */
/*    the module preamble are valid and prepares the module for           */
/*    execution.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance pointer           */
/*    module_name                       Module name pointer               */
/*    module_location                   Module code location              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_byte_allocate                 Allocate data area                */
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
/*  12-31-2020     Andres Mlinar            Initial Version 6.1.3         */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_absolute_load(TXM_MODULE_INSTANCE *module_instance, CHAR *module_name, VOID *module_location)
{

TX_INTERRUPT_SAVE_AREA

TXM_MODULE_PREAMBLE     *module_preamble;
ULONG                   code_size;
ULONG                   code_alignment;
ULONG                   code_allocation_size_ignored;
UINT                    status;

TXM_MODULE_INSTANCE     *next_module, *previous_module;
ULONG                   start_stop_stack_size;
ULONG                   callback_stack_size;
ULONG                   code_size_ignored;
ULONG                   code_alignment_ignored;
ALIGN_TYPE              data_start;
ULONG                   data_size;
ULONG                   data_alignment;
ULONG                   data_allocation_size;
ULONG                   module_properties;
CHAR                    *memory_ptr;


    /* Check for interrupt call.  */
    if (TX_THREAD_GET_SYSTEM_STATE() != 0)
    {

        /* Now, make sure the call is from an interrupt and not initialization.  */
        if (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)
        {

            /* Invalid caller of this function, return appropriate error code.  */
            return(TX_CALLER_ERROR);
        }
    }

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

    /* Determine if the module is already valid.  */
    if (module_instance -> txm_module_instance_id == TXM_MODULE_ID)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Module already loaded.  */
        return(TXM_MODULE_ALREADY_LOADED);
    }

    /* Pickup the module's information.  */
    module_preamble = (TXM_MODULE_PREAMBLE *) module_location;

    /* Check to make sure there is a valid module to load.  */
    if (module_preamble -> txm_module_preamble_id != TXM_MODULE_ID)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module preamble.  */
        return(TXM_MODULE_INVALID);
    }

    /* Check the properties of this module.  */
    module_properties =  module_preamble -> txm_module_preamble_property_flags & TXM_MODULE_OPTIONS_MASK;
    if (/* Ensure the requested properties are supported.  */
        ((module_properties & _txm_module_manager_properties_supported) != module_properties) ||
        /* Ensure the required properties are there.  */
        ((_txm_module_manager_properties_required & module_properties) != _txm_module_manager_properties_required) ||
        /* If memory protection is enabled, then so must user mode.  */
        ((module_properties & TXM_MODULE_MEMORY_PROTECTION) && !(module_properties & TXM_MODULE_USER_MODE))
        )
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid properties. Return error.  */
        return(TXM_MODULE_INVALID_PROPERTIES);
    }

    /* Check for valid module entry offsets.  */
    if ((module_preamble -> txm_module_preamble_shell_entry_function == 0) ||
        (module_preamble -> txm_module_preamble_start_function == 0))
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module preamble.  */
        return(TXM_MODULE_INVALID);
    }

    /* Check for valid sizes.  */
    if ((module_preamble -> txm_module_preamble_code_size == 0) ||
     /* (module_preamble -> txm_module_preamble_data_size == 0) || *** a zero data size is valid */
        (module_preamble -> txm_module_preamble_start_stop_stack_size == 0) ||
        (module_preamble -> txm_module_preamble_callback_stack_size == 0))
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module preamble.  */
        return(TXM_MODULE_INVALID);
    }

    /* Get the amount of the bytes we need to allocate for the module's code
       as well as the required alignment. Note that because this is an absolute
       load, we only want the code alignment so we can check it.  */
    status =  _txm_module_manager_util_code_allocation_size_and_alignment_get(module_preamble, &code_alignment, &code_allocation_size_ignored);
    if (status != TX_SUCCESS)
    {

        /* Math overflow error occurred.  */
        return(status);
    }

    /* Since this is an absolute load, check the alignment of the module's instruction area (code).  */
    TXM_MODULE_MANAGER_CHECK_CODE_ALIGNMENT(module_location, code_alignment)

    /* Initialize module control block to all zeros.  */
    TX_MEMSET(module_instance, 0, sizeof(TXM_MODULE_INSTANCE));

    /* Pickup the basic module sizes.  */
    code_size =              module_preamble -> txm_module_preamble_code_size;
    data_size =              module_preamble -> txm_module_preamble_data_size;
    start_stop_stack_size =  module_preamble -> txm_module_preamble_start_stop_stack_size;
    callback_stack_size =    module_preamble -> txm_module_preamble_callback_stack_size;

    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(data_size, TXM_MODULE_DATA_ALIGNMENT, data_size);
    data_size =              ((data_size - 1)/TXM_MODULE_DATA_ALIGNMENT) * TXM_MODULE_DATA_ALIGNMENT;

    /* Adjust the size of the module elements to be aligned to the default alignment. We do this
       so that when we partition the allocated memory, we can simply place these regions right beside
       each other without having to align their pointers. Note this only works when they all have
       the same alignment.  */

    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(start_stop_stack_size, TXM_MODULE_DATA_ALIGNMENT, start_stop_stack_size);
    start_stop_stack_size =  ((start_stop_stack_size - 1)/TXM_MODULE_DATA_ALIGNMENT) * TXM_MODULE_DATA_ALIGNMENT;

    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(callback_stack_size, TXM_MODULE_DATA_ALIGNMENT, callback_stack_size);
    callback_stack_size =    ((callback_stack_size - 1)/TXM_MODULE_DATA_ALIGNMENT) * TXM_MODULE_DATA_ALIGNMENT;

    /* Update the data size to account for the default thread stacks.  */
    data_allocation_size = 0;
    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(data_allocation_size, start_stop_stack_size, data_allocation_size);
    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(data_allocation_size, callback_stack_size, data_allocation_size);

    /* Setup the default code and data alignments.  */
    data_alignment =  (ULONG) TXM_MODULE_DATA_ALIGNMENT;

    /* Get the port-specific alignment for the data size. Note we only want data
       so we pass values of 1 for code (to avoid any possible div by 0 errors).  */
    code_size_ignored = 1;
    code_alignment_ignored = 1;
    TXM_MODULE_MANAGER_ALIGNMENT_ADJUST(module_preamble, code_size_ignored, code_alignment_ignored, data_allocation_size, data_alignment)

    /* Calculate the module's total RAM memory requirement. This entire area is allocated from the module
       manager's byte pool. The general layout is defined as follows:

    Lowest Address:         Start of start/stop thread stack
                            ... [note: thread entry info is embedded near end of stack areas]
                            End of start/stop thread stack

                            Start of callback thread stack
                            ... [note: thread entry info is embedded near end of stack areas]
                            End of callback thread stack

    Highest Address:    */

    /* Add an extra alignment increment so we can align the pointer after allocation.  */
    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(data_allocation_size, data_alignment, data_allocation_size);

    /* Allocate memory for the module.  */
    status =  _tx_byte_allocate(&_txm_module_manager_byte_pool, (VOID **) &memory_ptr, data_allocation_size, TX_NO_WAIT);

    /* Determine if the module memory allocation was successful.  */
    if (status)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* No memory, return an error.  */
        return(TX_NO_MEMORY);
    }

    /* Clear the allocated memory.  */
    TX_MEMSET(memory_ptr, ((UCHAR) 0), data_allocation_size);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup the module instance structure.  */
    module_instance -> txm_module_instance_id = TXM_MODULE_ID;

    /* Save the module name.  */
    module_instance -> txm_module_instance_name =  module_name;

    /* Save the module properties.  */
    module_instance -> txm_module_instance_property_flags =  module_preamble -> txm_module_preamble_property_flags;

    /* Set the module data memory allocation. This is the address released
       when the module is unloaded.  */
    module_instance -> txm_module_instance_data_allocation_ptr =  (VOID *) memory_ptr;

    /* Save the data allocation size.  */
    module_instance -> txm_module_instance_data_allocation_size =   data_allocation_size;

    /* Calculate the actual start of the data area. This needs to be adjusted based on the alignment.  */
    data_start =  (ALIGN_TYPE) memory_ptr;
    data_start =  (data_start + (((ALIGN_TYPE)data_alignment) - 1)) & ~(((ALIGN_TYPE)data_alignment) - 1);
    memory_ptr =  (CHAR *) data_start;

    /* Compute the beginning and end of the data area. */
#ifdef ALIGN_TYPE_DEFINED
    module_instance -> txm_module_instance_data_start =  (VOID *) (((CHAR *) (ALIGN_TYPE) module_preamble->txm_module_preamble_code_size) + sizeof(TXM_MODULE_PREAMBLE));
    module_instance -> txm_module_instance_data_end =  (VOID *) (((CHAR *) (ALIGN_TYPE) module_instance -> txm_module_instance_data_start) + module_preamble->txm_module_preamble_data_size);
#else
    module_instance -> txm_module_instance_data_start =  (VOID *) (((CHAR *) module_preamble->txm_module_preamble_code_size) + sizeof(TXM_MODULE_PREAMBLE));
    module_instance -> txm_module_instance_data_end =  (VOID *) (((CHAR *) module_instance -> txm_module_instance_data_start) + module_preamble->txm_module_preamble_data_size);
#endif

    /* Save the size of the data area.  */
    module_instance -> txm_module_instance_data_size =  data_size;

    /* Set the module code memory allocation. This is the address released
       when the module is unloaded.  */
    module_instance -> txm_module_instance_code_allocation_ptr =  (VOID *) NULL;

    /* Save the code allocation size.  */
    module_instance -> txm_module_instance_code_allocation_size =   0;

    /* Setup the code pointers.  Since the code was loaded in-place, this is effectively just the values supplied in the API call.  */
    module_instance -> txm_module_instance_code_start =     (VOID *) module_location;
    module_instance -> txm_module_instance_code_end =       (VOID *) (((CHAR *) module_location) + (code_size - 1));

    /* Setup the code size.  */
    module_instance -> txm_module_instance_code_size =      code_size;

    /* Save the module's total memory usage.  */
    module_instance -> txm_module_instance_total_ram_usage =  0 /* Code is executed in place */ + data_allocation_size /* just the size of stacks */;

    /* Set the module state to started.  */
    module_instance -> txm_module_instance_state =  TXM_MODULE_LOADED;

    /* Save the preamble pointer.  */
    module_instance -> txm_module_instance_preamble_ptr =  module_preamble;

    /* Save the module application ID in the module instance.  */
    module_instance -> txm_module_instance_application_module_id =  module_preamble -> txm_module_preamble_application_module_id;

    /* Setup the module's start/stop thread stack area.  */
    module_instance -> txm_module_instance_start_stop_stack_start_address =  (VOID *) (memory_ptr);
    module_instance -> txm_module_instance_start_stop_stack_size =           start_stop_stack_size;
    module_instance -> txm_module_instance_start_stop_stack_end_address =    (VOID *) (memory_ptr + (start_stop_stack_size - 1));

    /* Move the memory pointer forward.  */
    memory_ptr =  memory_ptr + start_stop_stack_size;

    /* Save the start/stop thread priority.  */
    module_instance -> txm_module_instance_start_stop_priority =     module_preamble -> txm_module_preamble_start_stop_priority;

    /* Setup the module's callback thread stack area.  */
    module_instance -> txm_module_instance_callback_stack_start_address =  (VOID *) (memory_ptr);
    module_instance -> txm_module_instance_callback_stack_size =           callback_stack_size;
    module_instance -> txm_module_instance_callback_stack_end_address =    (VOID *) (memory_ptr + (callback_stack_size - 1));

    /* Move the memory pointer forward.  */
    memory_ptr =  memory_ptr + callback_stack_size;

    /* Save the callback thread priority.  */
    module_instance -> txm_module_instance_callback_priority =  module_preamble -> txm_module_preamble_callback_priority;

    /* Setup the start of the module data section.  */
    module_instance -> txm_module_instance_module_data_base_address =  (VOID *) (memory_ptr);

    /* Build actual addresses based on load...  Setup all the function pointers. Any adjustments needed to shell entry, start function, and callback function are defined in the
       module preamble. */
#ifdef ALIGN_TYPE_DEFINED
    module_instance -> txm_module_instance_shell_entry_function  =          (VOID (*)(TX_THREAD *, TXM_MODULE_INSTANCE *)) ((VOID *) (ALIGN_TYPE) module_preamble -> txm_module_preamble_shell_entry_function);
    module_instance -> txm_module_instance_start_thread_entry =             (VOID (*)(ULONG)) ((VOID *) (ALIGN_TYPE) module_preamble -> txm_module_preamble_start_function);
    module_instance -> txm_module_instance_callback_request_thread_entry =  (VOID (*)(ULONG)) ((VOID *) (ALIGN_TYPE) module_preamble -> txm_module_preamble_callback_function);
#else
    module_instance -> txm_module_instance_shell_entry_function  =          (VOID (*)(TX_THREAD *, TXM_MODULE_INSTANCE *)) ((VOID *) module_preamble -> txm_module_preamble_shell_entry_function);
    module_instance -> txm_module_instance_start_thread_entry =             (VOID (*)(ULONG)) ((VOID *) module_preamble -> txm_module_preamble_start_function);
    module_instance -> txm_module_instance_callback_request_thread_entry =  (VOID (*)(ULONG)) ((VOID *) module_preamble -> txm_module_preamble_callback_function);
#endif

    /* Determine if there is a stop function for this module.  */
    if (module_preamble -> txm_module_preamble_stop_function)
    {

        /* Yes, there is a stop function, build the address.  */
#ifdef ALIGN_TYPE_DEFINED
    	module_instance -> txm_module_instance_stop_thread_entry =  (VOID (*)(ULONG)) ((VOID *) (ALIGN_TYPE) module_preamble -> txm_module_preamble_stop_function);
#else
    	module_instance -> txm_module_instance_stop_thread_entry =  (VOID (*)(ULONG)) ((VOID *) module_preamble -> txm_module_preamble_stop_function);
#endif
    }
    else
    {

        /* No, there is no stop function. Just set the pointer to NULL.  */
        module_instance -> txm_module_instance_stop_thread_entry =  TX_NULL;
    }

    /* Load the module control block with port-specific information. */
    TXM_MODULE_MANAGER_MODULE_SETUP(module_instance);

    /* Now add the module to the linked list of created modules.  */
    if (_txm_module_manger_loaded_count++ == 0)
    {

        /* The loaded module list is empty.  Add module to empty list.  */
        _txm_module_manager_loaded_list_ptr =                     module_instance;
        module_instance -> txm_module_instance_loaded_next =      module_instance;
        module_instance -> txm_module_instance_loaded_previous =  module_instance;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_module =      _txm_module_manager_loaded_list_ptr;
        previous_module =  next_module -> txm_module_instance_loaded_previous;

        /* Place the new module in the list.  */
        next_module -> txm_module_instance_loaded_previous =  module_instance;
        previous_module -> txm_module_instance_loaded_next =  module_instance;

        /* Setup this module's created links.  */
        module_instance -> txm_module_instance_loaded_previous =  previous_module;
        module_instance -> txm_module_instance_loaded_next =      next_module;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);

    /* Return success.  */
    return(TX_SUCCESS);
}
