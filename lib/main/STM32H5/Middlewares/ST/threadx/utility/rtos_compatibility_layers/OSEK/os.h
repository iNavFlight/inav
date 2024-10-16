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
/**   ThreadX Component                                                   */
/**                                                                       */
/**   OSEK IMPLEMENTATION                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  EKV DEFINITIONS                                        RELEASE        */
/*                                                                        */
/*    os.h                                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the constants, structures, etc. needed for the    */
/*    OSEK implementation.                                                */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/

#ifndef TX_OSEK_H
#define TX_OSEK_H

#include <tx_api.h>

#include "osek_user.h"

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif


/**************************************************************************/
/* System Macros.                                                         */
/**************************************************************************/

#define TASK(Taskname)                              void Func ## Taskname()
#define TaskEntry(Taskname)                         Func ## Taskname
#define ALARM(Alarmname)                            void Func ## Alarmname()
#define ISR(ISRname)                                void ISR_ ## ISRname()
#define ISREntry(ISRname)                           ISR_ ## ISRname

#define ALARMCALLBACK(Alarmcallbackfunction)        void Func ## Alarmcallbackfunction()
#define ALARMCALLBACKEntry(Alarmcallbackfunction)   Func ## Alarmcallbackfunction
#define DeclareALARMCALLBACK(Alarmcallbackfunction) ALARMCALLBACK(Alarmcallbackfunction)

#define DeclareISR(ISRname)                         ISR(ISRname)
#define DeclareResource(Resource_name)
#define DeclareEvent(Event_name)
#define DeclareAlarm(Alarmname)                     ALARM(Alarmname)
#define DeclareTask(Taskname)                       TASK(Taskname)

#define CALL_ISR(ISRname)                           ISR_ ## ISRname()

#define OSErrorGetServiceId()                       service_GetServiceId.id
#define OSError_ActivateTask_TaskID()               service_ActivateTask.TaskID
#define OSError_ChainTask_TaskID()                  service_ChainTask.TaskID
#define OSError_GetAlarm_AlarmID()                  service_GetAlarm.AlarmID
#define OSError_CancelAlarm_AlarmID()               service_CancelAlarm.AlarmID
#define OSError_SetAbsAlarm_AlarmID()               service_SetAbsAlarm.AlarmID
#define OSError_SetRelAlarm_AlarmID()               service_SetRelAlarm.AlarmID
#define OSError_GetResource_ResID()                 service_GetResource.ResID
#define OSError_ReleaseResource_ResID()             service_ReleaseResource.ResID
#define OSError_SetEvent_TaskID()                   service_SetEvent.TaskID
#define OSError_GetEvent_TaskID()                   service_SetEvent.TaskID
#define OSError_WaitEvent_EventID()                 service_SetEvent.EventID
#define OSError_ClearEvent_EventID()                service_ClearEvent.EventID

/**************************************************************************/
/*                         OSEK data Types                                */
/**************************************************************************/

typedef ULONG                   STATUS;
typedef void                    *VP;
typedef void                    (* FP)();
typedef ULONG                   PRI;
typedef ULONG                   CounterType;
typedef ULONG                   TaskType;
typedef TaskType                ISRType;                  /* TaskType and ISRType must be same as both often used interchangeably. */
typedef ULONG                   TaskStateType;
typedef ULONG                  *TaskStateRefType;
typedef ULONG                  *TaskRefType;
typedef ULONG                   TickType;
typedef ULONG                  *TickRefType;
typedef ULONG                   AppModeType;
typedef ULONG                   AlarmType;
typedef ULONG                   StatusType;
typedef ULONG                   OsServiceIdType;
typedef ULONG                   ResourceType;
typedef ULONG                  *ResourceRefType;
typedef ULONG                   EventMaskType;
typedef ULONG                  *EventMaskRefType;
typedef ULONG                   EventType;
typedef ULONG                  *EventRefType;


/**************************************************************************/
/*        OSEK  Error Codes                                               */
/**************************************************************************/

#define E_OK                    (0U)
#define E_OS_ACCESS             (1U)
#define E_OS_CALLEVEL           (2U)
#define E_OS_ID                 (3U)
#define E_OS_LIMIT              (4U)
#define E_OS_NOFUNC             (5U)
#define E_OS_RESOURCE           (6U)
#define E_OS_STATE              (7U)
#define E_OS_VALUE              (8U)


