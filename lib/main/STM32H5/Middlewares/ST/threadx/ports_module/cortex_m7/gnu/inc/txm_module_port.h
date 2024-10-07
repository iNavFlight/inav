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
/*    txm_module_port.h                                 Cortex-M7/GNU     */
/*                                                           6.1.12       */
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
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*  07-29-2022      Scott Larson            Enabled user-defined and      */
/*                                            default MPU settings,       */
/*                                            resulting in version 6.1.12 */
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

#define TX_THREAD_EXTENSION_2               VOID    *tx_thread_module_instance_ptr;         \
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

/* For the following 3 access control settings, change TEX and C, B, S (bits 21 through 16 of MPU_RASR)
 * to reflect your system memory attributes (cache, shareable, memory type).  */
/* Code region access control: privileged read-only, outer & inner write-back, normal memory, shareable.  */
#ifndef TXM_MODULE_MPU_CODE_ACCESS_CONTROL
#define TXM_MODULE_MPU_CODE_ACCESS_CONTROL      0x06070000
#endif
/* Data region access control: execute never, read/write, outer & inner write-back, normal memory, shareable.  */
#ifndef TXM_MODULE_MPU_DATA_ACCESS_CONTROL
#define TXM_MODULE_MPU_DATA_ACCESS_CONTROL      0x13070000
#endif
/* Shared region access control: execute never, read-only, outer & inner write-back, normal memory, shareable.  */
#ifndef TXM_MODULE_MPU_SHARED_ACCESS_CONTROL
#define TXM_MODULE_MPU_SHARED_ACCESS_CONTROL    0x12070000
#endif

/* For Cortex-M devices with 16 MPU regions, the last four regions (12-15)
   are not used by ThreadX. These may be defined by the user.  */
#define TXM_MODULE_MPU_USER_DEFINED_RBAR_12     0
#define TXM_MODULE_MPU_USER_DEFINED_RASR_12     0
#define TXM_MODULE_MPU_USER_DEFINED_RBAR_13     0
#define TXM_MODULE_MPU_USER_DEFINED_RASR_13     0
#define TXM_MODULE_MPU_USER_DEFINED_RBAR_14     0
#define TXM_MODULE_MPU_USER_DEFINED_RASR_14     0
#define TXM_MODULE_MPU_USER_DEFINED_RBAR_15     0
#define TXM_MODULE_MPU_USER_DEFINED_RASR_15     0


/* Users can define these default MPU configuration values.

   If TXM_MODULE_MPU_DEFAULT is *not* defined, the MPU is disabled
   when a thread that is not owned by a module is running
   and the defines below are not used.

   If TXM_MODULE_MPU_DEFAULT is defined, the MPU is configured to the
   below values when a thread that is not owned by a module is running.  */
#define TXM_MODULE_MPU_DEFAULT_RBAR_0           0
#define TXM_MODULE_MPU_DEFAULT_RASR_0           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_1           0
#define TXM_MODULE_MPU_DEFAULT_RASR_1           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_2           0
#define TXM_MODULE_MPU_DEFAULT_RASR_2           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_3           0
#define TXM_MODULE_MPU_DEFAULT_RASR_3           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_4           0
#define TXM_MODULE_MPU_DEFAULT_RASR_4           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_5           0
#define TXM_MODULE_MPU_DEFAULT_RASR_5           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_6           0
#define TXM_MODULE_MPU_DEFAULT_RASR_6           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_7           0
#define TXM_MODULE_MPU_DEFAULT_RASR_7           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_8           0
#define TXM_MODULE_MPU_DEFAULT_RASR_8           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_9           0
#define TXM_MODULE_MPU_DEFAULT_RASR_9           0
#define TXM_MODULE_MPU_DEFAULT_RBAR_10          0
#define TXM_MODULE_MPU_DEFAULT_RASR_10          0
#define TXM_MODULE_MPU_DEFAULT_RBAR_11          0
#define TXM_MODULE_MPU_DEFAULT_RASR_11          0
#define TXM_MODULE_MPU_DEFAULT_RBAR_12          0
#define TXM_MODULE_MPU_DEFAULT_RASR_12          0
#define TXM_MODULE_MPU_DEFAULT_RBAR_13          0
#define TXM_MODULE_MPU_DEFAULT_RASR_13          0
#define TXM_MODULE_MPU_DEFAULT_RBAR_14          0
#define TXM_MODULE_MPU_DEFAULT_RASR_14          0
#define TXM_MODULE_MPU_DEFAULT_RBAR_15          0
#define TXM_MODULE_MPU_DEFAULT_RASR_15          0


/* Define constants specific to the tools the module can be built with for this particular modules port.  */

#define TXM_MODULE_IAR_COMPILER                 0x00000000
#define TXM_MODULE_RVDS_COMPILER                0x01000000
#define TXM_MODULE_GNU_COMPILER                 0x02000000
#define TXM_MODULE_COMPILER_MASK                0xFF000000
#define TXM_MODULE_OPTIONS_MASK                 0x000000FF


/* Define the properties for this particular module port.  */

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

/* Define INLINE_DECLARE to inline for GNU compiler.  */

#define INLINE_DECLARE inline

#ifdef TXM_MODULE_MANAGER_16_MPU

/* Define the number of MPU entries assigned to the code and data sections.
   On some Cortex-M7 parts, there are 16 total entries. ThreadX uses one for access
   to the kernel entry function, thus 15 remain for code and data protection.  */
