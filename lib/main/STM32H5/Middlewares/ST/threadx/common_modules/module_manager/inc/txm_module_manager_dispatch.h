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

#ifndef TXM_BLOCK_ALLOCATE_CALL_NOT_USED
/* UINT _txe_block_allocate(
    TX_BLOCK_POOL *pool_ptr, -> param_0
    VOID **block_ptr, -> param_1
    ULONG wait_option -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_allocate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BLOCK_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(VOID *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_block_allocate(
        (TX_BLOCK_POOL *) param_0,
        (VOID **) param_1,
        (ULONG) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_POOL_CREATE_CALL_NOT_USED
/* UINT _txe_block_pool_create(
    TX_BLOCK_POOL *pool_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    ULONG block_size, -> extra_parameters[0]
    VOID *pool_start, -> extra_parameters[1]
    ULONG pool_size, -> extra_parameters[2]
    UINT pool_control_block_size -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_pool_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_BLOCK_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], extra_parameters[2]))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_block_pool_create(
        (TX_BLOCK_POOL *) param_0,
        (CHAR *) param_1,
        (ULONG) extra_parameters[0],
        (VOID *) extra_parameters[1],
        (ULONG) extra_parameters[2],
        (UINT) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_POOL_DELETE_CALL_NOT_USED
/* UINT _txe_block_pool_delete(
    TX_BLOCK_POOL *pool_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_pool_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BLOCK_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_block_pool_delete(
        (TX_BLOCK_POOL *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_POOL_INFO_GET_CALL_NOT_USED
/* UINT _txe_block_pool_info_get(
    TX_BLOCK_POOL *pool_ptr, -> param_0
    CHAR **name, -> param_1
    ULONG *available_blocks, -> extra_parameters[0]
    ULONG *total_blocks, -> extra_parameters[1]
    TX_THREAD **first_suspended, -> extra_parameters[2]
    ULONG *suspended_count, -> extra_parameters[3]
    TX_BLOCK_POOL **next_pool -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_pool_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BLOCK_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(TX_BLOCK_POOL *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_block_pool_info_get(
        (TX_BLOCK_POOL *) param_0,
        (CHAR **) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (TX_THREAD **) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (TX_BLOCK_POOL **) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_POOL_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_block_pool_performance_info_get(
    TX_BLOCK_POOL *pool_ptr, -> param_0
    ULONG *allocates, -> param_1
    ULONG *releases, -> extra_parameters[0]
    ULONG *suspensions, -> extra_parameters[1]
    ULONG *timeouts -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_pool_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BLOCK_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_block_pool_performance_info_get(
        (TX_BLOCK_POOL *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_block_pool_performance_system_info_get(
    ULONG *allocates, -> param_0
    ULONG *releases, -> param_1
    ULONG *suspensions, -> extra_parameters[0]
    ULONG *timeouts -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_pool_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_block_pool_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1]
    );
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_POOL_PRIORITIZE_CALL_NOT_USED
/* UINT _txe_block_pool_prioritize(
    TX_BLOCK_POOL *pool_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_pool_prioritize_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BLOCK_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_block_pool_prioritize(
        (TX_BLOCK_POOL *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_BLOCK_RELEASE_CALL_NOT_USED
/* UINT _txe_block_release(
    VOID *block_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_block_release_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;
ALIGN_TYPE block_header_start;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        /* Is the pointer non-null?  */
        if ((void *) param_0 != TX_NULL)
        {

            /* Calculate the beginning of the header info for this block (the header
               consists of 1 pointers.  */
            block_header_start =  param_0 - sizeof(ALIGN_TYPE);

            if (/* Did we underflow when doing the subtract?  */
                (block_header_start > param_0) ||
                /* Ensure the pointer is inside the module's data. Note that we only
                   check the pointer in the header because only that pointer is
                   dereferenced during the pointer's validity check in _tx_block_release. */
                (!TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, block_header_start, sizeof(ALIGN_TYPE))))
            {

                /* Invalid pointer.  */
                return(TXM_MODULE_INVALID_MEMORY);
            }
        }
    }

    return_value = (ALIGN_TYPE) _txe_block_release(
        (VOID *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_ALLOCATE_CALL_NOT_USED
/* UINT _txe_byte_allocate(
    TX_BYTE_POOL *pool_ptr, -> param_0
    VOID **memory_ptr, -> param_1
    ULONG memory_size, -> extra_parameters[0]
    ULONG wait_option -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_allocate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BYTE_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(VOID *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_byte_allocate(
        (TX_BYTE_POOL *) param_0,
        (VOID **) param_1,
        (ULONG) extra_parameters[0],
        (ULONG) extra_parameters[1]
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_POOL_CREATE_CALL_NOT_USED
/* UINT _txe_byte_pool_create(
    TX_BYTE_POOL *pool_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    VOID *pool_start, -> extra_parameters[0]
    ULONG pool_size, -> extra_parameters[1]
    UINT pool_control_block_size -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_pool_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_BYTE_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], extra_parameters[1]))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_byte_pool_create(
        (TX_BYTE_POOL *) param_0,
        (CHAR *) param_1,
        (VOID *) extra_parameters[0],
        (ULONG) extra_parameters[1],
        (UINT) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_POOL_DELETE_CALL_NOT_USED
/* UINT _txe_byte_pool_delete(
    TX_BYTE_POOL *pool_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_pool_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BYTE_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_byte_pool_delete(
        (TX_BYTE_POOL *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_BYTE_POOL_INFO_GET_CALL_NOT_USED
/* UINT _txe_byte_pool_info_get(
    TX_BYTE_POOL *pool_ptr, -> param_0
    CHAR **name, -> param_1
    ULONG *available_bytes, -> extra_parameters[0]
    ULONG *fragments, -> extra_parameters[1]
    TX_THREAD **first_suspended, -> extra_parameters[2]
    ULONG *suspended_count, -> extra_parameters[3]
    TX_BYTE_POOL **next_pool -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_pool_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BYTE_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(TX_BYTE_POOL *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_byte_pool_info_get(
        (TX_BYTE_POOL *) param_0,
        (CHAR **) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (TX_THREAD **) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (TX_BYTE_POOL **) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_POOL_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_byte_pool_performance_info_get(
    TX_BYTE_POOL *pool_ptr, -> param_0
    ULONG *allocates, -> param_1
    ULONG *releases, -> extra_parameters[0]
    ULONG *fragments_searched, -> extra_parameters[1]
    ULONG *merges, -> extra_parameters[2]
    ULONG *splits, -> extra_parameters[3]
    ULONG *suspensions, -> extra_parameters[4]
    ULONG *timeouts -> extra_parameters[5]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_pool_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BYTE_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[6])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[5], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_byte_pool_performance_info_get(
        (TX_BYTE_POOL *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (ULONG *) extra_parameters[4],
        (ULONG *) extra_parameters[5]
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_byte_pool_performance_system_info_get(
    ULONG *allocates, -> param_0
    ULONG *releases, -> param_1
    ULONG *fragments_searched, -> extra_parameters[0]
    ULONG *merges, -> extra_parameters[1]
    ULONG *splits, -> extra_parameters[2]
    ULONG *suspensions, -> extra_parameters[3]
    ULONG *timeouts -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_pool_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_byte_pool_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (ULONG *) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_POOL_PRIORITIZE_CALL_NOT_USED
/* UINT _txe_byte_pool_prioritize(
    TX_BYTE_POOL *pool_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_pool_prioritize_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_BYTE_POOL)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_byte_pool_prioritize(
        (TX_BYTE_POOL *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_BYTE_RELEASE_CALL_NOT_USED
/* UINT _txe_byte_release(
    VOID *memory_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_byte_release_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;
ALIGN_TYPE block_header_start;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        /* Is the pointer non-null?  */
        if ((void *) param_0 != TX_NULL)
        {

            /* Calculate the beginning of the header info for this block (the header
               consists of 2 pointers).  */
            block_header_start =  param_0 - 2*sizeof(ALIGN_TYPE);

            if (/* Did we underflow when doing the subtract?  */
                (block_header_start > param_0) ||
                /* Ensure the pointer is inside the module's data. Note that we only
                   check the pointers in the header because only those two are
                   dereferenced during the pointer's validity check in _tx_byte_release. */
                (!TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, block_header_start, 2*sizeof(ALIGN_TYPE))))
            {

                /* Invalid pointer.  */
                return(TXM_MODULE_INVALID_MEMORY);
            }
        }
    }

    return_value = (ALIGN_TYPE) _txe_byte_release(
        (VOID *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_CREATE_CALL_NOT_USED
/* UINT _txe_event_flags_create(
    TX_EVENT_FLAGS_GROUP *group_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    UINT event_control_block_size -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_event_flags_create(
        (TX_EVENT_FLAGS_GROUP *) param_0,
        (CHAR *) param_1,
        (UINT) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_DELETE_CALL_NOT_USED
/* UINT _txe_event_flags_delete(
    TX_EVENT_FLAGS_GROUP *group_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_event_flags_delete(
        (TX_EVENT_FLAGS_GROUP *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_GET_CALL_NOT_USED
/* UINT _txe_event_flags_get(
    TX_EVENT_FLAGS_GROUP *group_ptr, -> param_0
    ULONG requested_flags, -> param_1
    UINT get_option, -> extra_parameters[0]
    ULONG *actual_flags_ptr, -> extra_parameters[1]
    ULONG wait_option -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_event_flags_get(
        (TX_EVENT_FLAGS_GROUP *) param_0,
        (ULONG) param_1,
        (UINT) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_INFO_GET_CALL_NOT_USED
/* UINT _txe_event_flags_info_get(
    TX_EVENT_FLAGS_GROUP *group_ptr, -> param_0
    CHAR **name, -> param_1
    ULONG *current_flags, -> extra_parameters[0]
    TX_THREAD **first_suspended, -> extra_parameters[1]
    ULONG *suspended_count, -> extra_parameters[2]
    TX_EVENT_FLAGS_GROUP **next_group -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(TX_EVENT_FLAGS_GROUP *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_event_flags_info_get(
        (TX_EVENT_FLAGS_GROUP *) param_0,
        (CHAR **) param_1,
        (ULONG *) extra_parameters[0],
        (TX_THREAD **) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (TX_EVENT_FLAGS_GROUP **) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_event_flags_performance_info_get(
    TX_EVENT_FLAGS_GROUP *group_ptr, -> param_0
    ULONG *sets, -> param_1
    ULONG *gets, -> extra_parameters[0]
    ULONG *suspensions, -> extra_parameters[1]
    ULONG *timeouts -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_event_flags_performance_info_get(
        (TX_EVENT_FLAGS_GROUP *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_event_flags_performance_system_info_get(
    ULONG *sets, -> param_0
    ULONG *gets, -> param_1
    ULONG *suspensions, -> extra_parameters[0]
    ULONG *timeouts -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_event_flags_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1]
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_SET_CALL_NOT_USED
/* UINT _txe_event_flags_set(
    TX_EVENT_FLAGS_GROUP *group_ptr, -> param_0
    ULONG flags_to_set, -> param_1
    UINT set_option -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_set_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_event_flags_set(
        (TX_EVENT_FLAGS_GROUP *) param_0,
        (ULONG) param_1,
        (UINT) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_EVENT_FLAGS_SET_NOTIFY_CALL_NOT_USED
/* UINT _txe_event_flags_set_notify(
    TX_EVENT_FLAGS_GROUP *group_ptr, -> param_0
    VOID (*events_set_notify)(TX_EVENT_FLAGS_GROUP *) -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_event_flags_set_notify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;
TX_EVENT_FLAGS_GROUP *event_flags_ptr = (TX_EVENT_FLAGS_GROUP *)param_0;
VOID (*events_set_notify)(TX_EVENT_FLAGS_GROUP *);

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_EVENT_FLAGS_GROUP)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* Since we need to write to the object, ensure it's valid.  */
        if ((event_flags_ptr == TX_NULL) || (event_flags_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID))
            return(TX_GROUP_ERROR);
    }

    /* Is it a disable request?  */
    if ((void *) param_1 == TX_NULL)
    {

        /* Clear the callback.  */
        events_set_notify = (VOID (*)(TX_EVENT_FLAGS_GROUP *)) TX_NULL;
    }
    else
    {

        /* Setup trampoline values.  */
        event_flags_ptr -> tx_event_flags_group_module_instance = (VOID *) module_instance;
        event_flags_ptr -> tx_event_flags_group_set_module_notify = (VOID (*)(TX_EVENT_FLAGS_GROUP *)) param_1;
        events_set_notify = _txm_module_manager_event_flags_notify_trampoline;
    }

    return_value = (ALIGN_TYPE) _txe_event_flags_set_notify(
        (TX_EVENT_FLAGS_GROUP *) param_0,
        (VOID (*)(TX_EVENT_FLAGS_GROUP *)) events_set_notify
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_CREATE_CALL_NOT_USED
/* UINT _txe_mutex_create(
    TX_MUTEX *mutex_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    UINT inherit, -> extra_parameters[0]
    UINT mutex_control_block_size -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_mutex_create(
        (TX_MUTEX *) param_0,
        (CHAR *) param_1,
        (UINT) extra_parameters[0],
        (UINT) extra_parameters[1]
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_DELETE_CALL_NOT_USED
/* UINT _txe_mutex_delete(
    TX_MUTEX *mutex_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_mutex_delete(
        (TX_MUTEX *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_GET_CALL_NOT_USED
/* UINT _txe_mutex_get(
    TX_MUTEX *mutex_ptr, -> param_0
    ULONG wait_option -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_mutex_get(
        (TX_MUTEX *) param_0,
        (ULONG) param_1
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_INFO_GET_CALL_NOT_USED
/* UINT _txe_mutex_info_get(
    TX_MUTEX *mutex_ptr, -> param_0
    CHAR **name, -> param_1
    ULONG *count, -> extra_parameters[0]
    TX_THREAD **owner, -> extra_parameters[1]
    TX_THREAD **first_suspended, -> extra_parameters[2]
    ULONG *suspended_count, -> extra_parameters[3]
    TX_MUTEX **next_mutex -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(TX_MUTEX *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_mutex_info_get(
        (TX_MUTEX *) param_0,
        (CHAR **) param_1,
        (ULONG *) extra_parameters[0],
        (TX_THREAD **) extra_parameters[1],
        (TX_THREAD **) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (TX_MUTEX **) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_mutex_performance_info_get(
    TX_MUTEX *mutex_ptr, -> param_0
    ULONG *puts, -> param_1
    ULONG *gets, -> extra_parameters[0]
    ULONG *suspensions, -> extra_parameters[1]
    ULONG *timeouts, -> extra_parameters[2]
    ULONG *inversions, -> extra_parameters[3]
    ULONG *inheritances -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_mutex_performance_info_get(
        (TX_MUTEX *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (ULONG *) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_mutex_performance_system_info_get(
    ULONG *puts, -> param_0
    ULONG *gets, -> param_1
    ULONG *suspensions, -> extra_parameters[0]
    ULONG *timeouts, -> extra_parameters[1]
    ULONG *inversions, -> extra_parameters[2]
    ULONG *inheritances -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_mutex_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_PRIORITIZE_CALL_NOT_USED
/* UINT _txe_mutex_prioritize(
    TX_MUTEX *mutex_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_prioritize_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_mutex_prioritize(
        (TX_MUTEX *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_MUTEX_PUT_CALL_NOT_USED
/* UINT _txe_mutex_put(
    TX_MUTEX *mutex_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_mutex_put_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_MUTEX)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_mutex_put(
        (TX_MUTEX *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_CREATE_CALL_NOT_USED
/* UINT _txe_queue_create(
    TX_QUEUE *queue_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    UINT message_size, -> extra_parameters[0]
    VOID *queue_start, -> extra_parameters[1]
    ULONG queue_size, -> extra_parameters[2]
    UINT queue_control_block_size -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], extra_parameters[2]))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_create(
        (TX_QUEUE *) param_0,
        (CHAR *) param_1,
        (UINT) extra_parameters[0],
        (VOID *) extra_parameters[1],
        (ULONG) extra_parameters[2],
        (UINT) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_DELETE_CALL_NOT_USED
/* UINT _txe_queue_delete(
    TX_QUEUE *queue_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_delete(
        (TX_QUEUE *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_FLUSH_CALL_NOT_USED
/* UINT _txe_queue_flush(
    TX_QUEUE *queue_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_flush_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_flush(
        (TX_QUEUE *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_FRONT_SEND_CALL_NOT_USED
/* UINT _txe_queue_front_send(
    TX_QUEUE *queue_ptr, -> param_0
    VOID *source_ptr, -> param_1
    ULONG wait_option -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_front_send_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;
TX_QUEUE *queue_ptr;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* We need to get the size of the message from the queue.  */
        queue_ptr =  (TX_QUEUE *) param_0;
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_READ(module_instance, param_1, queue_ptr -> tx_queue_message_size))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_front_send(
        (TX_QUEUE *) param_0,
        (VOID *) param_1,
        (ULONG) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_INFO_GET_CALL_NOT_USED
/* UINT _txe_queue_info_get(
    TX_QUEUE *queue_ptr, -> param_0
    CHAR **name, -> param_1
    ULONG *enqueued, -> extra_parameters[0]
    ULONG *available_storage, -> extra_parameters[1]
    TX_THREAD **first_suspended, -> extra_parameters[2]
    ULONG *suspended_count, -> extra_parameters[3]
    TX_QUEUE **next_queue -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(TX_QUEUE *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_info_get(
        (TX_QUEUE *) param_0,
        (CHAR **) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (TX_THREAD **) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (TX_QUEUE **) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_queue_performance_info_get(
    TX_QUEUE *queue_ptr, -> param_0
    ULONG *messages_sent, -> param_1
    ULONG *messages_received, -> extra_parameters[0]
    ULONG *empty_suspensions, -> extra_parameters[1]
    ULONG *full_suspensions, -> extra_parameters[2]
    ULONG *full_errors, -> extra_parameters[3]
    ULONG *timeouts -> extra_parameters[4]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[5])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_queue_performance_info_get(
        (TX_QUEUE *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (ULONG *) extra_parameters[4]
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_queue_performance_system_info_get(
    ULONG *messages_sent, -> param_0
    ULONG *messages_received, -> param_1
    ULONG *empty_suspensions, -> extra_parameters[0]
    ULONG *full_suspensions, -> extra_parameters[1]
    ULONG *full_errors, -> extra_parameters[2]
    ULONG *timeouts -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_queue_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_PRIORITIZE_CALL_NOT_USED
/* UINT _txe_queue_prioritize(
    TX_QUEUE *queue_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_prioritize_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_prioritize(
        (TX_QUEUE *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_RECEIVE_CALL_NOT_USED
/* UINT _txe_queue_receive(
    TX_QUEUE *queue_ptr, -> param_0
    VOID *destination_ptr, -> param_1
    ULONG wait_option -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_receive_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;
TX_QUEUE *queue_ptr;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* We need to get the max size of the buffer from the queue.  */
        queue_ptr =  (TX_QUEUE *) param_0;
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)*queue_ptr -> tx_queue_message_size))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_receive(
        (TX_QUEUE *) param_0,
        (VOID *) param_1,
        (ULONG) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_SEND_CALL_NOT_USED
/* UINT _txe_queue_send(
    TX_QUEUE *queue_ptr, -> param_0
    VOID *source_ptr, -> param_1
    ULONG wait_option -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_send_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;
TX_QUEUE *queue_ptr;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* We need to get the size of the message from the queue.  */
        queue_ptr =  (TX_QUEUE *) param_0;
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_READ(module_instance, param_1, sizeof(ULONG)*queue_ptr -> tx_queue_message_size))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_queue_send(
        (TX_QUEUE *) param_0,
        (VOID *) param_1,
        (ULONG) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_QUEUE_SEND_NOTIFY_CALL_NOT_USED
/* UINT _txe_queue_send_notify(
    TX_QUEUE *queue_ptr, -> param_0
    VOID (*queue_send_notify)(TX_QUEUE *notify_queue_ptr) -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_queue_send_notify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;
TX_QUEUE *queue_ptr = (TX_QUEUE *) param_0;
VOID (*queue_send_notify)(TX_QUEUE *);

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_QUEUE)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* Since we need to write to the object, ensure it's valid.  */
        if ((queue_ptr == TX_NULL) || (queue_ptr -> tx_queue_id != TX_QUEUE_ID))
            return(TX_QUEUE_ERROR);
    }

    /* Is it a disable request?  */
    if ((void *) param_1 == TX_NULL)
    {

        /* Clear the callback.  */
        queue_send_notify = (VOID (*)(TX_QUEUE *)) TX_NULL;
    }
    else
    {

        /* Setup trampoline values.  */
        queue_ptr -> tx_queue_module_instance = (VOID *) module_instance;
        queue_ptr -> tx_queue_send_module_notify = (VOID (*)(TX_QUEUE *)) param_1;
        queue_send_notify = _txm_module_manager_queue_notify_trampoline;
    }

    return_value = (ALIGN_TYPE) _txe_queue_send_notify(
        (TX_QUEUE *) param_0,
        (VOID (*)(TX_QUEUE *notify_queue_ptr)) queue_send_notify
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_CEILING_PUT_CALL_NOT_USED
/* UINT _txe_semaphore_ceiling_put(
    TX_SEMAPHORE *semaphore_ptr, -> param_0
    ULONG ceiling -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_ceiling_put_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_ceiling_put(
        (TX_SEMAPHORE *) param_0,
        (ULONG) param_1
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_CREATE_CALL_NOT_USED
/* UINT _txe_semaphore_create(
    TX_SEMAPHORE *semaphore_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    ULONG initial_count, -> extra_parameters[0]
    UINT semaphore_control_block_size -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_create(
        (TX_SEMAPHORE *) param_0,
        (CHAR *) param_1,
        (ULONG) extra_parameters[0],
        (UINT) extra_parameters[1]
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_DELETE_CALL_NOT_USED
/* UINT _txe_semaphore_delete(
    TX_SEMAPHORE *semaphore_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_delete(
        (TX_SEMAPHORE *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_GET_CALL_NOT_USED
/* UINT _txe_semaphore_get(
    TX_SEMAPHORE *semaphore_ptr, -> param_0
    ULONG wait_option -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_get(
        (TX_SEMAPHORE *) param_0,
        (ULONG) param_1
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_INFO_GET_CALL_NOT_USED
/* UINT _txe_semaphore_info_get(
    TX_SEMAPHORE *semaphore_ptr, -> param_0
    CHAR **name, -> param_1
    ULONG *current_value, -> extra_parameters[0]
    TX_THREAD **first_suspended, -> extra_parameters[1]
    ULONG *suspended_count, -> extra_parameters[2]
    TX_SEMAPHORE **next_semaphore -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(TX_SEMAPHORE *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_info_get(
        (TX_SEMAPHORE *) param_0,
        (CHAR **) param_1,
        (ULONG *) extra_parameters[0],
        (TX_THREAD **) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (TX_SEMAPHORE **) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_semaphore_performance_info_get(
    TX_SEMAPHORE *semaphore_ptr, -> param_0
    ULONG *puts, -> param_1
    ULONG *gets, -> extra_parameters[0]
    ULONG *suspensions, -> extra_parameters[1]
    ULONG *timeouts -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_semaphore_performance_info_get(
        (TX_SEMAPHORE *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_semaphore_performance_system_info_get(
    ULONG *puts, -> param_0
    ULONG *gets, -> param_1
    ULONG *suspensions, -> extra_parameters[0]
    ULONG *timeouts -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_semaphore_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1]
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_PRIORITIZE_CALL_NOT_USED
/* UINT _txe_semaphore_prioritize(
    TX_SEMAPHORE *semaphore_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_prioritize_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_prioritize(
        (TX_SEMAPHORE *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_PUT_CALL_NOT_USED
/* UINT _txe_semaphore_put(
    TX_SEMAPHORE *semaphore_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_put_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_put(
        (TX_SEMAPHORE *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_SEMAPHORE_PUT_NOTIFY_CALL_NOT_USED
/* UINT _txe_semaphore_put_notify(
    TX_SEMAPHORE *semaphore_ptr, -> param_0
    VOID (*semaphore_put_notify)(TX_SEMAPHORE *notify_semaphore_ptr) -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_semaphore_put_notify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;
TX_SEMAPHORE *semaphore_ptr = (TX_SEMAPHORE *) param_0;
VOID (*semaphore_put_notify)(TX_SEMAPHORE *);

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_SEMAPHORE)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* Since we need to write to the object, ensure it's valid.  */
        if ((semaphore_ptr == TX_NULL) || (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID))
            return(TX_SEMAPHORE_ERROR);
    }

    /* Is it a disable request?  */
    if ((void *) param_1 == TX_NULL)
    {

        /* Clear the callback.  */
        semaphore_put_notify = (VOID (*)(TX_SEMAPHORE *)) TX_NULL;
    }
    else
    {

        /* Setup trampoline values.  */
        semaphore_ptr -> tx_semaphore_module_instance = (VOID *) module_instance;
        semaphore_ptr -> tx_semaphore_put_module_notify = (VOID (*)(TX_SEMAPHORE *)) param_1;
        semaphore_put_notify = _txm_module_manager_semaphore_notify_trampoline;
    }

    return_value = (ALIGN_TYPE) _txe_semaphore_put_notify(
        (TX_SEMAPHORE *) param_0,
        (VOID (*)(TX_SEMAPHORE *notify_semaphore_ptr)) semaphore_put_notify
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_CREATE_CALL_NOT_USED
/* UINT _txe_thread_create(
    TX_THREAD *thread_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    VOID (*entry_function)(ULONG entry_input), -> extra_parameters[0]
    ULONG entry_input, -> extra_parameters[1]
    VOID *stack_start, -> extra_parameters[2]
    ULONG stack_size, -> extra_parameters[3]
    UINT priority, -> extra_parameters[4]
    UINT preempt_threshold, -> extra_parameters[5]
    ULONG time_slice, -> extra_parameters[6]
    UINT auto_start, -> extra_parameters[7]
    UINT thread_control_block_size -> extra_parameters[8]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[9])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], extra_parameters[3]))
            return(TXM_MODULE_INVALID_MEMORY);

        if (extra_parameters[4] < module_instance -> txm_module_instance_maximum_priority)
            return(TX_PRIORITY_ERROR);

        if (extra_parameters[5] < module_instance -> txm_module_instance_maximum_priority)
            return(TX_THRESH_ERROR);
    }

    return_value = (ALIGN_TYPE) _txm_module_manager_thread_create(
        (TX_THREAD *) param_0,
        (CHAR *) param_1,
        module_instance -> txm_module_instance_shell_entry_function,
        (VOID (*)(ULONG entry_input)) extra_parameters[0],
        (ULONG) extra_parameters[1],
        (VOID *) extra_parameters[2],
        (ULONG) extra_parameters[3],
        (UINT) extra_parameters[4],
        (UINT) extra_parameters[5],
        (ULONG) extra_parameters[6],
        (UINT) extra_parameters[7],
        (UINT) extra_parameters[8],
        module_instance
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_DELETE_CALL_NOT_USED
/* UINT _txe_thread_delete(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_delete(
        (TX_THREAD *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_THREAD_ENTRY_EXIT_NOTIFY_CALL_NOT_USED
/* UINT _txe_thread_entry_exit_notify(
    TX_THREAD *thread_ptr, -> param_0
    VOID (*thread_entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT type) -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_entry_exit_notify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;
TX_THREAD *thread_ptr = (TX_THREAD *) param_0;
TXM_MODULE_THREAD_ENTRY_INFO *thread_entry_info_ptr;
VOID (*thread_entry_exit_notify)(TX_THREAD *, UINT);

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        /* Since we need to write to the object, ensure it's valid.  */
        if ((thread_ptr == TX_NULL) || (thread_ptr -> tx_thread_id != TX_THREAD_ID))
            return(TX_THREAD_ERROR);

        /* Ensure this thread is from the module trying to set the callback.  */
        if (thread_ptr -> tx_thread_module_instance_ptr != module_instance)
            return(TXM_MODULE_INVALID);
    }

    /* Is it a disable request?  */
    if ((void *) param_1 == TX_NULL)
    {

        /* Clear the callback.  */
        thread_entry_exit_notify = (VOID (*)(TX_THREAD *, UINT)) TX_NULL;
    }
    else
    {

        /* Setup trampoline values.  */
        thread_entry_info_ptr = (TXM_MODULE_THREAD_ENTRY_INFO *) thread_ptr -> tx_thread_module_entry_info_ptr;
        thread_entry_info_ptr -> txm_module_thread_entry_info_exit_notify = (VOID (*)(TX_THREAD *, UINT)) param_1;
        thread_entry_exit_notify = _txm_module_manager_thread_notify_trampoline;
    }

    return_value = (ALIGN_TYPE) _txe_thread_entry_exit_notify(
        (TX_THREAD *) param_0,
        (VOID (*)(TX_THREAD *notify_thread_ptr, UINT type)) thread_entry_exit_notify
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_IDENTIFY_CALL_NOT_USED
/* TX_THREAD *_tx_thread_identify(); */
static ALIGN_TYPE _txm_module_manager_tx_thread_identify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    return_value = (ALIGN_TYPE) _tx_thread_identify();
    return(return_value);
}
#endif

