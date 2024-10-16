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


/* Include necessary system files.  */

#define TX_SOURCE_CODE

#include "txm_module.h"
#include "txm_module_manager_util.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_object_memory_check             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if the object is inside a module's object pool */
/*    or, if it's a privileged module, inside the module's data area.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance that the object   */
/*                                        belongs to                      */
/*    object_ptr                        Pointer to object to check        */
/*    object_size                       Size of the object to check       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Whether the object resides in a   */
/*                                        valid location                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _txm_module_manager_kernel_dispatch   Kernel dispatch function      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_object_memory_check(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE object_ptr, ULONG object_size)
{

    /* Is the object pointer from the module manager's object pool?  */
    if ((_txm_module_manager_object_pool_created == TX_TRUE) &&
        (object_ptr >= (ALIGN_TYPE) _txm_module_manager_object_pool.tx_byte_pool_start) &&
        ((object_ptr+object_size) <= (ALIGN_TYPE) (_txm_module_manager_object_pool.tx_byte_pool_start + _txm_module_manager_object_pool.tx_byte_pool_size)))
    {
        /* Object is from manager object pool.  */
        return(TX_SUCCESS);
    }

    /* If memory protection is not required, check if object is in module data.  */
    else if (!(module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION))
    {
        if ((object_ptr >= (ALIGN_TYPE) module_instance -> txm_module_instance_data_start) &&
        ((object_ptr+object_size) <= (ALIGN_TYPE) module_instance -> txm_module_instance_data_end))
        {
            /* Object is from the local module memory.  */
            return(TX_SUCCESS);
        }
    }

    /* Object is from invalid memory.  */
    return(TXM_MODULE_INVALID_MEMORY);

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_created_object_check            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This functions checks if the specified object was created by the    */
/*    specified module                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   The module instance to check      */
/*    object_ptr                        The object to check               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Whether the module created the    */
/*                                        object                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    txm_module_manager*_stop              Module manager stop functions */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UCHAR _txm_module_manager_created_object_check(TXM_MODULE_INSTANCE *module_instance, VOID *object_ptr)
{

TXM_MODULE_ALLOCATED_OBJECT     *allocated_object_ptr;

    /* Determine if the socket control block is inside the module.  */
    if ( (((CHAR *) object_ptr) >= ((CHAR *) module_instance -> txm_module_instance_data_start)) &&
         (((CHAR *) object_ptr) < ((CHAR *) module_instance -> txm_module_instance_data_end)))
    {
        return TX_TRUE;
    }

    /* Determine if this object control block was allocated by this module instance.  */
    else if (_txm_module_manager_object_pool_created)
    {

        /* Determine if the current object is from the pool of dynamically allocated objects.  */
        if ((((UCHAR *) object_ptr) >= _txm_module_manager_object_pool.tx_byte_pool_start) &&
            (((UCHAR *) object_ptr) < (_txm_module_manager_object_pool.tx_byte_pool_start + _txm_module_manager_object_pool.tx_byte_pool_size)))
        {

            /* Pickup object pointer.  */
            allocated_object_ptr =  (TXM_MODULE_ALLOCATED_OBJECT *) object_ptr;

            /* Move back to get the header information.  */
            allocated_object_ptr--;

            /* Now determine if this object belongs to this module.  */
            if (allocated_object_ptr -> txm_module_allocated_object_module_instance == module_instance)
            {
                return TX_TRUE;
            }
        }
    }

    return TX_FALSE;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_object_size_check               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if the specified object's size matches what is */
/*    inside the object pool.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_ptr                        Pointer to object to check        */
/*    object_size                       Size of the object to check       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Whether the object's size matches */
/*                                        what's inside the object pool   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _txm_module_manager_kernel_dispatch   Kernel dispatch function      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_object_size_check(ALIGN_TYPE object_ptr, ULONG object_size)
{
TXM_MODULE_ALLOCATED_OBJECT *module_allocated_object_ptr;
UINT                        return_value;

    /* Pickup the allocated object pointer.  */
    module_allocated_object_ptr = ((TXM_MODULE_ALLOCATED_OBJECT *) object_ptr) - 1;

    /* Does the allocated memory match the expected object size?  */
    if (module_allocated_object_ptr -> txm_module_object_size == object_size)
        return_value =  TX_SUCCESS;
    else
        return_value =  TXM_MODULE_INVALID_MEMORY;

    return(return_value);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_name_compare                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compares the specified object names.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    search_name                       String pointer to the object's    */
/*                                        name being searched for         */
/*    search_name_length                Length of search_name             */
/*    object_name                       String pointer to an object's name*/
/*                                        to compare the search name to   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Whether the names are equal       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    *_object_pointer_get                  Kernel dispatch function      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_object_name_compare(CHAR *search_name, UINT search_name_length, CHAR *object_name)
{

CHAR    search_name_char;
CHAR    object_name_char;


    /* Is the object name null? Note that the search name has already been checked
       by the caller.  */
    if (object_name == TX_NULL)
    {

        /* The strings can't match.  */
        return(TX_FALSE);
    }

    /* Loop through the names.  */
    while (1)
    {

        /* Get the current characters from each name.  */
        search_name_char =  *search_name;
        object_name_char =  *object_name;

        /* Check for match.  */
        if (search_name_char == object_name_char)
        {

            /* Are they null-terminators?  */
            if (search_name_char == '\0')
            {

                /* The strings match.  */
                return(TX_TRUE);
            }
        }
        else
        {

            /* The strings don't match.  */
            return(TX_FALSE);
        }

        /* Are we at the end of the search name?  */
        if (search_name_length == 0)
        {

            /* The strings don't match.  */
            return(TX_FALSE);
        }

        /* Move to next character.  */
        search_name++;
        object_name++;
        search_name_length--;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_util_code_allocation_size_and_alignment_get     */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the required alignment and allocation size    */
/*    for a module's code area.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_preamble                   Preamble of module to return code */
/*                                        values for                      */
/*    code_alignment_dest               Address to return code alignment  */
/*    code_allocation_size_desk         Address to return code allocation */
/*                                        size                            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Success if no math overflow       */
/*                                        occurred during calculation     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    txm_module_manager_*_load             Module load functions         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_util_code_allocation_size_and_alignment_get(TXM_MODULE_PREAMBLE *module_preamble,
                                                                      ULONG *code_alignment_dest, ULONG *code_allocation_size_dest)
{

ULONG   code_size;
ULONG   code_alignment;
ULONG   data_size_ignored;
ULONG   data_alignment_ignored;


    /* Pickup the module code size.  */
    code_size =  module_preamble -> txm_module_preamble_code_size;

    /* Adjust the size of the module elements to be aligned to the default alignment.  */
    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(code_size, TXM_MODULE_CODE_ALIGNMENT, code_size);
    code_size =  ((code_size - 1)/TXM_MODULE_CODE_ALIGNMENT) * TXM_MODULE_CODE_ALIGNMENT;

    /* Setup the default code and data alignments.  */
    code_alignment =  (ULONG) TXM_MODULE_CODE_ALIGNMENT;

    /* Get the port-specific alignment for the code size. Note we only want code so we pass 'null' values for data.  */
    data_size_ignored = 1;
    data_alignment_ignored = 1;
    TXM_MODULE_MANAGER_ALIGNMENT_ADJUST(module_preamble, code_size, code_alignment, data_size_ignored, data_alignment_ignored)

    /* Calculate the code memory allocation size.  */
    TXM_MODULE_MANAGER_UTIL_MATH_ADD_ULONG(code_size, code_alignment, *code_allocation_size_dest);

    /* Write the alignment result into the caller's destination address.  */
    *code_alignment_dest =  code_alignment;

    /* Return success.  */
    return(TX_SUCCESS);
}
