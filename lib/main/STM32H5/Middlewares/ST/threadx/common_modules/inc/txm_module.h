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
/*    txm_module.h                                        PORTABLE C      */
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
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  12-31-2020      Scott Larson            Modified comment(s), added    */
/*                                            port-specific extension,    */
/*                                            resulting in version 6.1.3  */
/*  01-31-2022      Scott Larson            Modified comment(s), added    */
/*                                            callback thread prototype,  */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef TXM_MODULE_H
#define TXM_MODULE_H


/* Include the standard ThreadX API file.  */

#include "tx_api.h"

/* Include the module port specific file.  */

#include "txm_module_port.h"


/* Include any supported external component include files.  */

#ifdef TXM_MODULE_ENABLE_FILEX
#include "txm_module_filex.h"
#endif

#ifdef TXM_MODULE_ENABLE_GUIX
#include "txm_module_guix.h"
#endif

#ifdef TXM_MODULE_ENABLE_NETX
#include "txm_module_netx.h"
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO
#include "txm_module_netxduo.h"
#endif

#ifdef TXM_MODULE_ENABLE_USBX
#include "txm_module_usbx.h"
#endif


#ifdef FX_FILEX_PRESENT
#include "fx_api.h"
#endif


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


/* Define the Module ID, which is used to indicate a module is valid.  */

#define TXM_MODULE_ID                                       0x4D4F4455


/* Define valid module states.  */

#define TXM_MODULE_LOADED                                   1
#define TXM_MODULE_STARTED                                  2
#define TXM_MODULE_STOPPING                                 3
#define TXM_MODULE_STOPPED                                  4
#define TXM_MODULE_UNLOADED                                 5


/* Define module manager error codes.  */

#define TXM_MODULE_ALIGNMENT_ERROR                          0xF0
#define TXM_MODULE_ALREADY_LOADED                           0xF1
#define TXM_MODULE_INVALID                                  0xF2
#define TXM_MODULE_INVALID_PROPERTIES                       0xF3
#define TXM_MODULE_INVALID_MEMORY                           0xF4
#define TXM_MODULE_INVALID_CALLBACK                         0xF5
#define TXM_MODULE_INVALID_STACK_SIZE                       0xF6
#define TXM_MODULE_FILEX_INVALID_BYTES_READ                 0xF7
#define TXM_MODULE_MATH_OVERFLOW                            0xF8


/* Define the data area alignment mask, must be a power of 2.  */

#ifndef TXM_MODULE_DATA_ALIGNMENT
#define TXM_MODULE_DATA_ALIGNMENT                           4
#endif


/* Define the code area alignment mask, must be a power of 2.  */

#ifndef TXM_MODULE_CODE_ALIGNMENT
#define TXM_MODULE_CODE_ALIGNMENT                           4
#endif


/* Define module timeout for waiting for module to finish.  */

#ifndef TXM_MODULE_TIMEOUT
#define TXM_MODULE_TIMEOUT                                  100
#endif


/* Define module thread time-slice default.  */

#ifndef TXM_MODULE_TIME_SLICE
#define TXM_MODULE_TIME_SLICE                               4
#endif


/* Define each module's callback queue depth. This is used to queue up incoming call back requests.  */

#ifndef TXM_MODULE_CALLBACKS_QUEUE_DEPTH
#define TXM_MODULE_CALLBACKS_QUEUE_DEPTH                    8       /* Number queued callback requests.  */
#endif


/* Define the module manager thread's stack size.  */

#ifndef TXM_MODULE_MANAGER_THREAD_STACK_SIZE
#define TXM_MODULE_MANAGER_THREAD_STACK_SIZE                1024
#endif


/* Define the module manager thread's priority.  */

#ifndef TXM_MODULE_MANAGER_THREAD_PRIORITY
#define TXM_MODULE_MANAGER_THREAD_PRIORITY                  1
#endif


/* Define the module's callback handler thread's stack size.  */

#ifndef TXM_MODULE_CALLBACK_THREAD_STACK_SIZE
#define TXM_MODULE_CALLBACK_THREAD_STACK_SIZE                1024
#endif


/* Define the default port-specific macro for resetting the thread.  */