#ifndef TXM_THREAD_INFO_GET_CALL_NOT_USED
/* UINT _txe_thread_info_get(
    TX_THREAD *thread_ptr, -> param_0
    CHAR **name, -> param_1
    UINT *state, -> extra_parameters[0]
    ULONG *run_count, -> extra_parameters[1]
    UINT *priority, -> extra_parameters[2]
    UINT *preemption_threshold, -> extra_parameters[3]
    ULONG *time_slice, -> extra_parameters[4]
    TX_THREAD **next_thread, -> extra_parameters[5]
    TX_THREAD **next_suspended_thread -> extra_parameters[6]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[7])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(UINT)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(UINT)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(UINT)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[5], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[6], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_info_get(
        (TX_THREAD *) param_0,
        (CHAR **) param_1,
        (UINT *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (UINT *) extra_parameters[2],
        (UINT *) extra_parameters[3],
        (ULONG *) extra_parameters[4],
        (TX_THREAD **) extra_parameters[5],
        (TX_THREAD **) extra_parameters[6]
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_INTERRUPT_CONTROL_CALL_NOT_USED
/* UINT _tx_thread_interrupt_control(
    UINT new_posture -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_interrupt_control_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    return_value = (ALIGN_TYPE) _tx_thread_interrupt_control(
        (UINT) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_thread_performance_info_get(
    TX_THREAD *thread_ptr, -> param_0
    ULONG *resumptions, -> param_1
    ULONG *suspensions, -> extra_parameters[0]
    ULONG *solicited_preemptions, -> extra_parameters[1]
    ULONG *interrupt_preemptions, -> extra_parameters[2]
    ULONG *priority_inversions, -> extra_parameters[3]
    ULONG *time_slices, -> extra_parameters[4]
    ULONG *relinquishes, -> extra_parameters[5]
    ULONG *timeouts, -> extra_parameters[6]
    ULONG *wait_aborts, -> extra_parameters[7]
    TX_THREAD **last_preempted_by -> extra_parameters[8]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[9])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[5], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[6], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[7], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[8], sizeof(TX_THREAD *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_thread_performance_info_get(
        (TX_THREAD *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (ULONG *) extra_parameters[4],
        (ULONG *) extra_parameters[5],
        (ULONG *) extra_parameters[6],
        (ULONG *) extra_parameters[7],
        (TX_THREAD **) extra_parameters[8]
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_thread_performance_system_info_get(
    ULONG *resumptions, -> param_0
    ULONG *suspensions, -> param_1
    ULONG *solicited_preemptions, -> extra_parameters[0]
    ULONG *interrupt_preemptions, -> extra_parameters[1]
    ULONG *priority_inversions, -> extra_parameters[2]
    ULONG *time_slices, -> extra_parameters[3]
    ULONG *relinquishes, -> extra_parameters[4]
    ULONG *timeouts, -> extra_parameters[5]
    ULONG *wait_aborts, -> extra_parameters[6]
    ULONG *non_idle_returns, -> extra_parameters[7]
    ULONG *idle_returns -> extra_parameters[8]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[9])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[4], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[5], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[6], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[7], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[8], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_thread_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3],
        (ULONG *) extra_parameters[4],
        (ULONG *) extra_parameters[5],
        (ULONG *) extra_parameters[6],
        (ULONG *) extra_parameters[7],
        (ULONG *) extra_parameters[8]
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_PREEMPTION_CHANGE_CALL_NOT_USED
/* UINT _txe_thread_preemption_change(
    TX_THREAD *thread_ptr, -> param_0
    UINT new_threshold, -> param_1
    UINT *old_threshold -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_preemption_change_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_2, sizeof(UINT)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_preemption_change(
        (TX_THREAD *) param_0,
        (UINT) param_1,
        (UINT *) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_PRIORITY_CHANGE_CALL_NOT_USED
/* UINT _txe_thread_priority_change(
    TX_THREAD *thread_ptr, -> param_0
    UINT new_priority, -> param_1
    UINT *old_priority -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_priority_change_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_2, sizeof(UINT)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_priority_change(
        (TX_THREAD *) param_0,
        (UINT) param_1,
        (UINT *) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_RELINQUISH_CALL_NOT_USED
/* VOID _txe_thread_relinquish(); */
static ALIGN_TYPE _txm_module_manager_tx_thread_relinquish_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

    _txe_thread_relinquish();
    return(TX_SUCCESS);
}
#endif

