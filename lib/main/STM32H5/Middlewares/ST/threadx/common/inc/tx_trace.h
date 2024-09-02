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
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_trace.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX trace component, including constants  */
/*    and structure definitions as well as external references.  It is    */
/*    assumed that tx_api.h and tx_port.h have already been included.     */
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


/* Include necessary system files.  */

#ifndef TX_TRACE_H
#define TX_TRACE_H


/* Determine if tracing is enabled. If not, simply define the in-line trace
   macros to whitespace.  */

#ifndef TX_ENABLE_EVENT_TRACE
#define TX_TRACE_INITIALIZE
#define TX_TRACE_OBJECT_REGISTER(t,p,n,a,b)
#define TX_TRACE_OBJECT_UNREGISTER(o)
#define TX_TRACE_IN_LINE_INSERT(i,a,b,c,d,f)
#else

/* Event tracing is enabled.  */

/* Ensure that the thread component information is included.  */

#include "tx_thread.h"


/* Define trace port-specfic extension to white space if it isn't defined
   already.  */

#ifndef TX_TRACE_PORT_EXTENSION
#define TX_TRACE_PORT_EXTENSION
#endif


/* Define the default clock source for trace event entry time stamp. The following two item are port specific.
   For example, if the time source is at the address 0x0a800024 and is 16-bits in size, the clock
   source constants would be:

#define TX_TRACE_TIME_SOURCE                    *((ULONG *) 0x0a800024)
#define TX_TRACE_TIME_MASK                      0x0000FFFFUL

*/

#ifndef TX_TRACE_TIME_SOURCE
#define TX_TRACE_TIME_SOURCE                    ++_tx_trace_simulated_time
#endif
#ifndef TX_TRACE_TIME_MASK
#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL
#endif


/* Define the ID showing the event trace buffer is valid.  */

#define TX_TRACE_VALID                                      0x54585442UL


/* ThreadX Trace Description.  The ThreadX Trace feature is designed to capture
   events in real-time in a circular event buffer. This buffer may be analyzed by other
   tools.  The high-level format of the Trace structure is:

            [Trace Control Header              ]
            [Trace Object Registry - Entry 0   ]
                    ...
            [Trace Object Registry - Entry "n" ]
            [Trace Buffer - Entry 0            ]
                    ...
            [Trace Buffer - Entry "n"          ]

*/


/* Trace Control Header. The Trace Control Header contains information that
   defines the format of the Trace Object Registry as well as the location and
   current entry of the Trace Buffer itself.  The high-level format of the
   Trace Control Header is:

                Entry                               Size                     Description

            [Trace ID]                              4       This 4-byte field contains the ThreadX Trace
                                                            Identification. If the trace buffer is valid, the
                                                            contents are 0x54585442 (TXTB). Since it is written as
                                                            a 32-bit unsigned word, this value is also used to
                                                            determine if the event trace information is in
                                                            little or big endian format.
            [Timer Valid Mask]                      4       Mask of valid bits in the 32-bit time stamp. This
                                                            enables use of 32, 24, 16, or event 8-bit timers.
                                                            If the time source is 32-bits, the mask is
                                                            0xFFFFFFFF. If the time source is 16-bits, the
                                                            mask is 0x0000FFFF.
            [Trace Base Address]                    4       The base address for all trace pointer. Subtracting
                                                            the pointer and this address will yield the proper
                                                            offset into the trace buffer.
            [Trace Object Registry Start Pointer]   4       Pointer to the start of Trace Object Registry
            [Reserved]                              2       Reserved two bytes - should be 0x0000
            [Trace Object Object Name Size]         2       Number of bytes in object name
            [Trace Object Registry End Pointer]     4       Pointer to the end of Trace Object Registry
            [Trace Buffer Start Pointer]            4       Pointer to the start of the Trace Buffer Area
            [Trace Buffer End Pointer]              4       Pointer to the end of the Trace Buffer Area
            [Trace Buffer Current Pointer]          4       Pointer to the oldest entry in the Trace Buffer.
                                                            This entry will be overwritten on the next event and
                                                            incremented to the next event (wrapping to the top
                                                            if the buffer end pointer is exceeded).
            [Reserved]                              4       Reserved 4 bytes, should be 0xAAAAAAAA
            [Reserved]                              4       Reserved 4 bytes, should be 0xBBBBBBBB
            [Reserved]                              4       Reserved 4 bytes, should be 0xCCCCCCCC
*/


