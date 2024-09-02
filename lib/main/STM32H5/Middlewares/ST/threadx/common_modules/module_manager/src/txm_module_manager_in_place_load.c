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
/*    _txm_module_manager_in_place_load                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function ensures the code-related parts of the module preamble */
/*    are valid and calls _txm_module_manager_internal_load to load the   */
/*    data and prepare the module for execution.                          */
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
/*    _txm_module_manager_internal_load Load data and prepare module for  */
/*                                        execution                       */
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
UINT  _txm_module_manager_in_place_load(TXM_MODULE_INSTANCE *module_instance, CHAR *module_name, VOID *module_location)
{

TXM_MODULE_PREAMBLE     *module_preamble;
ULONG                   code_size;
ULONG                   code_alignment;
ULONG                   code_allocation_size_ignored;
UINT                    status;


    /* Pickup the module's information.  */
    module_preamble =  (TXM_MODULE_PREAMBLE *) module_location;

    /* Pickup the basic module sizes.  */
    code_size =  module_preamble -> txm_module_preamble_code_size;

    /* Check for valid sizes.  */
    if (code_size == 0)
    {

        /* Invalid module preamble.  */
        return(TXM_MODULE_INVALID);
    }

    /* Get the amount of the bytes we need to allocate for the module's code
       as well as the required alignment. Note that because this is an in-place
       load, we only want the code alignment so we can check it.  */
    status =  _txm_module_manager_util_code_allocation_size_and_alignment_get(module_preamble, &code_alignment, &code_allocation_size_ignored);
    if (status != TX_SUCCESS)
    {

        /* Math overflow error occurred.  */
        return(status);
    }

    /* Since this is an in-place load, check the alignment of the module's instruction area (code).  */
    TXM_MODULE_MANAGER_CHECK_CODE_ALIGNMENT(module_location, code_alignment)

    /* Now load the module in-place.  */
    status =  _txm_module_manager_internal_load(module_instance, module_name, module_location,
                                                code_size, TX_NULL, 0);

    /* Return status.  */
    return(status);
}