#ifndef TXM_THREAD_RESET_CALL_NOT_USED
/* UINT _txe_thread_reset(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_reset_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txm_module_manager_thread_reset(
        (TX_THREAD *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_RESUME_CALL_NOT_USED
/* UINT _txe_thread_resume(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_resume_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_resume(
        (TX_THREAD *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_SLEEP_CALL_NOT_USED
/* UINT _tx_thread_sleep(
    ULONG timer_ticks -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_sleep_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    return_value = (ALIGN_TYPE) _tx_thread_sleep(
        (ULONG) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_STACK_ERROR_NOTIFY_CALL_NOT_USED
/* UINT _tx_thread_stack_error_notify(
    VOID (*stack_error_handler)(TX_THREAD *thread_ptr) -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_stack_error_notify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    return_value = (ALIGN_TYPE) _tx_thread_stack_error_notify(
        (VOID (*)(TX_THREAD *thread_ptr)) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_SUSPEND_CALL_NOT_USED
/* UINT _txe_thread_suspend(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_suspend_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_suspend(
        (TX_THREAD *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_SYSTEM_SUSPEND_CALL_NOT_USED
/* VOID _tx_thread_system_suspend(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_system_suspend_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        /* Ensure the thread is suspending itself.  */
        if (((TX_THREAD *) param_0) != _tx_thread_current_ptr)
        {
            return(TXM_MODULE_INVALID_MEMORY);
        }
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    /* Get the thread pointer.  */
    thread_ptr = (TX_THREAD *) param_0;

    /* Disable interrupts temporarily.  */
    TX_DISABLE

    /* Set the status to suspending, in order to indicate the suspension
       is in progress.  */
    thread_ptr -> tx_thread_state =  TX_COMPLETED;

    /* Thread state change.  */
    TX_THREAD_STATE_CHANGE(thread_ptr, TX_COMPLETED)

    /* Set the suspending flag. */
    thread_ptr -> tx_thread_suspending =  TX_TRUE;

    /* Setup for no timeout period.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  0;

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_COMPLETED_EXTENSION(thread_ptr);

    _tx_thread_system_suspend(
        (TX_THREAD *) param_0
    );
    return(TX_SUCCESS);
}
#endif

#ifndef TXM_THREAD_TERMINATE_CALL_NOT_USED
/* UINT _txe_thread_terminate(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_terminate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_terminate(
        (TX_THREAD *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_TIME_SLICE_CHANGE_CALL_NOT_USED
/* UINT _txe_thread_time_slice_change(
    TX_THREAD *thread_ptr, -> param_0
    ULONG new_time_slice, -> param_1
    ULONG *old_time_slice -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_time_slice_change_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_2, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_time_slice_change(
        (TX_THREAD *) param_0,
        (ULONG) param_1,
        (ULONG *) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_THREAD_WAIT_ABORT_CALL_NOT_USED
/* UINT _txe_thread_wait_abort(
    TX_THREAD *thread_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_thread_wait_abort_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_thread_wait_abort(
        (TX_THREAD *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TIME_GET_CALL_NOT_USED
/* ULONG _tx_time_get(); */
static ALIGN_TYPE _txm_module_manager_tx_time_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    return_value = (ALIGN_TYPE) _tx_time_get();
    return(return_value);
}
#endif