#ifndef TXM_MODULE_MANAGER_THREAD_RESET_PORT_COMPLETION
#define TXM_MODULE_MANAGER_THREAD_RESET_PORT_COMPLETION(thread_ptr, module_instance)
#endif


/* Define object types for object search requests.  */

#define TXM_BLOCK_POOL_OBJECT                               1
#define TXM_BYTE_POOL_OBJECT                                2
#define TXM_EVENT_FLAGS_OBJECT                              3
#define TXM_MUTEX_OBJECT                                    4
#define TXM_QUEUE_OBJECT                                    5
#define TXM_SEMAPHORE_OBJECT                                6
#define TXM_THREAD_OBJECT                                   7
#define TXM_TIMER_OBJECT                                    8
#define TXM_THREAD_KERNEL_STACK_OBJECT                      77
#define TXM_FILEX_OBJECTS_START                             100
#define TXM_FILEX_OBJECTS_END                               199
#define TXM_NETX_OBJECTS_START                              200
#define TXM_NETX_OBJECTS_END                                299
#define TXM_NETXDUO_OBJECTS_START                           300
#define TXM_NETXDUO_OBJECTS_END                             399
#define TXM_USBX_OBJECTS_START                              400
#define TXM_USBX_OBJECTS_END                                499
#define TXM_GUIX_OBJECTS_START                              500
#define TXM_GUIX_OBJECT_END                                 599


/* Define callback types.  */
#define TXM_THREADX_CALLBACKS_START                         0
#define TXM_THREADX_CALLBACKS_END                           99
#define TXM_FILEX_CALLBACKS_START                           100
#define TXM_FILEX_CALLBACKS_END                             199
#define TXM_NETX_CALLBACKS_START                            200
#define TXM_NETX_CALLBACKS_END                              299
#define TXM_NETXDUO_CALLBACKS_START                         300
#define TXM_NETXDUO_CALLBACKS_END                           399
#define TXM_USBX_CALLBACKS_START                            400
#define TXM_USBX_CALLBACKS_END                              499
#define TXM_GUIX_CALLBACKS_START                            500
#define TXM_GUIX_CALLBACKS_END                              599

#define TXM_TIMER_CALLBACK                                  0
#define TXM_EVENTS_SET_CALLBACK                             1
#define TXM_QUEUE_SEND_CALLBACK                             2
#define TXM_SEMAPHORE_PUT_CALLBACK                          3
#define TXM_THREAD_ENTRY_EXIT_CALLBACK                      4


/* Determine the ThreadX kernel API call IDs.  */

