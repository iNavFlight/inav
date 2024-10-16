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
/**   Module                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    txm_module_port.h                                 Cortex-M33/GNU    */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic module constants, interface structures, */
/*    and function prototypes.                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-02-2021      Scott Larson            Initial Version 6.1.5         */
/*  01-31-2022      Scott Larson            Modified comment(s),          */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef TXM_MODULE_PORT_H
#define TXM_MODULE_PORT_H

/* Determine if the optional Modules user define file should be used.  */

#ifdef TXM_MODULE_INCLUDE_USER_DEFINE_FILE

/* Yes, include the user defines in txm_module_user.h. The defines in this file may
   alternately be defined on the command line.  */

#include "txm_module_user.h"
#endif

/* It is assumed that the base ThreadX tx_port.h file has been modified to add the
  following extensions to the ThreadX thread control block (this code should replace
  the corresponding macro define in tx_port.h):

#define TX_THREAD_EXTENSION_2                   VOID    *tx_thread_module_instance_ptr;         \
                                                VOID    *tx_thread_module_entry_info_ptr;       \
                                                ULONG   tx_thread_module_current_user_mode;     \
                                                ULONG   tx_thread_module_user_mode;             \
                                                ULONG   tx_thread_module_saved_lr;              \
                                                VOID    *tx_thread_module_kernel_stack_start;   \
                                                VOID    *tx_thread_module_kernel_stack_end;     \
                                                ULONG   tx_thread_module_kernel_stack_size;     \
                                                VOID    *tx_thread_module_stack_ptr;            \
                                                VOID    *tx_thread_module_stack_start;          \
                                                VOID    *tx_thread_module_stack_end;            \
                                                ULONG   tx_thread_module_stack_size;            \
                                                VOID    *tx_thread_module_reserved;

The following extensions must also be defined in tx_port.h:

#define TX_EVENT_FLAGS_GROUP_EXTENSION          VOID    *tx_event_flags_group_module_instance; \
                                                VOID   (*tx_event_flags_group_set_module_notify)(struct TX_EVENT_FLAGS_GROUP_STRUCT *group_ptr);

#define TX_QUEUE_EXTENSION                      VOID    *tx_queue_module_instance; \
                                                VOID   (*tx_queue_send_module_notify)(struct TX_QUEUE_STRUCT *queue_ptr);

#define TX_SEMAPHORE_EXTENSION                  VOID    *tx_semaphore_module_instance; \
                                                VOID   (*tx_semaphore_put_module_notify)(struct TX_SEMAPHORE_STRUCT *semaphore_ptr);

#define TX_TIMER_EXTENSION                      VOID    *tx_timer_module_instance; \
                                                VOID   (*tx_timer_module_expiration_function)(ULONG id);
*/

/* Define the kernel stack size for a module thread.  */
#ifndef TXM_MODULE_KERNEL_STACK_SIZE
#define TXM_MODULE_KERNEL_STACK_SIZE            768
#endif

/* Define constants specific to the tools the module can be built with for this particular modules port.  */

#define TXM_MODULE_IAR_COMPILER                 0x00000000
#define TXM_MODULE_RVDS_COMPILER                0x01000000
#define TXM_MODULE_GNU_COMPILER                 0x02000000
#define TXM_MODULE_COMPILER_MASK                0xFF000000
#define TXM_MODULE_OPTIONS_MASK                 0x000000FF


/* Define the properties for this particular module port.  */

#define TXM_MODULE_PORT_DISPATCH

#define TXM_MODULE_MEMORY_PROTECTION_ENABLED

#ifdef  TXM_MODULE_MEMORY_PROTECTION_ENABLED
#define TXM_MODULE_REQUIRE_ALLOCATED_OBJECT_MEMORY
#else
#define TXM_MODULE_REQUIRE_LOCAL_OBJECT_MEMORY
#endif

#define TXM_MODULE_USER_MODE                        0x00000001
#define TXM_MODULE_MEMORY_PROTECTION                0x00000002
#define TXM_MODULE_SHARED_EXTERNAL_MEMORY_ACCESS    0x00000004


/* Define the supported options for this module.   */