/* Additional error codes defined for this Implementation.  */

#define E_OS_EVENT              (9U)
#define E_OS_EXIST              (10U)   /* identical task name are found.  */
#define E_OS_SYSTEM             (11U)   /* Error for the OSEK system.      */
#define E_OS_SYS_STACK          (12U)   /* Error For OSEK Memory.          */

/* osek_internal_error() error codes.                       */

#define THREADX_OBJECT_CREATION_ERROR               (1U)
#define THREADX_THREAD_RESUME_IN_ACTIVATE_TASK      (2U)
#define THREADX_PREEMPTION_CHANGE_GETRESOURCE       (3U)
#define THREADX_PREEMPTION_CHANGE_RLSRESOURCE       (4U)
#define THREADX_THREAD_TERMINATE_TERMINATETASK      (5U)
#define THREADX_THREAD_DELETE_TERMINATETASK         (6U)
#define THREADX_THREAD_TERMINATE_CHAINTASK          (7U)
#define THREADX_THREAD_DELETE_CHAINTASK             (8U)
#define THREADX_THREAD_DELETE_DELETETASK            (9U)
#define THREADX_THREAD_TERMINATE_DELETETASK         (10U)
#define THREADX_PREEMPTION_CHANGE_WRAPPER           (11U)
#define THREADX_THREAD_RELINQUISH_SCHEDULE          (12U)
#define THREADX_MUTEX_CREATERESOURCE                (13U)
#define THREADX_EVENT_SETEVENT                      (14U)
#define THREADX_EVENT_CLEAREVENT                    (15U)
#define THREADX_EVENT_FLAG_GETEVENT                 (16U)
#define THREADX_EVENT_FLAG_WAITEVENT                (17U)
#define QUEUE_DELETETASK                            (18U)
#define NO_FREE_EVENT                               (20U)
#define SYSMGR_FATAL_ERROR                          (21U)
#define EVENT_DELETETASK                            (22U)
#define ERROR_FREEING_MEMORY                        (23U)
#define SYS_MGR_SEND_CHAINTASK                      (24U)
#define SYS_MGR_SEND_TERMINATETASK                  (25U)
#define INVALID_ALARMID_TIMERWRAPPER                (26U)
#define TASK_ENDING_WITHOUT_CHAIN_OR_TERMINATE      (27U)
#define SYSMGR_QUEUE_SEND                           (28U)
#define INVALID_OBJECT_CREATION_CALL                (29U)
#define SYS_MGR_START_OS                            (30U)
#define SYS_MGR_SEND_ACTIVATETASK                   (31U)
#define ERROR_OBJECT_CREATION                       (32U)


/**************************************************************************/
/*  OSEM SYSTEM SERVICES IDs                                              */
/**************************************************************************/

#define OSServiceId_ActivateTask                (1U)
#define OSServiceId_TerminateTask               (2U)
#define OSServiceId_ChainTask                   (3U)
#define OSServiceId_Schedule                    (4U)
#define OSServiceId_GetTaskID                   (5U)
#define OSServiceId_GetTaskState                (6U)
#define OSServiceId_DisableAllInterrupts        (7U)
#define OSServiceId_EnableAllInterrupts         (8U)
#define OSServiceId_SuspendAllInterrupts        (9U)
#define OSServiceId_ResumeAllInterrupts         (10U)
#define OSServiceId_SuspendOSInterrupts         (11U)
#define OSServiceId_ResumeOSInterrupts          (12U)
#define OSServiceId_GetResource                 (13U)
#define OSServiceId_ReleaseResource             (14U)
#define OSServiceId_SetEvent                    (15U)
#define OSServiceId_ClearEvent                  (16U)
#define OSServiceId_GetEvent                    (17U)
#define OSServiceId_WaitEvent                   (18U)
#define OSServiceId_GetAlarmBase                (19U)
#define OSServiceId_GetAlarm                    (20U)
#define OSServiceId_SetRelAlarm                 (21U)
#define OSServiceId_SetAbsAlarm                 (22U)
#define OSServiceId_CancelAlarm                 (23U)
#define OSServiceId_GetActiveApplicationMode    (24U)
#define OSServiceId_StartOS                     (25U)
#define OSServiceId_ShutdownOS                  (26U)


