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
/*    _txm_module_manager_properties_get                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the properties of the specified module so they*/
/*    may be checked before executing the module.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _txm_module_manager_properties_get(TXM_MODULE_INSTANCE *module_instance, ULONG *module_properties_ptr)
{

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

    /* Check the module ID.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Check for non-null buffer.  */
    if (module_properties_ptr == TX_NULL)
    {

        /* Invalid buffer pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Simply return the property bitmap.  */
    *module_properties_ptr =  module_instance -> txm_module_instance_property_flags;

    /* Return success.  */
    return(TX_SUCCESS);
}
