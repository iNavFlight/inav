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
#define TX_MODULE_MANAGER_INIT

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"
#include "tx_queue.h"
#include "tx_mutex.h"
#include "txm_module.h"


TXM_MODULE_MANAGER_VERSION_ID


/* Define global variables associated with the module manager.  */


/* Define the module properties supported by this module manager.  */

ULONG                   _txm_module_manager_properties_supported;


/* Define the module properties required by this module manager.   */

ULONG                   _txm_module_manager_properties_required;


/* Define byte pool that will be used for allocating module data areas.  */

TX_BYTE_POOL            _txm_module_manager_byte_pool;


/* Define byte pool that will be used for allocating external memory for module objects.  */

TX_BYTE_POOL            _txm_module_manager_object_pool;


/* Define the flag indicating that the module manager byte pool is created.  */

UINT                    _txm_module_manager_object_pool_created;


/* Define module manager protection mutex.  */

TX_MUTEX                _txm_module_manager_mutex;


/* Define the loaded modules list, which keeps track of all loaded modules.  */

TXM_MODULE_INSTANCE     *_txm_module_manager_loaded_list_ptr;


/* Define the count of loaded modules.  */

ULONG                   _txm_module_manger_loaded_count;


/* Define the ready flag, which is checked by other module manager APIs
   to make sure the manager has been initialized.  */

UINT                    _txm_module_manager_ready;


/* Define the total callback activation count. This is simply incremented on every
   callback activation.  */

ULONG                   _txm_module_manager_callback_total_count;


/* Define the callback activation error count. This occurs when the available callback
   structures have been exhausted.  */

ULONG                   _txm_module_manager_callback_error_count;



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_initialize                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the module manager.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_memory_start               Start of module area              */
/*    module_memory_size                Size in bytes of module area      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_byte_pool_create              Create module memory byte pool    */
/*    _tx_mutex_create                  Create module manager             */
/*                                        protection mutex                */
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
UINT  _txm_module_manager_initialize(VOID *module_memory_start, ULONG module_memory_size)
{

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

    /* Setup the module properties supported by this module manager.  */
    _txm_module_manager_properties_supported =  TXM_MODULE_MANAGER_SUPPORTED_OPTIONS;

    /* Setup the module properties required by this module manager.   */
    _txm_module_manager_properties_required =   TXM_MODULE_MANAGER_REQUIRED_OPTIONS;

    /* Clear the module manager ready flag.  */
    _txm_module_manager_ready =  TX_FALSE;

    /* Initialize the empty module list.  */
    _txm_module_manager_loaded_list_ptr =  TX_NULL;

    /* Clear the number of loaded modules.  */
    _txm_module_manger_loaded_count =  0;

    /* Create the module manager protection mutex.  */
    _tx_mutex_create(&_txm_module_manager_mutex, "Module Manager Protection Mutex", TX_NO_INHERIT);

    /* Create a byte pool for allocating RAM areas for modules.  */
    _tx_byte_pool_create(&_txm_module_manager_byte_pool, "Module Manager Byte Pool", module_memory_start, module_memory_size);

    /* Indicate the module manager object pool has not been created.  */
    _txm_module_manager_object_pool_created =  TX_FALSE;

    /* Mark the module manager as ready!  */
    _txm_module_manager_ready =  TX_TRUE;

    /* Return success.  */
    return(TX_SUCCESS);
}