/**************************************************************************/
/*     Implementation Specific OSEK operating modes                       */
/*                                                                        */
/**************************************************************************/

#define NORMAL_EXECUTION_MODE                       (0U)
#define STARTUPHOOK_MODE                            (1U)
#define SHUTDOWNHOOK_MODE                           (2U)
#define PRETASKHOOK_MODE                            (3U)
#define POSTTASKHOOK_MODE                           (4U)
#define ERRORHOOK_MODE                              (5U)
#define ISR1_MODE                                   (6U)
#define ISR2_MODE                                   (7U)
#define TIMER_MODE                                  (8U)
#define INITSYSTEM_MODE                             (9U)
#define ALARM_CALLBACK_MODE                         (10U)
#define OSEK_INIT_NOT_DONE                          (99U)


/**************************************************************************/
/*        OSEK  Constants and Definitions                                 */
/**************************************************************************/

/* OS APPLICATION MODES   */

#define OSDEFAULTAPPMODE                            (1U)


/* OSEK Task Types        */

#define EXTENDED                                    (1U)
#define BASIC                                       (0U)
#define TRUE                                        (1U)
#define FALSE                                       (0U)


/* OSEK Task States       */

#define RUNNING                                     (0U)
#define WAITING                                     (1U)
#define READY                                       (2U)
#define SUSPENDED                                   (3U)
#define THREADX_STATE                               (4U)

/* Invalid task id        */
#define INVALID_TASK                                (0U)


/* ALARM OPERATION MODES  */

#define ABSOLUTE_ALARM                              (1U)
#define RELATIVE_ALARM                              (0U)
#define AUTO_START                                  (1U)
#define NO_START                                    (0U)


/* RESOURCE TYPE */

#define STANDARD                                    (0U)
#define INTERNAL                                    (1U)
#define LINKED                                      (2U)

typedef enum                                        {NON, FULL} SCHEDULE;
typedef enum                                        {ACTIVATETASK, SETEVENT, CALLBACK, NONE} ACTION;
typedef ULONG                                       ACCR_TYPE;
typedef ULONG                                       TASK_TYPE;
typedef ULONG                                       COPY;
typedef ULONG                                       AUTOSTART;


/* ISR types */

#define CATEGORY1                                   (1U)
#define CATEGORY2                                   (2U)


/**************************************************************************/
/*                    SYSTEM CONFIGURATION PARAMETERS                     */
/**************************************************************************/

/* OSEK Priority Definitions  */

#define THREADX_MAX_PRIORITY        (31U)
#define THREADX_HIGHEST_PRIORITY    (0U)
#define THREADX_LOWEST_PRIORITY     (31U)

#define OSEK_MAX_PRIORITY           (23U)
#define OSEK_HIGHEST_PRIORITY       (OSEK_MAX_PRIORITY)
#define OSEK_LOWEST_PRIORITY        (0U)

#define OSEK_NON_SCHEDULE_PRIORITY  (OSEK_HIGHEST_PRIORITY + 1u)      /* 24 */
#define OSEK_ISR2_PRIORITY          (OSEK_NON_SCHEDULE_PRIORITY + 1u) /* 25 */
#define OSEK_ISR1_PRIORITY          (OSEK_ISR2_PRIORITY + 1u)         /* 26 */


/* Define maximum queued activation per task */

#define OSEK_MAX_ACTIVATION          (8U)

/* Define maximum time count for Counters / Alarms */

#define MAXALLOWEDVALUE              (0x7FFFFFFFUL)

#define OSEK_STACK_PADDING          (128U)
#define OSEK_SYSTEM_STACK_SIZE      (1024U)


/* Requests/commands to SysMgr task.  */

#define SYSMGR_START_OS             (0U)
#define SYSMGR_TERMINATE_TASK       (1U)
#define SYSMGR_CHAIN_TASK           (2U)
#define SYSMGR_ACTIVATE_TASK        (3U)
#define SYSMGR_SCHEDULE             (4U)
#define SYSMGR_WAITEVENT            (5U)
#define SYSMGR_RELEASE_RESOURCE     (6U)
#define SYSMGR_SETEVENT             (7U)
#define SYSMGR_SHUTDOWN_OS          (8U)
#define SYSMGR_ERRORHOOK            (9U)
#define SYSMGR_GET_RESOURCE         (10U)