/* Define the Trace Control Header.  */

typedef struct TX_TRACE_HEADER_STRUCT
{

    ULONG                                                   tx_trace_header_id;
    ULONG                                                   tx_trace_header_timer_valid_mask;
    ULONG                                                   tx_trace_header_trace_base_address;
    ULONG                                                   tx_trace_header_registry_start_pointer;
    USHORT                                                  tx_trace_header_reserved1;
    USHORT                                                  tx_trace_header_object_name_size;
    ULONG                                                   tx_trace_header_registry_end_pointer;
    ULONG                                                   tx_trace_header_buffer_start_pointer;
    ULONG                                                   tx_trace_header_buffer_end_pointer;
    ULONG                                                   tx_trace_header_buffer_current_pointer;
    ULONG                                                   tx_trace_header_reserved2;
    ULONG                                                   tx_trace_header_reserved3;
    ULONG                                                   tx_trace_header_reserved4;
} TX_TRACE_HEADER;


/* Trace Object Registry. The Trace Object Registry is used to map the object pointer in the trace buffer to
   the application's name for the object (defined during object creation in ThreadX).  */

#ifndef TX_TRACE_OBJECT_REGISTRY_NAME
#define TX_TRACE_OBJECT_REGISTRY_NAME                       32
#endif


/* Define the object name types as well as the contents of any additional parameters that might be useful in
   trace analysis.  */

#define TX_TRACE_OBJECT_TYPE_NOT_VALID                      ((UCHAR) 0)     /* Object is not valid                               */
#define TX_TRACE_OBJECT_TYPE_THREAD                         ((UCHAR) 1)     /* P1 = stack start address, P2 = stack size         */
#define TX_TRACE_OBJECT_TYPE_TIMER                          ((UCHAR) 2)     /* P1 = initial ticks, P2 = reschedule ticks         */
#define TX_TRACE_OBJECT_TYPE_QUEUE                          ((UCHAR) 3)     /* P1 = queue size, P2 = message size                */
#define TX_TRACE_OBJECT_TYPE_SEMAPHORE                      ((UCHAR) 4)     /* P1 = initial instances                            */
#define TX_TRACE_OBJECT_TYPE_MUTEX                          ((UCHAR) 5)     /* P1 = priority inheritance flag                    */
#define TX_TRACE_OBJECT_TYPE_EVENT_FLAGS                    ((UCHAR) 6)     /* none                                              */
#define TX_TRACE_OBJECT_TYPE_BLOCK_POOL                     ((UCHAR) 7)     /* P1 = total blocks, P2 = block size                */
#define TX_TRACE_OBJECT_TYPE_BYTE_POOL                      ((UCHAR) 8)     /* P1 = total bytes                                  */


typedef struct TX_TRACE_OBJECT_ENTRY_STRUCT
{

    UCHAR                                                   tx_trace_object_entry_available;                            /* TX_TRUE -> available                 */
    UCHAR                                                   tx_trace_object_entry_type;                                 /* Types defined above                  */
    UCHAR                                                   tx_trace_object_entry_reserved1;                            /* Should be zero - except for thread   */
    UCHAR                                                   tx_trace_object_entry_reserved2;                            /* Should be zero - except for thread   */
    ULONG                                                   tx_trace_object_entry_thread_pointer;                       /* ThreadX object pointer               */
    ULONG                                                   tx_trace_object_entry_param_1;                              /* Parameter value defined              */
    ULONG                                                   tx_trace_object_entry_param_2;                              /*   according to type above            */
    UCHAR                                                   tx_trace_object_entry_name[TX_TRACE_OBJECT_REGISTRY_NAME];  /* Object name                          */
} TX_TRACE_OBJECT_ENTRY;