#define TXM_MODULE_MANAGER_SUPPORTED_OPTIONS    (TXM_MODULE_USER_MODE | TXM_MODULE_MEMORY_PROTECTION | TXM_MODULE_SHARED_EXTERNAL_MEMORY_ACCESS)
#define TXM_MODULE_MANAGER_REQUIRED_OPTIONS     0


/* Define offset adjustments according to the compiler used to build the module.  */

#define TXM_MODULE_IAR_SHELL_ADJUST             24
#define TXM_MODULE_IAR_START_ADJUST             28
#define TXM_MODULE_IAR_STOP_ADJUST              32
#define TXM_MODULE_IAR_CALLBACK_ADJUST          44

#define TXM_MODULE_RVDS_SHELL_ADJUST            0
#define TXM_MODULE_RVDS_START_ADJUST            0
#define TXM_MODULE_RVDS_STOP_ADJUST             0
#define TXM_MODULE_RVDS_CALLBACK_ADJUST         0

#define TXM_MODULE_GNU_SHELL_ADJUST             24
#define TXM_MODULE_GNU_START_ADJUST             28
#define TXM_MODULE_GNU_STOP_ADJUST              32
#define TXM_MODULE_GNU_CALLBACK_ADJUST          44


/* Define other module port-specific constants.  */

/* Define INLINE_DECLARE to inline for ARM compiler.  */

#define INLINE_DECLARE inline

/* Define the number of MPU entries assigned to the code and data sections.
   On Cortex-M33 devices, there are 8 total entries. ThreadX uses one for access
   to the kernel entry function, one for module code protection, one for module data protection,
   leaving five regions available for shared memory.  */
#define TXM_MODULE_MPU_TOTAL_ENTRIES            8
#define TXM_MODULE_MPU_KERNEL_ENTRY_INDEX       0
#define TXM_MODULE_MPU_CODE_INDEX               1
#define TXM_MODULE_MPU_DATA_INDEX               2

#define TXM_MODULE_MPU_SHARED_INDEX             3
#define TXM_MODULE_MPU_SHARED_ENTRIES           5

#define TXM_MODULE_ATTRIBUTE_NON_SHAREABLE      0x00
#define TXM_MODULE_ATTRIBUTE_OUTER_SHAREABLE    0x10
#define TXM_MODULE_ATTRIBUTE_INNER_SHAREABLE    0x18
#define TXM_MODULE_ATTRIBUTE_READ_WRITE         0x02
#define TXM_MODULE_ATTRIBUTE_READ_ONLY          0x06
#define TXM_MODULE_ATTRIBUTE_EXECUTE_NEVER      0x01
#define TXM_MODULE_ATTRIBUTE_REGION_ENABLE      0x01
#define TXM_MODULE_ATTRIBUTE_MASK               0x1E
#define TXM_MODULE_ATTRIBUTE_INDEX              0x00

/* MAIR_ATTR settings
Device-nGnRE : 0b [Outer]0000 [Inner]0100
Code WT   RA : 0b [Outer]1010 [Inner]1010
DATA WBWA RA : 0b [Outer]1111 [Inner]1111
*/

/* Data aligned to 8 bytes (stacks must be 8-byte aligned).  */
#define TXM_MODULE_DATA_ALIGNMENT               8

/* MPU regions must be 32-byte aligned.  */
#define TXM_MODULE_MPU_ALIGNMENT                32

#define TXM_THREAD_SECURE_STACK_ALLOCATE_CALL   TXM_MODULE_PORT_EXTENSION_API_ID_START
#define TXM_THREAD_SECURE_STACK_FREE_CALL       (TXM_MODULE_PORT_EXTENSION_API_ID_START+1)

/* There are 2 registers to set up each MPU region: MPU_RBAR, MPU_RLAR.  */
typedef struct TXM_MODULE_MPU_INFO_STRUCT
{
    ULONG   txm_module_mpu_region_base_address;
    ULONG   txm_module_mpu_region_limit_address;
} TXM_MODULE_MPU_INFO;