#ifndef TXM_TIME_SET_CALL_NOT_USED
/* VOID _tx_time_set(
    ULONG new_time -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_time_set_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

    _tx_time_set(
        (ULONG) param_0
    );
    return(TX_SUCCESS);
}
#endif

#ifndef TXM_TIMER_ACTIVATE_CALL_NOT_USED
/* UINT _txe_timer_activate(
    TX_TIMER *timer_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_activate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_timer_activate(
        (TX_TIMER *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TIMER_CHANGE_CALL_NOT_USED
/* UINT _txe_timer_change(
    TX_TIMER *timer_ptr, -> param_0
    ULONG initial_ticks, -> param_1
    ULONG reschedule_ticks -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_change_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_timer_change(
        (TX_TIMER *) param_0,
        (ULONG) param_1,
        (ULONG) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_TIMER_CREATE_CALL_NOT_USED
/* UINT _txe_timer_create(
    TX_TIMER *timer_ptr, -> param_0
    CHAR *name_ptr, -> param_1
    VOID (*expiration_function)(ULONG), -> extra_parameters[0]
    ULONG expiration_input, -> extra_parameters[1]
    ULONG initial_ticks, -> extra_parameters[2]
    ULONG reschedule_ticks, -> extra_parameters[3]
    UINT auto_activate, -> extra_parameters[4]
    UINT timer_control_block_size -> extra_parameters[5]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_create_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;
TX_TIMER *timer_ptr;
VOID (*expiration_function)(ULONG);

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[6])))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    /* Is it a disable request?  */
    if ((void *) extra_parameters[0] == TX_NULL)
    {

        /* Clear the callback.  */
        expiration_function = (VOID (*)(ULONG)) TX_NULL;
    }
    else
    {

        /* Set trampoline callback.  */
        expiration_function = _txm_module_manager_timer_notify_trampoline;
    }

    return_value = (ALIGN_TYPE) _txe_timer_create(
        (TX_TIMER *) param_0,
        (CHAR *) param_1,
        (VOID (*)(ULONG)) expiration_function,
        (ULONG) extra_parameters[1],
        (ULONG) extra_parameters[2],
        (ULONG) extra_parameters[3],
        (UINT) extra_parameters[4],
        (UINT) extra_parameters[5]
    );

    if (return_value == TX_SUCCESS)
    {

        /* Get the object pointer.  */
        timer_ptr = (TX_TIMER *) param_0;

        /* Setup trampoline values.  */
        if ((void *) extra_parameters[0] != TX_NULL)
        {

            timer_ptr -> tx_timer_module_instance = (VOID *) module_instance;
            timer_ptr -> tx_timer_module_expiration_function = (VOID (*)(ULONG)) extra_parameters[0];
        }
    }
    return(return_value);
}
#endif