/* Trace Buffer Entry. The Trace Buffer Entry contains information about a particular
   event in the system. The high-level format of the Trace Buffer Entry is:

                Entry                  Size                     Description

            [Thread Pointer]            4           This 4-byte field contains the pointer to the
                                                    ThreadX thread running that caused the event.
                                                    If this field is NULL, the entry hasn't been used
                                                    yet. If this field is 0xFFFFFFFF, the event occurred
                                                    from within an ISR. If this entry is 0xF0F0F0F0, the
                                                    event occurred during initialization.
            [Thread Priority or         4           This 4-byte field contains the current thread pointer for interrupt
             Current Thread                         events or the thread preemption-threshold/priority for thread events.
             Preemption-Threshold/
             Priority]
            [Event ID]                  4           This 4-byte field contains the Event ID of the event. A value of
                                                    0xFFFFFFFF indicates the event is invalid. All events are marked
                                                    as invalid during initialization.
            [Time Stamp]                4           This 4-byte field contains the time stamp of the event.
            [Information Field 1]       4           This 4-byte field contains the first 4-bytes of information
                                                    specific to the event.
            [Information Field 2]       4           This 4-byte field contains the second 4-bytes of information
                                                    specific to the event.
            [Information Field 3]       4           This 4-byte field contains the third 4-bytes of information
                                                    specific to the event.
            [Information Field 4]       4           This 4-byte field contains the fourth 4-bytes of information
                                                    specific to the event.
*/

#define TX_TRACE_INVALID_EVENT                              0xFFFFFFFFUL


/* Define ThreadX Trace Events, along with a brief description of the additional information fields,
   where I1 -> Information Field 1, I2 -> Information Field 2, etc.  */

/* Event numbers 0 through 4095 are reserved by Azure RTOS. Specific event assignments are:

                                ThreadX events:     1-199
                                FileX events:       200-299
                                NetX events:        300-599
                                USBX events:        600-999

   User-defined event numbers start at 4096 and continue through 65535, as defined by the constants
   TX_TRACE_USER_EVENT_START and TX_TRACE_USER_EVENT_END, respectively. User events should be based
   on these constants in case the user event number assignment is changed in future releases.  */

/* Define the basic ThreadX thread scheduling events first.  */

#define TX_TRACE_THREAD_RESUME                              1           /* I1 = thread ptr, I2 = previous_state, I3 = stack ptr, I4 = next thread   */
#define TX_TRACE_THREAD_SUSPEND                             2           /* I1 = thread ptr, I2 = new_state, I3 = stack ptr  I4 = next thread        */
#define TX_TRACE_ISR_ENTER                                  3           /* I1 = stack_ptr, I2 = ISR number, I3 = system state, I4 = preempt disable */
#define TX_TRACE_ISR_EXIT                                   4           /* I1 = stack_ptr, I2 = ISR number, I3 = system state, I4 = preempt disable */
#define TX_TRACE_TIME_SLICE                                 5           /* I1 = next thread ptr, I2 = system state, I3 = preempt disable, I4 = stack*/
#define TX_TRACE_RUNNING                                    6           /* None                                                                     */


/* Define the rest of the ThreadX system events.  */