/* Define the port-extensions to the module manager instance structure.  */
#define TXM_MODULE_MANAGER_PORT_EXTENSION                                                               \
    TXM_MODULE_MPU_INFO     txm_module_instance_mpu_registers[TXM_MODULE_MPU_TOTAL_ENTRIES];            \
    ULONG                   txm_module_instance_shared_memory_count;                                    \
    ULONG                   txm_module_instance_shared_memory_address[TXM_MODULE_MPU_SHARED_ENTRIES];   \
    ULONG                   txm_module_instance_shared_memory_length[TXM_MODULE_MPU_SHARED_ENTRIES];


/* Define the memory fault information structure that is populated when a memory fault occurs.  */

typedef struct TXM_MODULE_MANAGER_MEMORY_FAULT_INFO_STRUCT
{
    TX_THREAD           *txm_module_manager_memory_fault_info_thread_ptr;
    VOID                *txm_module_manager_memory_fault_info_code_location;
    ULONG               txm_module_manager_memory_fault_info_shcsr;
    ULONG               txm_module_manager_memory_fault_info_cfsr;
    ULONG               txm_module_manager_memory_fault_info_mmfar;
    ULONG               txm_module_manager_memory_fault_info_bfar;
    ULONG               txm_module_manager_memory_fault_info_control;
    ULONG               txm_module_manager_memory_fault_info_sp;
    ULONG               txm_module_manager_memory_fault_info_r0;
    ULONG               txm_module_manager_memory_fault_info_r1;
    ULONG               txm_module_manager_memory_fault_info_r2;
    ULONG               txm_module_manager_memory_fault_info_r3;
    ULONG               txm_module_manager_memory_fault_info_r4;
    ULONG               txm_module_manager_memory_fault_info_r5;
    ULONG               txm_module_manager_memory_fault_info_r6;
    ULONG               txm_module_manager_memory_fault_info_r7;
    ULONG               txm_module_manager_memory_fault_info_r8;
    ULONG               txm_module_manager_memory_fault_info_r9;
    ULONG               txm_module_manager_memory_fault_info_r10;
    ULONG               txm_module_manager_memory_fault_info_r11;
    ULONG               txm_module_manager_memory_fault_info_r12;
    ULONG               txm_module_manager_memory_fault_info_lr;
    ULONG               txm_module_manager_memory_fault_info_xpsr;
} TXM_MODULE_MANAGER_MEMORY_FAULT_INFO;


#define TXM_MODULE_MANAGER_FAULT_INFO                                               \
    TXM_MODULE_MANAGER_MEMORY_FAULT_INFO    _txm_module_manager_memory_fault_info;


/* Define the macro to check the code alignment.  */

#define TXM_MODULE_MANAGER_CHECK_CODE_ALIGNMENT(module_location, code_alignment)    \
    {                                                                               \
        ULONG   temp;                                                               \
        temp =  (ULONG) module_location;                                            \
        temp =  temp & (code_alignment - 1);                                        \
        if (temp)                                                                   \
        {                                                                           \
            _tx_mutex_put(&_txm_module_manager_mutex);                              \
            return(TXM_MODULE_ALIGNMENT_ERROR);                                     \
        }                                                                           \
    }


/* Define the macro to adjust the alignment and size for code/data areas.  */

#define TXM_MODULE_MANAGER_ALIGNMENT_ADJUST(module_preamble, code_size, code_alignment, data_size, data_alignment)   _txm_module_manager_alignment_adjust(module_preamble, &code_size, &code_alignment, &data_size, &data_alignment);


/* Define the macro to adjust the symbols in the module preamble.  */