/**************************************************************************/
/* Define size of Region 0 memory segment.                                */
/* NOTE:  This region should be large enough to supply the memory         */
/*        for all task stacks, TCBs, thread control blocks in the system. */
/**************************************************************************/

#define TX_REGION0_SIZE             (OSEK_MEMORY_SIZE)
#define TX_REGION0_SIZE_IN_BYTES    (TX_REGION0_SIZE)


/**********************************************************************************/
/* OSEK System Object maximum numbers                                             */
/* NOTE: These are only suggested values, user may modify them as needed.         */
/*       Memory is the only limiting factor.                                      */
/**********************************************************************************/

/* Define the maximum number of simultaneous OSEK tasks supported.  */
/* Interrupt Service Routines are also treated as a task.           */
/* Total 8 Interrupt sources are supported.                         */
/* That gives 24 max OSEK tasks.                                    */

#define OSEK_MAX_TASKS              (32U)

/* Define the maximum number of simultaneous Internal OSEK Resources supported. */

#define OSEK_MAX_INTERNAL_RES       (8U)


/* Define the maximum number of simultaneous External OSEK Resources supported. */

#define OSEK_MAX_EXTERNAL_RES       (16U)


/* Define the maximum number of simultaneous OSEK Resources (Internal & External Combined) supported.  */

#define OSEK_MAX_RES                (OSEK_MAX_INTERNAL_RES + OSEK_MAX_EXTERNAL_RES)


/* Define the maximum number of simultaneous OSEK alarms supported.  */

#define OSEK_MAX_ALARMS             (16U)


/* Define the maximum number of simultaneous OSEK counters supported.  */
/* This includes a counter to be assigned as a SystemTimer.            */
/* A system can have 8 alarms so 8 counters (one counter for each alarm) would be enough.  */

#define OSEK_MAX_COUNTERS           (16U)


/* Define the maximum number of simultaneous OSEK events supported.  */
/* An event is just a bit , so a ULONG (32 bit data type) is used.    */

#define OSEK_MAX_EVENTS             (32U)


/* Define the maximum number of  OSEK ISR sources supported.   */

#define OSEK_MAX_ISR                (8U)


/* Define  OSEK Internal Task Queue depth per priority.   */
/* There is one such queue for each priority level.  */
/*     What is the maximum number of tasks that can go in a queue?                                       */
/*     Assuming that all tasks are of same priority and each task is activated to maximum                */
/*     allowable limits (OSEK_MAX_ACTIVATION) it should be  OSEK_MAX_ACTIVATION * OSEK_MAX_TASKS         */
/*     but sometimes (READY)tasks are moved from queue to queue based on its changed ceiling priority    */
/*     and that number could be equal to to maximum number of tasks supported.                           */

#define TASK_QUEUE_DEPTH1            (OSEK_MAX_ACTIVATION * OSEK_MAX_TASKS)
#define TASK_QUEUE_DEPTH             (TASK_QUEUE_DEPTH1 + OSEK_MAX_TASKS)

#define SYSMGR_QUEUE_MSG_LENGTH     (4U)
#define SYSMGR_QUEUE_MSG_COUNT      (OSEK_MAX_ALARMS)
#define SYSMGR_QUEUE_DEPTH          ((sizeof(ULONG)) * SYSMGR_QUEUE_MSG_COUNT)

#define SYSMGR_PRIORITY             (0U)
#define SYSMGR_THRESHOLD            (0U)


/**********************************************************************************/
/* OSEK System Object identifier this is placed in the object's control structure */
/**********************************************************************************/

#define OSEK_TASK_ID                (0x1234ABCDUL)
#define OSEK_ALARM_ID               (0x4567ABCDUL)
#define OSEK_COUNTER_ID             (0xABCD1234UL)
#define OSEK_EVENT_ID               (0x1234FEDCUL)
#define OSEK_RES_ID                 (0x5678CDEFUL)
#define OSEK_APPLICATION_ID         (0x789ABCDEUL)
#define OSEK_ISR_ID                 (0x3456CDEFUL)


/**************************************************************************/
/*                   OSEK SYSTEM OBJECT CONTROL STRUCTURES                */
/**************************************************************************/