#define TX_TRACE_BLOCK_ALLOCATE                             10          /* I1 = pool ptr, I2 = memory ptr, I3 = wait option, I4 = remaining blocks  */
#define TX_TRACE_BLOCK_POOL_CREATE                          11          /* I1 = pool ptr, I2 = pool_start, I3 = total blocks, I4 = block size       */
#define TX_TRACE_BLOCK_POOL_DELETE                          12          /* I1 = pool ptr, I2 = stack ptr                                            */
#define TX_TRACE_BLOCK_POOL_INFO_GET                        13          /* I1 = pool ptr                                                            */
#define TX_TRACE_BLOCK_POOL_PERFORMANCE_INFO_GET            14          /* I1 = pool ptr                                                            */
#define TX_TRACE_BLOCK_POOL__PERFORMANCE_SYSTEM_INFO_GET    15          /* None                                                                     */
#define TX_TRACE_BLOCK_POOL_PRIORITIZE                      16          /* I1 = pool ptr, I2 = suspended count, I3 = stack ptr                      */
#define TX_TRACE_BLOCK_RELEASE                              17          /* I1 = pool ptr, I2 = memory ptr, I3 = suspended, I4 = stack ptr           */
#define TX_TRACE_BYTE_ALLOCATE                              20          /* I1 = pool ptr, I2 = memory ptr, I3 = size requested, I4 = wait option    */
#define TX_TRACE_BYTE_POOL_CREATE                           21          /* I1 = pool ptr, I2 = start ptr, I3 = pool size, I4 = stack ptr            */
#define TX_TRACE_BYTE_POOL_DELETE                           22          /* I1 = pool ptr, I2 = stack ptr                                            */
#define TX_TRACE_BYTE_POOL_INFO_GET                         23          /* I1 = pool ptr                                                            */
#define TX_TRACE_BYTE_POOL_PERFORMANCE_INFO_GET             24          /* I1 = pool ptr                                                            */
#define TX_TRACE_BYTE_POOL__PERFORMANCE_SYSTEM_INFO_GET     25          /* None                                                                     */
#define TX_TRACE_BYTE_POOL_PRIORITIZE                       26          /* I1 = pool ptr, I2 = suspended count, I3 = stack ptr                      */
#define TX_TRACE_BYTE_RELEASE                               27          /* I1 = pool ptr, I2 = memory ptr, I3 = suspended, I4 = available bytes     */
#define TX_TRACE_EVENT_FLAGS_CREATE                         30          /* I1 = group ptr, I2 = stack ptr                                           */
#define TX_TRACE_EVENT_FLAGS_DELETE                         31          /* I1 = group ptr, I2 = stack ptr                                           */
#define TX_TRACE_EVENT_FLAGS_GET                            32          /* I1 = group ptr, I2 = requested flags, I3 = current flags, I4 = get option*/
#define TX_TRACE_EVENT_FLAGS_INFO_GET                       33          /* I1 = group ptr                                                           */
#define TX_TRACE_EVENT_FLAGS_PERFORMANCE_INFO_GET           34          /* I1 = group ptr                                                           */
#define TX_TRACE_EVENT_FLAGS__PERFORMANCE_SYSTEM_INFO_GET   35          /* None                                                                     */
#define TX_TRACE_EVENT_FLAGS_SET                            36          /* I1 = group ptr, I2 = flags to set, I3 = set option, I4= suspended count  */
#define TX_TRACE_EVENT_FLAGS_SET_NOTIFY                     37          /* I1 = group ptr                                                           */
#define TX_TRACE_INTERRUPT_CONTROL                          40          /* I1 = new interrupt posture, I2 = stack ptr                               */
#define TX_TRACE_MUTEX_CREATE                               50          /* I1 = mutex ptr, I2 = inheritance, I3 = stack ptr                         */
#define TX_TRACE_MUTEX_DELETE                               51          /* I1 = mutex ptr, I2 = stack ptr                                           */
#define TX_TRACE_MUTEX_GET                                  52          /* I1 = mutex ptr, I2 = wait option, I3 = owning thread, I4 = own count     */
#define TX_TRACE_MUTEX_INFO_GET                             53          /* I1 = mutex ptr                                                           */
#define TX_TRACE_MUTEX_PERFORMANCE_INFO_GET                 54          /* I1 = mutex ptr                                                           */
#define TX_TRACE_MUTEX_PERFORMANCE_SYSTEM_INFO_GET          55          /* None                                                                     */
#define TX_TRACE_MUTEX_PRIORITIZE                           56          /* I1 = mutex ptr, I2 = suspended count, I3 = stack ptr                     */
#define TX_TRACE_MUTEX_PUT                                  57          /* I1 = mutex ptr, I2 = owning thread, I3 = own count, I4 = stack ptr       */
#define TX_TRACE_QUEUE_CREATE                               60          /* I1 = queue ptr, I2 = message size, I3 = queue start, I4 = queue size     */
#define TX_TRACE_QUEUE_DELETE                               61          /* I1 = queue ptr, I2 = stack ptr                                           */
#define TX_TRACE_QUEUE_FLUSH                                62          /* I1 = queue ptr, I2 = stack ptr                                           */
#define TX_TRACE_QUEUE_FRONT_SEND                           63          /* I1 = queue ptr, I2 = source ptr, I3 = wait option, I4 = enqueued         */
#define TX_TRACE_QUEUE_INFO_GET                             64          /* I1 = queue ptr                                                           */
#define TX_TRACE_QUEUE_PERFORMANCE_INFO_GET                 65          /* I1 = queue ptr                                                           */
#define TX_TRACE_QUEUE_PERFORMANCE_SYSTEM_INFO_GET          66          /* None                                                                     */
#define TX_TRACE_QUEUE_PRIORITIZE                           67          /* I1 = queue ptr, I2 = suspended count, I3 = stack ptr                     */
#define TX_TRACE_QUEUE_RECEIVE                              68          /* I1 = queue ptr, I2 = destination ptr, I3 = wait option, I4 = enqueued    */
#define TX_TRACE_QUEUE_SEND                                 69          /* I1 = queue ptr, I2 = source ptr, I3 = wait option, I4 = enqueued         */
#define TX_TRACE_QUEUE_SEND_NOTIFY                          70          /* I1 = queue ptr                                                           */
#define TX_TRACE_SEMAPHORE_CEILING_PUT                      80          /* I1 = semaphore ptr, I2 = current count, I3 = suspended count,I4 =ceiling */
#define TX_TRACE_SEMAPHORE_CREATE                           81          /* I1 = semaphore ptr, I2 = initial count, I3 = stack ptr                   */
#define TX_TRACE_SEMAPHORE_DELETE                           82          /* I1 = semaphore ptr, I2 = stack ptr                                       */
#define TX_TRACE_SEMAPHORE_GET                              83          /* I1 = semaphore ptr, I2 = wait option, I3 = current count, I4 = stack ptr */
#define TX_TRACE_SEMAPHORE_INFO_GET                         84          /* I1 = semaphore ptr                                                       */
#define TX_TRACE_SEMAPHORE_PERFORMANCE_INFO_GET             85          /* I1 = semaphore ptr                                                       */
#define TX_TRACE_SEMAPHORE__PERFORMANCE_SYSTEM_INFO_GET     86          /* None                                                                     */
#define TX_TRACE_SEMAPHORE_PRIORITIZE                       87          /* I1 = semaphore ptr, I2 = suspended count, I2 = stack ptr                 */
#define TX_TRACE_SEMAPHORE_PUT                              88          /* I1 = semaphore ptr, I2 = current count, I3 = suspended count,I4=stack ptr*/
#define TX_TRACE_SEMAPHORE_PUT_NOTIFY                       89          /* I1 = semaphore ptr                                                       */
#define TX_TRACE_THREAD_CREATE                              100         /* I1 = thread ptr, I2 = priority, I3 = stack ptr, I4 = stack_size          */
#define TX_TRACE_THREAD_DELETE                              101         /* I1 = thread ptr, I2 = stack ptr                                          */
#define TX_TRACE_THREAD_ENTRY_EXIT_NOTIFY                   102         /* I1 = thread ptr, I2 = thread state, I3 = stack ptr                       */
#define TX_TRACE_THREAD_IDENTIFY                            103         /* None                                                                     */
#define TX_TRACE_THREAD_INFO_GET                            104         /* I1 = thread ptr, I2 = thread state                                       */
#define TX_TRACE_THREAD_PERFORMANCE_INFO_GET                105         /* I1 = thread ptr, I2 = thread state                                       */
#define TX_TRACE_THREAD_PERFORMANCE_SYSTEM_INFO_GET         106         /* None                                                                     */
#define TX_TRACE_THREAD_PREEMPTION_CHANGE                   107         /* I1 = thread ptr, I2 = new threshold, I3 = old threshold, I4 =thread state*/
#define TX_TRACE_THREAD_PRIORITY_CHANGE                     108         /* I1 = thread ptr, I2 = new priority, I3 = old priority, I4 = thread state */
#define TX_TRACE_THREAD_RELINQUISH                          109         /* I1 = stack ptr, I2 = next thread ptr                                     */
#define TX_TRACE_THREAD_RESET                               110         /* I1 = thread ptr, I2 = thread state                                       */
#define TX_TRACE_THREAD_RESUME_API                          111         /* I1 = thread ptr, I2 = thread state, I3 = stack ptr                       */
#define TX_TRACE_THREAD_SLEEP                               112         /* I1 = sleep value, I2 = thread state, I3 = stack ptr                      */
#define TX_TRACE_THREAD_STACK_ERROR_NOTIFY                  113         /* None                                                                     */
#define TX_TRACE_THREAD_SUSPEND_API                         114         /* I1 = thread ptr, I2 = thread state, I3 = stack ptr                       */
#define TX_TRACE_THREAD_TERMINATE                           115         /* I1 = thread ptr, I2 = thread state, I3 = stack ptr                       */
#define TX_TRACE_THREAD_TIME_SLICE_CHANGE                   116         /* I1 = thread ptr, I2 = new timeslice, I3 = old timeslice                  */
#define TX_TRACE_THREAD_WAIT_ABORT                          117         /* I1 = thread ptr, I2 = thread state, I3 = stack ptr                       */
#define TX_TRACE_TIME_GET                                   120         /* I1 = current time, I2 = stack ptr                                        */
#define TX_TRACE_TIME_SET                                   121         /* I1 = new time                                                            */
#define TX_TRACE_TIMER_ACTIVATE                             122         /* I1 = timer ptr                                                           */
#define TX_TRACE_TIMER_CHANGE                               123         /* I1 = timer ptr, I2 = initial ticks, I3= reschedule ticks                 */
#define TX_TRACE_TIMER_CREATE                               124         /* I1 = timer ptr, I2 = initial ticks, I3= reschedule ticks, I4 = enable    */
#define TX_TRACE_TIMER_DEACTIVATE                           125         /* I1 = timer ptr, I2 = stack ptr                                           */
#define TX_TRACE_TIMER_DELETE                               126         /* I1 = timer ptr                                                           */
#define TX_TRACE_TIMER_INFO_GET                             127         /* I1 = timer ptr, I2 = stack ptr                                           */
#define TX_TRACE_TIMER_PERFORMANCE_INFO_GET                 128         /* I1 = timer ptr                                                           */
#define TX_TRACE_TIMER_PERFORMANCE_SYSTEM_INFO_GET          129         /* None                                                                     */


