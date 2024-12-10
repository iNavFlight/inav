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
/**   Module Interface (API)                                              */ 
/**                                                                       */ 
/**************************************************************************/ 
/**************************************************************************/ 


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    txm_module_port.h                               Cortex-A7/MMU/AC5   */ 
/*                                                           6.1          */
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
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef TXM_MODULE_PORT_H
#define TXM_MODULE_PORT_H

/* It is assumed that the base ThreadX tx_port.h file has been modified to add the 
  following extensions to the ThreadX thread control block (this code should replace
  the corresponding macro define in tx_port.h):

#define TX_THREAD_EXTENSION_2                   ULONG   tx_thread_vfp_enable;                   \
                                                VOID    *tx_thread_module_instance_ptr;         \
                                                VOID    *tx_thread_module_entry_info_ptr;       \
                                                ULONG   tx_thread_module_current_user_mode;     \
                                                ULONG   tx_thread_module_user_mode;             \
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
#define TXM_MODULE_KERNEL_STACK_SIZE            512
#endif

/* Defined, this option enables the MMU hardware and requires memory protected
   module objects to be allocated from the module manager object pool. 
   If this is undefined, module objects can be created in the module's data area 
   or in the module manager object pool. If this is not defined (MMU hardware 
   is disabled), a module requiring memory protection will not run (the load 
   functions will return a TXM_MODULE_INVALID_PROPERTIES error).
   Default setting for this value is defined.  */
#define TXM_MODULE_MEMORY_PROTECTION_ENABLED

/* Define constants specific to the tools the module can be built with for this particular modules port.  */

#define TXM_MODULE_IAR_COMPILER                 0x00000000
#define TXM_MODULE_RVDS_COMPILER                0x01000000
#define TXM_MODULE_GNU_COMPILER                 0x02000000
#define TXM_MODULE_COMPILER_MASK                0xFF000000
#define TXM_MODULE_OPTIONS_MASK                 0x000000FF


/* Define the properties for this particular module port.  */
#ifdef TXM_MODULE_MEMORY_PROTECTION_ENABLED
#define TXM_MODULE_MEMORY_PROTECTION            0x00000001
#else
#define TXM_MODULE_MEMORY_PROTECTION            0x00000000
#endif

#define TXM_MODULE_USER_MODE                    0x00000001

/* Define the supported options for this module.   */

#define TXM_MODULE_MANAGER_SUPPORTED_OPTIONS    (TXM_MODULE_MEMORY_PROTECTION)
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

/* Define INLINE_DECLARE to whitespace for ARM compiler.  */
#define INLINE_DECLARE

#define TXM_MAXIMUM_MODULES                     16
#define TXM_MODULE_LEVEL1_PAGE_TABLE_SIZE       32
#define TXM_ASID_TABLE_LENGTH                   256

#define TXM_MODULE_CODE_PAGE_TABLE_START_OFFSET (TXM_MAXIMUM_MODULES * 0)
#define TXM_MODULE_CODE_PAGE_TABLE_END_OFFSET   (TXM_MAXIMUM_MODULES * 1)
#define TXM_MODULE_DATA_PAGE_TABLE_START_OFFSET (TXM_MAXIMUM_MODULES * 2)
#define TXM_MODULE_DATA_PAGE_TABLE_END_OFFSET   (TXM_MAXIMUM_MODULES * 3)

#define TXM_MASTER_PAGE_TABLE_INDEX             0

/* 1 entry per 1MB, so this covers 4G address space */
#define TXM_MASTER_PAGE_TABLE_ENTRIES           4096

/* Smallest MMU page size is 4kB.  */
#define TXM_MODULE_MEMORY_ALIGNMENT             4096
#define TXM_MMU_LEVEL1_PAGE_SHIFT               20
#define TXM_MMU_LEVEL2_PAGE_SHIFT               12
#define TXM_LEVEL_2_PAGE_TABLE_ENTRIES          256

/* Level 1 section base address mask. */
#define TXM_MMU_LEVEL1_MASK                     0xFFF00000

/* Level 2 section base address mask. */
#define TXM_MMU_LEVEL2_MASK                     0xFFFFF000