/*                 OSEK SERVICES                                          */

struct Service_ActivateTask
{
    StatusType      TaskID;
};

struct Service_TerminateTask
{
    StatusType      TaskID;
};

struct Service_ChainTask
{
    StatusType      TaskID;
};

struct Service_Schedule
{
    StatusType      TaskID;
};

struct Service_GetTaskID
{
    TaskRefType     TaskID;
};

struct Service_GetTaskState
{
    StatusType      TaskID;
};

struct Service_DisableAllInterrupts
{
    StatusType      TaskID;
};

struct Service_EnableAllInterrupts
{
    StatusType      TaskID;
};

struct Service_SuspendAllInterrupts
{
    StatusType      TaskID;
};

struct Service_ResumeAllInterrupts
{
    StatusType      TaskID;
};

struct Service_SuspendOSInterrupts
{
    StatusType      TaskID;
};

struct Service_ResumeOSInterrupts
{
    StatusType      TaskID;
};

struct Service_GetResource
{
    ResourceType     ResID;
};

struct Service_ReleaseResource
{
    ResourceType      ResID;
};

struct Service_SetEvent
{
    StatusType      EventID;
    TaskType        TaskID;
};

struct Service_ClearEvent
{
    EventMaskType      EventID;
};

struct Service_GetEvent
{
    StatusType      EventID;
    TaskType        TaskID;
};

struct Service_WaitEvent
{
    EventMaskType      EventID;
};

struct Service_GetAlarmBase
{
    AlarmType     AlarmID;
};

struct Service_GetAlarm
{
    AlarmType      AlarmID;
};

struct Service_SetRelAlarm
{
    AlarmType  AlarmID;
    TickType   increment;
    TickType   cycle;
};

struct Service_SetAbsAlarm
{
    AlarmType  AlarmID;
    TickType   start;
    TickType   cycle;
};

struct Service_CancelAlarm
{
    AlarmType  AlarmID;
};

struct Service_GetActiveApplicationMode
{
    StatusType      TaskID;
};

struct Service_StartOS
{
    StatusType      TaskID;
};

struct Service_ShutdownOS
{
    StatusType      TaskID;
};

struct Service_GetServiceId
{
    OsServiceIdType id;
};


/**************************************************************************/
/*                 OSEK Resource                                          */
/**************************************************************************/

typedef struct osek_resource_struct
{
     /* Name.  */
    const  CHAR                 *name;

    /* This Resource is in use.  */
    ULONG                       res_in_use;

    /* Ceiling priority of the resource.  */
    UINT                        c_priority;

    /* Task occupying this resource.  */
    TaskType                    taskid;

    /* Type.  */
    StatusType                  type;            /* Internal, External, Linked.  */

    /* Linked Resource.  */

    ResourceType                linked_res;      /* id of the resource , Linked with this res.  */

    ResourceType                resolved_res;   /* Id of the res linked to this , after all chained links.  */

    /* Resource Object id.  */
    ULONG                       osek_res_id;

} OSEK_RESOURCE;


/**************************************************************************/
/*               Task  structure                                          */
/**************************************************************************/

