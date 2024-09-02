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
/**   Lightweight M2M Protocol (LWM2M)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_lwm2m_client.h                                   PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX LWM2M Client component, including all    */
/*    data types and external references.                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-31-2020     Yuxin Zhou               Initial Version 6.1.3         */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            token and processing        */
/*                                            confirmable response,       */
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed compiler errors when  */
/*                                            TX_SAFETY_CRITICAL is       */
/*                                            enabled,                    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_LWM2M_CLIENT_H
#define NX_LWM2M_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Bypass NetX API error checking for internal calls.  */
#include "tx_port.h"

#ifdef NX_LWM2M_CLIENT_SOURCE_CODE
#ifndef TX_SAFETY_CRITICAL
#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif
#endif
#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif
#ifndef NX_SECURE_DISABLE_ERROR_CHECKING
#define NX_SECURE_DISABLE_ERROR_CHECKING
#endif
#endif


/* Include NetX and ThreadX definitions */

#include "nx_api.h"
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls_api.h"
#endif /* NX_SECURE_ENABLE_DTLS */


/* Define some basic data types used by LWM2M */

/* 16-bit unsigned integer */
typedef USHORT              NX_LWM2M_ID;

/* Boolean */
typedef CHAR                NX_LWM2M_BOOL;

/* 32-bit signed integer */
typedef LONG                NX_LWM2M_INT32;

/* 64-bit signed integer */
typedef long long           NX_LWM2M_INT64;

/* 64-bit unsigned integer */
typedef unsigned long long  NX_LWM2M_UINT64;

/* 32-bit floating point */
typedef float               NX_LWM2M_FLOAT32;

/* 64-bit floating point */
typedef double              NX_LWM2M_FLOAT64;


/* Define the maximum size of a CoAP message, including IP and UDP headers. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MTU
#define NX_LWM2M_CLIENT_MTU                                 1280
#endif /* NX_LWM2M_CLIENT_MTU */


/* Define the Type Of Service for the socket transmission . This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SOCKET_TOS
#define NX_LWM2M_CLIENT_SOCKET_TOS                          NX_IP_NORMAL
#endif /* NX_LWM2M_CLIENT_SOCKET_TOS */


/* Define the Time To Live for the socket transmission . This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SOCKET_TTL
#define NX_LWM2M_CLIENT_SOCKET_TTL                          NX_IP_TIME_TO_LIVE
#endif /* NX_LWM2M_CLIENT_SOCKET_TTL */


/* Define the maximum number of dtagrams that can be queued for the socket. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SOCKET_QUEUE_MAX
#define NX_LWM2M_CLIENT_SOCKET_QUEUE_MAX                    8
#endif /* NX_LWM2M_CLIENT_SOCKET_QUEUE_MAX */


/* Define the maximum length of the coap Uri-Path option, not including the '/rd' prefix. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_COAP_URI_PATH
#define NX_LWM2M_CLIENT_MAX_COAP_URI_PATH                   32
#endif /* NX_LWM2M_CLIENT_MAX_COAP_URI_PATH */


/* Define the maximum number of stored device errors. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_DEVICE_ERRORS
#define NX_LWM2M_CLIENT_MAX_DEVICE_ERRORS                   8
#endif /* NX_LWM2M_CLIENT_MAX_DEVICE_ERRORS */


/* Define the maximum number of Security Object Instances. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
#define NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES              2
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */


/* Define the maximum number of Server Object Instances. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_SERVER_INSTANCES
#define NX_LWM2M_CLIENT_MAX_SERVER_INSTANCES                1
#endif /* NX_LWM2M_CLIENT_MAX_SERVER_INSTANCES */


/* Define the maximum number of Access Control Object Instances. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES
#define NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES        0
#endif /* NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES */


/* Define the maximum number of ACL resources per Access Control Object Instance. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_ACLS
#define NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_ACLS             4
#endif /* NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_ACLS */


/* Define the maximum number of Notifications. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_NOTIFICATIONS
#define NX_LWM2M_CLIENT_MAX_NOTIFICATIONS                   8
#endif /* NX_LWM2M_CLIENT_MAX_NOTIFICATIONS */


/* Define the maximum number of resources per object. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_RESOURCES
#define NX_LWM2M_CLIENT_MAX_RESOURCES                       32
#endif /* NX_LWM2M_CLIENT_MAX_RESOURCES */


/* Define the maximum number of resource instances for multiple resource. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_MAX_MULTIPLE_RESOURCES
#define NX_LWM2M_CLIENT_MAX_MULTIPLE_RESOURCES              8
#endif /* NX_LWM2M_CLIENT_MAX_MULTIPLE_RESOURCES */


/* Define the maximum time allowed for the Bootstrap session between messages from the server. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_BOOTSTRAP_IDLE_TIMER
#define NX_LWM2M_CLIENT_BOOTSTRAP_IDLE_TIMER                (60 * NX_IP_PERIODIC_RATE)
#endif /* NX_LWM2M_CLIENT_BOOTSTRAP_IDLE_TIMER */


/* Define the maximum time to wait for DTLS handshake completion. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_DTLS_START_TIMEOUT
#define NX_LWM2M_CLIENT_DTLS_START_TIMEOUT                  (30 * NX_IP_PERIODIC_RATE)
#endif /* NX_LWM2M_CLIENT_DTLS_START_TIMEOUT */


/* Define the maximum time to wait for DTLS shutdown completion. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_DTLS_END_TIMEOUT
#define NX_LWM2M_CLIENT_DTLS_END_TIMEOUT                    (5 * NX_IP_PERIODIC_RATE)
#endif /* NX_LWM2M_CLIENT_DTLS_END_TIMEOUT */


/* Define the maximum length of the server URI, including terminating nul character. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_URI
#define NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_URI             128
#endif /* NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_URI */


/* Define the maximum public key or identity length for DTLS. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SECURITY_MAX_PUBLIC_KEY_OR_IDENTITY
#define NX_LWM2M_CLIENT_SECURITY_MAX_PUBLIC_KEY_OR_IDENTITY 128
#endif /* NX_LWM2M_CLIENT_SECURITY_MAX_PUBLIC_KEY_OR_IDENTITY */


/* Define the maximum server public key length for DTLS. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_PUBLIC_KEY
#define NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_PUBLIC_KEY      128
#endif /* NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_PUBLIC_KEY */


/* Define the maximum secret key length for DTLS. This value can be replaced by the user as a compilation option. */

#ifndef NX_LWM2M_CLIENT_SECURITY_MAX_SECRET_KEY
#define NX_LWM2M_CLIENT_SECURITY_MAX_SECRET_KEY             128
#endif /* NX_LWM2M_CLIENT_SECURITY_MAX_SECRET_KEY */


/* Define the number of seconds to wait before initiating a Client Initiated Bootstrap. */

#ifndef NX_LWM2M_CLIENT_HOLD_OFF
#define NX_LWM2M_CLIENT_HOLD_OFF                            1
#endif /* NX_LWM2M_CLIENT_HOLD_OFF */


/* Define the lifetime of the registration in seconds. */

#ifndef NX_LWM2M_CLIENT_LIFE_TIME
#define NX_LWM2M_CLIENT_LIFE_TIME                           600
#endif /* NX_LWM2M_CLIENT_LIFE_TIME */



/* Error codes */

#define NX_LWM2M_CLIENT_ERROR                               0x80
#define NX_LWM2M_CLIENT_ALREADY_EXIST                       0x81
#define NX_LWM2M_CLIENT_BAD_ENCODING                        0x82
#define NX_LWM2M_CLIENT_BUFFER_TOO_SMALL                    0x83
#define NX_LWM2M_CLIENT_METHOD_NOT_ALLOWED                  0x84
#define NX_LWM2M_CLIENT_NO_MEMORY                           0x85
#define NX_LWM2M_CLIENT_NOT_FOUND                           0x86
#define NX_LWM2M_CLIENT_NOT_SUPPORTED                       0x87
#define NX_LWM2M_CLIENT_NOT_ACCEPTABLE                      0x88
#define NX_LWM2M_CLIENT_PORT_UNAVAILABLE                    0x89
#define NX_LWM2M_CLIENT_ADDRESS_ERROR                       0x8a
#define NX_LWM2M_CLIENT_DTLS_ERROR                          0x8b
#define NX_LWM2M_CLIENT_SERVER_INSTANCE_DELETED             0x8c
#define NX_LWM2M_CLIENT_TIMED_OUT                           0x8d
#define NX_LWM2M_CLIENT_NOT_REGISTERED                      0x8e
#define NX_LWM2M_CLIENT_BAD_REQUEST                         0x8f
#define NX_LWM2M_CLIENT_UNAUTHORIZED                        0x90
#define NX_LWM2M_CLIENT_BAD_OPTION                          0x91
#define NX_LWM2M_CLIENT_FORBIDDEN                           0x92
#define NX_LWM2M_CLIENT_PRECONDITION_FAILED                 0x93
#define NX_LWM2M_CLIENT_UNSUPPORTED_CONTENT_FORMAT          0x94


/* Standard CoAP port number. */

#define NX_LWM2M_CLIENT_COAP_PORT                           5683


/* Standard Secure CoAP port number. */

#define NX_LWM2M_CLIENT_COAPS_PORT                          5684


/* Max length of a CoAP token */

#define NX_LWM2M_CLIENT_COAP_TOKEN_LEN                      8


/* Reserved Object/Instance/Resource ID */

#define NX_LWM2M_CLIENT_RESERVED_ID                         65535U


/* Define the standard OMA Objects IDs implemented by the LWM2M Client */

#define NX_LWM2M_CLIENT_SECURITY_OBJECT_ID                  0
#define NX_LWM2M_CLIENT_SERVER_OBJECT_ID                    1
#define NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID            2
#define NX_LWM2M_CLIENT_DEVICE_OBJECT_ID                    3
#define NX_LWM2M_CLIENT_FIRMWARE_OBJECT_ID                  5


/* Define the OMA Security Object Resources IDs */

#define NX_LWM2M_CLIENT_SECURITY_URI_ID                     0
#define NX_LWM2M_CLIENT_SECURITY_BOOTSTRAP_ID               1
#define NX_LWM2M_CLIENT_SECURITY_MODE_ID                    2
#define NX_LWM2M_CLIENT_SECURITY_PUBLIC_KEY_ID              3
#define NX_LWM2M_CLIENT_SECURITY_SERVER_PUBLIC_KEY_ID       4
#define NX_LWM2M_CLIENT_SECURITY_SECRET_KEY_ID              5
#define NX_LWM2M_CLIENT_SECURITY_SMS_SECURITY_ID            6
#define NX_LWM2M_CLIENT_SECURITY_SMS_KEY_PARAM_ID           7
#define NX_LWM2M_CLIENT_SECURITY_SMS_SECRET_KEY_ID          8
#define NX_LWM2M_CLIENT_SECURITY_SMS_SERVER_NUMBER_ID       9
#define NX_LWM2M_CLIENT_SECURITY_SHORT_SERVER_ID            10
#define NX_LWM2M_CLIENT_SECURITY_HOLD_OFF_ID                11


/* Define the OMA Server Object Resources IDs */

#define NX_LWM2M_CLIENT_SERVER_SHORT_SERVER_ID              0
#define NX_LWM2M_CLIENT_SERVER_LIFETIME_ID                  1
#define NX_LWM2M_CLIENT_SERVER_MIN_PERIOD_ID                2
#define NX_LWM2M_CLIENT_SERVER_MAX_PERIOD_ID                3
#define NX_LWM2M_CLIENT_SERVER_DISABLE_ID                   4
#define NX_LWM2M_CLIENT_SERVER_DISABLE_TIMEOUT_ID           5
#define NX_LWM2M_CLIENT_SERVER_STORING_ID                   6
#define NX_LWM2M_CLIENT_SERVER_BINDING_ID                   7
#define NX_LWM2M_CLIENT_SERVER_UPDATE_ID                    8