/* Non-global, outer & inner write-back, write-allocate, user read, no write.  */
#define TXM_MMU_LEVEL1_CODE_ATTRIBUTES          0x000219EE
/* Non-global, outer & inner write-back, write-allocate, user read, write, no-execute.  */
#define TXM_MMU_LEVEL1_DATA_ATTRIBUTES          0x00021DFE

/* Level 1 "level 2 descriptor base address" mask. */
#define TXM_MMU_LEVEL1_SECOND_MASK              0xFFFFFC00

/* Level 1 "level 2 descriptor" attributes.  */
#define TXM_MMU_LEVEL1_SECOND_ATTRIBUTES        0x0000001E1

/* Kernel level 2 attributes: global, outer & inner write-back, write-allocate, user read/write */
#define TXM_MMU_KERNEL_LEVEL2_CODE_ATTRIBUTES   0x0000006E
#define TXM_MMU_KERNEL_LEVEL2_DATA_ATTRIBUTES   0x0000005E

/* Module level 2 attributes: non-global, outer & inner write-back, write-allocate, user read, no write. */
#define TXM_MMU_LEVEL2_CODE_ATTRIBUTES          0x0000086E
#define TXM_MMU_LEVEL2_DATA_ATTRIBUTES          0x0000087F


/* Settings the user can use to set up shared memory attributes.  */
#define TXM_MMU_ATTRIBUTE_XN                    0x00000001
#define TXM_MMU_ATTRIBUTE_B                     0x00000002
#define TXM_MMU_ATTRIBUTE_C                     0x00000004
#define TXM_MMU_ATTRIBUTE_AP                    0x00000018
#define TXM_MMU_ATTRIBUTE_TEX                   0x000000E0

/* Masks for each attribute.  */
#define TXM_MMU_ATTRIBUTE_XN_MASK               0x00000001
#define TXM_MMU_ATTRIBUTE_B_MASK                0x00000001
#define TXM_MMU_ATTRIBUTE_C_MASK                0x00000001
#define TXM_MMU_ATTRIBUTE_AP_MASK               0x00000003
#define TXM_MMU_ATTRIBUTE_TEX_MASK              0x00000007

/* Shift amounts for bitfields above to correct register locations.  */
#define TXM_MMU_LEVEL1_USER_ATTRIBUTE_XN_SHIFT  4
#define TXM_MMU_LEVEL1_USER_ATTRIBUTE_B_SHIFT   1
#define TXM_MMU_LEVEL1_USER_ATTRIBUTE_C_SHIFT   1
#define TXM_MMU_LEVEL1_USER_ATTRIBUTE_AP_SHIFT  7
#define TXM_MMU_LEVEL1_USER_ATTRIBUTE_TEX_SHIFT 7
#define TXM_MMU_LEVEL1_USER_ATTRIBUTE_BASE      0x000201E2

#define TXM_MMU_LEVEL2_USER_ATTRIBUTE_XN_SHIFT  0
#define TXM_MMU_LEVEL2_USER_ATTRIBUTE_B_SHIFT   1
#define TXM_MMU_LEVEL2_USER_ATTRIBUTE_C_SHIFT   1
#define TXM_MMU_LEVEL2_USER_ATTRIBUTE_AP_SHIFT  1
#define TXM_MMU_LEVEL2_USER_ATTRIBUTE_TEX_SHIFT 1
#define TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE      0x00000802

/* Shift amounts from bit 0 position.  */
#define TXM_MMU_LEVEL1_ATTRIBUTE_XN_SHIFT       4
#define TXM_MMU_LEVEL1_ATTRIBUTE_B_SHIFT        2
#define TXM_MMU_LEVEL1_ATTRIBUTE_C_SHIFT        3
#define TXM_MMU_LEVEL1_ATTRIBUTE_AP_SHIFT       10
#define TXM_MMU_LEVEL1_ATTRIBUTE_TEX_SHIFT      12

#define TXM_MMU_LEVEL2_ATTRIBUTE_XN_SHIFT       0
#define TXM_MMU_LEVEL2_ATTRIBUTE_B_SHIFT        2
#define TXM_MMU_LEVEL2_ATTRIBUTE_C_SHIFT        3
#define TXM_MMU_LEVEL2_ATTRIBUTE_AP_SHIFT       4
#define TXM_MMU_LEVEL2_ATTRIBUTE_TEX_SHIFT      6

