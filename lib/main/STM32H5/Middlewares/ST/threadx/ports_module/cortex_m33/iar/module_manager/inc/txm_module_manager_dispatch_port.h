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

/* Define port-specific dispatch functions. */

/* UINT _txe_thread_secure_stack_allocate(
    TX_THREAD *thread_ptr, -> param_0
    ULONG stack_size -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_secure_stack_allocate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_secure_stack_allocate(
        (TX_THREAD *) param_0,
        (ULONG) param_1
    );
    return(return_value);
}

/* UINT _txe_thread_secure_stack_free(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_secure_stack_free_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_secure_stack_free(
        (TX_THREAD *) param_0
    );
    return(return_value);
}