#ifndef TXM_TIMER_DEACTIVATE_CALL_NOT_USED
/* UINT _txe_timer_deactivate(
    TX_TIMER *timer_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_deactivate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_timer_deactivate(
        (TX_TIMER *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TIMER_DELETE_CALL_NOT_USED
/* UINT _txe_timer_delete(
    TX_TIMER *timer_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_delete_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_timer_delete(
        (TX_TIMER *) param_0
    );

    /* Deallocate object memory.  */
    if (return_value == TX_SUCCESS)
    {
        return_value = _txm_module_manager_object_deallocate((VOID *) param_0);
    }
    return(return_value);
}
#endif

#ifndef TXM_TIMER_INFO_GET_CALL_NOT_USED
/* UINT _txe_timer_info_get(
    TX_TIMER *timer_ptr, -> param_0
    CHAR **name, -> param_1
    UINT *active, -> extra_parameters[0]
    ULONG *remaining_ticks, -> extra_parameters[1]
    ULONG *reschedule_ticks, -> extra_parameters[2]
    TX_TIMER **next_timer -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(CHAR *)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(UINT)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(TX_TIMER *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txe_timer_info_get(
        (TX_TIMER *) param_0,
        (CHAR **) param_1,
        (UINT *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (TX_TIMER **) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_TIMER_PERFORMANCE_INFO_GET_CALL_NOT_USED
/* UINT _tx_timer_performance_info_get(
    TX_TIMER *timer_ptr, -> param_0
    ULONG *activates, -> param_1
    ULONG *reactivates, -> extra_parameters[0]
    ULONG *deactivates, -> extra_parameters[1]
    ULONG *expirations, -> extra_parameters[2]
    ULONG *expiration_adjusts -> extra_parameters[3]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_performance_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_TIMER)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[4])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[3], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_timer_performance_info_get(
        (TX_TIMER *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2],
        (ULONG *) extra_parameters[3]
    );
    return(return_value);
}
#endif

#ifndef TXM_TIMER_PERFORMANCE_SYSTEM_INFO_GET_CALL_NOT_USED
/* UINT _tx_timer_performance_system_info_get(
    ULONG *activates, -> param_0
    ULONG *reactivates, -> param_1
    ULONG *deactivates, -> extra_parameters[0]
    ULONG *expirations, -> extra_parameters[1]
    ULONG *expiration_adjusts -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_timer_performance_system_info_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_1, sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[0], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[2], sizeof(ULONG)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_timer_performance_system_info_get(
        (ULONG *) param_0,
        (ULONG *) param_1,
        (ULONG *) extra_parameters[0],
        (ULONG *) extra_parameters[1],
        (ULONG *) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_TRACE_BUFFER_FULL_NOTIFY_CALL_NOT_USED
/* UINT _tx_trace_buffer_full_notify(
    VOID (*full_buffer_callback)(VOID *buffer) -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_buffer_full_notify_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    return_value = (ALIGN_TYPE) _tx_trace_buffer_full_notify(
        (VOID (*)(VOID *buffer)) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TRACE_DISABLE_CALL_NOT_USED
/* UINT _tx_trace_disable(); */
static ALIGN_TYPE _txm_module_manager_tx_trace_disable_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    return_value = (ALIGN_TYPE) _tx_trace_disable();
    return(return_value);
}
#endif