/* Define the OMA Access Control Object Resources IDs */

#define NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID_ID             0
#define NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_INSTANCE_ID_ID    1
#define NX_LWM2M_CLIENT_ACCESS_CONTROL_ACL_ID                   2
#define NX_LWM2M_CLIENT_ACCESS_CONTROL_OWNER_ID                 3


/* Define the OMA Device Object Resources IDs */

#define NX_LWM2M_CLIENT_DEVICE_MANUFACTURER_ID              0
#define NX_LWM2M_CLIENT_DEVICE_MODEL_NUMBER_ID              1
#define NX_LWM2M_CLIENT_DEVICE_SERIAL_NUMBER_ID             2
#define NX_LWM2M_CLIENT_DEVICE_FIRMWARE_VERSION_ID          3
#define NX_LWM2M_CLIENT_DEVICE_REBOOT_ID                    4
#define NX_LWM2M_CLIENT_DEVICE_FACTORY_RESET_ID             5
#define NX_LWM2M_CLIENT_DEVICE_AVAILABLE_POWER_SOURCES_ID   6
#define NX_LWM2M_CLIENT_DEVICE_POWER_SOURCE_VOLTAGE_ID      7
#define NX_LWM2M_CLIENT_DEVICE_POWER_SOURCE_CURRENT_ID      8
#define NX_LWM2M_CLIENT_DEVICE_BATTERY_LEVEL_ID             9
#define NX_LWM2M_CLIENT_DEVICE_MEMORY_FREE_ID               10
#define NX_LWM2M_CLIENT_DEVICE_ERROR_CODE_ID                11
#define NX_LWM2M_CLIENT_DEVICE_RESET_ERROR_CODE_ID          12
#define NX_LWM2M_CLIENT_DEVICE_CURRENT_TIME_ID              13
#define NX_LWM2M_CLIENT_DEVICE_UTC_OFFSET_ID                14
#define NX_LWM2M_CLIENT_DEVICE_TIMEZONE_ID                  15
#define NX_LWM2M_CLIENT_DEVICE_BINDING_MODES_ID             16
#define NX_LWM2M_CLIENT_DEVICE_DEVICE_TYPE_ID               17
#define NX_LWM2M_CLIENT_DEVICE_HARDWARE_VERSION_ID          18
#define NX_LWM2M_CLIENT_DEVICE_SOFTWARE_VERSION_ID          19
#define NX_LWM2M_CLIENT_DEVICE_BATTERY_STATUS_ID            20
#define NX_LWM2M_CLIENT_DEVICE_MEMORY_TOTAL_ID              21
#define NX_LWM2M_CLIENT_DEVICE_EXTDEVINFO_ID                22


/* Define the OMA Firmware Update Object Resources IDs */

#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_ID                     0
#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_URI_ID                 1
#define NX_LWM2M_CLIENT_FIRMWARE_UPDATE_ID                      2
#define NX_LWM2M_CLIENT_FIRMWARE_STATE_ID                       3
#define NX_LWM2M_CLIENT_FIRMWARE_UPDATE_SUPPORTED_OBJECTS_ID    4
#define NX_LWM2M_CLIENT_FIRMWARE_UPDATE_RESULT_ID               5
#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_NAME_ID                6
#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_VERSION_ID             7
#define NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_SUPPORT_ID            8
#define NX_LWM2M_CLIENT_FIRMWARE_DELIVERY_METHOD_ID             9


/* Define the data types of a LWM2M resource */

#define NX_LWM2M_CLIENT_RESOURCE_NONE                       0x00
#define NX_LWM2M_CLIENT_RESOURCE_STRING                     0x01
#define NX_LWM2M_CLIENT_RESOURCE_INTEGER32                  0x02
#define NX_LWM2M_CLIENT_RESOURCE_INTEGER64                  0x03
#define NX_LWM2M_CLIENT_RESOURCE_FLOAT32                    0x04
#define NX_LWM2M_CLIENT_RESOURCE_FLOAT64                    0x05
#define NX_LWM2M_CLIENT_RESOURCE_BOOLEAN                    0x06
#define NX_LWM2M_CLIENT_RESOURCE_OPAQUE                     0x07
#define NX_LWM2M_CLIENT_RESOURCE_OBJLNK                     0x08

#define NX_LWM2M_CLIENT_RESOURCE_TLV                        0x09
#define NX_LWM2M_CLIENT_RESOURCE_TEXT                       0x0a

#define NX_LWM2M_CLIENT_RESOURCE_OBJECT_INSTANCE            0x0b
#define NX_LWM2M_CLIENT_RESOURCE_OBJECT_INSTANCE_TLV        0x0c

#define NX_LWM2M_CLIENT_RESOURCE_MULTIPLE                   0x10
#define NX_LWM2M_CLIENT_RESOURCE_MULTIPLE_TLV               0x11
#define NX_LWM2M_CLIENT_RESOURCE_MULTIPLE_UCHAR             0x12
#define NX_LWM2M_CLIENT_RESOURCE_MULTIPLE_ACL               0x13

#define NX_LWM2M_CLIENT_RESOURCE_IS_MULTIPLE(t)             ((t) & 0x10)


/* Define the event flags of the LWM2M Client */

#define NX_LWM2M_CLIENT_EVENT_WAKEUP                        ((ULONG)0x00000001)
#define NX_LWM2M_CLIENT_EVENT_COAP_MESSAGE                  ((ULONG)0x00000002)
#define NX_LWM2M_CLIENT_EVENT_COAPS_MESSAGE                 ((ULONG)0x00000004)
#define NX_LWM2M_CLIENT_EVENT_TERM_REQ                      ((ULONG)0x00000008)
#define NX_LWM2M_CLIENT_EVENT_TERM_DONE                     ((ULONG)0x00000010)

/* Define the sub-state of a session */

#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_IDLE               0
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_BOOTSTRAP          1
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_REGISTER           2
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_UPDATE             3
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_DISABLE            4
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_DEREGISTER         5
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_REQUEST_SENT       6
#define NX_LWM2M_CLIENT_SESSION_SUBSTATE_WAIT               7

/* Define the session flags */

#define NX_LWM2M_CLIENT_SESSION_UPDATE_LIFETIME             ((ULONG)0x00000001)
#define NX_LWM2M_CLIENT_SESSION_UPDATE_BINDING_MODE         ((ULONG)0x00000002)
#define NX_LWM2M_CLIENT_SESSION_UPDATE_MSISDN               ((ULONG)0x00000004)
#define NX_LWM2M_CLIENT_SESSION_UPDATE_OBJECTS_LIST         ((ULONG)0x00000008)

#define NX_LWM2M_CLIENT_SESSION_UPDATE_ALL \
        (NX_LWM2M_CLIENT_SESSION_UPDATE_LIFETIME            | \
         NX_LWM2M_CLIENT_SESSION_UPDATE_BINDING_MODE        | \
         NX_LWM2M_CLIENT_SESSION_UPDATE_MSISDN              | \
         NX_LWM2M_CLIENT_SESSION_UPDATE_OBJECTS_LIST)

/* Define the resource operation */

#define NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ             ((ULONG)0x00000001)
#define NX_LWM2M_CLIENT_RESOURCE_OPERATION_WRITE            ((ULONG)0x00000002)
#define NX_LWM2M_CLIENT_RESOURCE_OPERATION_EXECUTABLE       ((ULONG)0x00000004)

#define NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ_WRITE       (NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ | NX_LWM2M_CLIENT_RESOURCE_OPERATION_WRITE)

/* Define the structure of an Object Link */

typedef struct NX_LWM2M_OBJLNK_STRUCT
{

    /* The Object ID */
    NX_LWM2M_ID             objlnk_object_id;

    /* The Object Instance ID */
    NX_LWM2M_ID             objlnk_instance_id;

} NX_LWM2M_OBJLNK;

/* Define the value of a LWM2M Resource */

typedef struct NX_LWM2M_CLIENT_RESOURCE_STRUCT
{

    /* The ID of the resource */
    NX_LWM2M_ID             resource_id;

    /* The type of the resource */
    UCHAR                   resource_type;

    /* The dim for multiple resource */
    UCHAR                   resource_dim;

    /* The operation of resource */
    ULONG                   resource_operation;

    /* The value of the resource */
    union
    {
        struct
        {
            const VOID     *resource_buffer_ptr;
            UINT            resource_buffer_length;
        } resource_bufferdata;
        NX_LWM2M_INT32      resource_integer32data;
        NX_LWM2M_INT64      resource_integer64data;
        NX_LWM2M_FLOAT32    resource_float32data;
        NX_LWM2M_FLOAT64    resource_float64data;
        NX_LWM2M_BOOL       resource_booleandata;
        NX_LWM2M_OBJLNK     resource_objlnkdata;
    } resource_value;

} NX_LWM2M_CLIENT_RESOURCE;


/* Define the operation type */

#define NX_LWM2M_CLIENT_OBJECT_READ                         0
#define NX_LWM2M_CLIENT_OBJECT_DISCOVER                     1
#define NX_LWM2M_CLIENT_OBJECT_WRITE                        2
#define NX_LWM2M_CLIENT_OBJECT_EXECUTE                      3
#define NX_LWM2M_CLIENT_OBJECT_CREATE                       4
#define NX_LWM2M_CLIENT_OBJECT_DELETE                       5

/* Define the type of 'Write' operation */

#define NX_LWM2M_CLIENT_OBJECT_WRITE_UPDATE                 0
#define NX_LWM2M_CLIENT_OBJECT_WRITE_REPLACE_INSTANCE       1
#define NX_LWM2M_CLIENT_OBJECT_WRITE_REPLACE_RESOURCE       2
#define NX_LWM2M_CLIENT_OBJECT_WRITE_CREATE                 3
#define NX_LWM2M_CLIENT_OBJECT_WRITE_BOOTSTRAP              4


/* Define the structure of the operation arguments */

typedef struct NX_LWM2M_CLIENT_OPERATION_ARGUMENT_STRUCT
{

    /* The instance ID */
    NX_LWM2M_ID                             instance_id;

    /* The flag of bootstrap */
    NX_LWM2M_BOOL                           bootstrap;

    /* Reserved */
    UCHAR                                   reserved;

} NX_LWM2M_CLIENT_OPERATION_ARGUMENT;


/* Define the structure of an instance of a LWM2M Object */

typedef struct NX_LWM2M_CLIENT_OBJECT_INSTANCE_STRUCT
{
    /* Pointer to the next instance in the list */
    struct NX_LWM2M_CLIENT_OBJECT_INSTANCE_STRUCT 
                                           *object_instance_next;

    /* The instance ID */
    NX_LWM2M_ID                             object_instance_id;

} NX_LWM2M_CLIENT_OBJECT_INSTANCE;


/* Define the LWM2M Client Object data type */

typedef struct NX_LWM2M_CLIENT_OBJECT_STRUCT NX_LWM2M_CLIENT_OBJECT;


/* Define the LWM2M Client Object operation callback */

typedef UINT (*NX_LWM2M_CLIENT_OBJECT_OPERATION_CALLBACK)(UINT operation, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT *resource_count, VOID *args_ptr, UINT args_length);


/* Define the structure of a LWM2M Object */

struct NX_LWM2M_CLIENT_OBJECT_STRUCT
{
    /* Pointer to the next object in the database */
    struct NX_LWM2M_CLIENT_OBJECT_STRUCT   *object_next;

    /* Pointer to the LWM2M Client */
    struct NX_LWM2M_CLIENT_STRUCT          *object_client_ptr;

    /* The object ID */
    NX_LWM2M_ID                             object_id;

    /* The operation callback function */
    NX_LWM2M_CLIENT_OBJECT_OPERATION_CALLBACK
                                            object_operation;