#define TXM_MODULE_MPU_TOTAL_ENTRIES        16
#define TXM_MODULE_MPU_CODE_ENTRIES         4
#define TXM_MODULE_MPU_DATA_ENTRIES         4
#define TXM_MODULE_MPU_SHARED_ENTRIES       3

#define TXM_MODULE_MPU_KERNEL_ENTRY_INDEX   0
#define TXM_MODULE_MPU_SHARED_INDEX         9

#define TXM_ENABLE_REGION                   0x01

/* There are 2 registers to set up each MPU region: MPU_RBAR, MPU_RASR.  */
typedef struct TXM_MODULE_MPU_INFO_STRUCT
{
    ULONG   txm_module_mpu_region_address;
    ULONG   txm_module_mpu_region_attribute_size;
} TXM_MODULE_MPU_INFO;
/* Shared memory region attributes.  */
#define TXM_MODULE_MANAGER_SHARED_ATTRIBUTE_WRITE   1
#define TXM_MODULE_MANAGER_ATTRIBUTE_WRITE_MPU_BIT  0x01000000

/* Define the port-extensions to the module manager instance structure.  */

#define TXM_MODULE_MANAGER_PORT_EXTENSION                                                               \
    TXM_MODULE_MPU_INFO     txm_module_instance_mpu_registers[TXM_MODULE_MPU_TOTAL_ENTRIES];            \
    ULONG                   txm_module_instance_shared_memory_count;                                    \
    ULONG                   txm_module_instance_shared_memory_address[TXM_MODULE_MPU_SHARED_ENTRIES];   \
    ULONG                   txm_module_instance_shared_memory_length[TXM_MODULE_MPU_SHARED_ENTRIES];

#else   /* TXM_MODULE_MANAGER_16_MPU is not defined */

/* Define the number of MPU entries assigned to the code and data sections.
   On Cortex-M3, M4, and some M7 parts, there are 8 total entries. ThreadX uses one for access
   to the kernel entry function, thus 7 remain for code and data protection.  */
#define TXM_MODULE_MANAGER_CODE_MPU_ENTRIES     4
#define TXM_MODULE_MANAGER_DATA_MPU_ENTRIES     3
#define TXM_MODULE_MANAGER_SHARED_MPU_INDEX     8
#define TXM_MODULE_MANAGER_SHARED_MPU_REGION    4

/* Shared memory region attributes.  */
#define TXM_MODULE_MANAGER_SHARED_ATTRIBUTE_WRITE   1
#define TXM_MODULE_MANAGER_ATTRIBUTE_WRITE_MPU_BIT  0x01000000

/* Define the port-extensions to the module manager instance structure.  */

#define TXM_MODULE_MANAGER_PORT_EXTENSION                                           \
    ULONG               txm_module_instance_mpu_registers[16];                      \
    ULONG               txm_module_instance_shared_memory_address;                  \
    ULONG               txm_module_instance_shared_memory_length;

#endif  /* TXM_MODULE_MANAGER_16_MPU */

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
#ifdef TXM_MODULE_MANAGER_16_MPU
#define TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size) \
    _txm_module_manager_inside_data_check(module_instance, obj_ptr, obj_size)
#else
#define TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size) \
    /* Check for overflow. */   \
    (((obj_ptr) < ((obj_ptr) + (obj_size))) && \
    /* Check if it's inside module data.  */ \
     ((((obj_ptr) >= (ALIGN_TYPE) module_instance -> txm_module_instance_data_start) && \
     (((obj_ptr) + (obj_size)) <= ((ALIGN_TYPE) module_instance -> txm_module_instance_data_end + 1))) || \
    /* Check if it's inside shared memory.  */ \
     (((obj_ptr) >= (ALIGN_TYPE) module_instance -> txm_module_instance_shared_memory_address) && \
     (((obj_ptr) + (obj_size)) <= (ALIGN_TYPE) (module_instance -> txm_module_instance_shared_memory_address + module_instance -> txm_module_instance_shared_memory_length)))))
#endif

/* Define some internal prototypes to this module port.  */

#ifndef TX_SOURCE_CODE
#define txm_module_manager_memory_fault_notify              _txm_module_manager_memory_fault_notify
#endif


#define TXM_MODULE_MANAGER_ADDITIONAL_PROTOTYPES                                                                                \
VOID  _txm_module_manager_alignment_adjust(TXM_MODULE_PREAMBLE *module_preamble, ULONG *code_size, ULONG *code_alignment, ULONG *data_size, ULONG *data_alignment);   \
VOID  _txm_module_manager_memory_fault_handler(VOID);                                                                           \
UINT  _txm_module_manager_memory_fault_notify(VOID (*notify_function)(TX_THREAD *, TXM_MODULE_INSTANCE *));                     \
VOID  _txm_module_manager_mm_register_setup(TXM_MODULE_INSTANCE *module_instance);                                              \
ULONG _txm_power_of_two_block_size(ULONG size);                                                                                 \
ULONG _txm_module_manager_calculate_srd_bits(ULONG block_size, ULONG length);                                                   \
ULONG _txm_module_manager_region_size_get(ULONG block_size);                                                                    \
UINT  _txm_module_manager_inside_data_check(TXM_MODULE_INSTANCE *module_instance, ALIGN_TYPE obj_ptr, UINT obj_size);

#define TXM_MODULE_MANAGER_VERSION_ID   \
CHAR                            _txm_module_manager_version_id[] =  \
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  ThreadX Module Cortex-M7/GNU Version 6.1.12 *";

#endif