/* Define the an Trace Buffer Entry.  */

typedef struct TX_TRACE_BUFFER_ENTRY_STRUCT
{

    ULONG                                                   tx_trace_buffer_entry_thread_pointer;
    ULONG                                                   tx_trace_buffer_entry_thread_priority;
    ULONG                                                   tx_trace_buffer_entry_event_id;
    ULONG                                                   tx_trace_buffer_entry_time_stamp;
#ifdef TX_MISRA_ENABLE
    ULONG                                                   tx_trace_buffer_entry_info_1;
    ULONG                                                   tx_trace_buffer_entry_info_2;
    ULONG                                                   tx_trace_buffer_entry_info_3;
    ULONG                                                   tx_trace_buffer_entry_info_4;
#else
    ULONG                                                   tx_trace_buffer_entry_information_field_1;
    ULONG                                                   tx_trace_buffer_entry_information_field_2;
    ULONG                                                   tx_trace_buffer_entry_information_field_3;
    ULONG                                                   tx_trace_buffer_entry_information_field_4;
#endif
} TX_TRACE_BUFFER_ENTRY;


/* Trace management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_TRACE_INIT
#define TRACE_DECLARE
#else
#define TRACE_DECLARE extern
#endif


/* Define the pointer to the start of the trace buffer control structure.   */