    /* Pointer to the list of instances */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE        *object_instances;

};


/* Define the internal type used to store floating point values */
typedef double NX_LWM2M_CLIENT_NOTIFY_NUMBER;


/* Define the request options */

typedef struct NX_LWM2M_CLIENT_REQUEST_OPTIONS_STRUCT
{

    /* Option flags */
    UINT                                    flags;

    /* Object ID */
    NX_LWM2M_ID                             object_id;

    /* Instance ID */
    NX_LWM2M_ID                             instance_id;

    /* Resource ID */
    NX_LWM2M_ID                             resource_id;

    /* CoAP options */
    UINT                                    accept;
    UINT                                    content_format;
    UINT                                    observe;

    /* Set attribute */
    CHAR                                    attr_set;

    /* Unset attribute */
    CHAR                                    attr_unset;

    /* Attributes */
    NX_LWM2M_INT32                          pmin;
    NX_LWM2M_INT32                          pmax;
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           gt;
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           lt;
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           stp;

} NX_LWM2M_CLIENT_REQUEST_OPTIONS;


/* Define the notification structure flags */

#define NX_LWM2M_CLIENT_NOTIFY_TOKEN_LEN                    0xF
#define NX_LWM2M_CLIENT_NOTIFY_OBSERVE                      (0x00000010)
#define NX_LWM2M_CLIENT_NOTIFY_ATTR_PMIN                    (0x00000020)
#define NX_LWM2M_CLIENT_NOTIFY_ATTR_PMAX                    (0x00000040)
#define NX_LWM2M_CLIENT_NOTIFY_ATTR_GT                      (0x00000080)
#define NX_LWM2M_CLIENT_NOTIFY_ATTR_LT                      (0x00000100)
#define NX_LWM2M_CLIENT_NOTIFY_ATTR_STP                     (0x00000200)
#define NX_LWM2M_CLIENT_NOTIFY_CHANGED                      (0x00000400)
#define NX_LWM2M_CLIENT_NOTIFY_TIMER                        (0x00000800)
#define NX_LWM2M_CLIENT_NOTIFY_ACCEPT_TLV                   (0x00001000)

#define NX_LWM2M_CLIENT_NOTIFY_ATTR_FLAGS                   (NX_LWM2M_CLIENT_NOTIFY_ATTR_PMIN | \
                                                             NX_LWM2M_CLIENT_NOTIFY_ATTR_PMAX | \
                                                             NX_LWM2M_CLIENT_NOTIFY_ATTR_GT   | \
                                                             NX_LWM2M_CLIENT_NOTIFY_ATTR_LT   | \
                                                             NX_LWM2M_CLIENT_NOTIFY_ATTR_STP)


/* Define the maximum length of token */

#define NX_LWM2M_CLIENT_NOTIFY_TOKEN_MAX_LEN                8


/* Define the notification structure */

typedef struct NX_LWM2M_CLIENT_NOTIFY_STRUCT
{
    /* Pointer to the next notification */
    struct NX_LWM2M_CLIENT_NOTIFY_STRUCT   *notify_next;

    /* Pointer to the Object */
    NX_LWM2M_CLIENT_OBJECT                 *notify_object_ptr;

    /* Pointer to the Object Instance */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE        *notify_instance_ptr;

    /* The Resource ID */
    NX_LWM2M_ID                             notify_resource_id;

    /* The ID of the last message */
    USHORT                                  notify_last_id;

    /* Some flags */
    UINT                                    notify_flags;

    /* Token */
    UCHAR                                   notify_token[NX_LWM2M_CLIENT_NOTIFY_TOKEN_MAX_LEN];

    /* The last notified number value of the resource */
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           notify_last_value;

    /* The time of the last notify message */
    ULONG                                   notify_last_time;

    /* Timer expiration time */
    ULONG                                   notify_timer;

    /* The 'Minimum Period' attribute */
    NX_LWM2M_INT32                          notify_attr_pmin;

    /* The 'Maximum Period' attribute */
    NX_LWM2M_INT32                          notify_attr_pmax;

    /* The 'Greater Than' attribute */
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           notify_attr_gt;

    /* The 'Less Than' attribute */
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           notify_attr_lt;

    /* The 'Step' attribute */
    NX_LWM2M_CLIENT_NOTIFY_NUMBER           notify_attr_stp;

} NX_LWM2M_CLIENT_NOTIFY;


/* Define the DTLS security mode */

#define NX_LWM2M_CLIENT_SECURITY_MODE_PRE_SHARED_KEY        0
#define NX_LWM2M_CLIENT_SECURITY_MODE_RAW_PUBLIC_KEY        1
#define NX_LWM2M_CLIENT_SECURITY_MODE_CERTIFICATE           2
#define NX_LWM2M_CLIENT_SECURITY_MODE_NOSEC                 3
#define NX_LWM2M_CLIENT_SECURITY_MODE_UNDEFINED             255


#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES


/* define the structure of an instance of the LWM2M Security Object */

typedef struct NX_LWM2M_CLIENT_SECURITY_INSTANCE_STRUCT
{
    /* The LWM2M Object Instance structure */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE         security_instance;

    /* The Short Server ID */
    NX_LWM2M_ID                             security_instance_server_id;

    /* Determine if the instance is a Bootstrap Server */
    NX_LWM2M_BOOL                           security_instance_bootstrap;

    /* The UDP security mode */
    UCHAR                                   security_instance_mode;

    /* The URI of the server */
    CHAR                                    security_instance_server_uri[NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_URI];
    UINT                                    security_instance_server_uri_len;

    /* Hold-off timer */
    NX_LWM2M_INT32                          security_instance_hold_off;

    /* Public Key or Identity */
    UCHAR                                   security_instance_pub_key_or_id[NX_LWM2M_CLIENT_SECURITY_MAX_PUBLIC_KEY_OR_IDENTITY];
    UINT                                    security_instance_pub_key_or_id_len;

    /* Server Public Key */
    UCHAR                                   security_instance_server_pub_key[NX_LWM2M_CLIENT_SECURITY_MAX_SERVER_PUBLIC_KEY];
    UINT                                    security_instance_server_pub_key_len;

    /* Secret Key */
    UCHAR                                   security_instance_secret_key[NX_LWM2M_CLIENT_SECURITY_MAX_SECRET_KEY];
    UINT                                    security_instance_secret_key_len;

} NX_LWM2M_CLIENT_SECURITY_INSTANCE;


/* Define the structure of the LWM2M Security Object */

typedef struct NX_LWM2M_CLIENT_SECURITY_OBJECT_STRUCT
{
    /* The LWM2M Object structure */
    NX_LWM2M_CLIENT_OBJECT                  security_object;

    /* The list of free Security Object Instances */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE        *security_object_instances_free_list;

    /* The Security Object Instances data */
    NX_LWM2M_CLIENT_SECURITY_INSTANCE       security_object_instances[NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES];

} NX_LWM2M_CLIENT_SECURITY_OBJECT;

#endif  /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */


/* Define the Current Transport Binding and Mode */

#define NX_LWM2M_CLIENT_BINDING_U           0x1
#define NX_LWM2M_CLIENT_BINDING_UQ          0x3
#define NX_LWM2M_CLIENT_BINDING_S           0x4
#define NX_LWM2M_CLIENT_BINDING_SQ          0xc
#define NX_LWM2M_CLIENT_BINDING_US          (NX_LWM2M_CLIENT_BINDING_U | NX_LWM2M_CLIENT_BINDING_S)
#define NX_LWM2M_CLIENT_BINDING_UQS         (NX_LWM2M_CLIENT_BINDING_UQ | NX_LWM2M_CLIENT_BINDING_S)
#define NX_LWM2M_CLIENT_BINDING_USQ         (NX_LWM2M_CLIENT_BINDING_U | NX_LWM2M_CLIENT_BINDING_SQ)
#define NX_LWM2M_CLIENT_BINDING_UQSQ        (NX_LWM2M_CLIENT_BINDING_UQ | NX_LWM2M_CLIENT_BINDING_SQ)


/* define the structure of an instance of the LWM2M Server Object */

typedef struct NX_LWM2M_CLIENT_SERVER_INSTANCE_STRUCT
{
    /* The LWM2M Object Instance structure */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE         server_instance;

    /* Short Server ID */
    NX_LWM2M_ID                             server_instance_short_id;

    /* Notification Storing When Disabled or Offline */
    NX_LWM2M_BOOL                           server_instance_storing;

    /* Binding */
    UCHAR                                   server_instance_binding;

    /* Lifetime */
    NX_LWM2M_INT32                          server_instance_lifetime;

    /* Default Minimum Period */
    NX_LWM2M_INT32                          server_instance_min_period;

    /* Default Maximum Period */
    NX_LWM2M_INT32                          server_instance_max_period;

    /* Disable Timeout */
    NX_LWM2M_INT32                          server_instance_disable_timeout;

} NX_LWM2M_CLIENT_SERVER_INSTANCE;


/* Define the structure of the LWM2M Server Object */

typedef struct NX_LWM2M_CLIENT_SERVER_OBJECT_STRUCT
{
    /* The LWM2M Object structure */
    NX_LWM2M_CLIENT_OBJECT                  server_object;

    /* The list of free Server Object Instances */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE        *server_object_instances_free_list;

    /* The Server Object Instances data */
    NX_LWM2M_CLIENT_SERVER_INSTANCE         server_object_instances[NX_LWM2M_CLIENT_MAX_SERVER_INSTANCES];

} NX_LWM2M_CLIENT_SERVER_OBJECT;


#if NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES

/* Define the ACL structure */

typedef struct NX_LWM2M_CLIENT_ACL_STRUCT
{
    /* The server ID */
    NX_LWM2M_ID                             acl_server_id;

    /* The ACL flags */
    USHORT                                  acl_flags;

} NX_LWM2M_CLIENT_ACL;


/* Define the structure of an instance of the LWM2M Access Control object */

typedef struct NX_LWM2M_CLIENT_ACCESS_CONTROL_INSTANCE_STRUCT
{
    /* The LWM2M Object Instance structure */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE         access_control_instance;

    /* The Object ID */
    NX_LWM2M_ID                             access_control_instance_object_id;

    /* The Object Instance ID */
    NX_LWM2M_ID                             access_control_instance_object_instance_id;

    /* The Access Control Owner */
    NX_LWM2M_ID                             access_control_instance_owner;

    /* The number of ACL resources */
    USHORT                                  access_control_instance_acl_count;

    /* The array of ACL resources */
    NX_LWM2M_CLIENT_ACL                     access_control_instance_acl[NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_ACLS];

} NX_LWM2M_CLIENT_ACCESS_CONTROL_INSTANCE;


/* Define the structure of the LWM2M Access Control Object */

typedef struct NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_STRUCT
{
    /* The LWM2M Object structure */
    NX_LWM2M_CLIENT_OBJECT                  access_control_object;

    /* The list of free Access Control Object Instances */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE        *access_control_object_instances_free_list;

    /* The Access Control Object Instances data */
    NX_LWM2M_CLIENT_ACCESS_CONTROL_INSTANCE access_control_object_instances[NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES];

} NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT;

#endif

/* Define the Device operation callback */

typedef UINT (*NX_LWM2M_CLIENT_DEVICE_OPERATION_CALLBACK)(UINT operation, struct NX_LWM2M_CLIENT_STRUCT *client_ptr, NX_LWM2M_CLIENT_RESOURCE *resources, UINT *num_resources, VOID *args_ptr, UINT args_length);


/* Define the structure of the LWM2M Device Object */