typedef struct osek_tcb_struct
{
    /* This task's ThreadX TCB.  */
    TX_THREAD                   task;

     /* Name.  */
    const CHAR                  *name;

    /* This field indicates if this task is in use.  */
    ULONG                       tcb_in_use;

    /* Task type BASIC or EXTENDED.  */
    UINT                        task_type;

    /* Task AUTOSTART mode.  */
    UINT                        task_autostart;

    /* Design time Scheduling policy of this Task.  */
    SCHEDULE                    policy;

    /* Task start address (entry point).  */
    FP                          task_entry;

    /* Design time task priority.  */
    UINT                        org_prio;


    /* Current ThreadX thread preemption threshold.  */
    UINT                        cur_threshold;

    /* task stack size.  */
    ULONG                       stack_size;

    /* start address of the task stack.  */
    CHAR                        *pStackBase;


    /* Task status Suspended/ Ready(Running).  */
    ULONG                       suspended;

    /* Wait status */
    ULONG                       waiting;

    /* Task to be activated when this task calls ChainTask().  */
    TaskType                    task_to_chain;

    /* Counter to indicate how many Resources are occupied.  */
    ULONG                       res_ocp;

    /* Maximum multiple activation allowed for this task.  */
    UINT                        max_active;

    /* Current (recorded) multiple activation  requests.  */
    UINT                        current_active;

    /* List of Event assigned to this task.  */
    EventMaskType               events;

    EventMaskType               waiting_events;

    EventMaskType               set_events;

    /* List of External resources assigned (Design time) to this task.  */
    ResourceType                external_resource_list[OSEK_MAX_EXTERNAL_RES];

    /* List of External Resources currently occupied (Run time) by this task.  */
    ResourceType                external_resource_occuplied_list[OSEK_MAX_EXTERNAL_RES];

    /* List of Internal resources assigned (Design Time) to this task.  */
    ResourceType                internal_resource_list[OSEK_MAX_INTERNAL_RES];

    /* List of Internal Resources currently occupied (Run Time)  by this task.  */
    ResourceType                internal_resource_occuplied_list[OSEK_MAX_INTERNAL_RES];

    /* Internal Resource assigned or not.  */
    UINT                        internal_res;

    /* RES_SCHEDULER taken or not.  */
    UINT                        resource_scheduler;

    AppModeType                 task_Appl_Mode;

    /* Task Object id.  */
    ULONG                       osek_task_id;

} OSEK_TCB;


/**************************************************************************/
/*                 Counter                                                */
/**************************************************************************/

typedef struct OSEK_COUNTER_STRUCT
{

    /* Max. allowable value.  */
    TickType                    maxallowedvalue;

    /* Tick base multiplier.  */
    TickType                    ticksperbase;

    /* Minimum cyclic numbers of counter ticks allowed for a cyclic alarm.  */
    TickType                    mincycle;

     /* Name.  */
    const CHAR*                 name;

    /* This field indicates if this counter is in use.  */
    UINT                        cntr_in_use;

    /* Pre-count needed to increment main count by 1.  */
    TickType                    sub_count;

    /* Current counter value in ticks.  */
    TickType                    counter_value;

    /* Whether attached to system timer.  */
    UINT                        system_timer;

    /* List of all alarms attached to this counter.  */
    AlarmType                   alarm_list[OSEK_MAX_ALARMS];

    /* Counter object id.  */
    ULONG                       osek_counter_id;


}OSEK_COUNTER;


/**************************************************************************/
/*                 ALARM                                                  */
/**************************************************************************/

typedef struct OSEK_ALARM_STRUCT
{

     /* Name.  */
    const CHAR*                 name;

    /* This field indicates if this entry is in use.  */
    UINT                        alarm_in_use;

    UINT                        occupied;

    /* Max allowed value of count for this alarm depends on the counter to which this alarm is attached.  */
    TickType                    max_allowed_value;

    /* Cycles programmed depends on the counter to which this alarm is attached.  */
    TickType                    min_cyc;

    /* Tick base multiplier.  */
    TickType                    ticks_per_base;

    /* Cycles programmed for this alarm.  */
    TickType                    cycle;

    /* alarm expiration count.  */

    TickType                    expiration_count;

    /* Alarm Callback function.  */
    void                        (*alarm_callback)();

    /* Alarm Activation.  */
    UINT                        armed;

    /* Alarm Action.  */
    UINT                        action;

    /* Attach task here.  */
    OSEK_TCB*                   task;

    /* Attach the counter.  */
    OSEK_COUNTER                *cntr;

    /* Event to set in case of SET_EVENT action. The event bits should be 1
       to SET that EVENT of the task.  */
    EventMaskType               events;

    /* Start up action.  */
    UINT                        auto_start;

    /* ALARM armed in relative mode or abs mode.  */
    UINT                        rel_abs_mode;

    /* Counter roll back flag for absolute alarm mode.  */
    UINT                        counter_rollback;

    /* Alarm object id.  */
    ULONG                       osek_alarm_id;

} OSEK_ALARM;


typedef struct USER_ALARM_STRUCT
{

    /* Max allowed value of count.  */
    TickType                    maxallowedvalue;

    /* Cycles.  */
    TickType                    mincycle;

    /* Tick base multiplier.  */
    TickType                    ticksperbase;

}AlarmBaseType;

typedef AlarmBaseType     *AlarmBaseRefType;