TRACE_DECLARE  TX_TRACE_HEADER                  *_tx_trace_header_ptr;


/* Define the pointer to the start of the trace object registry area in the trace buffer.  */

TRACE_DECLARE  TX_TRACE_OBJECT_ENTRY            *_tx_trace_registry_start_ptr;


/* Define the pointer to the end of the trace object registry area in the trace buffer.  */

TRACE_DECLARE  TX_TRACE_OBJECT_ENTRY            *_tx_trace_registry_end_ptr;


/* Define the pointer to the starting entry of the actual trace event area of the trace buffer.  */

TRACE_DECLARE  TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_start_ptr;


/* Define the pointer to the ending entry of the actual trace event area of the trace buffer.  */

TRACE_DECLARE  TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_end_ptr;


/* Define the pointer to the current entry of the actual trace event area of the trace buffer.  */

TRACE_DECLARE  TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_current_ptr;


/* Define the trace event enable bits, where each bit represents a type of event that can be enabled
   or disabled dynamically by the application.  */

TRACE_DECLARE  ULONG                            _tx_trace_event_enable_bits;


/* Define a counter that is used in environments that don't have a timer source. This counter
   is incremented on each use giving each event a unique timestamp.  */

TRACE_DECLARE  ULONG                             _tx_trace_simulated_time;