typedef struct NX_LWM2M_CLIENT_DEVICE_OBJECT_STRUCT
{
    /* The LWM2M Object structure */
    NX_LWM2M_CLIENT_OBJECT                  device_object;

    /* The callback for the operations implemented by the application */
    NX_LWM2M_CLIENT_DEVICE_OPERATION_CALLBACK
                                            device_object_user_operation;

    /* The number of errors */
    USHORT                                  device_object_error_count;

    /* The stack of errors */
    UCHAR                                   device_object_error[NX_LWM2M_CLIENT_MAX_DEVICE_ERRORS];

    /* The supported binding modes */
    UCHAR                                   device_object_binding_modes;

    /* The single instance of the object */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE         device_object_instance;

} NX_LWM2M_CLIENT_DEVICE_OBJECT;


/* Define the LWM2M Firmware Update Object Data Type */

typedef struct NX_LWM2M_CLIENT_FIRMWARE_STRUCT NX_LWM2M_CLIENT_FIRMWARE;


/* Define the firmware package commands */

#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_BEGIN          0
#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_BLOCK          1
#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_END            2
#define NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_ABORT          3


/* Define the firmware package callback */

typedef UINT (*NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_CALLBACK)(NX_LWM2M_CLIENT_FIRMWARE *firmware, UINT command, ULONG block_offset, const VOID *block_ptr, UINT block_length);


/* Define the firmware uri callback */

typedef UINT (*NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_URI_CALLBACK)(NX_LWM2M_CLIENT_FIRMWARE *firmware, const CHAR *uri, UINT uri_length);


/* Define the firmware update callback */

typedef UINT (*NX_LWM2M_CLIENT_FIRMWARE_UPDATE_CALLBACK)(NX_LWM2M_CLIENT_FIRMWARE *firmware, NX_LWM2M_BOOL update_objects, const CHAR *args_ptr, UINT args_length);


/* Define the supported protocols of the Firmware Update Object */

#define NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_COAP                  (0x00000001)
#define NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_COAPS                 (0x00000002)
#define NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_HTTP                  (0x00000004)
#define NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_HTTPS                 (0x00000008)


/* The maximum number of supported protocols */

#define NX_LWM2M_CLIENT_FIRMWARE_MAX_PROTOCOLS                  4


/* Define the state of the Firmware Update */

#define NX_LWM2M_CLIENT_FIRMWARE_STATE_IDLE                     0
#define NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADING              1
#define NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADED               2
#define NX_LWM2M_CLIENT_FIRMWARE_STATE_UPDATING                 3


/* Define the result of the Firmware Update */

#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_INIT                        0
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_SUCCESS                     1
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_NOT_ENOUGHT_FLASH           2
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_OUT_OF_RAM                  3
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_CONNECTION_LOST             4
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_INTEGRITY_CHECK_FAILURE     5
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_UNSUPPORTED_PACKAGE_TYPE    6
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_INVALID_URI                 7
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_UPDATE_FAILED               8
#define NX_LWM2M_CLIENT_FIRMWARE_RESULT_UNSUPPORTED_PROTOCOL        9


/* Define the structure of the LWM2M Firmware Update Object */

struct NX_LWM2M_CLIENT_FIRMWARE_STRUCT
{

    /* The LWM2M Object structure */
    NX_LWM2M_CLIENT_OBJECT                  firmware_object;

    /* The package callback */
    NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_CALLBACK
                                            firmware_package_callback;

    /* The uri callback */
    NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_URI_CALLBACK
                                            firmware_package_uri_callback;

    /* The update callback */
    NX_LWM2M_CLIENT_FIRMWARE_UPDATE_CALLBACK
                                            firmware_update_callback;

    /* The state of the firmware update */
    UCHAR                                   firmware_state;

    /* The value of Update Supported Objects resource */
    NX_LWM2M_BOOL                           firmware_update_supported_objects;

    /* The result of the firmware update */
    UCHAR                                   firmware_result;

    /* The number of supported protocols */
    UCHAR                                   firmware_protocols_count;

    /* The list of supported protocols */
    UCHAR                                   firmware_protocols[NX_LWM2M_CLIENT_FIRMWARE_MAX_PROTOCOLS];

    /* The name of the firmware package */
    const CHAR *                            firmware_package_name;

    /* The length of the firmware package name */
    UINT                                    firmware_package_name_length;

    /* The version of the firmware package */
    const CHAR *                            firmware_package_version;
    
    /* The length of the firmware package version */
    UINT                                    firmware_package_version_length;

    /* The single instance of the object */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE         firmware_instance;
};


/* Define a LWM2M Client socket */
typedef struct NX_LWM2M_CLIENT_SOCKET_STRUCT
{
    /* The UDP Socket */
    NX_UDP_SOCKET                           udp_socket;

    /* Pointer to the LWM2M Client */
    struct NX_LWM2M_CLIENT_STRUCT          *client_ptr;

} NX_LWM2M_CLIENT_SOCKET;


/* Define the LWM2M Client Session Data Type */

typedef struct NX_LWM2M_CLIENT_SESSION_STRUCT NX_LWM2M_CLIENT_SESSION;


/* Define the LWM2M Client Session State change callback */

typedef VOID (*NX_LWM2M_CLIENT_SESSION_STATE_CALLBACK)(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT state);


/* Define a LWM2M Client Session structure */

struct NX_LWM2M_CLIENT_SESSION_STRUCT
{

    /* Pointer to the next session in list */
    struct NX_LWM2M_CLIENT_SESSION_STRUCT  *nx_lwm2m_client_session_next;

    /* Pointer to LWM2M Client */
    struct NX_LWM2M_CLIENT_STRUCT          *nx_lwm2m_client_session_client_ptr;

    /* The LWM2M Server Object Instance, NULL for a Bootstrap Server */
    NX_LWM2M_CLIENT_SERVER_INSTANCE        *nx_lwm2m_client_session_server_instance_ptr;

    /* The state change callback */
    NX_LWM2M_CLIENT_SESSION_STATE_CALLBACK  nx_lwm2m_client_session_state_callback;

    /* The state of the session */
    UINT                                    nx_lwm2m_client_session_state;

    /* The sub-state of the session */
    UINT                                    nx_lwm2m_client_session_substate;

    /* Timer expiration clock value */
    ULONG                                   nx_lwm2m_client_session_timer;

    /* Retransmission timer */
    UINT                                    nx_lwm2m_client_session_retransmit_timer;

    /* Retransmission counter */
    UCHAR                                   nx_lwm2m_client_session_retransmit_count;

    /* Request ID */
    USHORT                                  nx_lwm2m_client_session_request_id;

    /* Token */
    UCHAR                                   nx_lwm2m_client_session_token[NX_LWM2M_CLIENT_COAP_TOKEN_LEN];

    /* The error code */
    UINT                                    nx_lwm2m_client_session_error;

    /* The address of the server */
    NXD_ADDRESS                             nx_lwm2m_client_session_server_address;

    /* The port number of the server */
    UINT                                    nx_lwm2m_client_session_server_port;

    /* The security ID (bootstrap) or Short Server ID (LWM2M) */
    NX_LWM2M_ID                             nx_lwm2m_client_session_server_id;

    /* The Uri-Path option */
    CHAR                                    nx_lwm2m_client_session_uri_path[NX_LWM2M_CLIENT_MAX_COAP_URI_PATH];

    /* The length of the Uri-Path option */
    UINT                                    nx_lwm2m_client_session_uri_path_length;

    /* Some flags */
    UINT                                    nx_lwm2m_client_session_flags;

    /* Pointer to the list of notifications for this session */
    NX_LWM2M_CLIENT_NOTIFY                 *nx_lwm2m_client_session_notifications;

#ifdef NX_SECURE_ENABLE_DTLS
    /* The socket used for the DTLS connection */
    NX_LWM2M_CLIENT_SOCKET                  nx_lwm2m_client_session_dtls_socket;

    /* The DTLS session */
    NX_SECURE_DTLS_SESSION                 *nx_lwm2m_client_session_dtls_session;
#endif /* NX_SECURE_ENABLE_DTLS */

};


/* Define the state of the LWM2M Client Session */

#define NX_LWM2M_CLIENT_SESSION_INIT                    0

#define NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_WAITING       1
#define NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_REQUESTING    2
#define NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_INITIATED     3
#define NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_FINISHED      4
#define NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_ERROR         5

#define NX_LWM2M_CLIENT_SESSION_REGISTERING             6
#define NX_LWM2M_CLIENT_SESSION_REGISTERED              7
#define NX_LWM2M_CLIENT_SESSION_UPDATING                8
#define NX_LWM2M_CLIENT_SESSION_DISABLING               9
#define NX_LWM2M_CLIENT_SESSION_DEREGISTERING           10
#define NX_LWM2M_CLIENT_SESSION_DEREGISTERED            11
#define NX_LWM2M_CLIENT_SESSION_DISABLED                12
#define NX_LWM2M_CLIENT_SESSION_ERROR                   13


/* Define the structure of a LWM2M Client */

typedef struct NX_LWM2M_CLIENT_STRUCT
{
    /* The mutex used to protect concurrent accesses to the LWM2M Client */
    TX_MUTEX                                nx_lwm2m_client_mutex;

    /* Pointer to the LWM2M Client endpoint name */
    const CHAR                             *nx_lwm2m_client_name;

    /* The length of LWM2M Client endpoint name */
    UINT                                    nx_lwm2m_client_name_length;

    /* Pointer to the MSISDN where the LWM2M Client can be reached */
    const CHAR                             *nx_lwm2m_client_msisdn;

    /* The length of MSISDN */
    UINT                                    nx_lwm2m_client_msisdn_length;

    /* Pointer to IP instance */
    NX_IP                                  *nx_lwm2m_client_ip_ptr;

    /* Pointer to IP Packet Pool */
    NX_PACKET_POOL                         *nx_lwm2m_client_pool_ptr;

    /* The list of free notifications */
    NX_LWM2M_CLIENT_NOTIFY                 *nx_lwm2m_client_notifications_free_list;

    /* The notification structures */
    NX_LWM2M_CLIENT_NOTIFY                  nx_lwm2m_client_notifications[NX_LWM2M_CLIENT_MAX_NOTIFICATIONS];

    /* The list of sessions  of the LWM2M Client*/
    NX_LWM2M_CLIENT_SESSION                *nx_lwm2m_client_sessions;

#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
    /* The Security Object */
    NX_LWM2M_CLIENT_SECURITY_OBJECT         nx_lwm2m_client_security;
#endif

    /* The Server Object */
    NX_LWM2M_CLIENT_SERVER_OBJECT           nx_lwm2m_client_server;

#if NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES
    /* The Access Control Object */
    NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT   nx_lwm2m_client_access_control;
#endif

    /* The Device Object */
    NX_LWM2M_CLIENT_DEVICE_OBJECT           nx_lwm2m_client_device;
    
    /* The head of Object list */
    NX_LWM2M_CLIENT_OBJECT                 *nx_lwm2m_client_object_list_head;

    /* The events flags */
    TX_EVENT_FLAGS_GROUP                    nx_lwm2m_client_event_flags;

    /* The CoAP socket */
    NX_LWM2M_CLIENT_SOCKET                  nx_lwm2m_client_coap_socket;

    /* Request ID generator */
    USHORT                                  nx_lwm2m_client_request_id;

    /* The LWM2M Client thread */
    TX_THREAD                               nx_lwm2m_client_thread;

    /* Temporary storage for requests resources */
    NX_LWM2M_CLIENT_RESOURCE                nx_lwm2m_client_temp_resources[NX_LWM2M_CLIENT_MAX_RESOURCES];

    /* Temporary storage for multiple resources */
    NX_LWM2M_CLIENT_RESOURCE                nx_lwm2m_client_mulitple_temp_resources[NX_LWM2M_CLIENT_MAX_MULTIPLE_RESOURCES];

    /* Temporary storage for requests options */
    NX_LWM2M_CLIENT_REQUEST_OPTIONS         nx_lwm2m_client_request_options;

} NX_LWM2M_CLIENT;