/* Masks for L1 page attributes.  */
#define TXM_MMU_LEVEL1_ATTRIBUTE_XN_MASK        (TXM_MMU_ATTRIBUTE_XN_MASK  << TXM_MMU_LEVEL1_ATTRIBUTE_XN_SHIFT)
#define TXM_MMU_LEVEL1_ATTRIBUTE_B_MASK         (TXM_MMU_ATTRIBUTE_B_MASK   << TXM_MMU_LEVEL1_ATTRIBUTE_B_SHIFT)
#define TXM_MMU_LEVEL1_ATTRIBUTE_C_MASK         (TXM_MMU_ATTRIBUTE_C_MASK   << TXM_MMU_LEVEL1_ATTRIBUTE_C_SHIFT)
#define TXM_MMU_LEVEL1_ATTRIBUTE_AP_MASK        (TXM_MMU_ATTRIBUTE_AP_MASK  << TXM_MMU_LEVEL1_ATTRIBUTE_AP_SHIFT)
#define TXM_MMU_LEVEL1_ATTRIBUTE_TEX_MASK       (TXM_MMU_ATTRIBUTE_TEX_MASK << TXM_MMU_LEVEL1_ATTRIBUTE_TEX_SHIFT)

/* Masks for L2 page attributes.  */
#define TXM_MMU_LEVEL2_ATTRIBUTE_XN_MASK        (TXM_MMU_ATTRIBUTE_XN_MASK  << TXM_MMU_LEVEL2_ATTRIBUTE_XN_SHIFT)
#define TXM_MMU_LEVEL2_ATTRIBUTE_B_MASK         (TXM_MMU_ATTRIBUTE_B_MASK   << TXM_MMU_LEVEL2_ATTRIBUTE_B_SHIFT)
#define TXM_MMU_LEVEL2_ATTRIBUTE_C_MASK         (TXM_MMU_ATTRIBUTE_C_MASK   << TXM_MMU_LEVEL2_ATTRIBUTE_C_SHIFT)
#define TXM_MMU_LEVEL2_ATTRIBUTE_AP_MASK        (TXM_MMU_ATTRIBUTE_AP_MASK  << TXM_MMU_LEVEL2_ATTRIBUTE_AP_SHIFT)
#define TXM_MMU_LEVEL2_ATTRIBUTE_TEX_MASK       (TXM_MMU_ATTRIBUTE_TEX_MASK << TXM_MMU_LEVEL2_ATTRIBUTE_TEX_SHIFT)

#define TXM_ADDRESS_TRANSLATION_FAULT_BIT       1

#define TXM_ASID_RESERVED                       0xFFFFFFFF

#define TXM_MODULE_ASID_ERROR                   0xF6
#define TXM_MODULE_EXTERNAL_MEMORY_ENABLE_ERROR 0xF7

/* Number of L2 pages each module can have.  */
#define TXM_MODULE_LEVEL2_EXTERNAL_PAGES        16
/* Size, in pages, of the L2 page pool.  */
#define TXM_LEVEL2_EXTERNAL_POOL_PAGES          (TXM_MODULE_LEVEL2_EXTERNAL_PAGES * TXM_MAXIMUM_MODULES)


/* Define the port-extensions to the module manager instance structure.  */

#define TXM_MODULE_MANAGER_PORT_EXTENSION                                           \
    ULONG               txm_module_instance_asid;                                   \
    ULONG               *txm_external_page_table[TXM_MODULE_LEVEL2_EXTERNAL_PAGES];

/* Define the memory fault information structure that is populated when a memory fault occurs.  */


typedef struct TXM_MODULE_MANAGER_MEMORY_FAULT_INFO_STRUCT
{
    TX_THREAD           *txm_module_manager_memory_fault_info_thread_ptr;
    VOID                *txm_module_manager_memory_fault_info_code_location;
    ULONG               txm_module_manager_memory_fault_info_dfar;
    ULONG               txm_module_manager_memory_fault_info_dfsr;
    ULONG               txm_module_manager_memory_fault_info_ifar;
    ULONG               txm_module_manager_memory_fault_info_ifsr;
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
    ULONG               txm_module_manager_memory_fault_info_cpsr;
} TXM_MODULE_MANAGER_MEMORY_FAULT_INFO;