/* Define the function pointer used to call the application when the trace buffer wraps. If NULL,
   the application has not registered a callback function.  */

TRACE_DECLARE  VOID                             (*_tx_trace_full_notify_function)(VOID *buffer);


/* Define the total number of registry entries.  */

TRACE_DECLARE  ULONG                             _tx_trace_total_registry_entries;


/* Define a counter that is used to track the number of available registry entries.  */

TRACE_DECLARE  ULONG                             _tx_trace_available_registry_entries;


/* Define an index that represents the start of the registry search.  */

TRACE_DECLARE  ULONG                             _tx_trace_registry_search_start;


/* Define the event trace macros that are expanded in-line when event tracing is enabled.  */

#ifdef TX_MISRA_ENABLE
#define TX_TRACE_INFO_FIELD_ASSIGNMENT(a,b,c,d)  trace_event_ptr -> tx_trace_buffer_entry_info_1 =  (ULONG) (a); trace_event_ptr -> tx_trace_buffer_entry_info_2 =  (ULONG) (b); trace_event_ptr -> tx_trace_buffer_entry_info_3 =  (ULONG) (c); trace_event_ptr -> tx_trace_buffer_entry_info_4 =  (ULONG) (d);
#else
#define TX_TRACE_INFO_FIELD_ASSIGNMENT(a,b,c,d)  trace_event_ptr -> tx_trace_buffer_entry_information_field_1 =  (ULONG) (a); trace_event_ptr -> tx_trace_buffer_entry_information_field_2 =  (ULONG) (b); trace_event_ptr -> tx_trace_buffer_entry_information_field_3 =  (ULONG) (c); trace_event_ptr -> tx_trace_buffer_entry_information_field_4 =  (ULONG) (d);
#endif


#define TX_TRACE_INITIALIZE                                     _tx_trace_initialize();
#define TX_TRACE_OBJECT_REGISTER(t,p,n,a,b)                     _tx_trace_object_register((UCHAR) (t), (VOID *) (p), (CHAR *) (n), (ULONG) (a), (ULONG) (b));
#define TX_TRACE_OBJECT_UNREGISTER(o)                           _tx_trace_object_unregister((VOID *) (o));
#ifndef TX_TRACE_IN_LINE_INSERT
#define TX_TRACE_IN_LINE_INSERT(i,a,b,c,d,e) \
        { \
        TX_TRACE_BUFFER_ENTRY     *trace_event_ptr; \
        ULONG                      trace_system_state; \
        ULONG                      trace_priority; \
        TX_THREAD                 *trace_thread_ptr; \
            trace_event_ptr =  _tx_trace_buffer_current_ptr; \
            if ((trace_event_ptr) && (_tx_trace_event_enable_bits & ((ULONG) (e)))) \
            { \
                TX_TRACE_PORT_EXTENSION \
                trace_system_state =  (ULONG) TX_THREAD_GET_SYSTEM_STATE(); \
                TX_THREAD_GET_CURRENT(trace_thread_ptr) \
                \
                if (trace_system_state == 0) \
                { \
                    trace_priority =  trace_thread_ptr -> tx_thread_priority; \
                    trace_priority =  trace_priority | 0x80000000UL | (trace_thread_ptr -> tx_thread_preempt_threshold << 16); \
                } \
                else if (trace_system_state < 0xF0F0F0F0UL) \
                { \
                    trace_priority =    (ULONG) trace_thread_ptr; \
                    trace_thread_ptr =  (TX_THREAD *) 0xFFFFFFFFUL; \
                } \
                else \
                { \
                    trace_thread_ptr =  (TX_THREAD *) 0xF0F0F0F0UL; \
                    trace_priority =    0; \
                } \
                trace_event_ptr -> tx_trace_buffer_entry_thread_pointer =       (ULONG) trace_thread_ptr; \
                trace_event_ptr -> tx_trace_buffer_entry_thread_priority =      (ULONG) trace_priority; \
                trace_event_ptr -> tx_trace_buffer_entry_event_id =             (ULONG) (i); \
                trace_event_ptr -> tx_trace_buffer_entry_time_stamp =           (ULONG) TX_TRACE_TIME_SOURCE; \
                TX_TRACE_INFO_FIELD_ASSIGNMENT((a),(b),(c),(d)) \
                trace_event_ptr++; \
                if (trace_event_ptr >= _tx_trace_buffer_end_ptr) \
                { \
                    trace_event_ptr =  _tx_trace_buffer_start_ptr; \
                    _tx_trace_buffer_current_ptr =  trace_event_ptr;  \
                    _tx_trace_header_ptr -> tx_trace_header_buffer_current_pointer =  (ULONG) trace_event_ptr; \
                    if (_tx_trace_full_notify_function) \
                        (_tx_trace_full_notify_function)((VOID *) _tx_trace_header_ptr); \
                } \
                else \
                { \
                    _tx_trace_buffer_current_ptr =  trace_event_ptr;  \
                    _tx_trace_header_ptr -> tx_trace_header_buffer_current_pointer =  (ULONG) trace_event_ptr; \
                } \
            } \
        }