/* TLV Header Type field: */

/* Bit 7-6: Indicates the type of Identifier */
#define NX_LWM2M_CLIENT_TLV_TYPE_ID_TYPE_MASK              0xc0

#define NX_LWM2M_CLIENT_TLV_OBJECT_INSTANCE                0x00
#define NX_LWM2M_CLIENT_TLV_RESOURCE_INSTANCE              0x40
#define NX_LWM2M_CLIENT_TLV_RESOURCE_MULTIPLE              0x80
#define NX_LWM2M_CLIENT_TLV_RESOURCE_VALUE                 0xc0

/* Bit 5: Indicates the Length of the Identifier */
#define NX_LWM2M_CLIENT_TLV_TYPE_ID_LENGTH                 0x20

/* Bit 4-3: Indicate the type of Length */
#define NX_LWM2M_CLIENT_TLV_TYPE_LENGTH_TYPE_MASK          0x18
#define NX_LWM2M_CLIENT_TLV_TYPE_LENGTH_NONE               0x00
#define NX_LWM2M_CLIENT_TLV_TYPE_LENGTH_8_BIT              0x08
#define NX_LWM2M_CLIENT_TLV_TYPE_LENGTH_16_BIT             0x10
#define NX_LWM2M_CLIENT_TLV_TYPE_LENGTH_24_BIT             0x18

/* Bit 2-0: Indicate the Length of the Value */
#define NX_LWM2M_CLIENT_TLV_TYPE_LENGTH_MASK               0x07

/* Length of fixed-size TLV Values */
#define NX_LWM2M_CLIENT_TLV_LENGTH_INTEGER8                1
#define NX_LWM2M_CLIENT_TLV_LENGTH_INTEGER16               2
#define NX_LWM2M_CLIENT_TLV_LENGTH_INTEGER32               4
#define NX_LWM2M_CLIENT_TLV_LENGTH_INTEGER64               8
#define NX_LWM2M_CLIENT_TLV_LENGTH_FLOAT32                 4
#define NX_LWM2M_CLIENT_TLV_LENGTH_FLOAT64                 8
#define NX_LWM2M_CLIENT_TLV_LENGTH_BOOLEAN                 1
#define NX_LWM2M_CLIENT_TLV_LENGTH_OBJLNK                  4


/* Define the maximum length of a string representing an ID (unsigned 16-bit integer) */

#define NX_LWM2M_CLIENT_STRCONV_ID_MAXLEN                  5   /* '65535' */

/* Define the maximum length of a string representing a 32-bit integer */

#define NX_LWM2M_CLIENT_STRCONV_INT32_MAXLEN               11  /* '-2147483647' */

/* Define the number of digital placed after decimal */

#define NX_LWM2M_CLIENT_DOUBLE_DECIMAL_PLACE_DIGITS        2


/* Define CoAP constants.  */

#define NX_LWM2M_CLIENT_COAP_HEADER_LEN                    4

#define NX_LWM2M_CLIENT_COAP_VERSION_1                     0x40
#define NX_LWM2M_CLIENT_COAP_VERSION_MASK                  0xc0

#define NX_LWM2M_CLIENT_COAP_TYPE_MASK                     0x30
#define NX_LWM2M_CLIENT_COAP_TYPE_CON                      0x00
#define NX_LWM2M_CLIENT_COAP_TYPE_NON                      0x10
#define NX_LWM2M_CLIENT_COAP_TYPE_ACK                      0x20
#define NX_LWM2M_CLIENT_COAP_TYPE_RST                      0x30

#define NX_LWM2M_CLIENT_COAP_TOKEN_LEN_MASK                0x0f

#define NX_LWM2M_CLIENT_COAP_TOKEN_MAXLEN                  8

#define NX_LWM2M_CLIENT_COAP_CLASS_MASK                    0xe0

#define NX_LWM2M_CLIENT_COAP_CLASS_REQUEST                 0x00
#define NX_LWM2M_CLIENT_COAP_CLASS_SUCCESS                 0x40
#define NX_LWM2M_CLIENT_COAP_CLASS_CLIENT_ERROR            0x80
#define NX_LWM2M_CLIENT_COAP_CLASS_SERVER_ERROR            0xa0

#define NX_LWM2M_CLIENT_COAP_REQUEST_GET                   0x01
#define NX_LWM2M_CLIENT_COAP_REQUEST_POST                  0x02
#define NX_LWM2M_CLIENT_COAP_REQUEST_PUT                   0x03
#define NX_LWM2M_CLIENT_COAP_REQUEST_DELETE                0x04

#define NX_LWM2M_CLIENT_COAP_CODE(c,dd)                    (((c)<<5)|(dd))

#define NX_LWM2M_CLIENT_COAP_STATUS_CREATED                NX_LWM2M_CLIENT_COAP_CODE(2,1)
#define NX_LWM2M_CLIENT_COAP_STATUS_DELETED                NX_LWM2M_CLIENT_COAP_CODE(2,2)
#define NX_LWM2M_CLIENT_COAP_STATUS_CHANGED                NX_LWM2M_CLIENT_COAP_CODE(2,4)
#define NX_LWM2M_CLIENT_COAP_STATUS_CONTENT                NX_LWM2M_CLIENT_COAP_CODE(2,5)

#define NX_LWM2M_CLIENT_COAP_STATUS_BAD_REQUEST            NX_LWM2M_CLIENT_COAP_CODE(4,0)
#define NX_LWM2M_CLIENT_COAP_STATUS_UNAUTHORIZED           NX_LWM2M_CLIENT_COAP_CODE(4,1)
#define NX_LWM2M_CLIENT_COAP_STATUS_NOT_FOUND              NX_LWM2M_CLIENT_COAP_CODE(4,4)
#define NX_LWM2M_CLIENT_COAP_STATUS_METHOD_NOT_ALLOWED     NX_LWM2M_CLIENT_COAP_CODE(4,5)
#define NX_LWM2M_CLIENT_COAP_STATUS_NOT_ACCEPTABLE         NX_LWM2M_CLIENT_COAP_CODE(4,6)

#define NX_LWM2M_CLIENT_COAP_STATUS_INTERNAL_SERVER_ERROR  NX_LWM2M_CLIENT_COAP_CODE(5,0)

#define NX_LWM2M_CLIENT_COAP_PAYLOAD_MARKER                0xff

#define NX_LWM2M_CLIENT_COAP_OPTION_NONE                   0
#define NX_LWM2M_CLIENT_COAP_OPTION_OBSERVE                6
#define NX_LWM2M_CLIENT_COAP_OPTION_LOCATION_PATH          8
#define NX_LWM2M_CLIENT_COAP_OPTION_URI_PATH               11
#define NX_LWM2M_CLIENT_COAP_OPTION_CONTENT_FORMAT         12
#define NX_LWM2M_CLIENT_COAP_OPTION_URI_QUERY              15
#define NX_LWM2M_CLIENT_COAP_OPTION_ACCEPT                 17

#define NX_LWM2M_CLIENT_COAP_CONTENT_TEXT_PLAIN            0
#define NX_LWM2M_CLIENT_COAP_CONTENT_LINK_FORMAT           40
#define NX_LWM2M_CLIENT_COAP_CONTENT_OCTET_STREAM          42
#define NX_LWM2M_CLIENT_COAP_CONTENT_VND_OMA_LWM2M_TLV     11542
#define NX_LWM2M_CLIENT_COAP_CONTENT_VND_OMA_LWM2M_JSON    11543


#ifndef NX_LWM2M_CLIENT_SOURCE_CODE
/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

#define nx_lwm2m_client_create                      _nx_lwm2m_client_create
#define nx_lwm2m_client_delete                      _nx_lwm2m_client_delete
#define nx_lwm2m_client_lock                        _nx_lwm2m_client_lock
#define nx_lwm2m_client_unlock                      _nx_lwm2m_client_unlock
#define nx_lwm2m_client_object_add                  _nx_lwm2m_client_object_add
#define nx_lwm2m_client_object_remove               _nx_lwm2m_client_object_remove
#define nx_lwm2m_client_object_resource_changed     _nx_lwm2m_client_object_resource_changed
#define nx_lwm2m_client_device_callback_set         _nx_lwm2m_client_device_callback_set
#define nx_lwm2m_client_device_resource_changed     _nx_lwm2m_client_device_resource_changed
#define nx_lwm2m_client_device_error_push           _nx_lwm2m_client_device_error_push
#define nx_lwm2m_client_device_error_reset          _nx_lwm2m_client_device_error_reset
#define nx_lwm2m_client_object_next_get             _nx_lwm2m_client_object_next_get
#define nx_lwm2m_client_object_instance_next_get    _nx_lwm2m_client_object_instance_next_get
#define nx_lwm2m_client_object_read                 _nx_lwm2m_client_object_read
#define nx_lwm2m_client_object_discover             _nx_lwm2m_client_object_discover
#define nx_lwm2m_client_object_write                _nx_lwm2m_client_object_write
#define nx_lwm2m_client_object_execute              _nx_lwm2m_client_object_execute
#define nx_lwm2m_client_object_create               _nx_lwm2m_client_object_create
#define nx_lwm2m_client_object_delete               _nx_lwm2m_client_object_delete
#define nx_lwm2m_client_object_instance_add         _nx_lwm2m_client_object_instance_add
#define nx_lwm2m_client_object_instance_remove      _nx_lwm2m_client_object_instance_remove
#define nx_lwm2m_client_session_create              _nx_lwm2m_client_session_create
#define nx_lwm2m_client_session_delete              _nx_lwm2m_client_session_delete
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
#define nx_lwm2m_client_session_bootstrap           _nx_lwm2m_client_session_bootstrap
#define nx_lwm2m_client_session_register_info_get   _nx_lwm2m_client_session_register_info_get
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
#define nx_lwm2m_client_session_register            _nx_lwm2m_client_session_register
#define nx_lwm2m_client_session_update              _nx_lwm2m_client_session_update
#define nx_lwm2m_client_session_deregister          _nx_lwm2m_client_session_deregister
#define nx_lwm2m_client_session_error_get           _nx_lwm2m_client_session_error_get
#ifdef NX_SECURE_ENABLE_DTLS
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
#define nx_lwm2m_client_session_bootstrap_dtls      _nx_lwm2m_client_session_bootstrap_dtls
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
#define nx_lwm2m_client_session_register_dtls       _nx_lwm2m_client_session_register_dtls
#endif /* NX_SECURE_ENABLE_DTLS */
#define nx_lwm2m_client_firmware_create             _nx_lwm2m_client_firmware_create
#define nx_lwm2m_client_firmware_state_set          _nx_lwm2m_client_firmware_state_set
#define nx_lwm2m_client_firmware_result_set         _nx_lwm2m_client_firmware_result_set
#define nx_lwm2m_client_firmware_package_info_set   _nx_lwm2m_client_firmware_package_info_set
#define nx_lwm2m_client_resource_info_get           _nx_lwm2m_client_resource_info_get
#define nx_lwm2m_client_resource_dim_get            _nx_lwm2m_client_resource_dim_get
#define nx_lwm2m_client_resource_instances_get      _nx_lwm2m_client_resource_instances_get
#define nx_lwm2m_client_resource_string_get         _nx_lwm2m_client_resource_string_get
#define nx_lwm2m_client_resource_integer32_get      _nx_lwm2m_client_resource_integer32_get
#define nx_lwm2m_client_resource_integer64_get      _nx_lwm2m_client_resource_integer64_get
#define nx_lwm2m_client_resource_float32_get        _nx_lwm2m_client_resource_float32_get
#define nx_lwm2m_client_resource_float64_get        _nx_lwm2m_client_resource_float64_get
#define nx_lwm2m_client_resource_boolean_get        _nx_lwm2m_client_resource_boolean_get
#define nx_lwm2m_client_resource_objlnk_get         _nx_lwm2m_client_resource_objlnk_get
#define nx_lwm2m_client_resource_opaque_get         _nx_lwm2m_client_resource_opaque_get
#define nx_lwm2m_client_resource_info_set           _nx_lwm2m_client_resource_info_set
#define nx_lwm2m_client_resource_dim_set            _nx_lwm2m_client_resource_dim_set
#define nx_lwm2m_client_resource_instances_set      _nx_lwm2m_client_resource_instances_set
#define nx_lwm2m_client_resource_string_set         _nx_lwm2m_client_resource_string_set
#define nx_lwm2m_client_resource_integer32_set      _nx_lwm2m_client_resource_integer32_set
#define nx_lwm2m_client_resource_integer64_set      _nx_lwm2m_client_resource_integer64_set
#define nx_lwm2m_client_resource_float32_set        _nx_lwm2m_client_resource_float32_set
#define nx_lwm2m_client_resource_float64_set        _nx_lwm2m_client_resource_float64_set
#define nx_lwm2m_client_resource_boolean_set        _nx_lwm2m_client_resource_boolean_set
#define nx_lwm2m_client_resource_objlnk_set         _nx_lwm2m_client_resource_objlnk_set
#define nx_lwm2m_client_resource_opaque_set         _nx_lwm2m_client_resource_opaque_set