#define TXM_BLOCK_ALLOCATE_CALL                             1
#define TXM_BLOCK_POOL_CREATE_CALL                          2
#define TXM_BLOCK_POOL_DELETE_CALL                          3
#define TXM_BLOCK_POOL_INFO_GET_CALL                        4
#define TXM_BLOCK_POOL_PERFORMANCE_INFO_GET_CALL            5
#define TXM_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL     6
#define TXM_BLOCK_POOL_PRIORITIZE_CALL                      7
#define TXM_BLOCK_RELEASE_CALL                              8
#define TXM_BYTE_ALLOCATE_CALL                              9
#define TXM_BYTE_POOL_CREATE_CALL                           10
#define TXM_BYTE_POOL_DELETE_CALL                           11
#define TXM_BYTE_POOL_INFO_GET_CALL                         12
#define TXM_BYTE_POOL_PERFORMANCE_INFO_GET_CALL             13
#define TXM_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_CALL      14
#define TXM_BYTE_POOL_PRIORITIZE_CALL                       15
#define TXM_BYTE_RELEASE_CALL                               16
#define TXM_EVENT_FLAGS_CREATE_CALL                         17
#define TXM_EVENT_FLAGS_DELETE_CALL                         18
#define TXM_EVENT_FLAGS_GET_CALL                            19
#define TXM_EVENT_FLAGS_INFO_GET_CALL                       20
#define TXM_EVENT_FLAGS_PERFORMANCE_INFO_GET_CALL           21
#define TXM_EVENT_FLAGS_PERFORMANCE_SYSTEM_INFO_GET_CALL    22
#define TXM_EVENT_FLAGS_SET_CALL                            23
#define TXM_EVENT_FLAGS_SET_NOTIFY_CALL                     24
#define TXM_THREAD_INTERRUPT_CONTROL_CALL                   25
#define TXM_MUTEX_CREATE_CALL                               26
#define TXM_MUTEX_DELETE_CALL                               27
#define TXM_MUTEX_GET_CALL                                  28
#define TXM_MUTEX_INFO_GET_CALL                             29
#define TXM_MUTEX_PERFORMANCE_INFO_GET_CALL                 30
#define TXM_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_CALL          31
#define TXM_MUTEX_PRIORITIZE_CALL                           32
#define TXM_MUTEX_PUT_CALL                                  33
#define TXM_QUEUE_CREATE_CALL                               34
#define TXM_QUEUE_DELETE_CALL                               35
#define TXM_QUEUE_FLUSH_CALL                                36
#define TXM_QUEUE_FRONT_SEND_CALL                           37
#define TXM_QUEUE_INFO_GET_CALL                             38
#define TXM_QUEUE_PERFORMANCE_INFO_GET_CALL                 39
#define TXM_QUEUE_PERFORMANCE_SYSTEM_INFO_GET_CALL          40
#define TXM_QUEUE_PRIORITIZE_CALL                           41
#define TXM_QUEUE_RECEIVE_CALL                              42
#define TXM_QUEUE_SEND_CALL                                 43
#define TXM_QUEUE_SEND_NOTIFY_CALL                          44
#define TXM_SEMAPHORE_CEILING_PUT_CALL                      45
#define TXM_SEMAPHORE_CREATE_CALL                           46
#define TXM_SEMAPHORE_DELETE_CALL                           47
#define TXM_SEMAPHORE_GET_CALL                              48
#define TXM_SEMAPHORE_INFO_GET_CALL                         49
#define TXM_SEMAPHORE_PERFORMANCE_INFO_GET_CALL             50
#define TXM_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET_CALL      51
#define TXM_SEMAPHORE_PRIORITIZE_CALL                       52
#define TXM_SEMAPHORE_PUT_CALL                              53
#define TXM_SEMAPHORE_PUT_NOTIFY_CALL                       54
#define TXM_THREAD_CREATE_CALL                              55
#define TXM_THREAD_DELETE_CALL                              56
#define TXM_THREAD_ENTRY_EXIT_NOTIFY_CALL                   57
#define TXM_THREAD_IDENTIFY_CALL                            58
#define TXM_THREAD_INFO_GET_CALL                            59
#define TXM_THREAD_PERFORMANCE_INFO_GET_CALL                60
#define TXM_THREAD_PERFORMANCE_SYSTEM_INFO_GET_CALL         61
#define TXM_THREAD_PREEMPTION_CHANGE_CALL                   62
#define TXM_THREAD_PRIORITY_CHANGE_CALL                     63
#define TXM_THREAD_RELINQUISH_CALL                          64
#define TXM_THREAD_RESET_CALL                               65
#define TXM_THREAD_RESUME_CALL                              66
#define TXM_THREAD_SLEEP_CALL                               67
#define TXM_THREAD_STACK_ERROR_NOTIFY_CALL                  68
#define TXM_THREAD_SUSPEND_CALL                             69
#define TXM_THREAD_TERMINATE_CALL                           70
#define TXM_THREAD_TIME_SLICE_CHANGE_CALL                   71
#define TXM_THREAD_WAIT_ABORT_CALL                          72
#define TXM_TIME_GET_CALL                                   73
#define TXM_TIME_SET_CALL                                   74
#define TXM_TIMER_ACTIVATE_CALL                             75
#define TXM_TIMER_CHANGE_CALL                               76
#define TXM_TIMER_CREATE_CALL                               77
#define TXM_TIMER_DEACTIVATE_CALL                           78
#define TXM_TIMER_DELETE_CALL                               79
#define TXM_TIMER_INFO_GET_CALL                             80
#define TXM_TIMER_PERFORMANCE_INFO_GET_CALL                 81
#define TXM_TIMER_PERFORMANCE_SYSTEM_INFO_GET_CALL          82
#define TXM_TRACE_ENABLE_CALL                               83
#define TXM_TRACE_EVENT_FILTER_CALL                         84
#define TXM_TRACE_EVENT_UNFILTER_CALL                       85
#define TXM_TRACE_DISABLE_CALL                              86
#define TXM_TRACE_INTERRUPT_CONTROL_CALL                    87
#define TXM_TRACE_ISR_ENTER_INSERT_CALL                     88
#define TXM_TRACE_ISR_EXIT_INSERT_CALL                      89
#define TXM_TRACE_BUFFER_FULL_NOTIFY_CALL                   90
#define TXM_TRACE_USER_EVENT_INSERT_CALL                    91
#define TXM_THREAD_SYSTEM_SUSPEND_CALL                      92
#define TXM_MODULE_OBJECT_POINTER_GET_CALL                  93
#define TXM_MODULE_OBJECT_POINTER_GET_EXTENDED_CALL         94
#define TXM_MODULE_OBJECT_ALLOCATE_CALL                     95
#define TXM_MODULE_OBJECT_DEALLOCATE_CALL                   96