#endif
#endif


#ifdef TX_SOURCE_CODE

/* Define internal function prototypes of the trace component, only if compiling ThreadX source code.  */

VOID    _tx_trace_initialize(VOID);
VOID    _tx_trace_object_register(UCHAR object_type, VOID *object_ptr, CHAR *object_name, ULONG parameter_1, ULONG parameter_2);
VOID    _tx_trace_object_unregister(VOID *object_ptr);


#ifdef TX_ENABLE_EVENT_TRACE

/* Check for MISRA compliance requirements.  */

#ifdef TX_MISRA_ENABLE

/* Define MISRA-specific routines.  */

UCHAR                   *_tx_misra_object_to_uchar_pointer_convert(TX_TRACE_OBJECT_ENTRY *pointer);
TX_TRACE_OBJECT_ENTRY   *_tx_misra_uchar_to_object_pointer_convert(UCHAR *pointer);
TX_TRACE_HEADER         *_tx_misra_uchar_to_header_pointer_convert(UCHAR *pointer);
TX_TRACE_BUFFER_ENTRY   *_tx_misra_uchar_to_entry_pointer_convert(UCHAR *pointer);
UCHAR                   *_tx_misra_entry_to_uchar_pointer_convert(TX_TRACE_BUFFER_ENTRY *pointer);


#define TX_OBJECT_TO_UCHAR_POINTER_CONVERT(a)           _tx_misra_object_to_uchar_pointer_convert((a))
#define TX_UCHAR_TO_OBJECT_POINTER_CONVERT(a)           _tx_misra_uchar_to_object_pointer_convert((a))
#define TX_UCHAR_TO_HEADER_POINTER_CONVERT(a)           _tx_misra_uchar_to_header_pointer_convert((a))
#define TX_UCHAR_TO_ENTRY_POINTER_CONVERT(a)            _tx_misra_uchar_to_entry_pointer_convert((a))
#define TX_ENTRY_TO_UCHAR_POINTER_CONVERT(a)            _tx_misra_entry_to_uchar_pointer_convert((a))

#else

#define TX_OBJECT_TO_UCHAR_POINTER_CONVERT(a)           ((UCHAR *) ((VOID *) (a)))
#define TX_UCHAR_TO_OBJECT_POINTER_CONVERT(a)           ((TX_TRACE_OBJECT_ENTRY *) ((VOID *) (a)))
#define TX_UCHAR_TO_HEADER_POINTER_CONVERT(a)           ((TX_TRACE_HEADER *) ((VOID *) (a)))
#define TX_UCHAR_TO_ENTRY_POINTER_CONVERT(a)            ((TX_TRACE_BUFFER_ENTRY *) ((VOID *) (a)))
#define TX_ENTRY_TO_UCHAR_POINTER_CONVERT(a)            ((UCHAR *) ((VOID *) (a)))

#endif
#endif
#endif
#endif