#ifndef TXM_TRACE_ENABLE_CALL_NOT_USED
/* UINT _tx_trace_enable(
    VOID *trace_buffer_start, -> param_0
    ULONG trace_buffer_size, -> param_1
    ULONG registry_entries -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_enable_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    return_value = (ALIGN_TYPE) _tx_trace_enable(
        (VOID *) param_0,
        (ULONG) param_1,
        (ULONG) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_TRACE_EVENT_FILTER_CALL_NOT_USED
/* UINT _tx_trace_event_filter(
    ULONG event_filter_bits -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_event_filter_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    return_value = (ALIGN_TYPE) _tx_trace_event_filter(
        (ULONG) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TRACE_EVENT_UNFILTER_CALL_NOT_USED
/* UINT _tx_trace_event_unfilter(
    ULONG event_unfilter_bits -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_event_unfilter_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    return_value = (ALIGN_TYPE) _tx_trace_event_unfilter(
        (ULONG) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TRACE_INTERRUPT_CONTROL_CALL_NOT_USED
/* UINT _tx_trace_interrupt_control(
    UINT new_posture -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_interrupt_control_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    return_value = (ALIGN_TYPE) _tx_trace_interrupt_control(
        (UINT) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_TRACE_ISR_ENTER_INSERT_CALL_NOT_USED
/* VOID _tx_trace_isr_enter_insert(
    ULONG isr_id -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_isr_enter_insert_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    _tx_trace_isr_enter_insert(
        (ULONG) param_0
    );
    return(TX_SUCCESS);
}
#endif

#ifndef TXM_TRACE_ISR_EXIT_INSERT_CALL_NOT_USED
/* VOID _tx_trace_isr_exit_insert(
    ULONG isr_id -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_isr_exit_insert_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)
        return(TXM_MODULE_INVALID_PROPERTIES);

    _tx_trace_isr_exit_insert(
        (ULONG) param_0
    );
    return(TX_SUCCESS);
}
#endif

#ifndef TXM_TRACE_USER_EVENT_INSERT_CALL_NOT_USED
/* UINT _tx_trace_user_event_insert(
    ULONG event_id, -> param_0
    ULONG info_field_1, -> param_1
    ULONG info_field_2, -> extra_parameters[0]
    ULONG info_field_3, -> extra_parameters[1]
    ULONG info_field_4 -> extra_parameters[2]
   ); */
