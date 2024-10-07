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

#include "txm_module.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_object_allocate                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates memory for an object from the memory pool   */
/*    supplied to txm_module_manager_initialize.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_ptr                        Destination of object pointer on  */
/*                                        successful allocation           */
/*    object_size                       Size in bytes of the object to be */
/*                                        allocated                       */
/*    module_instance                   The module instance that the      */
/*                                        object belongs to               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txe_mutex_get                        Get module instance mutex     */
/*    _txe_mutex_put                        Release module instance mutex */
/*    _txe_byte_allocate                    Allocate object from pool     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT _txm_module_manager_object_allocate(VOID **object_ptr_ptr, ULONG object_size, TXM_MODULE_INSTANCE *module_instance)
{

TXM_MODULE_ALLOCATED_OBJECT *object_ptr;
UINT                        return_value;


    /* Ensure the object pointer pointer is valid. */
    if (object_ptr_ptr == (VOID **) TX_NULL)
    {

        /* The object pointer pointer is invalid, return an error.  */
        return(TXM_MODULE_INVALID_MEMORY);
    }

    /* Initialize the return pointer to NULL.  */
    *((VOID **) object_ptr_ptr) =  TX_NULL;

    /* Get module manager protection mutex.  */
    _txe_mutex_get(&_txm_module_manager_mutex, TX_WAIT_FOREVER);

    /* Determine if an object pool was created.  */
    if (_txm_module_manager_object_pool_created)
    {

    TXM_MODULE_ALLOCATED_OBJECT   *next_object, *previous_object;

        /* Allocate the object requested by the module - adding an extra ULONG in order to
           store the module instance pointer.  */
        return_value =  (ULONG)  _txe_byte_allocate(&_txm_module_manager_object_pool, (VOID **) &object_ptr,
            (ULONG) (object_size + sizeof(TXM_MODULE_ALLOCATED_OBJECT)), TX_NO_WAIT);

        /* Determine if the request was successful.  */
        if (return_value == TX_SUCCESS)
        {
            /* Yes, now store the module instance in the allocated memory block.  */

            /* Link the allocated memory to the module instance.  */
            if (module_instance -> txm_module_instance_object_list_count++ == 0)
            {
                /* The allocated object list is empty.  Add object to empty list.  */
                module_instance -> txm_module_instance_object_list_head =  object_ptr;
                object_ptr -> txm_module_allocated_object_next =           object_ptr;
                object_ptr -> txm_module_allocated_object_previous =       object_ptr;
            }
            else
            {
                /* This list is not NULL, add to the end of the list.  */
                next_object =      module_instance -> txm_module_instance_object_list_head;
                previous_object =  next_object -> txm_module_allocated_object_previous;

                /* Place the new object in the list.  */
                next_object -> txm_module_allocated_object_previous =  object_ptr;
                previous_object -> txm_module_allocated_object_next =  object_ptr;

                /* Setup this object's allocated links.  */
                object_ptr -> txm_module_allocated_object_previous =  previous_object;
                object_ptr -> txm_module_allocated_object_next =      next_object;
            }

            /* Setup the module instance pointer in the allocated object.  */
            object_ptr -> txm_module_allocated_object_module_instance =  module_instance;

            /* Set the object size. */
            object_ptr -> txm_module_object_size =  object_size;

            /* Move the object pointer forward. This is what the module is given. */
            object_ptr++;

            /* Return this pointer to the application.  */
            *((VOID **) object_ptr_ptr) =  object_ptr;
        }
    }
    else
    {
        /* Set return value to not enabled.  */
        return_value =  TX_NOT_AVAILABLE;
    }

    /* Release the protection mutex.  */
    _txe_mutex_put(&_txm_module_manager_mutex);

    return(return_value);
}