#define TXM_MODULE_MANAGER_FAULT_INFO                                               \
    TXM_MODULE_MANAGER_MEMORY_FAULT_INFO    _txm_module_manager_memory_fault_info;

/* Define the macro to check the stack available in dispatch.  */
#define TXM_MODULE_MANAGER_CHECK_STACK_AVAILABLE 


/* Define the macro to check the code alignment.  */

#define TXM_MODULE_MANAGER_CHECK_CODE_ALIGNMENT(module_location, code_alignment)    \
    {                                                                               \
        ULONG   temp;                                                               \
        temp =  (ULONG) module_location;                                            \
        temp =  temp & (TXM_MODULE_MEMORY_ALIGNMENT - 1);                           \
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


/* Define the macro to populate the thread control block with module port-specific information.  */

#define TXM_MODULE_MANAGER_THREAD_SETUP(thread_ptr, module_instance)                                                                            \
    thread_ptr -> tx_thread_module_current_user_mode =  module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION;   \
    thread_ptr -> tx_thread_module_user_mode =          module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION;   \
    if (thread_ptr -> tx_thread_module_user_mode)                                                                                               \
    {                                                                                                                                           \
        thread_entry_info -> txm_module_thread_entry_info_kernel_call_dispatcher =   _txm_module_manager_user_mode_entry;                       \
    }                                                                                                                                           \
    else                                                                                                                                        \
    {                                                                                                                                           \
        thread_entry_info -> txm_module_thread_entry_info_kernel_call_dispatcher =   _txm_module_manager_kernel_dispatch;                       \
    }


/* Define the macro to populate the module control block with module port-specific information.
   If memory protection is enabled, set up the MMU registers.
*/
#define TXM_MODULE_MANAGER_MODULE_SETUP(module_instance)                                                                        \
    if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)                                   \
    {                                                                                                                           \
        _txm_module_manager_mm_register_setup(module_instance);                                                                \
    }                                                                                                                           \
    else                                                                                                                        \
    {                                                                                                                           \
        /* Do nothing.  */                                                                                                      \
    }

/* Define the macro to perform port-specific functions when unloading the module.  */
#define TXM_MODULE_MANAGER_MODULE_UNLOAD(module_instance)                                                                       \
    _txm_level2_page_clear(module_instance);                                                                                    \
    _txm_module_manager_remove_asid(module_instance);

/* Define the macros to perform port-specific checks when passing pointers to the kernel.  */

/* Define macro to make sure object is inside the module's data or shared memory.  */
#define TXM_MODULE_MANAGER_CHECK_INSIDE_DATA(module_instance, obj_ptr, obj_size) \
    _txm_module_manager_inside_data_check((ULONG) obj_ptr)


/* Define some internal prototypes to this module port.  */

#ifndef TX_SOURCE_CODE
#define txm_module_manager_memory_fault_notify              _txm_module_manager_memory_fault_notify
#define txm_module_manager_mm_initialize                    _txm_module_manager_mm_initialize
#endif


#define TXM_MODULE_MANAGER_ADDITIONAL_PROTOTYPES                                                                                \
VOID  _txm_module_manager_alignment_adjust(TXM_MODULE_PREAMBLE *module_preamble, ULONG *code_size, ULONG *code_alignment, ULONG *data_size, ULONG *data_alignment);   \
ULONG _txm_module_manager_data_pointer_check(ULONG pointer);                                                                    \
VOID  _txm_module_manager_memory_fault_handler(VOID);                                                                           \
UINT  _txm_module_manager_memory_fault_notify(VOID (*notify_function)(TX_THREAD *, TXM_MODULE_INSTANCE *));                     \
UINT  _txm_module_manager_mm_initialize(VOID);                                                                                  \
VOID  _txm_module_manager_mm_register_setup(TXM_MODULE_INSTANCE *module_instance);                                              \
VOID  _txm_level2_page_clear(TXM_MODULE_INSTANCE *module_instance);                                                             \
VOID  _txm_module_manager_remove_asid(TXM_MODULE_INSTANCE *module_instance);                                                    \
UINT  _txm_module_manager_inside_data_check(ULONG pointer);

#define TXM_MODULE_MANAGER_VERSION_ID   \
CHAR                            _txm_module_manager_version_id[] =  \
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  ThreadX Module Cortex-A7/MMU/AC5 Version 6.1.9 *";

#endif