#define TXM_MODULE_PORT_EXTENSION_API_ID_START              500
#define TXM_MODULE_PORT_EXTENSION_API_ID_END                999

/* Determine the API call IDs for other components.  */

#define TXM_FILEX_API_ID_START                              1000
#define TXM_FILEX_API_ID_END                                1999
#define TXM_NETX_API_ID_START                               2000
#define TXM_NETX_API_ID_END                                 2999
#define TXM_NETXDUO_API_ID_START                            3000
#define TXM_NETXDUO_API_ID_END                              3999
#define TXM_USBX_API_ID_START                               4000
#define TXM_USBX_API_ID_END                                 4999
#define TXM_GUIX_API_ID_START                               5000
#define TXM_GUIX_API_ID_END                                 5999


/* Determine the application's IDs for calling application code in the resident area.  */

#define TXM_APPLICATION_REQUEST_ID_BASE                     0x10000


/* Define the overlay for the module's preamble.  */

typedef struct TXM_MODULE_PREAMBLE_STRUCT
{
                                                                        /* Meaning                                      */
    ULONG               txm_module_preamble_id;                         /* Download Module ID (0x54584D44)              */
    ULONG               txm_module_preamble_version_major;              /* Major Version ID                             */
    ULONG               txm_module_preamble_version_minor;              /* Minor Version ID                             */
    ULONG               txm_module_preamble_preamble_size;              /* Module Preamble Size, in 32-bit words        */
    ULONG               txm_module_preamble_application_module_id;      /* Module ID (application defined)              */
    ULONG               txm_module_preamble_property_flags;             /* Properties Bit Map                           */
    ULONG               txm_module_preamble_shell_entry_function;       /* Module shell Entry Function                  */
    ULONG               txm_module_preamble_start_function;             /* Module Thread Start Function                 */
    ULONG               txm_module_preamble_stop_function;              /* Module Thread Stop Function                  */
    ULONG               txm_module_preamble_start_stop_priority;        /* Module Start/Stop Thread Priority            */
    ULONG               txm_module_preamble_start_stop_stack_size;      /* Module Start/Stop Thread Priority            */
    ULONG               txm_module_preamble_callback_function;          /* Module Callback Thread Function              */
    ULONG               txm_module_preamble_callback_priority;          /* Module Callback Thread Priority              */
    ULONG               txm_module_preamble_callback_stack_size;        /* Module Callback Thread Stack Size            */
    ULONG               txm_module_preamble_code_size;                  /* Module Instruction Area Size                 */
    ULONG               txm_module_preamble_data_size;                  /* Module Data Area Size                        */
    ULONG               txm_module_preamble_reserved_0;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_1;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_2;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_3;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_4;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_5;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_6;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_7;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_8;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_9;                 /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_10;                /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_11;                /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_12;                /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_13;                /* Reserved                                     */
    ULONG               txm_module_preamble_reserved_14;                /* Reserved                                     */
    ULONG               txm_module_preamble_checksum;                   /* Module Instruction Area Checksum [Optional]  */

} TXM_MODULE_PREAMBLE;


struct TXM_MODULE_ALLOCATED_OBJECT_STRUCT;


/* Define the callback notification structure used to communicate between the module's callback handling thread
   and the module manager.  */