static ALIGN_TYPE _txm_module_manager_tx_trace_user_event_insert_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[3])))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _tx_trace_user_event_insert(
        (ULONG) param_0,
        (ULONG) param_1,
        (ULONG) extra_parameters[0],
        (ULONG) extra_parameters[1],
        (ULONG) extra_parameters[2]
    );
    return(return_value);
}
#endif

#ifndef TXM_MODULE_OBJECT_ALLOCATE_CALL_NOT_USED
/* UINT _txm_module_object_allocate(
    VOID **object_ptr, -> param_0
    ULONG object_size -> param_1
   ); */
static ALIGN_TYPE _txm_module_manager_txm_module_object_allocate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_0, sizeof(VOID *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txm_module_manager_object_allocate(
        (VOID **) param_0,
        (ULONG) param_1,
        module_instance
    );
    return(return_value);
}
#endif

#ifndef TXM_MODULE_OBJECT_DEALLOCATE_CALL_NOT_USED
/* UINT _txm_module_object_deallocate(
    VOID *object_ptr -> param_0
   ); */
static ALIGN_TYPE _txm_module_manager_txm_module_object_deallocate_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0)
{

ALIGN_TYPE return_value;
TXM_MODULE_ALLOCATED_OBJECT *object_ptr;
ALIGN_TYPE object_end;
ALIGN_TYPE object_pool_end;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        /* Is the object pool created?  */
        if (_txm_module_manager_object_pool_created == TX_TRUE)
        {

            /* Get the module allocated object.  */
            object_ptr =  ((TXM_MODULE_ALLOCATED_OBJECT *) param_0) - 1;

            /* Get the end address of the object pool.  */
            object_pool_end = (ALIGN_TYPE) (_txm_module_manager_object_pool.tx_byte_pool_start + _txm_module_manager_object_pool.tx_byte_pool_size);

            /* Check that the pointer is in the object pool.  */
            if ((ALIGN_TYPE) object_ptr < (ALIGN_TYPE) _txm_module_manager_object_pool.tx_byte_pool_start ||
                (ALIGN_TYPE) object_ptr >= (ALIGN_TYPE) object_pool_end)
            {
                /* Pointer is outside of the object pool.  */
                return(TXM_MODULE_INVALID_MEMORY);
            }

            /* Get the end addresses of the object.  */
            object_end = ((ALIGN_TYPE) object_ptr) + sizeof(TXM_MODULE_ALLOCATED_OBJECT) + object_ptr -> txm_module_object_size;

            /* Check that the object is in the object pool.  */
            if (object_end >= object_pool_end)
            {
                /* Object is outside of the object pool.  */
                return(TXM_MODULE_INVALID_MEMORY);
            }
        }
    }

    return_value = (ALIGN_TYPE) _txm_module_manager_object_deallocate(
        (VOID *) param_0
    );
    return(return_value);
}
#endif