#define TXM_MODULE_MANAGER_CALCULATE_ADJUSTMENTS(properties, shell_function_adjust, start_function_adjust, stop_function_adjust, callback_function_adjust) \
    if ((properties & TXM_MODULE_COMPILER_MASK) == TXM_MODULE_IAR_COMPILER)         \
    {                                                                               \
        shell_function_adjust =     TXM_MODULE_IAR_SHELL_ADJUST;                    \
        start_function_adjust =     TXM_MODULE_IAR_START_ADJUST;                    \
        stop_function_adjust =      TXM_MODULE_IAR_STOP_ADJUST;                     \
        callback_function_adjust =  TXM_MODULE_IAR_CALLBACK_ADJUST;                 \
    }                                                                               \
    else if ((properties & TXM_MODULE_COMPILER_MASK) == TXM_MODULE_RVDS_COMPILER)   \
    {                                                                               \
        shell_function_adjust =     TXM_MODULE_RVDS_SHELL_ADJUST;                   \
        start_function_adjust =     TXM_MODULE_RVDS_START_ADJUST;                   \
        stop_function_adjust =      TXM_MODULE_RVDS_STOP_ADJUST;                    \
        callback_function_adjust =  TXM_MODULE_RVDS_CALLBACK_ADJUST;                \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        shell_function_adjust =     TXM_MODULE_GNU_SHELL_ADJUST;                    \
        start_function_adjust =     TXM_MODULE_GNU_START_ADJUST;                    \
        stop_function_adjust =      TXM_MODULE_GNU_STOP_ADJUST;                     \
        callback_function_adjust =  TXM_MODULE_GNU_CALLBACK_ADJUST;                 \
    }


/* Define the macro to populate the thread control block with module port-specific information.
   Check if the module is in user mode and set up txm_module_thread_entry_info_kernel_call_dispatcher accordingly.
*/

#define TXM_MODULE_MANAGER_THREAD_SETUP(thread_ptr, module_instance)                                                                    \
    thread_ptr -> tx_thread_module_current_user_mode =  module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE;   \
    thread_ptr -> tx_thread_module_user_mode =          module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE;   \
    if (thread_ptr -> tx_thread_module_user_mode)                                                                                       \
    {                                                                                                                                   \
        thread_entry_info -> txm_module_thread_entry_info_kernel_call_dispatcher =   _txm_module_manager_user_mode_entry;               \
    }                                                                                                                                   \
    else                                                                                                                                \
    {                                                                                                                                   \
        thread_entry_info -> txm_module_thread_entry_info_kernel_call_dispatcher =   _txm_module_manager_kernel_dispatch;               \
    }


/* Define the macro to populate the module control block with module port-specific information.
   If memory protection is enabled, set up the MPU registers.
*/
#define TXM_MODULE_MANAGER_MODULE_SETUP(module_instance)                                            \
    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_USER_MODE)               \
    {                                                                                               \
        if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)   \
        {                                                                                           \
            _txm_module_manager_mm_register_setup(module_instance);                                 \
        }                                                                                           \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        /* Do nothing.  */                                                                          \
    }

/* Define the macro to perform port-specific functions when unloading the module.  */
/* Nothing needs to be done for this port.  */
#define TXM_MODULE_MANAGER_MODULE_UNLOAD(module_instance)


/* Define the macros to perform port-specific checks when passing pointers to the kernel.  */

/* Define macro to make sure object is inside the module's data.  */
#define TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size) \
    _txm_module_manager_inside_data_check(module_instance, obj_ptr, obj_size)

/* Define some internal prototypes to this module port.  */

#ifndef TX_SOURCE_CODE
#define txm_module_manager_memory_fault_notify              _txm_module_manager_memory_fault_notify
#endif


#ifdef TX_SOURCE_CODE

#endif

#define TXM_MODULE_MANAGER_ADDITIONAL_PROTOTYPES                                                                                \
VOID  _txm_module_manager_alignment_adjust(TXM_MODULE_PREAMBLE *module_preamble, ULONG *code_size, ULONG *code_alignment, ULONG *data_size, ULONG *data_alignment);   \
VOID  _txm_module_manager_memory_fault_handler(VOID);                                                                           \
UINT  _txm_module_manager_memory_fault_notify(VOID (*notify_function)(TX_THREAD *, TXM_MODULE_INSTANCE *));                     \
VOID  _txm_module_manager_mm_register_setup(TXM_MODULE_INSTANCE *module_instance);                                              \
UINT  _txm_module_manager_inside_data_check(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE obj_ptr, UINT obj_size);           \
ALIGN_TYPE _txm_module_manager_port_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2);


#define TXM_MODULE_MANAGER_VERSION_ID   \
CHAR                            _txm_module_manager_version_id[] =  \
                                    "Copyright (c) Microsoft Corporation. All rights reserved. * ThreadX Module Cortex-M33/GNU Version 6.1.10 *";

#endif