typedef struct TXM_MODULE_CALLBACK_MESSAGE_STRUCT
{
    ULONG               txm_module_callback_message_type;
    ULONG               txm_module_callback_message_activation_count;
    VOID                (*txm_module_callback_message_application_function)(VOID);
    ALIGN_TYPE          txm_module_callback_message_param_1;
    ALIGN_TYPE          txm_module_callback_message_param_2;
    ALIGN_TYPE          txm_module_callback_message_param_3;
    ALIGN_TYPE          txm_module_callback_message_param_4;
    ALIGN_TYPE          txm_module_callback_message_param_5;
    ALIGN_TYPE          txm_module_callback_message_param_6;
    ALIGN_TYPE          txm_module_callback_message_param_7;
    ALIGN_TYPE          txm_module_callback_message_param_8;
    ALIGN_TYPE          txm_module_callback_message_reserved1;
    ALIGN_TYPE          txm_module_callback_message_reserved2;
} TXM_MODULE_CALLBACK_MESSAGE;


/* Define the module's instance for the manager.  */

typedef struct TXM_MODULE_INSTANCE_STRUCT
{
    ULONG               txm_module_instance_id;
    CHAR                *txm_module_instance_name;
    ULONG               txm_module_instance_state;
    ULONG               txm_module_instance_property_flags;
    VOID                *txm_module_instance_code_allocation_ptr;
    ULONG               txm_module_instance_code_allocation_size;
    VOID                *txm_module_instance_code_start;
    VOID                *txm_module_instance_code_end;
    ULONG               txm_module_instance_code_size;
    VOID                *txm_module_instance_data_allocation_ptr;
    ULONG               txm_module_instance_data_allocation_size;
    VOID                *txm_module_instance_data_start;
    VOID                *txm_module_instance_data_end;
    VOID                *txm_module_instance_module_data_base_address;
    ULONG               txm_module_instance_data_size;
    ULONG               txm_module_instance_total_ram_usage;
    VOID                *txm_module_instance_start_stop_stack_start_address;
    VOID                *txm_module_instance_start_stop_stack_end_address;
    VOID                *txm_module_instance_callback_stack_start_address;
    VOID                *txm_module_instance_callback_stack_end_address;
    TXM_MODULE_PREAMBLE *txm_module_instance_preamble_ptr;
    VOID                (*txm_module_instance_shell_entry_function)(TX_THREAD *, struct TXM_MODULE_INSTANCE_STRUCT *);
    VOID                (*txm_module_instance_start_thread_entry)(ULONG);
    VOID                (*txm_module_instance_stop_thread_entry)(ULONG);
    VOID                (*txm_module_instance_callback_request_thread_entry)(ULONG);

    /* Define the port extention to the module manager structure.  */
    TXM_MODULE_MANAGER_PORT_EXTENSION

    TX_THREAD           txm_module_instance_start_stop_thread;
    TX_THREAD           txm_module_instance_callback_request_thread;
    TX_QUEUE            txm_module_instance_callback_request_queue;
    ULONG               txm_module_instance_callback_request_queue_area[TXM_MODULE_CALLBACKS_QUEUE_DEPTH * (sizeof(TXM_MODULE_CALLBACK_MESSAGE)/sizeof(ULONG))];
    ULONG               txm_module_instance_start_stop_stack_size;
    ULONG               txm_module_instance_start_stop_priority;
    ULONG               txm_module_instance_callback_stack_size;
    ULONG               txm_module_instance_callback_priority;
    ULONG               txm_module_instance_application_module_id;
    UINT                txm_module_instance_maximum_priority;

    /* Define the head pointer of the list of objects allocated by the module.  */
    struct TXM_MODULE_ALLOCATED_OBJECT_STRUCT
                        *txm_module_instance_object_list_head;
    ULONG               txm_module_instance_object_list_count;

    struct TXM_MODULE_INSTANCE_STRUCT
                        *txm_module_instance_loaded_next,
                        *txm_module_instance_loaded_previous;
} TXM_MODULE_INSTANCE;


/* Determine if the thread entry info control block has an extension defined. If not, define the extension to
   whitespace.  */

#ifndef TXM_MODULE_THREAD_ENTRY_INFO_USER_EXTENSION
#define TXM_MODULE_THREAD_ENTRY_INFO_USER_EXTENSION
#endif


/* Define the thread entry information structure. This structure is placed on the thread's stack such that the
   module's _txm_thread_shell_entry function does not need to access anything in the thread control block.  */