#ifndef TXM_MODULE_OBJECT_POINTER_GET_CALL_NOT_USED
/* UINT _txm_module_object_pointer_get(
    UINT object_type, -> param_0
    CHAR *name, -> param_1
    VOID **object_ptr -> param_2
   ); */
static ALIGN_TYPE _txm_module_manager_txm_module_object_pointer_get_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, param_2, sizeof(VOID *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txm_module_manager_object_pointer_get(
        (UINT) param_0,
        (CHAR *) param_1,
        (VOID **) param_2
    );
    return(return_value);
}
#endif

#ifndef TXM_MODULE_OBJECT_POINTER_GET_EXTENDED_CALL_NOT_USED
/* UINT _txm_module_object_pointer_get_extended(
    UINT object_type, -> param_0
    CHAR *name, -> param_1
    UINT name_length, -> extra_parameters[0]
    VOID **object_ptr -> extra_parameters[1]
   ); */
static ALIGN_TYPE _txm_module_manager_txm_module_object_pointer_get_extended_dispatch(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE *extra_parameters)
{

ALIGN_TYPE return_value;

    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
    {
        if (!TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, param_1))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, (ALIGN_TYPE)extra_parameters, sizeof(ALIGN_TYPE[2])))
            return(TXM_MODULE_INVALID_MEMORY);

        if (!TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, extra_parameters[1], sizeof(VOID *)))
            return(TXM_MODULE_INVALID_MEMORY);
    }

    return_value = (ALIGN_TYPE) _txm_module_manager_object_pointer_get_extended(
        (UINT) param_0,
        (CHAR *) param_1,
        (UINT) extra_parameters[0],
        (VOID **) extra_parameters[1]
    );
    return(return_value);
}
#endif
