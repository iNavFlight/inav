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
/** NetX Component                                                        */
/**                                                                       */
/**   Cloud Helper                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_cloud.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Cloud component, including all data      */
/*    types and external references.                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Yuxin Zhou               Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CLOUD_H
#define NX_CLOUD_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

   /* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "tx_api.h"
#include "nx_api.h"


/* Define CLOUD constants.  */
#define NX_CLOUD_ID                                     ((ULONG)0x434C4F44)


/* API return values.  */
#define NX_CLOUD_MODULE_ALREADY_REGISTERED              0xF0
#define NX_CLOUD_MODULE_NOT_REGISTERED                  0xF1
#define NX_CLOUD_MODULE_BOUND                           0xF2
#define NX_CLOUD_MODULE_EVENT_INVALID                   0xF3


/* Define events processed by cloud helper. */
#define NX_CLOUD_ALL_EVENTS                             0xFFFFFFFF      /* All event flags.                */

/* Define the common events for all modules.  */
#define NX_CLOUD_COMMON_PERIODIC_EVENT                  0x00000001u     /* Periodic event, 1s              */

/* Define the module events.  */
#define NX_CLOUD_MODULE_MQTT_EVENT                      0x00010000u     /* MQTT event                      */
#define NX_CLOUD_MODULE_AZURE_SDK_EVENT                 0x00020000u     /* Azure SDK event                 */
#define NX_CLOUD_MODULE_AZURE_ADU_EVENT                 0x00040000u     /* Azure Device Update event       */
#define NX_CLOUD_MODULE_AZURE_ISM_EVENT                 0x00080000u     /* Azure IoT Security Module event */


/* Define the all common events.  */
#define NX_CLOUD_COMMON_ALL_EVENT                      (NX_CLOUD_COMMON_PERIODIC_EVENT)


typedef struct NX_CLOUD_MODULE_STRUCT
{

    /* Define the module name.  */
    const CHAR                     *nx_cloud_module_name;

    /* Define the module registered events including common events and module event
       that are processed in cloud helper thread.  */
    ULONG                           nx_cloud_module_registered_events;

    /* Define the actual current event flags in this module that are processed
       in module processing routine.  */
    ULONG                           nx_cloud_module_own_events;

    /* Define the module processing routine.  */
    VOID                          (*nx_cloud_module_process)(VOID *module_context, ULONG common_events, ULONG module_own_events);

    /* Define the context that is passed to module processing routine.  */
    VOID                           *nx_cloud_module_context;

    /* Define the next pointer of created module.  */
    struct NX_CLOUD_MODULE_STRUCT  *nx_cloud_module_next;

    /* Define the cloud pointer associated with the module.  */
    struct NX_CLOUD_STRUCT         *nx_cloud_ptr;

} NX_CLOUD_MODULE;

typedef struct NX_CLOUD_STRUCT
{

    /* Define the cloud ID.  */
    ULONG                           nx_cloud_id;

    /* Define the cloud name.  */
    const CHAR                     *nx_cloud_name;

    /* Define the cloud helper thread that process cloud modules.  */
    TX_THREAD                       nx_cloud_thread;

    /* Define the event flags that are used to stimulate the cloud helper
       thread.  */
    TX_EVENT_FLAGS_GROUP            nx_cloud_events;

    /* Define the internal mutex used for protection .  */
    TX_MUTEX                        nx_cloud_mutex;

    /* Define the periodic timer for cloud modules.  */
    TX_TIMER                        nx_cloud_periodic_timer;

    /* Define the head pointer of the created module list.  */
    NX_CLOUD_MODULE                *nx_cloud_modules_list_header;

    /* Define the number of created module instances.  */
    ULONG                           nx_cloud_modules_count;

} NX_CLOUD;


#ifndef NX_CLOUD_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map CLOUD API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_cloud_create                                 _nx_cloud_create
#define nx_cloud_delete                                 _nx_cloud_delete
#define nx_cloud_module_register                        _nx_cloud_module_register
#define nx_cloud_module_deregister                      _nx_cloud_module_deregister
#define nx_cloud_module_event_set                       _nx_cloud_module_event_set
#define nx_cloud_module_event_clear                     _nx_cloud_module_event_clear

#else

/* Services with error checking.  */

#define nx_cloud_create                                 _nxe_cloud_create
#define nx_cloud_delete                                 _nxe_cloud_delete
#define nx_cloud_module_register                        _nxe_cloud_module_register
#define nx_cloud_module_deregister                      _nxe_cloud_module_deregister
#define nx_cloud_module_event_set                       _nxe_cloud_module_event_set
#define nx_cloud_module_event_clear                     _nxe_cloud_module_event_clear

#endif

/* Define the prototypes accessible to the application software.  */

/* Create/delete cloud helper thread.  */
UINT nx_cloud_create(NX_CLOUD* cloud_ptr, const CHAR* cloud_name, VOID* memory_ptr, ULONG memory_size, UINT priority);
UINT nx_cloud_delete(NX_CLOUD* cloud_ptr);

/* Register/deregister module in cloud thread.  */
UINT nx_cloud_module_register(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr, const CHAR* module_name, ULONG module_event,
                              VOID (*module_process)(VOID* module_context, ULONG common_events, ULONG module_own_events), VOID* module_context);
UINT nx_cloud_module_deregister(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr);
UINT nx_cloud_module_event_set(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event);
UINT nx_cloud_module_event_clear(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event);

#else

/* Cloud source code is being compiled, do not perform any API mapping.  */

UINT _nxe_cloud_create(NX_CLOUD* cloud_ptr, const CHAR* cloud_name, VOID* memory_ptr, ULONG memory_size, UINT priority);
UINT _nx_cloud_create(NX_CLOUD* cloud_ptr, const CHAR* cloud_name, VOID* memory_ptr, ULONG memory_size, UINT priority);
UINT _nxe_cloud_delete(NX_CLOUD* cloud_ptr);
UINT _nx_cloud_delete(NX_CLOUD* cloud_ptr);
UINT _nxe_cloud_module_register(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr, const CHAR* module_name, ULONG module_event,
                                VOID(*module_process)(VOID* module_context, ULONG common_events, ULONG module_own_events), VOID* module_context);
UINT _nx_cloud_module_register(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr, const CHAR* module_name, ULONG module_event,
                               VOID(*module_process)(VOID* module_context, ULONG common_events, ULONG module_own_events), VOID* module_context);
UINT _nxe_cloud_module_deregister(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr);
UINT _nx_cloud_module_deregister(NX_CLOUD* cloud_ptr, NX_CLOUD_MODULE* module_ptr);
UINT _nxe_cloud_module_event_set(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event);
UINT _nx_cloud_module_event_set(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event);
UINT _nxe_cloud_module_event_clear(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event);
UINT _nx_cloud_module_event_clear(NX_CLOUD_MODULE *cloud_module, ULONG module_own_event);

#endif


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif

#endif /* NX_CLOUD_H  */