#else

#define nx_lwm2m_client_create                      _nxe_lwm2m_client_create
#define nx_lwm2m_client_delete                      _nxe_lwm2m_client_delete
#define nx_lwm2m_client_lock                        _nxe_lwm2m_client_lock
#define nx_lwm2m_client_unlock                      _nxe_lwm2m_client_unlock
#define nx_lwm2m_client_object_add                  _nxe_lwm2m_client_object_add
#define nx_lwm2m_client_object_remove               _nxe_lwm2m_client_object_remove
#define nx_lwm2m_client_object_resource_changed     _nxe_lwm2m_client_object_resource_changed
#define nx_lwm2m_client_device_callback_set         _nxe_lwm2m_client_device_callback_set
#define nx_lwm2m_client_device_resource_changed     _nxe_lwm2m_client_device_resource_changed
#define nx_lwm2m_client_device_error_push           _nxe_lwm2m_client_device_error_push
#define nx_lwm2m_client_device_error_reset          _nxe_lwm2m_client_device_error_reset
#define nx_lwm2m_client_object_next_get             _nxe_lwm2m_client_object_next_get
#define nx_lwm2m_client_object_instance_next_get    _nxe_lwm2m_client_object_instance_next_get
#define nx_lwm2m_client_object_read                 _nxe_lwm2m_client_object_read
#define nx_lwm2m_client_object_discover             _nxe_lwm2m_client_object_discover
#define nx_lwm2m_client_object_write                _nxe_lwm2m_client_object_write
#define nx_lwm2m_client_object_execute              _nxe_lwm2m_client_object_execute
#define nx_lwm2m_client_object_create               _nxe_lwm2m_client_object_create
#define nx_lwm2m_client_object_delete               _nxe_lwm2m_client_object_delete
#define nx_lwm2m_client_object_instance_add         _nxe_lwm2m_client_object_instance_add
#define nx_lwm2m_client_object_instance_remove      _nxe_lwm2m_client_object_instance_remove
#define nx_lwm2m_client_session_create              _nxe_lwm2m_client_session_create
#define nx_lwm2m_client_session_delete              _nxe_lwm2m_client_session_delete
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
#define nx_lwm2m_client_session_bootstrap           _nxe_lwm2m_client_session_bootstrap
#define nx_lwm2m_client_session_register_info_get   _nxe_lwm2m_client_session_register_info_get
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
#define nx_lwm2m_client_session_register            _nxe_lwm2m_client_session_register
#define nx_lwm2m_client_session_update              _nxe_lwm2m_client_session_update
#define nx_lwm2m_client_session_deregister          _nxe_lwm2m_client_session_deregister
#define nx_lwm2m_client_session_error_get           _nxe_lwm2m_client_session_error_get
#ifdef NX_SECURE_ENABLE_DTLS
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
#define nx_lwm2m_client_session_bootstrap_dtls      _nxe_lwm2m_client_session_bootstrap_dtls
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
#define nx_lwm2m_client_session_register_dtls       _nxe_lwm2m_client_session_register_dtls
#endif /* NX_SECURE_ENABLE_DTLS */
#define nx_lwm2m_client_firmware_create             _nxe_lwm2m_client_firmware_create
#define nx_lwm2m_client_firmware_state_set          _nxe_lwm2m_client_firmware_state_set
#define nx_lwm2m_client_firmware_result_set         _nxe_lwm2m_client_firmware_result_set
#define nx_lwm2m_client_firmware_package_info_set   _nxe_lwm2m_client_firmware_package_info_set
#define nx_lwm2m_client_resource_info_get           _nxe_lwm2m_client_resource_info_get
#define nx_lwm2m_client_resource_dim_get            _nxe_lwm2m_client_resource_dim_get
#define nx_lwm2m_client_resource_instances_get      _nxe_lwm2m_client_resource_instances_get
#define nx_lwm2m_client_resource_string_get         _nxe_lwm2m_client_resource_string_get
#define nx_lwm2m_client_resource_integer32_get      _nxe_lwm2m_client_resource_integer32_get
#define nx_lwm2m_client_resource_integer64_get      _nxe_lwm2m_client_resource_integer64_get
#define nx_lwm2m_client_resource_float32_get        _nxe_lwm2m_client_resource_float32_get
#define nx_lwm2m_client_resource_float64_get        _nxe_lwm2m_client_resource_float64_get
#define nx_lwm2m_client_resource_boolean_get        _nxe_lwm2m_client_resource_boolean_get
#define nx_lwm2m_client_resource_objlnk_get         _nxe_lwm2m_client_resource_objlnk_get
#define nx_lwm2m_client_resource_opaque_get         _nxe_lwm2m_client_resource_opaque_get
#define nx_lwm2m_client_resource_info_set           _nxe_lwm2m_client_resource_info_set
#define nx_lwm2m_client_resource_dim_set            _nxe_lwm2m_client_resource_dim_set
#define nx_lwm2m_client_resource_instances_set      _nxe_lwm2m_client_resource_instances_set
#define nx_lwm2m_client_resource_string_set         _nxe_lwm2m_client_resource_string_set
#define nx_lwm2m_client_resource_integer32_set      _nxe_lwm2m_client_resource_integer32_set
#define nx_lwm2m_client_resource_integer64_set      _nxe_lwm2m_client_resource_integer64_set
#define nx_lwm2m_client_resource_float32_set        _nxe_lwm2m_client_resource_float32_set
#define nx_lwm2m_client_resource_float64_set        _nxe_lwm2m_client_resource_float64_set
#define nx_lwm2m_client_resource_boolean_set        _nxe_lwm2m_client_resource_boolean_set
#define nx_lwm2m_client_resource_objlnk_set         _nxe_lwm2m_client_resource_objlnk_set
#define nx_lwm2m_client_resource_opaque_set         _nxe_lwm2m_client_resource_opaque_set

#endif


/* Define the functions prototypes. */

/* Client Management */
UINT nx_lwm2m_client_create(NX_LWM2M_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr,
                            const CHAR *name_ptr, UINT name_length, const CHAR *msisdn_ptr, UINT msisdn_length,
                            UCHAR binding_modes, VOID *stack_ptr, ULONG stack_size, UINT priority);
UINT nx_lwm2m_client_delete(NX_LWM2M_CLIENT *client_ptr);
UINT nx_lwm2m_client_lock(NX_LWM2M_CLIENT *client_ptr);
UINT nx_lwm2m_client_unlock(NX_LWM2M_CLIENT *client_ptr);
UINT nx_lwm2m_client_object_add(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_ID object_id, 
                                NX_LWM2M_CLIENT_OBJECT_OPERATION_CALLBACK object_operation);
UINT nx_lwm2m_client_object_remove(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr);

/* Objects Implementation */
UINT nx_lwm2m_client_object_resource_changed(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource);

/* Device Object Implementation */
UINT nx_lwm2m_client_device_callback_set(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_DEVICE_OPERATION_CALLBACK operation_callback);
UINT nx_lwm2m_client_device_resource_changed(NX_LWM2M_CLIENT *client_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource);
UINT nx_lwm2m_client_device_error_push(NX_LWM2M_CLIENT *client_ptr, UCHAR code);
UINT nx_lwm2m_client_device_error_reset(NX_LWM2M_CLIENT *client_ptr);

/* Local Device Management */
UINT nx_lwm2m_client_object_next_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID *object_id_ptr);
UINT nx_lwm2m_client_object_instance_next_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID *instance_id_ptr);
UINT nx_lwm2m_client_object_read(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT num_values, NX_LWM2M_CLIENT_RESOURCE *values);
UINT nx_lwm2m_client_object_discover(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT *num_resources, NX_LWM2M_CLIENT_RESOURCE *resources);
UINT nx_lwm2m_client_object_write(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values, UINT write_op);
UINT nx_lwm2m_client_object_execute(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, NX_LWM2M_ID resource_id, const CHAR *args_ptr, UINT args_length);
UINT nx_lwm2m_client_object_create(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID *instance_id_ptr, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values);
UINT nx_lwm2m_client_object_delete(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id);
UINT nx_lwm2m_client_object_instance_add(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_ID *instance_id_ptr);
UINT nx_lwm2m_client_object_instance_remove(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr);

/* Client Session Management */
UINT nx_lwm2m_client_session_create(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_SESSION_STATE_CALLBACK state_callback);
UINT nx_lwm2m_client_session_delete(NX_LWM2M_CLIENT_SESSION *session_ptr);
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
UINT nx_lwm2m_client_session_bootstrap(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID security_id, const NXD_ADDRESS *ip_address, UINT port);
UINT nx_lwm2m_client_session_register_info_get(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT security_instance_id, NX_LWM2M_ID *server_id, CHAR **server_uri, UINT *server_uri_len, UCHAR *security_mode,
                                               UCHAR **pub_key_or_id, UINT *pub_key_or_id_len, UCHAR **server_pub_key, UINT *server_pub_key_len, UCHAR **secret_key, UINT *secret_key_len);
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
UINT nx_lwm2m_client_session_register(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port);
UINT nx_lwm2m_client_session_update(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT nx_lwm2m_client_session_deregister(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT nx_lwm2m_client_session_error_get(NX_LWM2M_CLIENT_SESSION *session_ptr);
#ifdef NX_SECURE_ENABLE_DTLS
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
UINT nx_lwm2m_client_session_bootstrap_dtls(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID security_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
UINT nx_lwm2m_client_session_register_dtls(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
#endif /* NX_SECURE_ENABLE_DTLS */

/* Get Resource Value */
UINT nx_lwm2m_client_resource_info_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID *resource_id, ULONG *operation);
UINT nx_lwm2m_client_resource_dim_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UCHAR *dim);
UINT nx_lwm2m_client_resource_string_get(NX_LWM2M_CLIENT_RESOURCE *value, const CHAR **string_ptr, UINT *string_length);
UINT nx_lwm2m_client_resource_integer32_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_INT32 *int32_ptr);
UINT nx_lwm2m_client_resource_integer64_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_INT64 *int64_ptr);
UINT nx_lwm2m_client_resource_float32_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_FLOAT32 *float32_ptr);
UINT nx_lwm2m_client_resource_float64_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_FLOAT64 *float64_ptr);
UINT nx_lwm2m_client_resource_boolean_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_BOOL *bool_ptr);
UINT nx_lwm2m_client_resource_objlnk_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_ID *object_id, NX_LWM2M_ID *instance_id);
UINT nx_lwm2m_client_resource_opaque_get(NX_LWM2M_CLIENT_RESOURCE *value, const VOID **opaque_ptr, UINT *opaque_length);
UINT nx_lwm2m_client_resource_instances_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_RESOURCE *resource_instances, UINT *count);

