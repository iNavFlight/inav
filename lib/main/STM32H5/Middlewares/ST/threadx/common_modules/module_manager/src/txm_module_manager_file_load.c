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

#ifdef FX_FILEX_PRESENT

#define TX_SOURCE_CODE

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_mutex.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"
#include "fx_api.h"
#include "txm_module.h"
#include "txm_module_manager_util.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_file_load                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the module preamble, allocates memory for       */
/*    module code, loads the module code from the file, and calls         */
/*    _txm_module_manager_internal_load to load the data and prepare the  */
/*    module for execution.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance pointer           */
/*    module_name                       Name of module                    */
/*    media_ptr                         FileX media pointer               */
/*    file_name                         Name of module binary file        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    fx_file_close                     Close file                        */
/*    fx_file_open                      Open file                         */
/*    fx_file_read                      File read                         */
/*    fx_file_seek                      File seek                         */
/*    _tx_byte_allocate                 Allocate data area                */
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
UINT  _txm_module_manager_file_load(TXM_MODULE_INSTANCE *module_instance, CHAR *module_name, FX_MEDIA *media_ptr, CHAR *file_name)
{


TXM_MODULE_PREAMBLE     *module_preamble;
FX_FILE                 module_file;
TXM_MODULE_PREAMBLE     preamble;
ULONG                   code_start;
ULONG                   code_size;
ULONG                   code_alignment;
ULONG                   code_allocation_size;
CHAR                    *code_memory_ptr;
UCHAR                   *destination_ptr;
ULONG                   actual_size;
UINT                    status;


    /* Attempt to open the file.  */
    status =  fx_file_open(media_ptr, &module_file, file_name, FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status == FX_SUCCESS)
    {

        /* Read the preamble of the module.  */
        status =  fx_file_read(&module_file, (VOID *) &preamble, sizeof(TXM_MODULE_PREAMBLE), &actual_size);

        /* Check the file read status.  */
        if (status == FX_SUCCESS)
        {

            /* Check the number of bytes read.  */
            if (actual_size == sizeof(TXM_MODULE_PREAMBLE))
            {

                /* Pickup the module's information.  */
                module_preamble =  (TXM_MODULE_PREAMBLE *) &preamble;

                /* Pickup the module code size.  */
                code_size =  module_preamble -> txm_module_preamble_code_size;

                /* Check for valid sizes.  */
                if (code_size != 0)
                {

                    /* Initialize module control block to all zeros.  */
                    TX_MEMSET(module_instance, 0, sizeof(TXM_MODULE_INSTANCE));

                    /* Get the amount of the bytes we need to allocate for the module's code as well as the required alignment.  */
                    status =  _txm_module_manager_util_code_allocation_size_and_alignment_get(module_preamble, &code_alignment, &code_allocation_size);
                    if (status == TX_SUCCESS)
                    {

                        /* Allocate code memory for the module.  */
                        status =  _tx_byte_allocate(&_txm_module_manager_byte_pool, (VOID **) &code_memory_ptr, code_allocation_size, TX_NO_WAIT);

                        /* Determine if the module's code memory allocation was successful.  */
                        if (status == TX_SUCCESS)
                        {

                            /* Prepare to read the module code into memory.  */
                            code_start =       (ULONG) code_memory_ptr;
                            code_start =       (code_start + (code_alignment - 1)) & ~(code_alignment - 1);
                            destination_ptr =  (UCHAR *)  code_start;

                            /* Seek back to the beginning of the file.  */
                            status =  fx_file_seek(&module_file, 0);
                            if (status == FX_SUCCESS)
                            {

                                /* Read the module into memory.  */
                                status =  fx_file_read(&module_file, (VOID *) destination_ptr, code_size, &actual_size);
                                if (status == FX_SUCCESS)
                                {

                                    /* Check the actual size read.  */
                                    if (actual_size == code_size)
                                    {

                                        /* At this point, the module's instruction area is now in the RAM code area.  */

                                        /* Now load it in-place.  */
                                        status =  _txm_module_manager_internal_load(module_instance, module_name, (VOID *) code_start,
                                                                                    code_size, code_memory_ptr, code_allocation_size);
                                        if (status == TX_SUCCESS)
                                        {

                                            /* Close the file.  */
                                            fx_file_close(&module_file);

                                            /* Return success.  */
                                            return(TX_SUCCESS);
                                        }
                                    }
                                    else
                                    {

                                        /* Invalid number of bytes read.  */
                                        status =  TXM_MODULE_FILEX_INVALID_BYTES_READ;
                                    }
                                }
                            }

                            /* Release the memory.  */
                            _tx_byte_release(code_memory_ptr);
                        }
                    }
                }
                else
                {

                    /* Invalid module preamble.  */
                    status =  TXM_MODULE_INVALID;
                }
            }
            else
            {

                /* Invalid number of bytes read.  */
                status =  TXM_MODULE_FILEX_INVALID_BYTES_READ;
            }
        }

        /* Close the file.  */
        fx_file_close(&module_file);
    }

    /* Return success.  */
    return(status);
}

#endif
