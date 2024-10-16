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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_kernel_setup                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called by the compiler's startup code to make      */
/*    ThreadX objects accessible to the compiler's library.  If this      */
/*    function is not called by the compiler, all ThreadX initialization  */
/*    takes place from the kernel enter function defined previously.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_initialize_low_level          Low-level initialization          */
/*    _tx_initialize_high_level         High-level initialization         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    startup code                      Compiler startup code             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_initialize_kernel_setup(VOID)
{

    /* Ensure that the system state variable is set to indicate
       initialization is in progress.  Note that this variable is
       later used to represent interrupt nesting.  */
    _tx_thread_system_state =  TX_INITIALIZE_IN_PROGRESS;

    /* Call any port specific preprocessing.  */
    TX_PORT_SPECIFIC_PRE_INITIALIZATION

    /* Invoke the low-level initialization to handle all processor specific
       initialization issues.  */
    _tx_initialize_low_level();

    /* Invoke the high-level initialization to exercise all of the
       ThreadX components and the application's initialization
       function.  */
    _tx_initialize_high_level();

    /* Call any port specific post-processing.  */
    TX_PORT_SPECIFIC_POST_INITIALIZATION

    /* Set the system state to indicate initialization is almost done.  */
    _tx_thread_system_state =  TX_INITIALIZE_ALMOST_DONE;
}