typedef struct TXM_MODULE_THREAD_ENTRY_INFO_STRUCT
{
    TX_THREAD           *txm_module_thread_entry_info_thread;
    TXM_MODULE_INSTANCE *txm_module_thread_entry_info_module;
    VOID                *txm_module_thread_entry_info_data_base_address;     /* Don't move this, referenced in stack build to setup module data base register. */
    VOID                *txm_module_thread_entry_info_code_base_address;
    VOID               (*txm_module_thread_entry_info_entry)(ULONG);
    ULONG                txm_module_thread_entry_info_parameter;
    VOID               (*txm_module_thread_entry_info_exit_notify)(struct TX_THREAD_STRUCT *, UINT);
    UINT                 txm_module_thread_entry_info_start_thread;
    TX_THREAD           *txm_module_thread_entry_info_callback_request_thread;
    TX_QUEUE            *txm_module_thread_entry_info_callback_request_queue;
    VOID                *txm_module_thread_entry_info_reserved;
    ALIGN_TYPE          (*txm_module_thread_entry_info_kernel_call_dispatcher)(ULONG kernel_request, ALIGN_TYPE param_1, ALIGN_TYPE param_2, ALIGN_TYPE param_3);
    TXM_MODULE_THREAD_ENTRY_INFO_USER_EXTENSION
} TXM_MODULE_THREAD_ENTRY_INFO;


/* Define the linked-list structure used to maintain the module's object allocation.  */

typedef struct TXM_MODULE_ALLOCATED_OBJECT_STRUCT
{

    TXM_MODULE_INSTANCE *txm_module_allocated_object_module_instance;
    struct TXM_MODULE_ALLOCATED_OBJECT_STRUCT
                        *txm_module_allocated_object_next,
                        *txm_module_allocated_object_previous;
    ULONG               txm_module_object_size;
} TXM_MODULE_ALLOCATED_OBJECT;


/*  Determine if module code is being compiled. If so, remap the ThreadX API to
    the module shell functions that will go through the module <-> module manager
    interface.  */

#ifdef TXM_MODULE


/* Define the external reference to the module manager kernel dispatcher function pointer. This is supplied to the module by the module
   manager when the module is created and started.  */

extern ALIGN_TYPE (*_txm_module_kernel_call_dispatcher)(ULONG type, ALIGN_TYPE param_1, ALIGN_TYPE param_2, ALIGN_TYPE param3);


/* Define specific module function prototypes.  */

#define txm_module_application_request                  _txm_module_application_request
#define txm_module_object_allocate                      _txm_module_object_allocate
#define txm_module_object_deallocate                    _txm_module_object_deallocate
#define txm_module_object_pointer_get                   _txm_module_object_pointer_get
#define txm_module_object_pointer_get_extended          _txm_module_object_pointer_get_extended

VOID  _txm_module_thread_shell_entry(TX_THREAD *thread_ptr, TXM_MODULE_THREAD_ENTRY_INFO *thread_info);
UINT  _txm_module_thread_system_suspend(TX_THREAD *thread_ptr);

UINT  _txm_module_application_request(ULONG request, ALIGN_TYPE param_1, ALIGN_TYPE param_2, ALIGN_TYPE param_3);
VOID  _txm_module_callback_request_thread_entry(ULONG id);
UINT  _txm_module_object_allocate(VOID **object_ptr, ULONG object_size);
UINT  _txm_module_object_deallocate(VOID *object_ptr);
UINT  _txm_module_object_pointer_get(UINT object_type, CHAR *name, VOID **object_ptr);
UINT  _txm_module_object_pointer_get_extended(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);

/* Module callback functions.  */

#ifdef TXM_MODULE_ENABLE_NETX
VOID  _txm_module_netx_callback_request(TXM_MODULE_CALLBACK_MESSAGE *callback_message);
#endif
#ifdef TXM_MODULE_ENABLE_NETXDUO
VOID  _txm_module_netxduo_callback_request(TXM_MODULE_CALLBACK_MESSAGE *callback_message);
#endif
#ifdef TXM_MODULE_ENABLE_FILEX
VOID  _txm_module_filex_callback_request(TXM_MODULE_CALLBACK_MESSAGE *callback_message);
#endif
#ifdef TXM_MODULE_ENABLE_GUIX
VOID  _txm_module_guix_duo_callback_request(TXM_MODULE_CALLBACK_MESSAGE *callback_message);
#endif
#ifdef TXM_MODULE_ENABLE_USBX
VOID  _txm_module_usbx_duo_callback_request(TXM_MODULE_CALLBACK_MESSAGE *callback_message);
#endif