/* Set Resource Value */
UINT nx_lwm2m_client_resource_info_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID resource_id, ULONG operation);
UINT nx_lwm2m_client_resource_dim_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UCHAR dim);
UINT nx_lwm2m_client_resource_string_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, CHAR *string_ptr, UINT string_length);
UINT nx_lwm2m_client_resource_integer32_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_INT32 int32_data);
UINT nx_lwm2m_client_resource_integer64_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_INT64 int64_data);
UINT nx_lwm2m_client_resource_float32_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_FLOAT32 float32_data);
UINT nx_lwm2m_client_resource_float64_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_FLOAT64 float64_data);
UINT nx_lwm2m_client_resource_boolean_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_BOOL bool_data);
UINT nx_lwm2m_client_resource_objlnk_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id);
UINT nx_lwm2m_client_resource_opaque_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, VOID *opaque_ptr, UINT opaque_length);
UINT nx_lwm2m_client_resource_instances_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_RESOURCE *resource_instances, UINT count);

/* Firmware Update Object */
UINT nx_lwm2m_client_firmware_create(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, NX_LWM2M_CLIENT *client_ptr, UINT protocols, NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_CALLBACK package_callback, NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_URI_CALLBACK package_uri_callback, NX_LWM2M_CLIENT_FIRMWARE_UPDATE_CALLBACK update_callback);
UINT nx_lwm2m_client_firmware_state_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, UCHAR state);
UINT nx_lwm2m_client_firmware_result_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, UCHAR result);
UINT nx_lwm2m_client_firmware_package_info_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, const CHAR *name, UINT name_length, const CHAR *version, UINT version_length);

#else

/* Define the functions prototypes. */

/* Client Management */
UINT _nx_lwm2m_client_create(NX_LWM2M_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr,
                             const CHAR *name_ptr, UINT name_length, const CHAR *msisdn_ptr, UINT msisdn_length,
                             UCHAR binding_modes, VOID *stack_ptr, ULONG stack_size, UINT priority);
UINT _nxe_lwm2m_client_create(NX_LWM2M_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr,
                              const CHAR *name_ptr, UINT name_length, const CHAR *msisdn_ptr, UINT msisdn_length,
                              UCHAR binding_modes, VOID *stack_ptr, ULONG stack_size, UINT priority);
UINT _nx_lwm2m_client_delete(NX_LWM2M_CLIENT *client_ptr);
UINT _nxe_lwm2m_client_delete(NX_LWM2M_CLIENT *client_ptr);
UINT _nx_lwm2m_client_lock(NX_LWM2M_CLIENT *client_ptr);
UINT _nxe_lwm2m_client_lock(NX_LWM2M_CLIENT *client_ptr);
UINT _nx_lwm2m_client_unlock(NX_LWM2M_CLIENT *client_ptr);
UINT _nxe_lwm2m_client_unlock(NX_LWM2M_CLIENT *client_ptr);
UINT _nx_lwm2m_client_object_add(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_ID object_id, 
                                 NX_LWM2M_CLIENT_OBJECT_OPERATION_CALLBACK object_operation);
UINT _nxe_lwm2m_client_object_add(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_ID object_id, 
                                  NX_LWM2M_CLIENT_OBJECT_OPERATION_CALLBACK object_operation);
UINT _nx_lwm2m_client_object_remove(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr);
UINT _nxe_lwm2m_client_object_remove(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr);

/* Objects Implementation */
UINT _nx_lwm2m_client_object_resource_changed(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource);
UINT _nxe_lwm2m_client_object_resource_changed(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource);

/* Device Object Implementation */
UINT _nx_lwm2m_client_device_callback_set(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_DEVICE_OPERATION_CALLBACK operation_callback);
UINT _nxe_lwm2m_client_device_callback_set(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_DEVICE_OPERATION_CALLBACK operation_callback);
UINT _nx_lwm2m_client_device_resource_changed(NX_LWM2M_CLIENT *client_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource);
UINT _nxe_lwm2m_client_device_resource_changed(NX_LWM2M_CLIENT *client_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource);
UINT _nx_lwm2m_client_device_error_push(NX_LWM2M_CLIENT *client_ptr, UCHAR code);
UINT _nxe_lwm2m_client_device_error_push(NX_LWM2M_CLIENT *client_ptr, UCHAR code);
UINT _nx_lwm2m_client_device_error_reset(NX_LWM2M_CLIENT *client_ptr);
UINT _nxe_lwm2m_client_device_error_reset(NX_LWM2M_CLIENT *client_ptr);

/* Local Device Management */
UINT _nx_lwm2m_client_object_next_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID *object_id_ptr);
UINT _nxe_lwm2m_client_object_next_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID *object_id_ptr);
UINT _nx_lwm2m_client_object_instance_next_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID *instance_id_ptr);
UINT _nxe_lwm2m_client_object_instance_next_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID *instance_id_ptr);
UINT _nx_lwm2m_client_object_read(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT num_values, NX_LWM2M_CLIENT_RESOURCE *values);
UINT _nxe_lwm2m_client_object_read(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT num_values, NX_LWM2M_CLIENT_RESOURCE *values);
UINT _nx_lwm2m_client_object_discover(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT *num_resources, NX_LWM2M_CLIENT_RESOURCE *resources);
UINT _nxe_lwm2m_client_object_discover(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT *num_resources, NX_LWM2M_CLIENT_RESOURCE *resources);
UINT _nx_lwm2m_client_object_write(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values, UINT write_op);
UINT _nxe_lwm2m_client_object_write(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values, UINT write_op);
UINT _nx_lwm2m_client_object_execute(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, NX_LWM2M_ID resource_id, const CHAR *args_ptr, UINT args_length);
UINT _nxe_lwm2m_client_object_execute(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, NX_LWM2M_ID resource_id, const CHAR *args_ptr, UINT args_length);
UINT _nx_lwm2m_client_object_create(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID *instance_id_ptr, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values);
UINT _nxe_lwm2m_client_object_create(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID *instance_id_ptr, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values);
UINT _nx_lwm2m_client_object_delete(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id);
UINT _nxe_lwm2m_client_object_delete(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id);
UINT _nx_lwm2m_client_object_instance_add(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_ID *instance_id_ptr);
UINT _nxe_lwm2m_client_object_instance_add(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_ID *instance_id_ptr);
UINT _nx_lwm2m_client_object_instance_remove(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr);
UINT _nxe_lwm2m_client_object_instance_remove(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr);

/* Client Session Management */
UINT _nx_lwm2m_client_session_create(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_SESSION_STATE_CALLBACK state_callback);
UINT _nxe_lwm2m_client_session_create(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_SESSION_STATE_CALLBACK state_callback);
UINT _nx_lwm2m_client_session_delete(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nxe_lwm2m_client_session_delete(NX_LWM2M_CLIENT_SESSION *session_ptr);
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
UINT _nx_lwm2m_client_session_bootstrap(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID security_id, const NXD_ADDRESS *ip_address, UINT port);
UINT _nxe_lwm2m_client_session_bootstrap(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID security_id, const NXD_ADDRESS *ip_address, UINT port);
UINT _nx_lwm2m_client_session_register_info_get(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT security_instance_id, NX_LWM2M_ID *server_id, CHAR **server_uri, UINT *server_uri_len, UCHAR *security_mode,
                                                UCHAR **pub_key_or_id, UINT *pub_key_or_id_len, UCHAR **server_pub_key, UINT *server_pub_key_len, UCHAR **secret_key, UINT *secret_key_len);
UINT _nxe_lwm2m_client_session_register_info_get(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT security_instance_id, NX_LWM2M_ID *server_id, CHAR **server_uri, UINT *server_uri_len, UCHAR *security_mode,
                                                 UCHAR **pub_key_or_id, UINT *pub_key_or_id_len, UCHAR **server_pub_key, UINT *server_pub_key_len, UCHAR **secret_key, UINT *secret_key_len);
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
UINT _nx_lwm2m_client_session_register(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port);
UINT _nxe_lwm2m_client_session_register(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port);
UINT _nx_lwm2m_client_session_update(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nxe_lwm2m_client_session_update(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nx_lwm2m_client_session_deregister(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nxe_lwm2m_client_session_deregister(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nx_lwm2m_client_session_error_get(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nxe_lwm2m_client_session_error_get(NX_LWM2M_CLIENT_SESSION *session_ptr);
#ifdef NX_SECURE_ENABLE_DTLS
#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
UINT _nx_lwm2m_client_session_bootstrap_dtls(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID security_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
UINT _nxe_lwm2m_client_session_bootstrap_dtls(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID security_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */
UINT _nx_lwm2m_client_session_register_dtls(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
UINT _nxe_lwm2m_client_session_register_dtls(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
#endif /* NX_SECURE_ENABLE_DTLS */

/* Get Resource Value */
UINT _nx_lwm2m_client_resource_info_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID *resource_id, ULONG *operation);
UINT _nxe_lwm2m_client_resource_info_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID *resource_id, ULONG *operation);
UINT _nx_lwm2m_client_resource_dim_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UCHAR *dim);
UINT _nxe_lwm2m_client_resource_dim_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UCHAR *dim);
UINT _nx_lwm2m_client_resource_string_get(NX_LWM2M_CLIENT_RESOURCE *value, const CHAR **string_ptr, UINT *string_length);
UINT _nxe_lwm2m_client_resource_string_get(NX_LWM2M_CLIENT_RESOURCE *value, const CHAR **string_ptr, UINT *string_length);
UINT _nx_lwm2m_client_resource_integer32_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_INT32 *int32_ptr);
UINT _nxe_lwm2m_client_resource_integer32_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_INT32 *int32_ptr);
UINT _nx_lwm2m_client_resource_integer64_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_INT64 *int64_ptr);
UINT _nxe_lwm2m_client_resource_integer64_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_INT64 *int64_ptr);
UINT _nx_lwm2m_client_resource_float32_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_FLOAT32 *float32_ptr);
UINT _nxe_lwm2m_client_resource_float32_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_FLOAT32 *float32_ptr);
UINT _nx_lwm2m_client_resource_float64_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_FLOAT64 *float64_ptr);
UINT _nxe_lwm2m_client_resource_float64_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_FLOAT64 *float64_ptr);
UINT _nx_lwm2m_client_resource_boolean_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_BOOL *bool_ptr);
UINT _nxe_lwm2m_client_resource_boolean_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_BOOL *bool_ptr);
UINT _nx_lwm2m_client_resource_objlnk_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_ID *object_id, NX_LWM2M_ID *instance_id);
UINT _nxe_lwm2m_client_resource_objlnk_get(NX_LWM2M_CLIENT_RESOURCE *value, NX_LWM2M_ID *object_id, NX_LWM2M_ID *instance_id);
UINT _nx_lwm2m_client_resource_opaque_get(NX_LWM2M_CLIENT_RESOURCE *value, const VOID **opaque_ptr, UINT *opaque_length);
UINT _nxe_lwm2m_client_resource_opaque_get(NX_LWM2M_CLIENT_RESOURCE *value, const VOID **opaque_ptr, UINT *opaque_length);
UINT _nx_lwm2m_client_resource_instances_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_RESOURCE *resource_instances, UINT *count);
UINT _nxe_lwm2m_client_resource_instances_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_RESOURCE *resource_instances, UINT *count);