typedef struct REGISTER_APPLICATION_STRUCT
{

    AppModeType                 application_mode;

    /* ErrorHook.  */
    void                        (*error_hook_handler)(StatusType);
    /* Startup hook.  */
    void                        (*startup_hook_handler)();
    /* Shutdown Hook.  */
    void                        (*shutdown_hook_handler)(StatusType);

    void                        (*pretask_hook_handler)(void);

    void                        (*posttask_hook_handler)(void);

    /* Any errors generated while creating the application.  */
    StatusType                  osek_object_creation_error;


    /* Application object ID.  */
    ULONG                       osek_application_id;


}APPLICATION_INFO;

typedef APPLICATION_INFO    *APPLICATION_INFO_PTR;


/**************************************************************************/
/*      OSEK API CALLS                                                    */
/**************************************************************************/


/* TASK MANAGEMENT */

StatusType     ActivateTask(TaskType TaskId);
StatusType     GetTaskID(TaskType *TaskID);
StatusType     TerminateTask(void);
StatusType     ChainTask(TaskType TaskID);
StatusType     GetTaskState(TaskType TaskID, TaskStateRefType State);
TaskType       CreateTask(const CHAR *name, void(*entry_function)(), UINT priority, UINT max_activation,
                            ULONG stack_size, SCHEDULE policy, AUTOSTART start, UINT, AppModeType mode);
StatusType     Schedule (void);


/* RESOURCE MANAGEMENT */

ResourceType   CreateResource(const CHAR *name, StatusType type, ResourceType linked_res);
StatusType     GetResource(ResourceType id);
StatusType     ReleaseResource(ResourceType id);
StatusType     RegisterTasktoResource(ResourceType Resource, TaskType TaskID);

/* EVENT MANAGEMENT   */

StatusType     SetEvent(TaskType task_id, EventMaskType mask);
StatusType     ClearEvent(EventMaskType mask);
StatusType     GetEvent(TaskType task_id, EventMaskRefType event);
StatusType     WaitEvent(EventMaskType mask);
EventMaskType  CreateEvent(void);
StatusType     RegisterEventtoTask(EventType eventid, TaskType TaskID);

/* INTERUUPT MANAGEMENT */

StatusType     EnableInterrupt(void);
StatusType     DisableInterrupt(void);

StatusType     GetInterruptDescriptor(UINT *mask);

void           SuspendAllInterrupts(void);
void           ResumeAllInterrupts(void);

void           SuspendOSInterrupts(void);
void           ResumeOSInterrupts(void);

void           DisableAllInterrupts(void);
void           EnableAllInterrupts(void);

ISRType        CreateISR(const CHAR *name, void(*entry_function)(), UINT category, ULONG stack_size);
StatusType     RegisterISRtoResource(ResourceType Resource, ISRType ISRID);


/* COUNTER MANAGEMENT  */

StatusType     GetCounterValue(OSEK_COUNTER *counter_ptr, TickRefType ticks);
CounterType    CreateCounter(const CHAR *name, TickType max_allowed_value, TickType ticks_per_base,
                             TickType min_cycle, TickType start_value);
StatusType     IncrCounter(CounterType cntr);
StatusType     DefineSystemCounter(CounterType cntr);

/* ALARM MANAGEMENT */

StatusType     GetAlarmBase(AlarmType AlarmID, AlarmBaseRefType info);
StatusType     SetAbsAlarm(AlarmType AlarmID, TickType start, TickType cycle);
StatusType     SetRelAlarm(AlarmType AlarmID, TickType increment, TickType cycle);
StatusType     CancelAlarm(AlarmType AlarmID);
StatusType     GetAlarm(AlarmType AlarmID, TickRefType tick_ptr);
AlarmType      CreateAlarm(const CHAR *name, CounterType cntr, UINT action, ULONG events,
                       TaskType task, void (*callback)(), UINT Startup, TickType Alarmtime, TickType Cycle);

/* OS MANAGEMENT */

UCHAR         *osek_initialize(void *osek_memory, APPLICATION_INFO_PTR application1);
UINT           osek_cleanup(APPLICATION_INFO_PTR application1);
void           ShutdownOS(StatusType error);
void           StartOS(StatusType os_mode);
AppModeType    GetActiveApplicationMode(void);


void           process_ISR2(ISRType isrname);

#endif


/******************************* End of file ************************/