/* Define the module's thread shell entry function macros.  */

#define TXM_THREAD_COMPLETED_EXTENSION(a)
#define TXM_THREAD_STATE_CHANGE(a, b)

#else


/* Map the module manager APIs just in case this is being included from the module manager in the
   resident portion of the application.  */

#define txm_module_manager_initialize                   _txm_module_manager_initialize
#define txm_module_manager_absolute_load                _txm_module_manager_absolute_load
#define txm_module_manager_in_place_load                _txm_module_manager_in_place_load
#define txm_module_manager_file_load                    _txm_module_manager_file_load
#define txm_module_manager_memory_load                  _txm_module_manager_memory_load
#define txm_module_manager_object_pointer_get           _txm_module_manager_object_pointer_get
#define txm_module_manager_object_pointer_get_extended  _txm_module_manager_object_pointer_get_extended
#define txm_module_manager_object_pool_create           _txm_module_manager_object_pool_create
#define txm_module_manager_properties_get               _txm_module_manager_properties_get
#define txm_module_manager_start                        _txm_module_manager_start
#define txm_module_manager_stop                         _txm_module_manager_stop
#define txm_module_manager_unload                       _txm_module_manager_unload
#define txm_module_manager_maximum_module_priority_set  _txm_module_manager_maximum_module_priority_set
#define txm_module_manager_external_memory_enable       _txm_module_manager_external_memory_enable

/* Define external variables used by module manager functions.  */

#ifndef TX_MODULE_MANAGER_INIT
extern ULONG                                        _txm_module_manager_properties_supported;
extern ULONG                                        _txm_module_manager_properties_required;
extern TX_BYTE_POOL                                 _txm_module_manager_byte_pool;
extern TX_BYTE_POOL                                 _txm_module_manager_object_pool;
extern UINT                                         _txm_module_manager_object_pool_created;
extern TXM_MODULE_INSTANCE                          *_txm_module_manager_loaded_list_ptr;
extern ULONG                                        _txm_module_manger_loaded_count;
extern UINT                                         _txm_module_manager_ready;
extern TX_MUTEX                                     _txm_module_manager_mutex;
extern ULONG                                        _txm_module_manager_callback_total_count;
extern ULONG                                        _txm_module_manager_callback_error_count;
#endif

/* Define internal module manager function prototypes.  */

UINT  _txm_module_manager_application_request(ULONG request, ALIGN_TYPE param_1, ALIGN_TYPE param_2, ALIGN_TYPE param_3);
#ifdef FX_FILEX_PRESENT
UINT  _txm_module_manager_file_load(TXM_MODULE_INSTANCE *module_instance, CHAR *module_name, FX_MEDIA *media_ptr, CHAR *file_name);
#endif
UINT  _txm_module_manager_initialize(VOID *module_memory_start, ULONG module_memory_size);
UINT  _txm_module_manager_absolute_load(TXM_MODULE_INSTANCE *module_instance, CHAR *name, VOID *module_location);
UINT  _txm_module_manager_in_place_load(TXM_MODULE_INSTANCE *module_instance, CHAR *name, VOID *module_location);
UINT  _txm_module_manager_internal_load(TXM_MODULE_INSTANCE *module_instance, CHAR *name, VOID *module_location,
                                        ULONG code_size, VOID *code_allocation_ptr, ULONG code_allocation_size);