/* Set Resource Value */
UINT _nx_lwm2m_client_resource_info_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID resource_id, ULONG operation);
UINT _nxe_lwm2m_client_resource_info_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID resource_id, ULONG operation);
UINT _nx_lwm2m_client_resource_dim_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UCHAR dim);
UINT _nxe_lwm2m_client_resource_dim_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UCHAR dim);
UINT _nx_lwm2m_client_resource_string_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, CHAR *string_ptr, UINT string_length);
UINT _nxe_lwm2m_client_resource_string_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, CHAR *string_ptr, UINT string_length);
UINT _nx_lwm2m_client_resource_integer32_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_INT32 int32_data);
UINT _nxe_lwm2m_client_resource_integer32_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_INT32 int32_data);
UINT _nx_lwm2m_client_resource_integer64_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_INT64 int64_data);
UINT _nxe_lwm2m_client_resource_integer64_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_INT64 int64_data);
UINT _nx_lwm2m_client_resource_float32_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_FLOAT32 float32_data);
UINT _nxe_lwm2m_client_resource_float32_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_FLOAT32 float32_data);
UINT _nx_lwm2m_client_resource_float64_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_FLOAT64 float64_data);
UINT _nxe_lwm2m_client_resource_float64_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_FLOAT64 float64_data);
UINT _nx_lwm2m_client_resource_boolean_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_BOOL bool_data);
UINT _nxe_lwm2m_client_resource_boolean_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_BOOL bool_data);
UINT _nx_lwm2m_client_resource_objlnk_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id);
UINT _nxe_lwm2m_client_resource_objlnk_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id);
UINT _nx_lwm2m_client_resource_opaque_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, VOID *opaque_ptr, UINT opaque_length);
UINT _nxe_lwm2m_client_resource_opaque_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, VOID *opaque_ptr, UINT opaque_length);
UINT _nx_lwm2m_client_resource_instances_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_RESOURCE *resource_instances, UINT count);
UINT _nxe_lwm2m_client_resource_instances_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_RESOURCE *resource_instances, UINT count);

/* Firmware Update Object */
UINT _nx_lwm2m_client_firmware_create(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, NX_LWM2M_CLIENT *client_ptr, UINT protocols, NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_CALLBACK package_callback, NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_URI_CALLBACK package_uri_callback, NX_LWM2M_CLIENT_FIRMWARE_UPDATE_CALLBACK update_callback);
UINT _nxe_lwm2m_client_firmware_create(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, NX_LWM2M_CLIENT *client_ptr, UINT protocols, NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_CALLBACK package_callback, NX_LWM2M_CLIENT_FIRMWARE_PACKAGE_URI_CALLBACK package_uri_callback, NX_LWM2M_CLIENT_FIRMWARE_UPDATE_CALLBACK update_callback);
UINT _nx_lwm2m_client_firmware_state_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, UCHAR state);
UINT _nxe_lwm2m_client_firmware_state_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, UCHAR state);
UINT _nx_lwm2m_client_firmware_result_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, UCHAR result);
UINT _nxe_lwm2m_client_firmware_result_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, UCHAR result);
UINT _nx_lwm2m_client_firmware_package_info_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, const CHAR *name, UINT name_length, const CHAR *version, UINT version_length);
UINT _nxe_lwm2m_client_firmware_package_info_set(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, const CHAR *name, UINT name_length, const CHAR *version, UINT version_length);

#endif /* NX_LWM2M_CLIENT_SOURCE_CODE */


/* Define internal LWM2M Client functions.  */
VOID _nx_lwm2m_client_thread_entry(ULONG client_address);
NX_LWM2M_CLIENT_OBJECT* _nx_lwm2m_client_object_ptr_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_ID object_id);
NX_LWM2M_CLIENT_OBJECT_INSTANCE* _nx_lwm2m_client_object_instance_ptr_get(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_ID instance_id);
UINT _nx_lwm2m_client_object_create_internal(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_ID *instance_id_ptr, UINT num_values, const NX_LWM2M_CLIENT_RESOURCE *values, NX_LWM2M_BOOL bootstrap);
VOID _nx_lwm2m_client_object_delete_all(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT *del_object_ptr, NX_LWM2M_ID bs_id);
VOID _nx_lwm2m_client_object_list_changed(NX_LWM2M_CLIENT *client_ptr);
NX_LWM2M_CLIENT_NOTIFY* _nx_lwm2m_client_notify_allocate(NX_LWM2M_CLIENT *client_ptr);
VOID _nx_lwm2m_client_notify_free(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_NOTIFY *notify_ptr);
NX_LWM2M_CLIENT_SESSION *_nx_lwm2m_client_server_session_get(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *server_instance_ptr);

#ifdef NX_SECURE_ENABLE_DTLS
UINT _nx_lwm2m_client_session_start(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_BOOL bootstrap, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port, NX_SECURE_DTLS_SESSION *dtls_session_ptr);
#else
UINT _nx_lwm2m_client_session_start(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_BOOL bootstrap, NX_LWM2M_ID server_id, const NXD_ADDRESS *ip_address, UINT port);
#endif /* NX_SECURE_ENABLE_DTLS */
NX_LWM2M_BOOL _nx_lwm2m_client_session_step(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_BOOL *has_timer_ptr, ULONG *timer_expire_ptr);
VOID _nx_lwm2m_client_session_receive(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_PACKET *packet_ptr);
VOID _nx_lwm2m_client_session_state_update(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT state);
VOID _nx_lwm2m_client_session_cleanup(NX_LWM2M_CLIENT_SESSION *session_ptr);
NX_PACKET* _nx_lwm2m_client_session_packet_allocate(NX_LWM2M_CLIENT_SESSION *session_ptr);
UINT _nx_lwm2m_client_session_send(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_PACKET *packet_ptr);
UINT _nx_lwm2m_client_session_send_request(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_BOOL first);
UINT _nx_lwm2m_client_session_send_response(NX_LWM2M_CLIENT_SESSION *session_ptr, UCHAR code, USHORT id, NX_LWM2M_BOOL ack, const VOID *token_ptr, UINT token_length, NX_LWM2M_BOOL observe, UINT format, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, const NX_LWM2M_CLIENT_RESOURCE *resource_ptr);
VOID _nx_lwm2m_client_session_update_flags(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT flags);
NX_LWM2M_CLIENT_NOTIFY* _nx_lwm2m_client_session_notify_get(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_ID resource_id, NX_LWM2M_BOOL create);
VOID _nx_lwm2m_client_session_notify_free(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_CLIENT_NOTIFY *notify_ptr);
VOID _nx_lwm2m_client_session_notify_attributes(NX_LWM2M_CLIENT_SESSION *session_ptr, NX_LWM2M_CLIENT_NOTIFY *notify_ptr, UINT *flags_ptr, NX_LWM2M_INT32 *pmin_ptr, NX_LWM2M_INT32 *pmax_ptr, NX_LWM2M_CLIENT_NOTIFY_NUMBER *gt_ptr, NX_LWM2M_CLIENT_NOTIFY_NUMBER *lt_ptr, NX_LWM2M_CLIENT_NOTIFY_NUMBER *stp_ptr);

UINT _nx_lwm2m_client_resource_notify_number_get(const NX_LWM2M_CLIENT_RESOURCE *resource_ptr, NX_LWM2M_CLIENT_NOTIFY_NUMBER *number_ptr);

/* Binding Modes String Handling functions.  */
const CHAR* _nx_lwm2m_client_binding_mode_string_get(UCHAR binding_mode, UINT *length);
UCHAR _nx_lwm2m_client_binding_mode_string_parse(const CHAR *string_ptr, UINT string_length);

/* CoAP functions.  */
VOID *_nx_lwm2m_client_coap_option_header_add(VOID *ptr, VOID *ptr_max, UINT *last_option_ptr, UINT option, UINT length);
VOID *_nx_lwm2m_client_coap_option_uint_add(VOID *ptr, VOID *ptr_max, UINT *last_option_ptr, UINT option, ULONG value);
const VOID *_nx_lwm2m_client_coap_header_parse(const VOID *ptr, const VOID *ptr_max, UCHAR *type_ptr, UCHAR *code_ptr, USHORT *id, const VOID **token_ptr, UINT *token_length_ptr);
const VOID *_nx_lwm2m_client_coap_option_parse(const VOID *ptr, const VOID *ptr_max, UINT *option_ptr, const VOID **value_ptr, UINT *value_length_ptr);

/* Corelink functions.  */
CHAR* _nx_lwm2m_client_corelink_path_add(CHAR *ptr, CHAR *ptr_max, NX_LWM2M_ID object_id, NX_LWM2M_ID instance_id, NX_LWM2M_ID resource_id);
CHAR* _nx_lwm2m_client_corelink_attributes_add(CHAR *ptr, CHAR *ptr_max, NX_LWM2M_ID server_id, const NX_LWM2M_CLIENT_RESOURCE *resource_ptr, UINT attr_flags, NX_LWM2M_INT32 attr_pmin, NX_LWM2M_INT32 attr_pmax, NX_LWM2M_CLIENT_NOTIFY_NUMBER attr_gt, NX_LWM2M_CLIENT_NOTIFY_NUMBER attr_lt, NX_LWM2M_CLIENT_NOTIFY_NUMBER attr_stp);

#if NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES
/* Access control object functions.  */
VOID _nx_lwm2m_client_access_control_object_create(NX_LWM2M_CLIENT *client_ptr);
#endif /* NX_LWM2M_CLIENT_MAX_ACCESS_CONTROL_INSTANCES */

/* Device object functions.  */
VOID _nx_lwm2m_client_device_object_create(NX_LWM2M_CLIENT *client_ptr, UCHAR binding_modes);
UINT _nx_lwm2m_client_device_object_error_push(NX_LWM2M_CLIENT_DEVICE_OBJECT *device_ptr, UCHAR code);
VOID _nx_lwm2m_client_device_object_error_reset(NX_LWM2M_CLIENT_DEVICE_OBJECT *device_ptr);

#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES
/* Security object functions.  */
VOID _nx_lwm2m_client_security_object_create(NX_LWM2M_CLIENT *client_ptr);
#endif /* NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES */

/* Server object functions.  */
VOID _nx_lwm2m_client_server_object_create(NX_LWM2M_CLIENT *client_ptr);

/* Parse TLV Header */
const VOID *_nx_lwm2m_client_tlv_header_parse(const VOID *ptr, const VOID *ptr_max, UCHAR *type_ptr, NX_LWM2M_ID *id_ptr, UINT *length_ptr);

/* Parse TLV resource */
const VOID *_nx_lwm2m_client_tlv_resource_parse(const VOID *ptr, const VOID *ptr_max, NX_LWM2M_CLIENT_RESOURCE *resource_ptr);

/* Encode TLV resource value */
VOID *_nx_lwm2m_client_tlv_resource_add(VOID *ptr, VOID *ptr_max, const NX_LWM2M_CLIENT_RESOURCE *resource_ptr);

/* Define string conversion functions prototypes */
UINT _nx_lwm2m_client_strconv_parse_id(const CHAR *string_ptr, UINT string_length, NX_LWM2M_ID *id_ptr);
UINT _nx_lwm2m_client_strconv_parse_int32(const CHAR *string_ptr, UINT string_length, NX_LWM2M_INT32 *value_ptr);
UINT _nx_lwm2m_client_strconv_parse_notify_number(const CHAR *string_ptr, UINT string_length, NX_LWM2M_CLIENT_NOTIFY_NUMBER *number_ptr);
UINT _nx_lwm2m_client_strconv_format_id(CHAR *string_ptr, UINT string_length, NX_LWM2M_ID id);
UINT _nx_lwm2m_client_strconv_format_int32(CHAR *string_ptr, UINT string_length, NX_LWM2M_INT32 value);
UINT _nx_lwm2m_client_strconv_format_notify_number(CHAR *string_ptr, UINT string_length, NX_LWM2M_CLIENT_NOTIFY_NUMBER value);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
        }
#endif

#endif  /* NX_LWM2M_CLIENT_H */
