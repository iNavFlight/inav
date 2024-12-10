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


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    txm_module_manager_util.h                           PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file declares prototypes of utility functions used by the      */
/*    module manager.                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  04-02-2021      Scott Larson            Modified comment(s) and       */
/*                                            optimized object checks,    */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/

#ifndef TXM_MODULE_MANAGER_UTIL_H
#define TXM_MODULE_MANAGER_UTIL_H

/* Define check macros for modules.  */

#define TXM_MODULE_MANAGER_CHECK_OUTSIDE_DATA(module_instance, obj_ptr, obj_size) \
    (!(TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size)))

#define TXM_MODULE_MANAGER_CHECK_INSIDE_CODE(module_instance, obj_ptr, obj_size) \
    (((obj_ptr) < ((obj_ptr) + (obj_size))) && \
     ((obj_ptr) >= (ALIGN_TYPE) module_instance -> txm_module_instance_code_start) && \
     (((obj_ptr) + (obj_size)) <= ((ALIGN_TYPE) module_instance -> txm_module_instance_code_end + 1)))

#define TXM_MODULE_MANAGER_CHECK_OUTSIDE_CODE(module_instance, obj_ptr, obj_size) \
    (!(TXM_MODULE_MANAGER_CHECK_INSIDE_CODE(module_instance, obj_ptr, obj_size)))

/* Add sizeof(TXM_MODULE_ALLOCATED_OBJECT) to pool start because the object can't exist before that. */
#define TXM_MODULE_MANAGER_CHECK_INSIDE_OBJ_POOL(module_instance, obj_ptr, obj_size) \
    ((_txm_module_manager_object_pool_created == TX_TRUE) && \
     ((obj_ptr) < ((obj_ptr) + (obj_size))) && \
     (((obj_ptr) >= ((ALIGN_TYPE) _txm_module_manager_object_pool.tx_byte_pool_start + sizeof(TXM_MODULE_ALLOCATED_OBJECT))) && \
      (((obj_ptr) + (obj_size)) <= (ALIGN_TYPE) (_txm_module_manager_object_pool.tx_byte_pool_start + _txm_module_manager_object_pool.tx_byte_pool_size))))

/* Define macros for module.  */

#define TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE(module_instance, obj_ptr, obj_size) \
    (TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size) || \
     TXM_MODULE_MANAGER_CHECK_INSIDE_CODE(module_instance, obj_ptr, obj_size))

#define TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, obj_ptr, obj_size) \
    TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size)

#define TXM_MODULE_MANAGER_ENSURE_OUTSIDE_MODULE(module_instance, obj_ptr, obj_size) \
    (TXM_MODULE_MANAGER_CHECK_OUTSIDE_DATA(module_instance, obj_ptr, obj_size) && \
     TXM_MODULE_MANAGER_CHECK_OUTSIDE_CODE(module_instance, obj_ptr, obj_size))

#define TXM_MODULE_MANAGER_ENSURE_INSIDE_OBJ_POOL(module_instance, obj_ptr, obj_size) \
    TXM_MODULE_MANAGER_CHECK_INSIDE_OBJ_POOL(module_instance, obj_ptr, obj_size)

/* Define macros for parameter types.  */

/* Buffers we read from can be in RW/RO/Shared areas.  */
#define TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_READ(module_instance, buffer_ptr, buffer_size) \
    ((TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE(module_instance, buffer_ptr, buffer_size)) || \
     ((void *) (buffer_ptr) == TX_NULL))

/* Buffers we write to can only be in RW/Shared areas.  */
#define TXM_MODULE_MANAGER_PARAM_CHECK_BUFFER_WRITE(module_instance, buffer_ptr, buffer_size) \
    ((TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE_DATA(module_instance, buffer_ptr, buffer_size)) || \
     ((void *) (buffer_ptr) == TX_NULL))

/* Kernel objects should be outside the module at the very least.  */
#define TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, obj_ptr, obj_size) \
    ((TXM_MODULE_MANAGER_ENSURE_OUTSIDE_MODULE(module_instance, obj_ptr, obj_size)) || \
     ((void *) (obj_ptr) == TX_NULL))

/* When creating an object, the object must be inside the object pool.  */
#define TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_CREATION(module_instance, obj_ptr, obj_size) \
    ((TXM_MODULE_MANAGER_ENSURE_INSIDE_OBJ_POOL(module_instance, obj_ptr, obj_size) && \
      (_txm_module_manager_object_size_check(obj_ptr, obj_size) == TX_SUCCESS)) || \
     ((void *) (obj_ptr) == TX_NULL))

/* Strings we dereference can be in RW/RO/Shared areas.  */
#define TXM_MODULE_MANAGER_PARAM_CHECK_DEREFERENCE_STRING(module_instance, string_ptr) \
    ((TXM_MODULE_MANAGER_ENSURE_INSIDE_MODULE(module_instance, string_ptr, 1)) || \
     ((void *) (string_ptr) == TX_NULL))

#define TXM_MODULE_MANAGER_UTIL_MAX_VALUE_OF_TYPE_UNSIGNED(type) ((1ULL << (sizeof(type) * 8)) - 1)

#define TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(augend, addend, result) \
    if ((ULONG)-1 - (augend) < (addend))                 \
    {                                                    \
        return(TXM_MODULE_MATH_OVERFLOW);                \
    }                                                    \
    else                                                 \
    {                                                    \
        (result) = (augend) + (addend);                  \
    }

/* Define utility functions.  */

UINT    _txm_module_manager_object_memory_check(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE object_ptr, ULONG object_size);
UINT    _txm_module_manager_object_size_check(ALIGN_TYPE object_ptr, ULONG object_size);
UINT    _txm_module_manager_object_name_compare(CHAR *object_name1, UINT object_name1_length, CHAR *object_name2);
UCHAR   _txm_module_manager_created_object_check(TXM_MODULE_INSTANCE *module_instance, void *object_ptr);
UINT    _txm_module_manager_util_code_allocation_size_and_alignment_get(TXM_MODULE_PREAMBLE *module_preamble, ULONG *code_alignment_dest, ULONG *code_allocation_size_dest);

#endif