ALIGN_TYPE _txm_module_manager_kernel_dispatch(ULONG kernel_request, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2);
UINT  _txm_module_manager_object_allocate(VOID **object_ptr_ptr, ULONG object_size, TXM_MODULE_INSTANCE *module_instance);
UINT  _txm_module_manager_object_deallocate(VOID *object_ptr);
UINT  _txm_module_manager_object_pointer_get(UINT object_type, CHAR *name, VOID **object_ptr);
UINT  _txm_module_manager_object_pointer_get_extended(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);
UINT  _txm_module_manager_object_pool_create(VOID *object_memory, ULONG object_memory_size);
VOID  _txm_module_manager_object_type_set(ULONG object_ptr, ULONG object_size, ULONG object_type);
UINT  _txm_module_manager_memory_load(TXM_MODULE_INSTANCE *module_instance, CHAR *module_name, VOID *module_location);
UINT  _txm_module_manager_properties_get(TXM_MODULE_INSTANCE *module_instance, ULONG *module_properties_ptr);
UINT  _txm_module_manager_start(TXM_MODULE_INSTANCE *module_instance);
UINT  _txm_module_manager_stop(TXM_MODULE_INSTANCE *module_instance);
UINT  _txm_module_manager_thread_create(TX_THREAD *thread_ptr, CHAR *name, VOID (*shell_function)(TX_THREAD *, TXM_MODULE_INSTANCE *),
                               VOID (*entry_function)(ULONG), ULONG entry_input,
                               VOID *stack_start, ULONG stack_size, UINT priority, UINT preempt_threshold,
                               ULONG time_slice, UINT auto_start, UINT thread_control_block_size, TXM_MODULE_INSTANCE *module_instance);
UINT  _txm_module_manager_thread_reset(TX_THREAD *thread_ptr);
VOID  _txm_module_manager_name_build(CHAR *module_name, CHAR *thread_name, CHAR *combined_name);
VOID  _txm_module_manager_thread_stack_build(TX_THREAD *thread_ptr, VOID (*shell_function)(TX_THREAD *, TXM_MODULE_INSTANCE *));
UINT  _txm_module_manager_unload(TXM_MODULE_INSTANCE *module_instance);
ALIGN_TYPE  _txm_module_manager_user_mode_entry(ULONG kernel_request, ALIGN_TYPE param_1, ALIGN_TYPE param_2, ALIGN_TYPE param_3);
UINT  _txm_module_manager_maximum_module_priority_set(TXM_MODULE_INSTANCE *module_instance, UINT priority);
UINT  _txm_module_manager_external_memory_enable(TXM_MODULE_INSTANCE *module_instance, VOID *start_address, ULONG length, UINT attributes);

#ifdef TXM_MODULE_ENABLE_NETX
ULONG  _txm_module_manager_netx_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ULONG param_1, ULONG param_2, ULONG param_3);
UINT  _txm_module_manager_netx_object_pointer_get(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO
ALIGN_TYPE  _txm_module_manager_netxduo_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ALIGN_TYPE param_1, ALIGN_TYPE param_2, ALIGN_TYPE param_3);
UINT  _txm_module_manager_netxduo_object_pointer_get(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);
#endif

#ifdef TXM_MODULE_ENABLE_FILEX
ALIGN_TYPE _txm_module_manager_filex_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2);
UINT  _txm_module_manager_filex_object_pointer_get(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);
#endif

#ifdef TXM_MODULE_ENABLE_GUIX
ULONG  _txm_module_manager_guix_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ULONG param_1, ULONG param_2, ULONG param_3);
UINT  _txm_module_manager_guix_object_pointer_get(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);
#endif

#ifdef TXM_MODULE_ENABLE_USBX
ULONG  _txm_module_manager_usbx_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ULONG param_1, ULONG param_2, ULONG param_3);
UINT  _txm_module_manager_usbx_object_pointer_get(UINT object_type, CHAR *name, UINT name_length, VOID **object_ptr);
#endif

/* Define the callback deferred processing routines necessary for executing callbacks in the module code.  */

VOID  _txm_module_manager_callback_request(TX_QUEUE *module_callback_queue, TXM_MODULE_CALLBACK_MESSAGE  *callback_request);
VOID  _txm_module_manager_event_flags_notify_trampoline(TX_EVENT_FLAGS_GROUP *group_ptr);
VOID  _txm_module_manager_queue_notify_trampoline(TX_QUEUE *queue_ptr);
VOID  _txm_module_manager_semaphore_notify_trampoline(TX_SEMAPHORE *semaphore_ptr);
VOID  _txm_module_manager_thread_notify_trampoline(TX_THREAD *thread_ptr, UINT type);
VOID  _txm_module_manager_timer_notify_trampoline(ULONG id);


/* Define port specific module manager prototypes.  */

TXM_MODULE_MANAGER_ADDITIONAL_PROTOTYPES

#endif


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
        }
#endif

#endif
