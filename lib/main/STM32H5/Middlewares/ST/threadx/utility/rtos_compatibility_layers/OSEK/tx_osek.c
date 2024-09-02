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
/**   OSEK IMPLEMENTATION                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary files.  */

#include "os.h"

/* Include some required internal headers. */

#include <tx_timer.h>
#include <tx_thread.h>
#include <tx_initialize.h>


/* Declare a structure to hold application information.  */

APPLICATION_INFO_PTR                    Application;


/* Define a global variable to hold operating mode of this wrapper.  */

static UINT                             osek_wrapper_operation_mode = OSEK_INIT_NOT_DONE;


/* Define the threads for the System Manager.  */

TX_THREAD                               osek_system_manager;


/* Define ThreadX message queues for System Manager.  */

TX_QUEUE                                osek_work_queue;


/* Define a byte pool control block for the osek region0 memory used.  */

TX_BYTE_POOL                            osek_region0_byte_pool;


/* Define ThreadX timer to act as a system timer.  */

TX_TIMER                                osek_system_timer;


/************ OSEK System objects Definitions **************/

/* Define a static pool of Task structures.  */

static OSEK_TCB                         osek_tcb_pool[OSEK_MAX_TASKS];


/* Define a static pool of resource structures.  */

static OSEK_RESOURCE                    osek_res_pool[OSEK_MAX_RES];


/* Defines a static pool of alarm structures.  */

static OSEK_ALARM                       osek_alarm_pool[OSEK_MAX_ALARMS];


/* Defines a static pool of counter structures.  */

static OSEK_COUNTER                     osek_counter_pool[OSEK_MAX_COUNTERS];


/* Define a Task Ready Queue.  */
/* For each OSEK priority there is a sub queue, each queue entry holds an id of a task ready to run.  */

TaskType                                task_table [OSEK_ISR1_PRIORITY + 1u][TASK_QUEUE_DEPTH];


/* A global variable holding all 32 events (bit flags) for this OS implementation.  */

EventMaskType                           global_events;
EventMaskType                           global_event_count;


/* Define some default system objects.  */

ResourceType                            RES_SCHEDULER;
CounterType                             SYS_TIMER;


/* Variables to store current and last executed task details.  */

ULONG                                   last_run_task;
ULONG                                   system_start;                   /* If TX_TRUE indicates a fresh system start.  */
UINT                                    task_terminated;


/* Define ISR2 controlling flags.  */
/* disable_ISR2 is the global enable/disable flag , if this flag is set no ISR2 is allowed or even kept pending.  */
/* suspend_ISR2 is NON Zero then any ISR2 is kept pending (provided that disable_ISR2 must be zero before hand).  */

/* disable_ISR2 = TX_FALSE & suspend_ISR2 = TX_FALSE :: process ISR2 immediately
   disable_ISR2 = TX_FALSE & suspend_ISR2 = TX_TRUE  :: log ISR2 but don't process till suspend_ISR2 becomes TX_FALSE
   disable_ISR2 = TX_TRUE  & suspend_ISR2 DON't care :: No ISR2 logged and executed  */

UINT                                    disable_ISR2 = TX_TRUE;         /* This flag must be TX_FALSE to allow ISR 2.  */
UINT                                    suspend_ISR2 = TX_TRUE;         /* This flag must be TX_FALSE to register and execute ISR2.  */
UINT                                    ISR2_pending = TX_FALSE;

/* Track initialization status.  */

static UINT                             osek_init_state; /* Set to 0 for uninitialized, 1 for initialized by not yet started and 2 if started.  */

#define OSEK_NOT_INIT (0U)
#define OSEK_INIT     (1U)
#define OSEK_STARTED  (2U)

#define OSEK_MAX_LINK_DEPTH (1024U) /* Maximum depth of linked resource, to prevent infinite loops in case of a corrupted list.  */

/* OSEK API Services and associated structures.  */

struct Service_ActivateTask             service_ActivateTask;
struct Service_TerminateTask            service_TerminateTask;
struct Service_ChainTask                service_ChainTask;
struct Service_Schedule                 service_Schedule;
struct Service_GetTaskID                service_GetTaskID;
struct Service_GetTaskState             service_GetTaskState;

struct Service_DisableAllInterrupts     service_DisableAllInterrupts;
struct Service_EnableAllInterrupts      service_EnableAllInterrupts;
struct Service_SuspendAllInterrupts     service_SuspendAllInterrupts;
struct Service_ResumeAllInterrupts      service_ResumeAllInterrupts;
struct Service_SuspendOSInterrupts      service_SuspendOSInterrupts;
struct Service_ResumeOSInterrupts       service_ResumeOSInterrupts;

struct Service_GetResource              service_GetResource;
struct Service_ReleaseResource          service_ReleaseResource;

struct Service_SetEvent                 service_SetEvent;
struct Service_ClearEvent               service_ClearEvent;
struct Service_GetEvent                 service_GetEvent;
struct Service_WaitEvent                service_WaitEvent;

struct Service_GetAlarmBase             service_GetAlarmBase;
struct Service_GetAlarm                 service_GetAlarm;
struct Service_SetRelAlarm              service_SetRelAlarm;
struct Service_SetAbsAlarm              service_SetAbsAlarm;
struct Service_CancelAlarm              service_CancelAlarm;

struct Service_GetActiveApplicationMode service_GetActiveApplicationMode;
struct Service_StartOS                  service_StartOS;
struct Service_ShutdownOS               service_ShutdownOS;

struct Service_GetServiceId             service_GetServiceId;


/**************************************************************************/
/*          OSEK internal functions prototypes                            */
/**************************************************************************/


/* Entry functions for various ThreadX threads used for OS management.  */

static void             osek_system_manager_entry(ULONG input);
static void             osek_system_timer_entry(ULONG input);
static void             osek_task_wrapper(ULONG tcb);


/* Initialization and reset for OSEK system objects.  */

static UINT             osek_memory_init (void *region0_ptr);
static void             osek_alarm_init(void);
static void             osek_tcb_init(void);
static void             osek_resource_init(void);
static void             osek_counter_init(void);

static void             osek_reset_alarm(OSEK_ALARM  *alarm_ptr);
static void             osek_reset_counter(OSEK_COUNTER  *counter_ptr);
static void             osek_reset_tcb(OSEK_TCB  *tcb_ptr);
static void             osek_reset_res(OSEK_RESOURCE  *res_ptr);

/* Some utilities.  */

static ULONG            osek_task_independent_area(void);
static OSEK_TCB         *osek_thread2tcb(TX_THREAD *thread_ptr);
static UINT             osek_remap_priority(UINT osek_priority);

/* Functions to obtain OSEK objects from predefined pools.  */

static UINT            osek_memory_allocate(ULONG size, void **memory_ptr);
static UINT            osek_get_alarm(void);
static CounterType     osek_get_counter(void);
static ResourceType    osek_get_resource(void);
static EventMaskType   osek_get_event(void);
static ULONG           osek_allocate_tcb(ULONG stack_size, OSEK_TCB **tcb_ptr);

/* Internal operation functions.  */

static StatusType      osek_do_task_terminate(OSEK_TCB  *tcb_ptr);
static StatusType      osek_do_activate_task (OSEK_TCB  *tcb_ptr);
static StatusType      osek_create_task(OSEK_TCB *tcb_ptr);
static StatusType      get_internal_resource(OSEK_TCB  *tcb_ptr);
static StatusType      release_internal_resource(OSEK_TCB  *tcb_ptr);
static StatusType      check_external_resource(OSEK_TCB  *tcb_ptr);
static StatusType      TerminateISR(void);


/* OSEK internal error function and Hook routine executors.  */

static void            osek_internal_error(ULONG error_code);
static void            exec_ErrorHook (StatusType error);
static void            exec_PreTaskHook(void);
static void            exec_PostTaskHook(void);


/*OSEK Scheduler helper functions.  */

static void            add_task_to_table(OSEK_TCB *tcb_ptr);
static void            start_osek_tasks(void);
static void            push_task_to_table(OSEK_TCB *tcb_ptr);
static void            pop_task_from_table(OSEK_TCB *tcb_ptr);
static UINT            check_task_to_run (OSEK_TCB *tcb_ptr );
static void            check_linked_resources(void);
static StatusType      ActivateISR(ISRType ISRID);


/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    StartOS                                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts the OS. If StartOSHook is defined it is        */
/*    called.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    StatusType                     Application mode                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    StartupHook                    If defined                           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   StartOS(StatusType  os_mode)
{

ULONG               request[SYSMGR_QUEUE_MSG_LENGTH];
StatusType          status;

   check_linked_resources();

   Application->application_mode = OSDEFAULTAPPMODE;

   /* Check for any startup hook routine.  */
   if (Application->startup_hook_handler != TX_NULL)
   {
       /* Change operation mode for to startuphook mode.  */
       osek_wrapper_operation_mode = STARTUPHOOK_MODE;
       (Application->startup_hook_handler)(os_mode);
   }

   /* Change back to default operations mode.  */
   osek_wrapper_operation_mode = NORMAL_EXECUTION_MODE;

   /* Now send message to system thread to start the task scheduler.  */

   /* Build the request.  */
   request[0] = SYSMGR_START_OS;            /* Request type.  */
   request[1] = 0u;                         /* Dummy.  */
   request[2] = 0u;                         /* Dummy.  */
   request[3] = 0u;                         /* Dummy.  */

   /* Now send a message to the SysMgr supervisor thread.        */
   /* to start OS task scheduler.                                */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   /* This should always succeed.  */
   if (status != TX_SUCCESS)
   {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_START_OS);
   }

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CreateTask                                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a new task with specified attributes.         */
/*    This call is allowed only during Application Initialization.        */
/*    This is not a standard OSEK API call.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    priority                      Priority of the Task                  */
/*    stack_size                    Stack size of the Task                */
/*    entry_function                Task entry function pointer           */
/*    policy                        Scheduling policy for this Task       */
/*    active_no                     Maximum activation number             */
/*    start                         Starting state of the task            */
/*    type                          Task type: Basic/Extended             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TaskId                        Task Id if successful                 */
/*    TX_NULL                       Error while creating the task         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_allocate_tcb             Get a TCB from the pool               */
/*    osek_create_task              Create an OSEK Task                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Initialization code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
TaskType       CreateTask(const CHAR *name, void(*entry_function)(), UINT priority, UINT max_activation,
                            ULONG stack_size, SCHEDULE policy, AUTOSTART start, UINT type, AppModeType mode)
{

OSEK_TCB       *tcb_ptr;      /* Pointer to task control block.  */
ULONG           temp32;
ULONG           status;


   /* Check whether we are called during initialization.  */
   /* This will ensure that no one calls this function after system is started.  */
   if (osek_init_state != OSEK_INIT)
   {
       Application->osek_object_creation_error++;

       /* Return OSEK internal error. NOTE: This is not a standard OSEK error.  */
       return ((TaskType)TX_NULL);
   }

   /* Now check the validity of all input parameters.  */

   /* Entry point function.  */
   if (entry_function == TX_NULL)
   {
       /* Entry function not specified! Return an error. This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /*  Task priority. */
   if (priority > OSEK_HIGHEST_PRIORITY)
   {
       /* Return an internal error. This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* The max_activation parameter must be within limit, that is a non zero value but less
      than OSEK_MAX_ACTIVATION. */
   if ((max_activation > OSEK_MAX_ACTIVATION) || (max_activation == 0u))
   {
       /* Return an internal error. This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Adjust the input stack size and check. */
   /* Force stack size to a multiple of 4 bytes, round up if needed.  */
   temp32 = ((stack_size + 3u) & ~0x3u);

   /* Add a little extra padding to stack.  */
   temp32 += OSEK_STACK_PADDING;

   /* Is task stack big enough? ThreadX needs a minimum stack size for each task.  */
   if (temp32 < TX_MINIMUM_STACK)
   {
       /* Return an error as there is no enough stack.  This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Scheduling Policy */
   if ((policy != FULL) && (policy != NON))
   {
       /* Return an error as policy supplied is unknown.  This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Task Type */
   if ((type != BASIC) && (type != EXTENDED))
   {
     /* Return an error as task type supplied is unknown.  This is not a standard OSEK error.  */
     Application->osek_object_creation_error++;
     return ((TaskType)TX_NULL);
   }

   /* AUTO START option.  */
   if ((start != TRUE) && (start != FALSE))
   {
       /* Return an error as Start type is unknown.This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Got all input parameters within limits, now try to get a free Task Control Block (TCB) for this new task.  */
   tcb_ptr = TX_NULL;
   status = osek_allocate_tcb(temp32, &tcb_ptr);

   /* Make sure we got a TCB.  */
   if((status != TRUE) || (tcb_ptr == TX_NULL))
   {
       /* Return an error since no memory is available.  This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Now fill up the TCB with all supplied parameters.  */

   /* Store TASK object ID this is same for all TCB, this helps in checking whether the data
      structure is really a TASK CONTROL BLOCK.  */
   tcb_ptr->osek_task_id = OSEK_TASK_ID;

   /* Store task's name and type.  */
   tcb_ptr->name = name;
   tcb_ptr->task_type = type;

   /* Store the statically assigned (design time) priority and same would be RUN time priority.  */
   tcb_ptr->org_prio = priority;
   tcb_ptr->cur_threshold = priority;

   /* Store the task entry point. This is where the task will begin upon activation or chaining.  */
   tcb_ptr->task_entry = entry_function;

   /* Store current task activation number as well as the maximum activations defined for this task.  */
   tcb_ptr->max_active = max_activation;
   tcb_ptr->current_active = 0u;
   tcb_ptr->internal_res = TX_FALSE;

   /* Store the scheduling policy defined for the task.  */
   tcb_ptr->policy = policy;

   /* Store the start up condition of the task.  */
   tcb_ptr->task_autostart = start;

   /* A task is always created in suspended state even though it is defined as an AUTO START.  */
   tcb_ptr->suspended = TX_TRUE;
   tcb_ptr->waiting = TX_FALSE;

   /* Store task's application mode.  */
   tcb_ptr->task_Appl_Mode = mode;

   /* Now create the ThreadX thread which actually runs this task.  */
   status = osek_create_task(tcb_ptr);

   /* Check whether we encounter any error during ThreadX thread creation.  */
   if (status == E_OS_SYSTEM)
   {
       /* Can't create a task? This is not a standard OSEK error.*/
       Application->osek_object_creation_error++;
       return ((TaskType) TX_NULL);
   }

   /* Everything worked fine so far. Return TaskID. This TaskID is nothing  but a TCB structure
      type casted to TaskType UDT.  */
   return ((TaskType)tcb_ptr);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CreateISR                                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a new task which will execute an ISR.         */
/*    This call is allowed only during Application Initialization.        */
/*    This is not a standard OSEK API call.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    priority                      Priority of the ISR                   */
/*    stack_size                    Stack size of the ISR                 */
/*    entry_function                Task entry function pointer           */
/*    policy                        Scheduling policy for this task       */
/*    active_no                     Maximum activation number             */
/*    start                         Starting state of the task            */
/*    type                          Task type: Basic/Extended             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TaskId                        Task Id if successful                 */
/*    TX_NULL                       Error while creating the ISR          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_allocate_tcb             Get a TCB from the pool               */
/*    osek_create_task              Create an OSEK Task                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Initialization code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/

ISRType CreateISR(const CHAR *name, void (*entry_function)(), UINT category, ULONG stack_size)
{

OSEK_TCB       *tcb_ptr;      /* Pointer to task control block.  */
ULONG           temp32;
ULONG           status;


   /* Check if we are called during initialization.  */
   if((osek_init_state != OSEK_INIT) ||  /* Not in initialization.  */
        (_tx_thread_current_ptr == &_tx_timer_thread))
   {
       /* Return default error.  */
       Application->osek_object_creation_error++;

       return((ResourceType)TX_NULL);
   }

   /* Scheduling policy.  */
   if ((category != CATEGORY1) && (category != CATEGORY2))
   {
       /* Return an error as policy supplied is unknown.  This is not a standard OSEK error. */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Entry point function.  */
   if(entry_function == NULL)
   {
       /* Entry function not specified! Return an error. This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Adjust the input stack size and check. */
   /* Force stack size to a multiple of 4 bytes, round up if needed.  */
   temp32 = ((stack_size + 3u) & ~0x3u);

   /* Add a little extra padding to stack.  */
   temp32 += OSEK_STACK_PADDING;

   /* Is task stack big enough? ThreadX needs a minimum stack size for each task.  */
   if (temp32 < TX_MINIMUM_STACK)
   {
       /* Return an error as there is not enough stack.  This is not a standard OSEK error. */
       Application->osek_object_creation_error++;
       return ((ISRType)TX_NULL);
   }

   /* Got all input parameters within limits, now try to get a free task control block(TCB) for this new ISR.  */
   tcb_ptr = TX_NULL;
   status = osek_allocate_tcb(temp32, &tcb_ptr);
   /* Make sure we got a TCB.  */
   if((status != TRUE) || (tcb_ptr == TX_NULL))
   {
       /* Return an error since no memory is available.  This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((TaskType)TX_NULL);
   }

   /* Got a tcb , now fill up the TCB with all supplied parameters.  */

   /* Store ISR objectID ,this is same for all ISRs, this helps in checking whether the data
      structure is really a ISR CONTROL BLOCK.  */
   tcb_ptr->osek_task_id = OSEK_ISR_ID;

   /* Store ISR name.  */
   tcb_ptr->name = name;
   /* Store ISR category.  */
   tcb_ptr->task_type = category;

   /* Store the statically assigned (design time) priority and same would be runtime priority.  */

   if (category == 1u)
   {
       tcb_ptr->org_prio = OSEK_ISR1_PRIORITY;
   }
   else
   {
       tcb_ptr->org_prio = OSEK_ISR2_PRIORITY;
   }

   tcb_ptr->cur_threshold = tcb_ptr->org_prio;

   /* Store the ISR entry point. This is where the ISR will begin.  */
   tcb_ptr->task_entry = entry_function;

   /* Store maximum activations defined for ISR.  */
   /* Only 1 ISR of this name can be pending.   */
   tcb_ptr->max_active = 1u;
   tcb_ptr->current_active = 0u;
   tcb_ptr->internal_res = TX_FALSE;

   /* Store the scheduling policy defined for the task.  */
   tcb_ptr->policy = NON;

   /* Store the start up condition of the ISR which of course is DO NOT START.  */
   tcb_ptr->task_autostart = FALSE;

   /* An ISR is always created in suspended state.  */
   tcb_ptr->suspended = TX_TRUE;
   tcb_ptr->waiting = TX_FALSE;

   /* Now create the ThreadX thread which actually mimics this ISR.  */
   status = osek_create_task(tcb_ptr);

   /* Check whether we encounter any error during ThreadX thread creation.  */
   if (status == E_OS_SYSTEM)
   {
       /* Can't create a task? This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((ISRType)TX_NULL);
   }

   /* Everything worked fine so far. Return ISRID. This  is nothing  but a TCB structure
      type casted to ISRType UDT.  */
   return ((ISRType)tcb_ptr);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ActivateTask                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This call transfers a task from suspended state into the ready      */
/*    state. The operating system ensures that the task code is being     */
/*    executed from the first statement. The service may be called both   */
/*    from interrupt level and from task level. Rescheduling after this   */
/*    call depends on   from where it is called from. Also if required    */
/*    resources are freed as well as this being the highest priority task */
/*    than the task executing at this instance. If E_OS_LIMIT is returned */
/*    then activation is ignored. When an extended task is transferred    */
/*    from suspended to ready state all its events are cleared.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    TaskId                                Task Name                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                  If successful                 */
/*    E_OS_ID                               Invalid TaskId                */
/*    E_OS_LIMIT                            Task activation limit         */
/*                                          exceeded                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_queue_send                         Send message to Sys Manager   */
/*    osek_internal_error                   OSEK internal error           */
/*    osek_task_independent_area            Check calling context         */
/*    tx_thread_identify                    Get ThreadX thread of caller  */
/*    osek_thread2tcb                       Convert ThreadX  to OSEK      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType ActivateTask(TaskType TaskId)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
OSEK_TCB       *tcb_ptr_self;
UINT            status;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
ULONG           area;
TX_THREAD       *p_thread;


   /* Log this call and arguments passed, this is required for the macro 'OSErrorGetServiceId'.  */
   service_GetServiceId.id = (OsServiceIdType)OSServiceId_ActivateTask;
   service_ActivateTask.TaskID = TaskId;

   /* Check if we are in task context.  */
   area = osek_task_independent_area();
   if(area != TX_TRUE)
   {
       exec_ErrorHook(E_OS_CALLEVEL);
       /* Return error. */
       return (E_OS_CALLEVEL);
   }

   /* Check operating mode */
   if ((osek_wrapper_operation_mode != NORMAL_EXECUTION_MODE) && (osek_wrapper_operation_mode != ISR2_MODE))
   {
       /* Hook routines and alarm callbacks can't call  this service. */
       /* This explicit check is required because all hook routines are
          executed in task's context!  */
       exec_ErrorHook(E_OS_ID);
       return (E_OS_CALLEVEL);
   }

   /* Get OSEK TCB of the calling task.  */
   p_thread = tx_thread_identify();
   tcb_ptr_self = osek_thread2tcb(p_thread);

   if ((tcb_ptr_self->osek_task_id != OSEK_TASK_ID) && (tcb_ptr_self->osek_task_id != OSEK_ISR_ID))
   {
       /* This call is allowed only from TASK and ISR */
       return (E_OS_CALLEVEL);
   }

   /* Get OSEK TCB of the TASK to activate.  */
   tcb_ptr = (OSEK_TCB *)TaskId;
   if ((tcb_ptr == TX_NULL) || (tcb_ptr->osek_task_id != OSEK_TASK_ID))
   {
       exec_ErrorHook(E_OS_ID);
       return (E_OS_ID);
   }

   TX_DISABLE

   /* Check whether the task to be activated is activated up to its defined activation limit.  */
   if(tcb_ptr->current_active >= tcb_ptr->max_active)
   {
       TX_RESTORE
       exec_ErrorHook(E_OS_LIMIT);
       /* Reached its max activation limit.  */
       return (E_OS_LIMIT);
   }

   /* Now send a message to the system manager thread to activate this task.   */
   /* Build the request. */
   request[0] = SYSMGR_ACTIVATE_TASK;          /* Request type.  */
   request[1] = (ULONG)tcb_ptr;                /* Task to activate.  */

   /* Now check who is calling this service, a task or an ISR?  */
   if (tcb_ptr_self->osek_task_id == OSEK_TASK_ID)
   {
       request[2] = (ULONG)tcb_ptr_self;      /* Task id of the calling task.  */
   }
   else
   {
       request[2] = 0u;  /* When ISR activates a task rescheduling (if any) is held
                            till ISR completes so no need to suspend ISR task.  */
   }

   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority, this call   */
   /* will be preempted by SysMgr supervisor thread.                           */
   /* SysMgr will eventually call osek_do_task_activate.                       */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed.  */
   if (status != TX_SUCCESS)
   {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_SEND_ACTIVATETASK);
   }

   /* Return status.  */
   return(E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    TerminateTask                                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This service terminates the calling task, which means transferring   */
/*   the calling task from the running state into the suspended state.    */
/*   Only internal resources held by this task are released here. While   */
/*   it is assumed that any external resources occupied by the task must  */
/*   have been released before the call to TerminateTask. In case         */
/*   a resource is still occupied while calling this service, then the    */
/*   behaviour is undefined in STANDARD version of OSEK. In the EXTENDED  */
/*   version of OSEK, this service returns an error, which can be         */
/*   evaluated by the application.                                        */
/*   If successful, this call will causes rescheduling, this also means   */
/*   that upon success TerminateTask does not return to the call level.   */
/*                                                                        */
/*   NOTE:                                                                */
/*                                                                        */
/*   Ending a task function without call to TerminateTask or ChainTask is */
/*   strictly forbidden and may leave the system in an undefined state.   */
/*   But in this implementation TerminateTask service is called if needed.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                            If success                          */
/*    Error Code.                     If error                            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area      Check if we are in task context     */
/*    osek_thread2tcb                 Get TCB pointer for thread pointer  */
/*    tx_queue_send                   Send message to Sys Manager Thread  */
/*    osek_internal_error             In case of any internal error       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code  (TASKs only)                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  TerminateTask(void)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
UINT            index;
UINT            status;
ULONG           area;
TX_THREAD      *p_thread;


   service_GetServiceId.id = (OsServiceIdType)OSServiceId_TerminateTask;
   service_TerminateTask.TaskID = (StatusType)0u;

   /* Check for Task or ISR context. */
   /* All ISRs are treated as as a high priority tasks.  */
   area = osek_task_independent_area();
   if(area != TX_TRUE)
   {
       exec_ErrorHook(E_OS_CALLEVEL);
       /* Return error.  */
       return (E_OS_CALLEVEL);
   }

   /* Get OSEK TCB of this TASK/ISR.  */
   p_thread = tx_thread_identify();
   tcb_ptr = osek_thread2tcb(p_thread);
   if (tcb_ptr == TX_NULL)
   {
       exec_ErrorHook(E_OS_CALLEVEL);
       return (E_OS_CALLEVEL);
   }

   if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
   {
       /* This call is allowed only from TASK and ISR.  */
       exec_ErrorHook(E_OS_CALLEVEL);
       return (E_OS_CALLEVEL);
   }


   /* Check operating mode */
   if (osek_wrapper_operation_mode != NORMAL_EXECUTION_MODE)
   {
       /* Hook routines and alarm callbacks can't call  this service.  */
       /* This explicit check is required because all hook routines are
          executed in task's context!  */
       exec_ErrorHook(E_OS_CALLEVEL);

       return (E_OS_CALLEVEL);
   }

   TX_DISABLE

   /* Check if any resource is occupied. A task can not be terminated if it is holding any resource.   */
   if(tcb_ptr->res_ocp != 0u)
   {
      TX_RESTORE

      exec_ErrorHook(E_OS_RESOURCE);

      /* Return.  */
      return (E_OS_RESOURCE);
   }

   /* Release any internal resources held.  */
   if (tcb_ptr->internal_res != 0u)
   {
       for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
       {
           if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
           {
               break;
           }

           ((OSEK_RESOURCE *)(tcb_ptr->internal_resource_occuplied_list[index]))->taskid = 0u;
           tcb_ptr->internal_resource_occuplied_list[index] = 0u;

       }   /* End of for loop.  */
   }

   /* Now it's okay to terminate this task, clear its events.  */
   tcb_ptr->waiting_events = 0u;
   tcb_ptr->set_events = 0u;

   /* Now all set to terminate this task.  */
   /* Send a message to the System Manager to terminate this task.  */
   /* Build the request.  */

   request[0] = SYSMGR_TERMINATE_TASK;      /* Request type.  */
   request[1] = (ULONG)tcb_ptr;             /* ID of the task to kill.  */
   request[2] = 0u;
   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority,        */
   /* this call will be preempted by SysMgr supervisor thread             */
   /* SysMgr will eventually call osek_do_task_terminate and reschedule.  */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed. and Sys Manager terminates this task    */
   /* This point will never be reached, as this thread itself will be     */
   /* deleted by the system manager!                                      */

   if (status != TX_SUCCESS)
   {
       osek_internal_error(SYS_MGR_SEND_TERMINATETASK);
   }

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ChainTask                                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service causes the termination of the calling task. After      */
/*    termination of the calling task, a succeeding task (supplied as an  */
/*    input argument for the call) is activated. If the succeeding task   */
/*    is the same calling task, then this does not result in multiple     */
/*    requests and the task is not transferred to the suspended state.    */
/*    Only internal resources held by this task are released here, even   */
/*    in case the calling task is identical with the task to chain.       */
/*    While it is assumed that any external resources occupied by the     */
/*    task must have been released before the call to TerminateTask.      */
/*    In case a resource is still occupied while calling this service,    */
/*    then the behaviour is undefined in STANDARD version of OSEK.        */
/*    In the EXTENDED version of OSEK, an error is returned, which can be */
/*    evaluated by the application.                                       */
/*    If called successfully, ChainTask does not return to call level and */
/*    the status can not be evaluated. In case of error the service       */
/*    returns to the calling task and provides a status which can then be */
/*    checked by the application.                                         */
/*    If successful, this call will causes rescheduling, this also means  */
/*    that upon success ChainTask does not return to the call level.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Task_ID                              Task id to activate.           */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                 If successful.                 */
/*    E_OS_LIMIT                           Too many activations           */
/*    E_OS_CALLEVEL                        Called at interrupt level      */
/*    E_OS_RESOURCE                        Resources are not released     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area       Make sure called in Task context   */
/*    osek_thread2tcb                  Get TCB pointer for the thread     */
/*    tx_queue_send                    send message to sys manager thread */
/*    osek_internal_error              Any internal errors                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code  (TASKs only)                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType ChainTask(TaskType  TaskID)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
OSEK_TCB       *tcb_ptr1;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
UINT            index;
ULONG           area;
TX_THREAD      *p_thread;

   service_GetServiceId.id = (OsServiceIdType)OSServiceId_ChainTask;
   service_ChainTask.TaskID = TaskID;

   /* Check if we are in task context.  */
   area = osek_task_independent_area();
   if(area != TX_TRUE)
   {
       exec_ErrorHook(E_OS_CALLEVEL);
       /* Return error.  */
       return(E_OS_CALLEVEL);
   }

   /* Get OSEK TCB of this TASK/ISR.  */
   p_thread = tx_thread_identify();
   tcb_ptr = osek_thread2tcb(p_thread);
   if (tcb_ptr == TX_NULL)
   {
       exec_ErrorHook(E_OS_ID);
       return (E_OS_ID);
   }

   if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
   {
       /* This call is allowed only from TASK.  */
       exec_ErrorHook(E_OS_CALLEVEL);
       return (E_OS_CALLEVEL);
   }

   /* Check operating mode.  */
   if (osek_wrapper_operation_mode != NORMAL_EXECUTION_MODE)
   {
       /* ISRs, hook routines and alarm callbacks can't call  this service.  */
       /* This explicit check is required because all ISRs and hook routines are
          executed in task's context!  */
       exec_ErrorHook(E_OS_CALLEVEL);

       return (E_OS_CALLEVEL);
   }

   /* Check if any external resources are occupied.  */
   if(tcb_ptr->res_ocp != 0u)
   {
       exec_ErrorHook(E_OS_RESOURCE);

       /* The external resource is not released.  */
       return (E_OS_RESOURCE);
   }

   /* Get tcb of task to be chained.  */
   tcb_ptr1 = (OSEK_TCB *)TaskID;

   /* First, check for an invalid task pointer.  */
   if ((tcb_ptr1 == TX_NULL) || ((tcb_ptr1->osek_task_id) != OSEK_TASK_ID))
   {
       exec_ErrorHook(E_OS_ID);

       /* Return Error.  */
       return (E_OS_ID);
   }

   TX_DISABLE

   /* Check both calling task and task_to_be_chained are one and the same.  */
   if (tcb_ptr != tcb_ptr1)
   {
       /* Check for proper multiple activation.  */
       if(tcb_ptr1->current_active >= tcb_ptr1->max_active)
       {
           TX_RESTORE

           /* Reached its max activation limit.  */
           exec_ErrorHook(E_OS_LIMIT);

           return (E_OS_LIMIT);
       }
   }

   /* Store TaskID which is to be chained after termination of this task.  */
   tcb_ptr->task_to_chain = TaskID;

   /* Now release any internal resources held by the calling task.  */
   if (tcb_ptr->internal_res != 0u)
   {
       for (index = 0u ; index < OSEK_MAX_INTERNAL_RES; index++)
       {
           if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
           {
               break;
           }

           ((OSEK_RESOURCE *)(tcb_ptr->internal_resource_occuplied_list[index]))->taskid = 0u;
           tcb_ptr->internal_resource_occuplied_list[index] = 0u;

       }   /* End of for loop.  */
   }

   /* Now it's ok to terminate this task , clear its events.  */
   tcb_ptr->waiting_events = 0u;
   tcb_ptr->set_events = 0u;

   /* Send message to the system manager to suspend calling task and activate
      task to be chained.  */

   /* Build the request. */
   request[0] = SYSMGR_CHAIN_TASK;          /* Request type.  */
   request[1] = (ULONG)tcb_ptr;             /* TCB ptr of calling task.  */
   request[2] = TaskID;                     /* Task to chain.  */
   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority,      */
   /* this routine will be preempted by SysMgr supervisor thread.       */

   tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed and sys manager terminates this task   */
   /* This point will never be reached, as this thread itself will be    */
   /* deleted by the System Manager!                                     */

   osek_internal_error(SYS_MGR_SEND_CHAINTASK);

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetTaskID                                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    GetTaskID returns the task id of the task currently running.        */
/*    Calling GetTaskID is allowed from task level, ISR level and in      */
/*    several hook routines. This service is intended to be used by       */
/*    library functions and hook routines. If <TaskID> can’t be evaluated */
/*    (no task currently running), the service returns INVALID_TASK as    */
/*    TaskType.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    TaskID                         Pointer to stored Task ID            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                           If success                           */
/*    INVALID_TASK                   If failure                           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_identify             Identifies the current thread        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType GetTaskID(TaskRefType TaskID)
{

TX_THREAD      *thread_ptr;


   /* Check for valid pointer to store the task ID.  */
   if(TaskID == TX_NULL)
   {
       /* Return error.  */
       return (E_OS_ID);
   }

   /* Check for the pointer to thread.  */
   thread_ptr = tx_thread_identify();

   if ((thread_ptr == TX_NULL) || (thread_ptr == &_tx_timer_thread))
   {
       /* No task running TaskID can’t be evaluated return a special Error.  */
       *TaskID = INVALID_TASK;

       return (E_OK);
   }

   if ((osek_wrapper_operation_mode == STARTUPHOOK_MODE)     ||
       (osek_wrapper_operation_mode == SHUTDOWNHOOK_MODE)    ||
       (osek_wrapper_operation_mode == ALARM_CALLBACK_MODE))
   {
       *TaskID = INVALID_TASK;
   }
   else
   {
       *TaskID = last_run_task;
   }

    /* Return success.  */
    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetTaskState                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Returns the state of a task (running, ready, waiting, suspended) at */
/*    the time of calling GetTaskState. The service may be called from    */
/*    interrupt service routines, task level, and some hook routines.     */
/*    Within a full preemptive system, calling this operating system      */
/*    service only provides a meaningful result if the task runs in an    */
/*    interrupt disabling state at the time of calling.                   */
/*    When a call is made from a task in a full preemptive system, the    */
/*    result may already be incorrect at the time of evaluation. When the */
/*    service is called for a task, which is multiply activated, the      */
/*    state is set to running if any instance of the task is running.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Task_ID                              Task id to query               */
/*    State                                Pointer to result state        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                 If successful                  */
/*    E_OS_ID                              Task is invalid                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area           Check if called from task      */
/*                                         independent area               */
/*    tx_thread_identify                   Identify the current thread    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType GetTaskState(TaskType TaskID, TaskStateRefType State)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
TX_THREAD      *this_thread;
ULONG           area;
TX_THREAD      *p_thread;


   service_GetServiceId.id = (OsServiceIdType)OSServiceId_GetTaskState;
   service_GetTaskState.TaskID = (StatusType)0;

   /* Check for valid task ID.  */
   if(TaskID == 0u)
   {
       exec_ErrorHook(E_OS_ID);
       /* Return error.  */
       return (E_OS_ID);
   }

   /* Get OSEK TCB.  */
   tcb_ptr = (OSEK_TCB *)TaskID;

   /* First, check for an invalid task pointer.  */
   if((tcb_ptr == TX_NULL) || ((tcb_ptr->osek_task_id) != OSEK_TASK_ID))
   {
       exec_ErrorHook(E_OS_ID);
       /* Return error.  */
       return (E_OS_ID);
   }

   TX_DISABLE

   if ((osek_wrapper_operation_mode == STARTUPHOOK_MODE)     ||
       (osek_wrapper_operation_mode == SHUTDOWNHOOK_MODE)    ||
       (osek_wrapper_operation_mode == ALARM_CALLBACK_MODE))
   {
       TX_RESTORE

       /* Return error.  */
       return (E_OS_ID);
   }

   /* Get the ThreadX TCB.  */
   this_thread = (TX_THREAD *)tcb_ptr;

   /* Check if we are only in task context.  */
   area = osek_task_independent_area();
   if (area == TX_TRUE)
   {
       /* Get the OSEK TCB.  */
       p_thread = tx_thread_identify();
       if(this_thread == p_thread)
       {
           TX_RESTORE

           /* This is the running thread.  */
           *State = RUNNING;
           return (E_OK);
       }
   }

   if (tcb_ptr->waiting == TX_TRUE)
   {
       TX_RESTORE

       *State = WAITING;
       return (E_OK);
   }
   if (tcb_ptr->suspended == TX_TRUE)
   {
       TX_RESTORE

       *State = SUSPENDED;
       return (E_OK);
   }

   TX_RESTORE

   *State = READY;
   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    Schedule                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    If a higher-priority task is ready, the internal resource of the    */
/*    task is released, the current task is put into the ready state, its */
/*    context is saved and the higher-priority task is executed.          */
/*    Otherwise the calling task is continued.                            */
/*    Rescheduling can only take place if an internal resource is         */
/*    assigned to the calling task during system generation. For these    */
/*    tasks, schedule enables a processor assignment to other tasks with  */
/*    lower or equal priority than the ceiling priority of the internal   */
/*    resource and higher priority than the priority of the calling task  */
/*    in application-specific locations. When returning from schedule,    */
/*    the internal resource has been taken again.                         */
/*    This service has no influence on tasks with no internal resource    */
/*    assigned (preemptable tasks).                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                 If success.                    */
/*    E_OS_CALLLEVEL                       Called at interrupt level      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area           Check if called from task      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType Schedule (void)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
TX_THREAD      *this_thread;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
UINT            status;
ULONG           area;

   service_GetServiceId.id = (OsServiceIdType)OSServiceId_Schedule;
   service_GetTaskState.TaskID = (StatusType)0u;

   /* Check if we are in task context.  */
   area = osek_task_independent_area();
   if(area != TX_TRUE)
   {
       /* Return error.  */
       return (E_OS_CALLEVEL);
   }

   /* Get the ThreadX thread and OSEK TCB.  */
   this_thread = tx_thread_identify();
   tcb_ptr = osek_thread2tcb(this_thread);

   if (tcb_ptr == TX_NULL)
   {
       return (E_OS_ID);
   }

   if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
   {
       /* This call is allowed only from TASK.  */
       return(E_OS_ID);
   }

   TX_DISABLE

   /* Check operating mode.  */
   if (osek_wrapper_operation_mode != NORMAL_EXECUTION_MODE)
   {

       TX_RESTORE

       /* Hook routines, alarm callbacks and ISRs can't call this service. */
       /* This explicit check is required because all hook routines are
          executed in task's context. */
       return (E_OS_ID);
   }

   /* Check if any resource is occupied.  */
   if(tcb_ptr->res_ocp != 0u)
   {
       TX_RESTORE

       exec_ErrorHook(E_OS_RESOURCE);
       /* Return.  */
       return (E_OS_RESOURCE);

   }

   /* Now release internal resources, if any, held by this task   */
   /* This call releases internal resources and moves the task to */
   /* a new queue position based on its new ceiling priority.     */
   /* release_internal_resource(tcb_ptr); */

   /* Now send message to system Manager to check any higher priority task is ready.  */
   /* Build the request.  */
   request[0] = SYSMGR_SCHEDULE;          /* Request type.  */
   request[1] = (ULONG)this_thread;       /* Self ID.  */
   request[2] = 0u;
   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority,      */
   /* this routine will be preempted by SysMgr supervisor thread when   */
   /* queue read is successful.                                         */
   /* System Manager will eventually call start_osek_tasks.             */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed.  */
   if (status != TX_SUCCESS)
   {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_SEND_CHAINTASK);
   }

   /* Assume that the sys manager does its job.  */
   /* And has returned from schedule, restore preemption threshold if any.  */

   /* Now we need to move this task back to its priority level */
   /* before coming here all internal resources (if any are taken).  */

   TX_DISABLE

   /* Now check task's scheduling policy */
   if (tcb_ptr->policy == NON)
   {
       pop_task_from_table(tcb_ptr);
       /* If policy is NON then restore to OSEK_NON_SCHEDULE priority.  */
       tcb_ptr->cur_threshold = OSEK_NON_SCHEDULE_PRIORITY;
       push_task_to_table(tcb_ptr);
   }

   TX_RESTORE

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CreateResource                                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Creates  resources for inter-task mutual exclusion for resource     */
/*    protection                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    name                         Name of the Resource.                  */
/*    type                         Type of the resource standard/internal */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    RES_ID                       If successful                          */
/*    ZERO                         If error while creating resource       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_memory_allocate         Allocate memory from system            */
/*    osek_get_resource            Get one resource from the pool         */
/*    tx_mutex_create              Create a Mutex in ThreadX              */
/*    osek_internal_error          OSEK internal Error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
ResourceType  CreateResource(const CHAR *name, StatusType type, ResourceType linked_res)
{

OSEK_RESOURCE  *res_ptr;
OSEK_RESOURCE  *linked_res_ptr;
OSEK_RESOURCE  *res1_ptr;

ResourceType    ID;
ResourceType    linked_res_id;
UINT            done_flag;
UINT            iter_max;

   /* Check whether calling from task context or ISR.  */
   if ((osek_init_state != OSEK_INIT) ||  /* Not in Initialization.  */
       (_tx_thread_current_ptr == &_tx_timer_thread))
   {
       /* Return default error.  */
       Application->osek_object_creation_error++;
       return ((ResourceType) TX_NULL);
   }

   /* Check whether the linked resource (if specified) exists.  */
   if (type == LINKED)
   {
       if (linked_res == 0u)
       {
       /* Specified linked resource doesn't exist  */
       /* OR Resource is trying to link itself.  */
           Application->osek_object_creation_error++;
           return ((ResourceType) TX_NULL);
       }

       /* Check if the resource to which this resource is linked is external.  */
       linked_res_ptr = (OSEK_RESOURCE *)linked_res;
       if (linked_res_ptr->type == INTERNAL)
       {
           /* Specified linked resource is of INTERNAL type.  */
           Application->osek_object_creation_error++;
           return ((ResourceType) TX_NULL);
       }
   }

   /* Find out any resource is available.  */
   ID = osek_get_resource();
   if(ID == 0u)
   {
       /* Resource is not available.  */
       Application->osek_object_creation_error++;
       return ((ResourceType) TX_NULL);
   }

   /* Get the OSEK resource.  */
   res_ptr = (OSEK_RESOURCE *)ID;

   res_ptr->name = name;

   res_ptr->c_priority = 0u;

   res_ptr->type = type;

   res_ptr->linked_res = linked_res;

   res_ptr->resolved_res = 0u;

   /* Now resolve if chain of linked linked resources.  */

   if (res_ptr->type == LINKED)
   {
       done_flag = FALSE;
       res1_ptr = res_ptr;
       iter_max = OSEK_MAX_LINK_DEPTH; /* Safety count to prevent infinite loop.  */
       while(done_flag == 0u)
       {
           if(iter_max == 0u) {
               /* Maximum iteration count exceeded, return an error. */
               Application->osek_object_creation_error++;
               return ((ResourceType) TX_NULL);
           }

           linked_res_id = res1_ptr->linked_res;
           linked_res_ptr = (OSEK_RESOURCE *)linked_res_id;
           if (linked_res_ptr->type == STANDARD)
           {
               res_ptr->resolved_res = linked_res_id;
               done_flag = TRUE;
           }
           res1_ptr = linked_res_ptr;

           iter_max--;
       }

   }


   /* No task is occupying this resource.  */
   res_ptr->taskid = 0u;

   res_ptr->osek_res_id = OSEK_RES_ID;

   return (ID);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetResource                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call serves to enter critical sections in the code that are     */
/*   assigned to the resource referenced by <ResID>. A critical section   */
/*   must always be left using ReleaseResource. Nested resource           */
/*   occupation is only allowed if the inner critical sections are        */
/*   completely executed within the surrounding critical section.         */
/*   Nested occupation of one and the same resource is also forbidden.    */
/*   Corresponding calls to GetResource and ReleaseResource should appear */
/*   within the same function on the same function level.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                           Id of the resource.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If success                             */
/*    E_OS_CALLEVEL                Called from ISR                        */
/*    E_OS_ACCESS                  Attempt to get a resource which is     */
/*                                 already occupied by any task or ISR,   */
/*                                 or the statically assigned priority of */
/*                                 the calling task or interrupt routine  */
/*                                 is higher than the calculated ceiling  */
/*                                 priority,                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area   See if called from task independent    */
/*                                 area                                   */
/*    tx_thread_identify           Identify the current thread            */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  GetResource(ResourceType id)
{
TX_INTERRUPT_SAVE_AREA
OSEK_RESOURCE  *osek_res;
OSEK_TCB       *tcb_ptr;
TX_THREAD      *this_thread;
UINT            index ;
UINT            new_prio;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
UINT            status;
ULONG           area;



   service_GetServiceId.id = (OsServiceIdType)OSServiceId_GetResource;
   service_GetResource.ResID = id;

   /* Check we are calling from task context only.  */
   area = osek_task_independent_area();
   if(area == 0u)
   {
       exec_ErrorHook(E_OS_CALLEVEL);
       /* Return default error.  */
       return (E_OS_CALLEVEL);
   }

   /* Check for valid ID.  */
   if(id == 0u)
   {
       exec_ErrorHook(E_OS_ID);
       /* Return error.  */
       return (E_OS_ID);
   }

   /* Get thread currently executed.  */
   this_thread = tx_thread_identify();
   if(this_thread == NULL)
   {
       exec_ErrorHook(E_OS_CALLEVEL);
       return (E_OS_CALLEVEL);
   }

   /* Get RES's control block.  */
   osek_res = (OSEK_RESOURCE *)id;

   /* First, check for an invalid resource pointer.  */
   if((osek_res == TX_NULL) || ((osek_res->osek_res_id) != OSEK_RES_ID))
   {
       exec_ErrorHook(E_OS_ID);

       /* Return Error.  */
       return (E_OS_ID);
   }

   /* Get the OSEK TCB.  */
   tcb_ptr = (OSEK_TCB *) this_thread;

   /* Now check whether this resource is standard or internal.  */
   if (osek_res->type == INTERNAL)
   {
       /* Internal Resource can not be occupied via GetResource call.  */
       exec_ErrorHook(E_OS_ID);
       return (E_OS_ID);
   }

   TX_DISABLE

   /* Now check whether this resource is occupied.  */
   if (osek_res->taskid != 0u)
   {
       TX_RESTORE

       /* Already occupied by this task or any other task, this also prevent double occupancy.  */
       exec_ErrorHook(E_OS_ACCESS);

       return (E_OS_ACCESS);
   }

   /* OK up to this point, now what type of resource is it? */
   /* Because 'RES_SCEDULER' is a special type of resource.  */

   if (id == RES_SCHEDULER)
   {

       /* This task has taken this resource so update this task's resource hold count.  */
       tcb_ptr->res_ocp++;

       /* Save this task's id in the res's control block.  */
       osek_res->taskid = (TaskType)tcb_ptr;

       tcb_ptr->resource_scheduler = TX_TRUE;

       /* Being RES_SCHEDULER no need to add RES_SCEDULER in the task's list of occupied resources.  */

       /* As task is occupying RES_SCHEDULER all preemptions are blocked so make the policy to NON PREEMP.  */
       /* A NON preempt type task is already having OSEK_NON_SCHEDULE_PRIORITY during RUN time, so need
          to change it.  */

       if (tcb_ptr->policy == FULL)
       {
           /* For a FULL preempt type task assign having highest preemption threshold */
           /* Now ask the System Manager to change this task's priority, and also check if there is
              any possibility of preemption.  */
           /* Send message to System Manager to check any preemption out of ReleaseResource call.  */
           new_prio = OSEK_NON_SCHEDULE_PRIORITY;

           /* Build the request.  */
           request[0] = SYSMGR_GET_RESOURCE;          /* Request type.  */
           request[1] = (ULONG)this_thread;
           request[2] = (ULONG)new_prio;
           request[3] = 0u;

           /* Since the SysMgr supervisor thread has the highest priority,      */
           /* this routine will be preempted by SysMgr supervisor thread when   */
           /* queue is successful.                                              */
           /* System Manager will eventually call start_osek_tasks.             */

           status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

           /* This should always succeed.  */
           if (status != TX_SUCCESS)
           {
               /* System internal error.  */
               osek_internal_error(SYS_MGR_SEND_CHAINTASK);

           }
       }

       TX_RESTORE

       return (E_OK);

   }   /* End if ( id == RES_SCHEDULER ).  */


   /* Not RES_SCEDULER. */

   /* Check whether this resource is assigned to this task or not?  */
   for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
   {
       if (tcb_ptr-> external_resource_list[index] == id) {
           /* Yes it is registered with this task, stop searching.  */
           break;
       }
   } /* Check next in the list.  */

   /* Checked entire list and found that this resource is not in the list?  */
   if (index >= OSEK_MAX_EXTERNAL_RES)
   {
       TX_RESTORE

       exec_ErrorHook(E_OS_NOFUNC);

       /* Return error.  */
       return (E_OS_NOFUNC);
   }

   /* Yes this resource is assigned to this task.  */
   /* First check the task has room to hold one more resource?  */

   /* Find the next free entry in the task's res occupied list.  */
   for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
   {
       if(tcb_ptr->external_resource_occuplied_list[index] == 0u)
       {
           /* Found next free entry but do not add this resource to the list at this moment.  */
           break;
       }
   }

   /* We got an entry or not?  */
   if (index >= OSEK_MAX_EXTERNAL_RES)
   {
       TX_RESTORE

       /* This is very unlikely, as list is big enough to hold all resources defined in the system.  */
       exec_ErrorHook(E_OS_ACCESS);

       /* Return Error.  */
       return (E_OS_ACCESS);
   }

   /* Now entry is available, take this resource.  */

   tcb_ptr->external_resource_occuplied_list[index] = id;
   /* Save this task's id in the res's control block to indicate that this is the owner of this res.  */
   osek_res->taskid = (TaskType)(tcb_ptr);
   /* This task has taken this resource so update this task's resource hold count.  */
   tcb_ptr->res_ocp++;

   /* Now we need to change this task's preemption threshold to this resource's ceiling priority.  */
   /* But if the task is of NON scheduling no need to take any action , it is already having
      highest preemption threshold.  */

   if ((tcb_ptr->policy == FULL) && (tcb_ptr->cur_threshold < osek_res->c_priority))
   {
       /* Need to change the at the task is having lower preemption threshold. */
       /* Remove this task from its current queue and place it in front of
          occupied resource's ceiling priority queue.  */

       new_prio = osek_res->c_priority;

       /* Build the request. */
       request[0] = SYSMGR_GET_RESOURCE;          /* Request type.  */
       request[1] = (ULONG)this_thread;
       request[2] = (ULONG)new_prio;
       request[3] = 0u;

       /* Since the SysMgr supervisor thread has the highest priority,      */
       /* this routine will be preempted by SysMgr supervisor thread when   */
       /* queue is successful.                                              */
       /* System Manager will eventually call start_osek_tasks.             */

       status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

       /* This should always succeed.  */
       if (status != TX_SUCCESS)
       {
           /* System internal error.  */
           osek_internal_error(SYS_MGR_SEND_CHAINTASK);
       }
   }

   TX_RESTORE

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ReleaseResource                                     PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   ReleaseResource is the counterpart of GetResource and serves to      */
/*   leave critical sections in the code that are assigned to the         */
/*   resource referenced by <ResID>. For information on nesting           */
/*   conditions, see GetResource. The service may be called from an ISR   */
/*   and from task level                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                           Id of the resource.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    E_OS_NOFUNC                  Attempt to release a resource which is */
/*                                 not occupied by any task or ISR, or    */
/*                                 another resource has to be released    */
/*                                 before,If error occurs while creating  */
/*                                 task                                   */
/*    E_OS_ACCESS                  Attempt to release a resource which has*/
/*                                 a lower ceiling priority than the      */
/*                                 statically assigned priority of the    */
/*                                 calling task or interrupt routine.     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*    osek_task_independent_area   See if called from task independent    */
/*                                 area                                   */
/*    tx_thread_identify           Identify the current thread            */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  ReleaseResource(ResourceType id)
{
TX_INTERRUPT_SAVE_AREA
OSEK_RESOURCE  *osek_res;
OSEK_TCB       *tcb_ptr;
TX_THREAD      *this_thread;
UINT           index;
UINT           res_prio;
UINT           location;
UINT           new_prio;
ULONG          request[SYSMGR_QUEUE_MSG_LENGTH];
UINT           status;
ULONG          area;

   service_GetServiceId.id = (OsServiceIdType)OSServiceId_ReleaseResource;
   service_ReleaseResource.ResID = id;

   /* Fist check valid resource id and proper calling context.  */

   /* Check we calling from from task context only.  */
   area = osek_task_independent_area();
   if(area == 0u)
   {
       exec_ErrorHook(E_OS_CALLEVEL);

       /* Return default error.  */
       return (E_OS_CALLEVEL);
   }

   /* Check for valid Resource ID.  */
   if(id == 0u)
   {
       exec_ErrorHook(E_OS_ID);

       /* Return error.  */
       return (E_OS_ID);
   }

   /* Get thread currently executed.  */
   this_thread = tx_thread_identify();
   if(this_thread == TX_NULL)
   {
       exec_ErrorHook(E_OS_CALLEVEL);

       return (E_OS_CALLEVEL);
   }

   /* Get OSEK resource control block.  */
   osek_res = (OSEK_RESOURCE *)id;

   /* Get the OSEK TCB.  */
   tcb_ptr = (OSEK_TCB *)this_thread;

   /* Now check whether this resource is standard or internal.  */

   if (osek_res->type == INTERNAL)
   {
       /* Internal resource can not be released via ReleaseResource call.  */
       exec_ErrorHook(E_OS_ID);

       return (E_OS_ID);
   }

   TX_DISABLE

   /* Is this task holding any resource? Check for resource count.  */
   if(tcb_ptr->res_ocp == 0u)
   {
       TX_RESTORE

       /* No resource taken yet trying to release resource?  */
       exec_ErrorHook(E_OS_NOFUNC);

       /* Return error.  */
       return(E_OS_NOFUNC);
   }

   /* Check whether this resource is occupied by this task.  */
   /* A task cannot release resource which are not occupied by itself.  */

   if (osek_res->taskid != (TaskType)tcb_ptr)
   {
       TX_RESTORE

       exec_ErrorHook(E_OS_NOFUNC);

       return (E_OS_NOFUNC);
   }

   /* Everything is valid up to this point, now try to release the resource requested.  */
   /* Check what type of Resource it is as RES_SCEDULER is a special case of resource.  */

   if (id == RES_SCHEDULER)
   {

       /* Remove this task's id from the res's control block which usually indicate that this is the
          owner of this res  and also update this task's resource hold count.  */
       osek_res->taskid = 0u;
       tcb_ptr->res_ocp--;

       /* As the task is giving up RES_SCHEDULER, make the associated flag FALSE.  */
       tcb_ptr->resource_scheduler = TX_FALSE;

       /* If Task is NON scheduling type, no preemption takes place.  */
       if (tcb_ptr->policy == NON)
       {
           TX_RESTORE

           /* Release this resource.  */
           return (E_OK);
       }

       /* With FULL scheduling, a change in preemption to highest
          ceiling priority of any other resources held by this task is needed.  */
       /* Check RES_SCEDULER is the only resource occupied by this task, then restore this task's threshold
          to its original priority.  */

       /* Assume no other resources are held.  */
       new_prio = tcb_ptr->org_prio;

       if (tcb_ptr->res_ocp > 0u)
       {
           /* If no other external resources are held the task will revert back to
              to its original design time priority.  */
           /* Or check the ceiling priorities of other external resources held by this task.  */

           for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
           {
               if (tcb_ptr->external_resource_occuplied_list[index] == 0u)
               {
                   break;
               }

               res_prio = ((OSEK_RESOURCE *) (tcb_ptr->external_resource_occuplied_list[index]))->c_priority;
               if  (new_prio < res_prio) {
                   new_prio = res_prio;
               }
           }
       }

       /* Done with external resource, now check if there are any internal resources held?  */
       for ( index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
       {
           if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
           {
               break;
           }

           res_prio = ((OSEK_RESOURCE *) (tcb_ptr->internal_resource_occuplied_list[index]))->c_priority;

           if (new_prio < res_prio)
           {
               new_prio = res_prio;
           }
       }


       /* new_prio now holds the highest of ceiling priority of the remaining
      resources held by this task.  */

       /* Now ask the system manager to change this task's priority, and also check if there is
          any possibility of preemption.  */
       /* Send message to the system manager to check any preemption out of ReleaseResource call.  */

       /* Build the request. */
       request[0] = SYSMGR_RELEASE_RESOURCE;          /* Request type.  */
       request[1] = (ULONG)this_thread;
       request[2] = (ULONG)new_prio;
       request[3] = 0u;

       /* Since the SysMgr supervisor thread has the highest priority,      */
       /* this routine will be preempted by SysMgr supervisor thread when   */
       /* queue is successful.                                              */
       /* System Manager will eventually call start_osek_tasks.             */

       status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

       /* This should always succeed.  */
       if (status != TX_SUCCESS)
       {
           /* System internal error.  */
           osek_internal_error(SYS_MGR_SEND_CHAINTASK);

       }

       TX_RESTORE

       /* Assume that the system manager does its job.  */
       return (E_OK);

   } /* End if ( id == RES_SCHEDULER ).  */


   /* Not a RES_SCHEDULER.  */

   /* Are we releasing the resource which is last taken?  */
   for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
   {
       if ( tcb_ptr->external_resource_occuplied_list[index] == 0u)
       {
           /* Reached at the last entry, stop searching.  */
           break;
       }
   } /* Next entry.  */

   if (index == 0u)
   {
       /* List is empty and resource is taken?  */
       return(E_OS_NOFUNC);
   }

   location = index - 1u; /* Save the location of last resource entry.  */

   /* Check the last resource taken.  */
   if (tcb_ptr->external_resource_occuplied_list[location] != id)
   {
       /* Are we trying to release the resource which was not in LIFO order.  */
       exec_ErrorHook(E_OS_NOFUNC);

       /* Return error.  */
       return(E_OS_NOFUNC);
   }

   /* Now to change preemption threshold to suit next resource with highest ceiling priority */
   /* But if the task has got NON scheduling policy then do nothing.  */

   tcb_ptr->external_resource_occuplied_list[location] = 0u;
   /* And remove this task's id from the res's control block.  */
   osek_res->taskid = 0u;
   tcb_ptr->res_ocp--;

   if ((tcb_ptr->policy == NON ) || (tcb_ptr->resource_scheduler == TX_TRUE))
   {
       /* In case of NON scheduling policy or if a Task holds a RES_SCHEDULER, no need to change preemption.  */
       /* Even if a resource is released the task is still remains in OSEK_NON_SCHEDULE_PRIORITY.  */
       /* just  remove this resource from the list.  */
       return (E_OK);
   }

   /* FULL scheduling policy and no RES_SCHEDULER held.  */

   /* Assume no other resources are held.  */
   new_prio = tcb_ptr->org_prio;

   if (tcb_ptr->res_ocp > 0u)
   {
       /* This task is holding more than one resources.  */
       /* Find out new highest ceiling priority.  */
       /* Assume task's original priority is the highest.  */
       for ( index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
       {

           /* Get this resource's ceiling priority.  */
           if ( tcb_ptr->external_resource_occuplied_list[index] == 0u) {
               break;
           }

           res_prio = ((OSEK_RESOURCE *) (tcb_ptr->external_resource_occuplied_list[index]) )->c_priority;
           /* Check if this is more than Task's static priority.  */
           if (new_prio < res_prio)
           {
                new_prio = res_prio;
           }
       }

   }

   /* Done with external resource, now check if there are any internal resources held?  */
   for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
   {
       if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
       {
           break;
       }

       res_prio = ( (OSEK_RESOURCE *) (tcb_ptr->internal_resource_occuplied_list[index]) )->c_priority;

       if (new_prio < res_prio )
       {
           new_prio = res_prio;
       }
   }

   /* new_prio now holds the highest of ceiling priority of the remaining
      resources held by this task.  */

   /* Now ask the system manager to change this task's priority, and also check if there is
      any possibility of preemption.  */
   /* Send message to system manager to check any preemption out of ReleaseResource call.  */

   /* Build the request.  */
   request[0] = SYSMGR_RELEASE_RESOURCE;          /* Request type.  */
   request[1] = (ULONG)this_thread;
   request[2] = (ULONG)new_prio;
   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority,      */
   /* this routine will be preempted by SysMgr supervisor thread when   */
   /* queue is successful.                                              */
   /* System manager will eventually call start_osek_tasks.             */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed.  */
   if (status != TX_SUCCESS)
   {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_SEND_CHAINTASK);
   }

   /* Assume that system manager does its job.  */
   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    RegisterTasktoResource                              PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This function registers a resource to the task specified, provided   */
/*   a free entry is available in the task's list of resources.           */
/*   If entry is registered then resource's ceiling priority is also      */
/*   adjusted to this task's priority provided that the task's priority   */
/*   is higher than the resource's current ceiling priority.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Resource                     Id of the resource.                    */
/*    TaskID                       ID of the task                         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    E_OS_NOFUNC                  No free entries                        */
/*    E_OS_ID                      Invalid task or resource id.           */
/*                                 not occupied by any task or ISR, or    */
/*                                 another resource has to be released    */
/*                                 before.                                */
/*   E_OS_ACCESS                   Attempt to release an invalid resource */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code  (System Creation section)                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType     RegisterTasktoResource(ResourceType Resource, TaskType TaskID)
{

OSEK_TCB       *tcb_ptr;
OSEK_RESOURCE  *resource_ptr;
UINT            index;

   /* Check if we are in initialization.  */
   /* This will ensure that no one calls this function after system is started.  */
   if (osek_init_state != OSEK_INIT)
   {
       /* Return OSEK internal error. This is not a standard OSEK error.  */
       Application->osek_object_creation_error++;
       return ((StatusType)TX_NULL);
   }

   /* Check for valid Resource ID.  */
   if(Resource == 0u)
   {
       /* Return error.  */
       Application->osek_object_creation_error++;
       return (E_OS_ID);
   }

   /* Check if task id is valid.  */
   if(TaskID == 0u)
   {
       /* Return error.  */
       return (E_OS_ID);
   }

   /* Convert object IDs to object control block pointers.  */
   resource_ptr = (OSEK_RESOURCE *)Resource;
   tcb_ptr = (OSEK_TCB *)TaskID;

    /* Check for valid resource pointer.  */
   if (resource_ptr == TX_NULL)
   {
       Application->osek_object_creation_error++;
       return (E_OS_ID);
   }

   if (resource_ptr->osek_res_id != OSEK_RES_ID)
   {
       Application->osek_object_creation_error++;
       return (E_OS_ID);
   }

   /* Check for valid task pointer.  */
   if (tcb_ptr == TX_NULL)
   {
       Application->osek_object_creation_error++;
       return (E_OS_ID);
   }

   if ((tcb_ptr->osek_task_id != OSEK_TASK_ID) && (tcb_ptr->osek_task_id != OSEK_ISR_ID))
   {
       Application->osek_object_creation_error++;
       return (E_OS_ID);
   }

   /* This task is to be registered with this resource.  */
   /* Add this Resource to the task's list of resource.  */
   /* provided there is space left for this.  */

   /* Before check whether the resource is internal or standard?  */
   if ((resource_ptr->type == STANDARD)|| (resource_ptr->type == LINKED))
   {

       for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
       {
           if (tcb_ptr->external_resource_list[index] == 0u)
           {
               /* Empty entry found, write this resource there.  */
               tcb_ptr->external_resource_list[index] = Resource;

               /* Also clear the resource occupancy count.  */
               tcb_ptr->external_resource_occuplied_list[index] = 0u;

               /* As this resource is attached to a TASK or ISR change this resource's ceiling priority
                  to task's (or ISR's) priority provided that  it is higher than resource's current ceiling priority.  */
               if (tcb_ptr->org_prio > resource_ptr->c_priority)
               {
                   resource_ptr->c_priority = tcb_ptr->org_prio;
               }

               /* Job done.  */
               break;
           } /* Try next entry.  */
       }

       if (index >= OSEK_MAX_EXTERNAL_RES) /* No free entry.  */
       {
           Application->osek_object_creation_error++;
           return (E_OS_NOFUNC);
       }
       else
       {
           return (E_OK);
       }
   } /* END STANDARD.  */
   else
   {
       /* Attaching Internal resource to this task, so set the flag.  */
       tcb_ptr->internal_res = TX_TRUE;

       for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
       {
           if (tcb_ptr->internal_resource_list[index] == 0u)
           {
               /* Empty entry found write this resource there.  */
               tcb_ptr->internal_resource_list[index] = Resource;

               /* As this resource is attached to the resource change this resource's ceiling priority
                  to this task's original priority provide it is higher than resource's current ceiling priority.  */
               if (tcb_ptr->org_prio > resource_ptr->c_priority)
               {
                   resource_ptr->c_priority =  tcb_ptr->org_prio;
               }

               /* Job done.  */
               break;
           } /* Try next entry.  */
       }

       if (index >= OSEK_MAX_INTERNAL_RES) /* No free entry.  */
       {
           Application->osek_object_creation_error++;
           return (E_OS_NOFUNC);
       }
       else
       {
           return (E_OK);
       }
   }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    RegisterISRtoResource                               PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This function registers a resource to the ISR specified, provided    */
/*   a free entry is available in the ISR's list of resources.            */
/*   If entry is registered then resource's ceiling priority is also      */
/*   adjusted to this task's priority provided that the task's priority   */
/*   is higher than the resource's current ceiling priority.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Resource                     Id of the resource.                    */
/*    TaskID                       ID of the task                         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    E_OS_NOFUNC                  No free entries                        */
/*    E_OS_ID                      Invalid task or resource id.           */
/*                                 not occupied by any task or ISR, or    */
/*                                 another resource has to be released    */
/*                                 before                                 */
/*   E_OS_ACCESS                   Attempt to release an invalid resource */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    RegisterTasktoResource       Register resource.                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType     RegisterISRtoResource(ResourceType Resource, ISRType ISRID)
{
StatusType     status;

   status = RegisterTasktoResource(Resource, ISRID);
   return (status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CreateEvent                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Creates an event. It is a 32 bit number with only one bit set.      */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    eventid                      If success                             */
/*    0                            If error occurs                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_get_event               Get the event                          */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   OSEK internal call                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
EventMaskType CreateEvent(void)
{
EventMaskType  event;

   event = osek_get_event();
   if (event == 0u)
   {
       /* Error will be returned whenever the ThreadX call fails.  */
       Application->osek_object_creation_error++;
       return (0u);
   }

   return (event);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SetEvent                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Sets events of the given task according to the event mask.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    task ID                      ID for the task                        */
/*    mask                         Mask of the events to be set.          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    E_OS_ID                      Invalid task or event id               */
/*    E_OS_STATE                   Invalid target task state              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set            Set ThreadX event flag.               */
/*    osek_internal_error           OSEK Internal error                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  SetEvent(TaskType task_id, EventMaskType mask)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
TX_THREAD      *this_thread;
EventMaskType   balance_events;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
UINT            status;
OSEK_TCB       *p_this_tcb;


   service_GetServiceId.id = (OsServiceIdType)OSServiceId_SetEvent;
   service_SetEvent.TaskID = task_id;

   /* Check if task id not valid.  */
   if(task_id == 0u)
   {
       exec_ErrorHook(E_OS_ID);

       /* Return error.  */
       return (E_OS_ID);
   }

   /* Get TCB_PTR.  */
   tcb_ptr = (OSEK_TCB *)task_id;

   if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
   {
       exec_ErrorHook(E_OS_ID);

       /* Return error.  */
       return (E_OS_ID);
   }

   /* Check if any event is assigned to task.  */
   if ((tcb_ptr->events == 0u) || (tcb_ptr->task_type == BASIC))
   {
       exec_ErrorHook(E_OS_ACCESS);

       /* No event assigned to task, return error.  */
       return (E_OS_ACCESS);
   }

   TX_DISABLE

   if (tcb_ptr->suspended == TX_TRUE)
   {
       TX_RESTORE

       exec_ErrorHook(E_OS_STATE);

       /* Return error.  */
       return (E_OS_STATE);
   }

   /* Check this event mask with the task .  */
   /* Update the set events for this task with this new mask.  */
   tcb_ptr->set_events = tcb_ptr->set_events | mask;

   /* See how many events are set.  */
   balance_events = tcb_ptr->set_events & tcb_ptr->waiting_events;

   /* Is there any from waiting events.  */
   if (balance_events != 0u)
   {
       /* Get calling task's ThreadX thread */
       this_thread = tx_thread_identify();

       /* Check whether the same running task is setting its own events or
          the task is already out of wait state.  */
       /* if yes then no need to add this task in the ready list and of course no
          need to check for any preemption possibilities.  */

       p_this_tcb = osek_thread2tcb(this_thread);
       if ((tcb_ptr != p_this_tcb) && (tcb_ptr->waiting == TX_TRUE))
       {
            tcb_ptr->waiting = TX_FALSE;
            /* Now, send message to the system manager to check any preemption out of SetEvent() call.  */

           /* Build the request. */
           request[0] = SYSMGR_SETEVENT;              /* Request type.  */
           request[1] = (ULONG)this_thread;           /* ThreadX Thread for this task.  */
           request[2] = (ULONG)tcb_ptr;               /* id in OSEK_TCB format.  */
           request[3] = 0u;

           /* Since the SysMgr supervisor thread has the highest priority,      */
           /* this routine will be preempted by SysMgr supervisor thread when   */
           /* queue is successful.                                              */
           /* System Manager will eventually call start_osek_tasks.             */

           status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

           /* This should always succeed.  */
           if (status != TX_SUCCESS)
           {
               /* System internal error.  */
               osek_internal_error(SYS_MGR_SEND_CHAINTASK);

           }
       }

   }

   TX_RESTORE

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ClearEvent                                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Clears events of the calling task according to the event mask.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mask                         Mask of the events to be cleared       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If success                             */
/*    ERROR                        If error occurs while creating task    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set            Set or clears ThreadX event flag      */
/*    osek_task_independent_area    See if out of task context            */
/*    tx_thread_identify            Identify the current thread           */
/*    tx_event_flags_set            Set the event Flags                   */
/*    osek_internal_error           OSEK internal error                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  ClearEvent(EventMaskType mask)
{
TX_INTERRUPT_SAVE_AREA
TX_THREAD               *this_thread;
OSEK_TCB                *tcb_ptr;
ULONG                    area;

    service_GetServiceId.id = (OsServiceIdType)OSServiceId_ClearEvent;
    service_ClearEvent.EventID = mask;
    /* Check if we are calling from initialization.  */
    if (osek_init_state == OSEK_INIT)
    {
        exec_ErrorHook(E_OS_CALLEVEL);

        /* Return error.  */
        return (E_OS_CALLEVEL);
    }

    /* Check if we are not at task level.  */
    area = osek_task_independent_area();
    if(area == 0u)
    {
        exec_ErrorHook(E_OS_CALLEVEL);

        /* Return error.  */
        return (E_OS_CALLEVEL);
    }

    this_thread = tx_thread_identify();

    /* Get OSEK TCB.  */
    tcb_ptr = (OSEK_TCB *)this_thread;

    /* Check if any event is assigned to task.  */
    if ((tcb_ptr->events == 0u) || (tcb_ptr->task_type == BASIC))
    {
        exec_ErrorHook(E_OS_ACCESS);

        /* No event assigned to task, return error.  */
        return (E_OS_ACCESS);
    }

    TX_DISABLE

    /* Update the events.  */
    tcb_ptr->set_events = tcb_ptr->set_events & (~mask);
    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetEvent                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Gets the current event setting of the given task.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    task_id                      ID or the task                         */
/*    mask                         Mask of the events to be set.          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    ERROR                        If an error occurs                     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  GetEvent(TaskType task_id, EventMaskRefType event)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB               *tcb_ptr;

    service_GetServiceId.id = (OsServiceIdType)OSServiceId_GetEvent;
    service_GetEvent.TaskID = task_id;

    /* Check if task id is valid.  */
    if(task_id == 0u)
    {
        exec_ErrorHook(E_OS_ID);

        /* Return error.  */
        return (E_OS_ID);
    }

    /* Get the tcb pointer.  */
    tcb_ptr = (OSEK_TCB *)task_id;

    if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
    {
        exec_ErrorHook(E_OS_ID);

        /* Return error.  */
        return (E_OS_ID);
    }

    /* Check if any event is assigned to task and task is extended type.  */
    if ((tcb_ptr->events == 0u) || (tcb_ptr->task_type == BASIC))
    {
        exec_ErrorHook(E_OS_ACCESS);

        /* No event assigned to task, return error.  */
        return (E_OS_ACCESS);
    }

    TX_DISABLE

    if (tcb_ptr->suspended == TX_TRUE)
    {
        TX_RESTORE

        exec_ErrorHook(E_OS_STATE);

        /* Return error.  */
        return (E_OS_STATE);
    }

    /* Get the event.  */
    *event = tcb_ptr->set_events;

    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    WaitEvent                                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Transfers the calling task into the waiting state until specified   */
/*    events are set.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mask                         Mask of the events to be set           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If success                             */
/*    ERROR                        If error occurs while creating task    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_get           Retrieves ThreadX event flag from      */
/*                                 specified ThreadX event flag group     */
/*    osek_internal_error          OSEK internal error                    */
/*    tx_thread_identify           Identify current thread                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  WaitEvent(EventMaskType mask)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB              *tcb_ptr;
TX_THREAD             *this_thread;
ULONG                  request[SYSMGR_QUEUE_MSG_LENGTH];
UINT                   index;
UINT                   status;


    service_GetServiceId.id = (OsServiceIdType)OSServiceId_WaitEvent;
    service_WaitEvent.EventID = mask;


    /* Check if we are in interrupt.  */
    if((_tx_thread_current_ptr == TX_NULL) ||           /* Not in a task.  */
       (_tx_thread_system_state != 0u))                 /* In an ISR.      */
    {
        exec_ErrorHook(E_OS_CALLEVEL);

        /* Return error.  */
        return (E_OS_CALLEVEL);
    }

    /* Check if the thread is currently executed.  */
    this_thread = tx_thread_identify();
    if(this_thread == TX_NULL)
    {
        exec_ErrorHook(E_OS_CALLEVEL);

        /* Not allowed on the calling thread.  */
        return (E_OS_CALLEVEL);
    }

    /* Get OSEK TCB.  */
    tcb_ptr = (OSEK_TCB *)this_thread;

    /* Check if any event is assigned to task or the task is of BASIC type.  */
    if ((tcb_ptr->events == 0u) || (tcb_ptr->task_type == BASIC))
    {
        exec_ErrorHook(E_OS_ACCESS);

        /* No event assigned to task, return error.  */
        return (E_OS_ACCESS);
    }

    /* Check if any resource is occupied by this task.  */
    if(tcb_ptr->res_ocp != 0u)
    {
        exec_ErrorHook(E_OS_RESOURCE);

        /* Return error.  */
        return (E_OS_RESOURCE);
    }

    TX_DISABLE

    tcb_ptr->waiting_events = mask;

    /* Is any event set?  */
    if ((tcb_ptr->set_events & tcb_ptr->waiting_events) != 0u)
    {
        TX_RESTORE

        return (E_OK);
    }
    else
    {
        /* Events are not set wait for them.  */
        tcb_ptr->waiting = TX_TRUE;
    }

    /* Release any internal resources held.  */
    if (tcb_ptr->internal_res != 0u)
    {
        for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
        {
            if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
            {
                break;
            }

            ((OSEK_RESOURCE *)(tcb_ptr->internal_resource_occuplied_list[index]))->taskid = 0u;
            tcb_ptr->internal_resource_occuplied_list[index] = 0u;

        }
    }

    /* Now, send message to system thread to check any preemption out of the WaitEvent() call.  */

    /* Build the request.  */
    request[0] = SYSMGR_WAITEVENT;               /* Request type.  */
    request[1] = (ULONG)this_thread;             /* Self ID.  */
    request[2] = (ULONG)tcb_ptr;                 /* Self Id in OSEK_TCB format.  */
    request[3] = 0u;
    /* Now send a message to the SysMgr supervisor thread and ask        */
    /* to check any preemption is possible.                              */
    /* Since the SysMgr supervisor thread has the highest priority,      */
    /* this routine will be preempted by SysMgr supervisor thread when   */
    /* queue is successful.                                              */
    /* System Manager will eventually call start_osek_tasks.             */

    status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

    TX_RESTORE

    /* This should always succeed.  */
    if (status != TX_SUCCESS)
    {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_SEND_CHAINTASK);
    }

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    RegisterEventtoTask                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This function registers an event to the task specified, provided     */
/*   a free entry is available in the task's list of events.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    event_id                     Id of the event                        */
/*    TaskID                       Id of the task                         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    E_OS_NOFUNC                  No free entries                        */
/*    E_OS_ID                      Invalid task or resource id.           */
/*                                 not occupied by any task or ISR, or    */
/*                                 another resource has to be released    */
/*                                 before.                                */
/*   E_OS_ACCESS                   Attempt to release an invalid resource */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code  (System Creation section)                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType    RegisterEventtoTask(EventMaskType eventid, TaskType TaskID)
{

OSEK_TCB      *tcb_ptr;

    /* Check if we are in initialization.  */
    /* This will ensure that no one calls this function after system is started.  */
    if (osek_init_state != OSEK_INIT)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;
        return ((StatusType)TX_NULL);
    }

    /* Check if task id is valid.  */
    if(TaskID == 0u)
    {

        /* Return error.  */
        Application->osek_object_creation_error++;

        return (E_OS_ID);
    }



    /* Convert object IDs to object control block pointers.  */
    tcb_ptr = (OSEK_TCB*)TaskID;


    /* Check for valid task pointer.  */
    if (tcb_ptr == TX_NULL)
    {
        Application->osek_object_creation_error++;

        return (E_OS_ID);
    }

    if(tcb_ptr->osek_task_id != OSEK_TASK_ID)
    {
        Application->osek_object_creation_error++;

        return (E_OS_ID);
    }

    /* This task is to be registered with this event.  */
    tcb_ptr->events = tcb_ptr->events | eventid;


    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    EnableInterrupt                                     PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service enables interrupts.                                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  EnableInterrupt(void)
{
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE

    if (disable_ISR2 != 0u) {
        disable_ISR2--;
    }

    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    DisableInterrupt                                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service disables interrupts.                                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  DisableInterrupt(void)
{
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE

    disable_ISR2++;

    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetInterruptDescriptor                              PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Query of interrupt status and returns the current CPU interrupt     */
/*    mask                                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mask                        A reference to the interrupt mask to be */
/*                                filled. In the mask, a "1" means (Group */
/*                                of) Interrupt is enabled.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If success                             */
/*    ERROR                        If error occurs while creating task    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_interrupt_control         Enables or disables interrupts         */
/*                                 specified by new_posture               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  GetInterruptDescriptor(UINT *mask)
{
UINT       old_posture;
UINT       new_posture;

    /* Get the old posture of the interrupt.  */
    old_posture = tx_interrupt_control(TX_INT_DISABLE);

    new_posture = old_posture;

    /* Invert the mask for OSEK interrupt.  */
    *mask = ~old_posture ;

    /* Restore the same interrupts.  */
    tx_interrupt_control(new_posture);

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SuspendAllInterrupts                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call serves to enter critical sections by disabling all         */
/*   interrupts.                                                          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   SuspendAllInterrupts (void)
{
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    suspend_ISR2++;
    TX_RESTORE
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ResumeAllInterrupts                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   Exits a critical section entered by calling SuspendAllInterrupts.    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area   See if called from task independent    */
/*                                 area                                   */
/*    tx_thread_identify           Identify the current thread            */
/*    osek_internal_error          Osek internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   ResumeAllInterrupts (void)
{
TX_INTERRUPT_SAVE_AREA
TX_THREAD      *thread_ptr;
UINT           status;
ULONG          request[SYSMGR_QUEUE_MSG_LENGTH];

    TX_DISABLE
   if (suspend_ISR2 != 0u)
   {
       suspend_ISR2--;
   }

   if (suspend_ISR2 == 0u)
   {
       /* Interrupts resume, check if any pending.  */
       /* get the pointer to the calling thread.  */
       thread_ptr = tx_thread_identify();

       /* Now send a message to the SysMgr supervisor thread to execute the error hook.  */
       /* Build the request.  */
       request[0] = SYSMGR_ERRORHOOK;           /* Request type.            */
       request[1] = (ULONG)thread_ptr;          /* Ptr of calling thread.   */
       request[2] = 0u;                         /* Input to Error hook.     */
       request[3] = 0u;

       /* Since the SysMgr supervisor thread is has the highest priority,  */
       /* this routine will be preempted by SysMgr supervisor thread when   */
       /* queue read is successful.                                         */

       status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

       /* This should always succeed.  */
       if (status != TX_SUCCESS)
       {
           TX_RESTORE

           /* System internal error.  */
           osek_internal_error(SYS_MGR_SEND_CHAINTASK);

           /* Return.  */
           return;
       }

   } /* end  if (!suspend_ISR2).  */

   TX_RESTORE

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    DisableAllInterrupts                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call serves to enter critical sections by disabling all         */
/*   interrupts.                                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   DisableAllInterrupts (void)
{
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    disable_ISR2++;
    TX_RESTORE
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    EnableAllInterrupts                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   Exits a critical section entered by calling DisableAllInterrupts.    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   EnableAllInterrupts (void)
{
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    if (disable_ISR2 != 0u)
    {
        disable_ISR2--;
    }
    TX_RESTORE
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SuspendOSInterrupts                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call serves to enter critical sections by disabling all         */
/*   interrupts.                                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   SuspendOSInterrupts (void)
{
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    suspend_ISR2++;
    TX_RESTORE
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ResumeOSInterrupts                                  PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   Exits a critical section entered by calling SuspendOSInterrupts.     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area   See if called from task independent    */
/*                                 area                                   */
/*    tx_thread_identify           Identify the current thread            */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   ResumeOSInterrupts (void)
{
TX_INTERRUPT_SAVE_AREA
TX_THREAD      *thread_ptr;
UINT           status;
ULONG          request[SYSMGR_QUEUE_MSG_LENGTH];

    TX_DISABLE

   if (suspend_ISR2 != 0u)
   {
       suspend_ISR2--;
   }

   if (suspend_ISR2 == 0u)
   {
       /* Interrupts resume, check if any pending.  */
       /* get the pointer to the calling thread.  */
       thread_ptr = tx_thread_identify();

       /* Now send a message to the SysMgr supervisor thread to execute the error hook  */
       /* Build the request.  */
       request[0] = SYSMGR_ERRORHOOK;           /* Request type.  */
       request[1] = (ULONG)thread_ptr;          /* Pointer of calling thread.  */
       request[2] = 0u;                         /* Input to Error hook.  */
       request[3] = 0u;

       /* Since the SysMgr supervisor thread is with the highest priority,  */
       /* this routine will be preempted by SysMgr supervisor thread when   */
       /* queue read is successful.                                         */

       status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

       /* This should always succeed.  */
       if (status != TX_SUCCESS)
       {
           TX_RESTORE

           /* System internal error.  */
           osek_internal_error(SYS_MGR_SEND_CHAINTASK);

           /* Return.  */
           return;
       }

   } /* end  if (!suspend_ISR2). */

   TX_RESTORE

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetActiveApplicationMode                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call returns the current application mode.                      */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Current application mode                                            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
AppModeType GetActiveApplicationMode(void)
{
    return (Application->application_mode);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetCounterValue                                     PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Returns the current value of a counter.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    counter_ptr                          Pointer to OSEK Counter        */
/*    tick_ptr                             Pointer to ticks               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                 If successful                  */
/*    E_OS_ID                              Invalid counter object         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   None                                                                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  GetCounterValue(OSEK_COUNTER *counter_ptr, TickRefType tick_ptr)
{

    /* Check for valid counter.  */
    if ((counter_ptr != TX_NULL) && (counter_ptr->osek_counter_id == OSEK_COUNTER_ID))
    {
        *tick_ptr = counter_ptr->counter_value;
    }
    else
    {
        /* There is error in counter object supplied.  */
        return (E_OS_ID);
    }

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CreateCounter                                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Creates a counter.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    N/A                                                                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    COUNTER_ID                   Reference to counter                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_memory_allocate         Allocate memory from system            */
/*    osek_counter_name_check      Check if counter name duplicate        */
/*    osek_get_counter             Get pointer to counter structure       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
CounterType  CreateCounter(const CHAR *name, TickType max_allowed_value, TickType ticks_per_base,
                       TickType min_cycle, TickType start_value)
{

CounterType      cntr_id;
OSEK_COUNTER     *cntr_ptr;
UINT             index;

    /* Check if we are in initialization.  */
    /* This will ensure that no one calls this function after system is started.  */
    if (osek_init_state != OSEK_INIT)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;

        return((TaskType)TX_NULL);
    }

    /* Check if there is space for this counter.  */
    cntr_id = osek_get_counter();
    if (cntr_id == 0u)
    {
        /* Return error.  */
        Application->osek_object_creation_error++;

        return ((CounterType) TX_NULL);
    }

    /* Get the pointer to counter structure.  */
    cntr_ptr = (OSEK_COUNTER *)cntr_id;


    if (max_allowed_value > MAXALLOWEDVALUE)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;

        return ((TaskType)TX_NULL);
    }

    /* Store maximum value of this counter.  */
    cntr_ptr->maxallowedvalue = max_allowed_value;

    if (min_cycle > MAXALLOWEDVALUE)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;

        return ((TaskType)TX_NULL);
    }

    /* Store minimum allowed number of ticks of the counter.  */
    cntr_ptr->mincycle = min_cycle;

    if (ticks_per_base > MAXALLOWEDVALUE)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;

        return ((TaskType)TX_NULL);
    }

    /* Store ticksperbase.  */
    cntr_ptr->ticksperbase = ticks_per_base;

    /* Store start up value.  */
    if (start_value > max_allowed_value)
    {
        cntr_ptr->counter_value = start_value;
    }
    else
    {
        cntr_ptr->counter_value = 0u;
    }

    /* Store object identifier.  */
    cntr_ptr->osek_counter_id = OSEK_COUNTER_ID;

    /* Now initialize the list of alarms attached to this counter.  */

    for (index = 0u; index < OSEK_MAX_ALARMS; index++)
    {
        cntr_ptr-> alarm_list[ index ] = 0u;
    }

    cntr_ptr->cntr_in_use = TX_TRUE;

    cntr_ptr->name = name;

    /* Return the created counter.  */
    return ((CounterType) cntr_ptr);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    IncrCouner                                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function increments the specified counter, checks its new      */
/*    value with all alarms attached to it and triggers expired alarms.   */
/*    If an alarm is expired then its actions are executed from here.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Counter                       Counter to update                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                          If successful                         */
/*    E_OS_ID                       If counter id is not correct          */
/*    E_OS_NOFUNC                   In case of any errors                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType     IncrCounter(CounterType cntr)
{

TX_INTERRUPT_SAVE_AREA

OSEK_COUNTER   *cntr_ptr;
UINT           index;
UINT           msg_cnt;
UINT           save_op_mode;
AlarmType      alarmid;
OSEK_ALARM     *this_alarm;
UINT           this_action;
UINT           alarm_fired;
UINT           status;
OSEK_TCB       *tcb_ptr;
ULONG          request[SYSMGR_QUEUE_MSG_LENGTH];
EventMaskType  balance_events;

cntr_ptr = (OSEK_COUNTER *)cntr;

    /* Check for valid counter.  */
    if ((cntr_ptr == TX_NULL) && (cntr_ptr->osek_counter_id != OSEK_COUNTER_ID))
    {
        /* There is error in counter object supplied.  */
        return (E_OS_ID);
    }

    /* Disable interrupt.  */
    TX_DISABLE

    if (cntr_ptr->cntr_in_use == TX_FALSE)
    {
        TX_RESTORE

        return (E_OS_ID);
    }

    if (cntr_ptr->ticksperbase == 0u)
    {
        TX_RESTORE

        return (E_OS_NOFUNC);
    }


    /* Increment counters sub count and check it with tick_pre-Base.  */
    cntr_ptr->sub_count++;
    if (cntr_ptr->sub_count < cntr_ptr->ticksperbase)
    {
        /* Still need to count enough to reach tics-per-base, no need to increment
           main count.  */
        TX_RESTORE

        return (E_OK);
    }

    /* Reached to tick-per-base, reset it and increment main count.  */
    cntr_ptr->sub_count = 0u;

    /* Now increment the counter value.  */
    if ( cntr_ptr->counter_value >= cntr_ptr->maxallowedvalue)
    {
        cntr_ptr->counter_value = 0u;
    }
    else
    {
        (cntr_ptr->counter_value)++;
    }

    /* Now check if counter is rolled back to zero.  */
    if ( cntr_ptr->counter_value == 0u)
    {
        /* Counter is rolled back set flags in all Alarms attached to this counter.  */
        for (index = 0u; index < OSEK_MAX_ALARMS; index++)
        {
            alarmid = cntr_ptr->alarm_list[index];
            if (alarmid == 0u)
            {
                continue;
            }

            /* Something is defined.  */
            this_alarm = (OSEK_ALARM *)alarmid;

            /* But is it a valid OSEK ALARM?  */
            if((this_alarm == TX_NULL) || (this_alarm->osek_alarm_id != OSEK_ALARM_ID))
            {
                continue;
            }

            /* Yes this is a valid alarm , but is it armed?  */
            if ( this_alarm->armed == TX_FALSE)
            {
                continue;
            }
            /* This is an ALARM and is armed, set its rollback flag.  */
            /* This flag will be cleared when an alarm is armed again or expired.  */
            this_alarm->counter_rollback = TX_TRUE;
        }
    }

    /* Reset a count of messages sent to the system manager.  */
    msg_cnt = 0u;

    /* Now it is time to check all alarms for expiration.  */
    for (index = 0u; index < OSEK_MAX_ALARMS; index++)
    {

        alarm_fired = TX_FALSE;

        /* Get the alarm id.  */
        alarmid = cntr_ptr->alarm_list[index];
        if (alarmid == 0u)
        {
            continue;                                           /* This alarm is not for me, go to check next alarm.  */
        }

        /* Something is defined, try to get the AlarmID.  */
        this_alarm = (OSEK_ALARM *)alarmid;

        /* But is it a valid OSEK ALARM?  */
        if((this_alarm == TX_NULL) || (this_alarm->osek_alarm_id != OSEK_ALARM_ID))
        {
            continue;
        }

        /* Yes this is a valid alarm , but is it armed?  */
        if (this_alarm->armed == TX_FALSE)                     /* This alarm is not armed, go to check next alarm.  */
        {
            continue;
        }

        /* Now check this with counter's current value, if it is less than it do not fire alarm.  */
        if (this_alarm->rel_abs_mode == RELATIVE_ALARM)
        {
            /* It is a REALTIVE ALARM.  */
            if (cntr_ptr->counter_value < this_alarm->expiration_count)
            {
                continue;                                                      /* Not yet expired.  */
            }
            else
            {
                /* Alarm expired, set flag.  */
                alarm_fired = TX_TRUE;
            }
        } /* End  It is a REALTIVE ALARM.  */
        else
        {
            /* It is ABSOLUTE ALARM.  */
            if (cntr_ptr->counter_value < this_alarm->expiration_count)
            {
                /* counter is less than expiration. */
                continue;                                                         /* Not yet expired.  */
            }

            if (cntr_ptr->counter_value == this_alarm->expiration_count)
            {
                /* If both are equal alarm is fired.  */
                alarm_fired = TX_TRUE;
            }
            else
            {
            /* count > expiration alarm to fire only if roll back occurred.  */
            /* This check is required, because when an Alarm is started in ABS mode, counter's current value could
               be more than the alarm's expiration count, so that may fire an Alarm the moment it is Armed,
               in fact, under such condition (counter's current value  more than
               the alarm's expiration count, at the time of arming the alarm) the alarm will expires only when , the
               counter counts to its max value then rolls back to zero then again counts up and now when it
               reaches the alarm's expiration count, the alarm gets fired.  */

                if (this_alarm->counter_rollback == TX_FALSE)
                {
                    continue;
                }
                else
                {
                    alarm_fired = TX_TRUE;
                }

            }

        }  /*   END:  else { ABSOLUTE ALARM ..  */


        /* STILL IN for  ( index = 0; index < OSEK_MAX_ALARMS; index++)  LOOP.  */

        /* Check alarm has fired?  */

        if (alarm_fired == TX_FALSE)
        {
            continue;
        }

        /* Reached here means an alarm has fired.  */
        /* Clear the counter roll back flag.  */
        this_alarm->counter_rollback = TX_FALSE;

        /* Dis arm the Alarm */
        this_alarm->armed = TX_FALSE;

        /* But is it cyclic?  */
        if (this_alarm->cycle != 0u)
        {
            /* Yes, then load new expiration count.  */
            this_alarm->expiration_count += this_alarm->cycle;
            /* Arm the alarm for next cycle.  */
            this_alarm->armed = TX_TRUE;
        }

        /* Get what action to be taken for this alarm.  */
        this_action = this_alarm->action;

        /* Check for action to be taken.  */
        switch(this_action)
        {
            case CALLBACK:

                /* Call a call-back function.  */
                if (this_alarm->alarm_callback != TX_NULL)
                {
                    save_op_mode  = osek_wrapper_operation_mode;
                    osek_wrapper_operation_mode = ALARM_CALLBACK_MODE;
                    (this_alarm->alarm_callback)();
                    osek_wrapper_operation_mode = save_op_mode;
                 }
                 break;

            case ACTIVATETASK:
                /* Activate a task.  */
                /* Get the OSEK TCB for the task to be activated.  */
                tcb_ptr = (OSEK_TCB *)(this_alarm->task);

                /* Check for any invalid thread pointer.  */
                if((tcb_ptr == TX_NULL) || ((tcb_ptr->task.tx_thread_id) != TX_THREAD_ID))
                {
                    break;
                }

                /* Check whether the task is activated up to its defined multiple activations.  */
                if(tcb_ptr->current_active >= tcb_ptr->max_active)
                {
                    break;
                }

               /* Now send a message to the System Manager thread to activate this task.  */
               /* Build the request.  */
               request[0] = SYSMGR_ACTIVATE_TASK;                /* Request type.  */
               request[1] = (ULONG)tcb_ptr;                      /* task to Activate.  */
               request[2] = last_run_task;                       /* task currently executing.  */
               request[3] = 0u;
               status = tx_queue_send (&osek_work_queue, request, TX_NO_WAIT);

               /* This should always succeed.  */
               if (status != TX_SUCCESS)
               {
                  /* System internal error.  */
                  osek_internal_error(SYS_MGR_SEND_ACTIVATETASK);
               }

               /* One message sent, increment the msg counter, this is required because
                  the System manager message queue can hold only certain number of messages.  */
               msg_cnt++;
               break;

            case SETEVENT:
                /* Set the specified events for the specified task.  */
                /* Check if task id not valid.  */
                if (this_alarm->task == TX_NULL)
                {
                    break;
                }

                /* Get TCB_PTR.  */
                tcb_ptr = (OSEK_TCB *) (this_alarm->task);

                if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
                {
                    break;
                }

                    /* Check if any event is assigned to task.  */
                    if ((tcb_ptr->events == 0u) || (tcb_ptr->task_type == BASIC))
                    {
                        break;
                    }

                    if (tcb_ptr->suspended == TX_TRUE)
                    {
                        break;
                    }

                    /* Check this event mask with the task.  */
                    /* Update the set events for this task with this new mask.  */
                    tcb_ptr->set_events = tcb_ptr->set_events | (this_alarm->events);

                    /* See if any events is set.  */
                    balance_events = tcb_ptr->set_events & tcb_ptr->waiting_events;

                    /* Is there any from waiting events.  */
                    if (balance_events != 0u)
                    {

                        if (tcb_ptr->waiting == TX_TRUE)
                        {
                            tcb_ptr->waiting = TX_FALSE;
                            /* Now, send message to the system manager to check any preemption out of SetEvent call. */

                            /* Build the request. */
                            request[0] = SYSMGR_SETEVENT;            /* Request type.  */
                            request[1] = last_run_task;              /* Task currently executing.  */
                            request[2] = (ULONG)tcb_ptr;             /* Task id for which events are set.  */
                            request[3] = 0u;
                            status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

                            /* This should always succeed.  */
                            if (status != TX_SUCCESS)
                            {
                                /* System internal error.  */
                                osek_internal_error(SYS_MGR_SEND_CHAINTASK);
                            }


                            /* One message sent, increment the msg counter, this required because
                               System manager message queue can hold only certain number of messages.  */
                            msg_cnt++;

                        }  /* End if (tcb_ptr->waiting == TX_TRUE).  */
                    }      /* End if (balance_events).  */
                    break;

            default:
           break;
        } /* switch ends.  */

        /* A situation may arise that multiple alarms expire at time (being set to same expiration count and mode)
           Which means timer has to send multiple alarm action messages to the system manager. But as the system manager accepts
           only certain number of messages at a time, we can't send more than that number of alarm action
           messages (SetEvent, ActivateTask) in one loop,
           so check if the system manager queue is filled up if yes stop checking for next alarm, we'll come again to check
           balance alarms.  */

        if (msg_cnt == (SYSMGR_QUEUE_MSG_COUNT))
        {
            break;
        }

    }  /* for ends.  */

    /* Enable interrupt.  */
    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    DefineSystemCounter                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This functions sets the system counter by assigning the specified   */
/*    counter as a system counter.                                        */
/*    If a counter is already defined as a system counter this call will  */
/*    return an error.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Counter                       Counter to assign as system counter   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                          If successful                         */
/*    E_OS_ID                       If counter id is invalid              */
/*    E_OS_NOFUNC                   Counter already assigned              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_timer_activate             starts the Thread timer to update     */
/*                                  this SystemCounter                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType DefineSystemCounter (CounterType cntr)
{

OSEK_COUNTER  *cntr_ptr;
OSEK_COUNTER  *this_counter;
UINT          index;
UINT          attached;

    /* Check if we are in initialization.  */
    /* This will ensure that no one calls this function after system is started.  */
    if (osek_init_state != OSEK_INIT)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;

        return ((TaskType)TX_NULL);
    }

    /* Convert counter id to counter pointer.  */

    cntr_ptr = (OSEK_COUNTER *)cntr;

    /* Check for valid counter.  */
    if ((cntr_ptr == TX_NULL) && (cntr_ptr->osek_counter_id != OSEK_COUNTER_ID))
    {
        /* There is error in counter object supplied.  */
        Application->osek_object_creation_error++;

        return (E_OS_ID);
    }

    if (cntr_ptr->cntr_in_use == TX_FALSE)
    {
        Application->osek_object_creation_error++;

        return (E_OS_ID);
    }

    /* Now check if there any a counter exists attached to system timer?  */
    attached = TX_FALSE;

    /* Search for a free counter.  */
    this_counter = osek_counter_pool;
    for (index = 0u; index < OSEK_MAX_COUNTERS; index++)
    {
        /* Is this guy is attached to system timer?  */
        if (this_counter->system_timer == TX_TRUE)
        {
            /* This counter is attached to system timer.  */
            attached = TX_TRUE;
            /* Stop searching.  */
            break;
        }

        this_counter++;
    } /* check all counters.  */

    /* No body attached to system timer?  */
    if (attached == TX_FALSE)
    {
        /* No one is attached, attach counter supplied to system timer.  */
        cntr_ptr->system_timer = TX_TRUE;

        /* Now start the system timer.  */
        tx_timer_activate(&osek_system_timer);

        return (E_OK);
    }

    /* One counter is already attached to system timer.  */
    Application->osek_object_creation_error++;

    return (E_OS_NOFUNC);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CreateAlarm                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Creates an alarm.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ticks                        Tick count                             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ALARM_ID                     Reference to alarm                     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_get_alarm               Get the alarm if possible              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
AlarmType  CreateAlarm(const CHAR *name, CounterType cntr, UINT action, ULONG events, TaskType task,
                     void (*callback)(), UINT Startup, TickType Alarmtime, TickType Cycle )
{

AlarmType        alarm_id;
OSEK_COUNTER     *cntr_ptr;
OSEK_TCB         *tcb_ptr;
OSEK_ALARM       *this_alarm;
UINT             index;
TickType         current_value;
StatusType       status;

    /* Check if we are in Initialization.  */
    /* This will ensure that no one calls this function after system is started.  */
    if (osek_init_state != OSEK_INIT)
    {
        /* Return OSEK internal error. This is not a standard OSEK error.  */
        Application->osek_object_creation_error++;

        return ((TaskType)TX_NULL);
    }

    /* Start with validating arguments passed.  */
    cntr_ptr = (OSEK_COUNTER*)cntr;
    if ((cntr_ptr == TX_NULL) || (cntr_ptr->osek_counter_id != OSEK_COUNTER_ID)|| (cntr_ptr->cntr_in_use == TX_FALSE))
    {
        /* Counter is not valid.  */
        Application->osek_object_creation_error++;
        return (E_OS_ID);
    }

    if (cntr_ptr->cntr_in_use != TX_TRUE)
    {
        /* Counter is not in use.  */
        Application->osek_object_creation_error++;

        return (E_OS_ID);
    }

    tcb_ptr = (OSEK_TCB *)task;

    /* Check the action to be taken and parameters passed.  */
    switch(action)
    {
        case SETEVENT:

            /* For SetEvents there should not be call back function.  */
            if((events == 0u) || (task == 0u) || (callback != TX_NULL))
            {
                /* Return error.  */
                Application->osek_object_creation_error++;

                return (E_OS_NOFUNC);
            }

            if ((tcb_ptr == TX_NULL) || (tcb_ptr->osek_task_id != OSEK_TASK_ID))
            {
                /* Task is not valid.  */
                Application->osek_object_creation_error++;

                return (E_OS_ID);
            }

            if (tcb_ptr->tcb_in_use != TX_TRUE)
            {
                /* Task is not in use */
                Application->osek_object_creation_error++;
                return (E_OS_ID);
            }

            break;

        case ACTIVATETASK:
            /* For activate task there should not be any call-back function or any event.  */
            if((task == 0u) || (events != 0u) || (callback != TX_NULL))
            {
                /* Return error.  */
                Application->osek_object_creation_error++;

                return (E_OS_NOFUNC);
            }

            if ((tcb_ptr == TX_NULL) || (tcb_ptr->osek_task_id != OSEK_TASK_ID))
            {
                /* Task is not valid.  */
                Application->osek_object_creation_error++;
                return (E_OS_ID);
            }

            if (tcb_ptr->tcb_in_use != TX_TRUE)
            {
                /* Task is not in use.  */
                Application->osek_object_creation_error++;
                return (E_OS_ID);
            }

            break;

        case CALLBACK:
            /* For ALARMCALLBACK there should not be task or any event.  */
            if((callback == TX_NULL) || (events != 0u) || (task != 0u))
            {
                /* Return error.  */
                Application->osek_object_creation_error++;

                return (E_OS_NOFUNC);
            }

            break;

        default:
            /* Return error.  */
            Application->osek_object_creation_error++;

            return (E_OS_NOFUNC);

            break;
    }

    /* Check if there is space for this alarm.  */
    alarm_id = osek_get_alarm();
    if (alarm_id == 0u)
    {
        /* Return error.  */
        Application->osek_object_creation_error++;

        return (E_OS_NOFUNC);
    }

    /* Got correct inputs, get pointer to alarm.  */
    this_alarm = (OSEK_ALARM *)alarm_id;

    /* Store object id.  */
    this_alarm->osek_alarm_id = OSEK_ALARM_ID;

    /* Before attaching this alarm to a counter, disarm it.  */
    this_alarm->armed = TX_FALSE;

    this_alarm->name = name;

    /* Now, try attaching this alarm to the counter specified.  */
    for (index = 0u; index < OSEK_MAX_ALARMS; index++)
    {
        /* Find a free entry in the list maintained by the counter.  */
        if (cntr_ptr->alarm_list[index] == 0u)
        {
            /* Found one.  */
            cntr_ptr->alarm_list[index] = alarm_id;

            break;
        }
    }

    /* Searched the entire list and came empty handed?  */
    if (index >= OSEK_MAX_ALARMS)
    {
        /* Return error.  */
        osek_reset_alarm(this_alarm);

        Application->osek_object_creation_error++;

        return (E_OS_NOFUNC);
    }

    /* Got the counter, now save the counter in to ALARM structure.  */
    this_alarm->cntr = cntr_ptr;

    /*  Maximum allowed value and minimum cycles for this alarms this values are from */
    /*  the counter to which this alarm is attached.  */
    this_alarm->max_allowed_value = cntr_ptr->maxallowedvalue;
    this_alarm->min_cyc = cntr_ptr->mincycle;
    this_alarm->ticks_per_base = cntr_ptr->ticksperbase;

    this_alarm->name = name;

    /* Store the ACTION upon alarm expiration.  */
    this_alarm->action = action;

    /* Store event in case of SETEVENT action.  */
    this_alarm->events = events;

    /* Check for call back and store it.  */
    if(callback != TX_NULL)
    {
        this_alarm->alarm_callback = callback;
    }

    if (Startup != 0u)
    {
        this_alarm->auto_start = TX_TRUE;
    }

    /* Save the TASK in to ALARM structure.  */
    this_alarm->task = tcb_ptr;

    /* Everything is OK so far, now fill this alarm control block with some default parameters.  */

    /* Reset max cycles, current cycles and expiration count.  */
    this_alarm->expiration_count = 0u;
    this_alarm->cycle = 0u;

    /* Now it is time to check for AUTOSTART.  */
    if (this_alarm->auto_start != 0u)
    {
        /* Check start.  */
        if ((Alarmtime == 0u) || (Alarmtime > this_alarm->max_allowed_value))
        {
            /* Invalid input parameter.  */
            Application->osek_object_creation_error++;

            return (E_OS_VALUE);
        }

        /* Check cycle. */
        if (Cycle != 0u)
        {
            if ((Cycle < this_alarm->min_cyc) || (Cycle > this_alarm->max_allowed_value))
            {
                /* Invalid input parameter.  */
                Application->osek_object_creation_error++;

                return (E_OS_VALUE);
            }
        }

        /* All input parameters are ok, store relative value and cycle.  */
        this_alarm->cycle = Cycle;

        status = GetCounterValue(cntr_ptr, &current_value);
        if (status != E_OK)
        {
            Application->osek_object_creation_error++;

            return (E_OS_VALUE);  /* Some problem in getting counter's current value.  */
        }

        this_alarm->expiration_count = current_value + Alarmtime; /* Load the counter with its expiration count.  */

        /* Now set the relative mode.  */
        this_alarm->rel_abs_mode = RELATIVE_ALARM;
        this_alarm->counter_rollback = TX_FALSE;
        this_alarm->armed = TX_TRUE;
        this_alarm->occupied = TX_TRUE;

    }  /* End..   this_alarm->auto_start.  */

    return(alarm_id);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetAlarmBase                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Reads the alarm base characteristics. Returns info which is a       */
/*    in which the information of the data type AlarmbaseType is          */
/*    stored.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    AlarmID                       Reference to alarm                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Info                          user_defined parameters of the        */
/*                                  counter associated with alarm.        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  GetAlarmBase(AlarmType AlarmID, AlarmBaseRefType info)
{

OSEK_ALARM          *alarm_ptr;


    service_GetServiceId.id = (OsServiceIdType)OSServiceId_GetAlarmBase;
    service_GetAlarmBase.AlarmID = AlarmID;

    /* Get the OSEK alarm.  */
    alarm_ptr = (OSEK_ALARM *) AlarmID;

    /* Check valid ID.  */
    if((alarm_ptr == 0u) || (alarm_ptr->osek_alarm_id != OSEK_ALARM_ID))
    {
        exec_ErrorHook (E_OS_ID);

        /* Invalid AlarmID.  */
        return (E_OS_ID);
    }

    info->maxallowedvalue = alarm_ptr->max_allowed_value;
    info->mincycle = alarm_ptr->min_cyc;
    info->ticksperbase = alarm_ptr->ticks_per_base;

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SetAbsAlarm                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Sets an alarm to an absolute cycle value.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    AlarmID                      Reference to alarm.                    */
/*    start                        Absolute value in ticks.               */
/*    cycle                        Cycle value in case of cyclic alarm.   */
/*                                 In case of single alarm it must be     */
/*                                 zero.                                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    ERROR                        If error occurs while creating task    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_internal_error          OSEK internal error                    */
/*    tx_time_get                  Get system time                        */
/*    tx_timer_create              Create the timer is created            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType SetAbsAlarm(AlarmType  AlarmID, TickType start, TickType cycle)
{
    TX_INTERRUPT_SAVE_AREA
    OSEK_ALARM          *osek_alarm;


    service_GetServiceId.id = (OsServiceIdType)OSServiceId_SetAbsAlarm;
    service_SetAbsAlarm.AlarmID = AlarmID;
    service_SetAbsAlarm.start = start;
    service_SetAbsAlarm.cycle = cycle;

    /* Convert AlarmID to OSEK_ALARM.  */
    osek_alarm = (OSEK_ALARM *) AlarmID;

    /* Check valid ID.  */
    if((osek_alarm == 0u) || (osek_alarm->osek_alarm_id != OSEK_ALARM_ID))
    {
        exec_ErrorHook (E_OS_ID);

        /* Invalid AlarmID.  */
        return (E_OS_ID);
    }

    TX_DISABLE

    /* Check already armed?  */
    if (osek_alarm->armed == TX_TRUE)
    {
        TX_RESTORE

        exec_ErrorHook(E_OS_STATE);

        return (E_OS_STATE);
    }

    /* Check start.  */
    if ((start == 0u) || ( start > osek_alarm->max_allowed_value))
    {
        TX_RESTORE

        /* Invalid input parameter.  */
        exec_ErrorHook(E_OS_VALUE);

        return (E_OS_VALUE);
    }


    /* Check cycle.  */
    if (cycle != 0u)
    {
        if ((cycle < osek_alarm->min_cyc) || ( cycle > osek_alarm->max_allowed_value))
        {
            TX_RESTORE

            exec_ErrorHook(E_OS_VALUE);

            /* Invalid input parameter.  */
            return (E_OS_VALUE);
        }
    }

    /* All input parameters are ok, store abs value and cycle.  */
    osek_alarm->cycle = cycle;
    osek_alarm->expiration_count = start; /* Load the counter with its expiration count.  */

    /* Now set the absolute mode.  */
    osek_alarm->rel_abs_mode = ABSOLUTE_ALARM;

    /* Now, arm the alarm.  */
    osek_alarm->armed = TX_TRUE;
    osek_alarm->occupied = TX_TRUE;
    osek_alarm->counter_rollback = TX_FALSE;

    TX_RESTORE

   /* Return OK.  */
    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SetRelAlarm                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Sets an alarm to a relative cycle value from the current time.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    AlarmID                      Reference to alarm.                    */
/*    increment                    Relative value in ticks                */
/*    cycle                        Cycle value in case of cyclic alarm    */
/*                                 In case of single alarm it must be     */
/*                                 zero.                                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    ERROR                        If an error occurs                     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_internal_error          OSEK internal error                    */
/*    tx_timer_change              Change the timer                       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  SetRelAlarm(AlarmType AlarmID, TickType increment, TickType cycle)
{
TX_INTERRUPT_SAVE_AREA
OSEK_ALARM          *osek_alarm;
OSEK_COUNTER        *counter_ptr;
TickType            current_value;
StatusType          status;


    service_GetServiceId.id = (OsServiceIdType)OSServiceId_SetRelAlarm;
    service_SetRelAlarm.AlarmID = AlarmID;
    service_SetRelAlarm.increment = increment;
    service_SetRelAlarm.cycle = cycle;

    /* Convert AlarmID to OSEK_ALARM.  */
    osek_alarm = (OSEK_ALARM *)AlarmID;

    /* Check valid ID.  */
    if((osek_alarm == TX_NULL) || (osek_alarm->osek_alarm_id != OSEK_ALARM_ID))
    {
        exec_ErrorHook(E_OS_ID);

        /* Invalid AlarmID.  */
        return (E_OS_ID);
    }

    TX_DISABLE

    /* Check already armed?  */
    if (osek_alarm->armed == TX_TRUE)
    {
        TX_RESTORE

        exec_ErrorHook(E_OS_STATE);

        return (E_OS_STATE);
    }

    /* Check start.  */
    if ((increment == 0u) || (increment > osek_alarm->max_allowed_value))
    {
        TX_RESTORE

        exec_ErrorHook(E_OS_VALUE);

        /* Invalid input parameter.  */
        return (E_OS_VALUE);
    }

    /* Check cycle.  */
    if (cycle != 0u)
    {
       if ((cycle < osek_alarm->min_cyc) || (cycle > osek_alarm->max_allowed_value))
        {
           TX_RESTORE

            exec_ErrorHook(E_OS_VALUE);

            /* Invalid input parameter.  */
            return (E_OS_VALUE);
        }
    }

    /* All input parameters are ok, store rel value and cycle.  */
    osek_alarm->cycle = cycle;
    counter_ptr = osek_alarm->cntr;

    status = GetCounterValue(counter_ptr, &current_value);
    if (status != E_OK)
    {
        TX_RESTORE
        return (E_OS_STATE);
    }

    osek_alarm->expiration_count = current_value + increment; /* Load the counter with its expiration count.  */

    /* Now set the relative mode.  */
    osek_alarm->rel_abs_mode = RELATIVE_ALARM;
    osek_alarm->counter_rollback = TX_FALSE;

    /* Now, arm the alarm.  */
    osek_alarm->armed = TX_TRUE;
    osek_alarm->occupied = TX_TRUE;

    TX_RESTORE

    /* Return OK.  */
    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CancelAlarm                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Cancels the alarm: the alarm transition into the stop state.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    AlarmID                      Reference to alarm                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If successful                          */
/*    ERROR                        If an error occurs                     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_timer_deactivate         Timer deactivate                        */
/*    osek_internal_error         OSEK internal error                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  CancelAlarm(AlarmType  AlarmID)
{
TX_INTERRUPT_SAVE_AREA
    OSEK_ALARM          *osek_alarm;

    service_GetServiceId.id = (OsServiceIdType)OSServiceId_CancelAlarm;
    service_CancelAlarm.AlarmID = AlarmID;

    /* Check valid ID and absolute ticks.  */
    osek_alarm = (OSEK_ALARM *) AlarmID;
    if((osek_alarm == TX_NULL) || (osek_alarm->osek_alarm_id != OSEK_ALARM_ID))
    {
        exec_ErrorHook(E_OS_ID);

        /* Invalid AlarmID.  */
        return (E_OS_ID);
    }

    TX_DISABLE

    /* Is this alarm in use?  */
    if ((osek_alarm->armed == TX_FALSE) || (osek_alarm->occupied == TX_FALSE))
    {
        TX_RESTORE

        exec_ErrorHook(E_OS_NOFUNC);

        return (E_OS_NOFUNC);
    }

    /* Alarm is in use so make it inactive.  */
    osek_alarm->armed = TX_FALSE;

    TX_RESTORE

    /* Return OK.  */
    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    GetAlarm                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Returns the no. of ticks before the alarm expires.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ticks                        Pointer to the returned tick count     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    N/A                                                                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_timer_info_get            Retrieves information about specified  */
/*                                 timer.                                 */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
StatusType  GetAlarm(AlarmType AlarmID, TickRefType tick_ptr)
{
TX_INTERRUPT_SAVE_AREA
OSEK_ALARM          *osek_alarm;
OSEK_COUNTER        *counter_ptr;
TickType             current_ticks;
StatusType           status;


    service_GetServiceId.id = (OsServiceIdType)OSServiceId_GetAlarm;
    service_GetAlarm.AlarmID = AlarmID;

    /* Check valid ID and absolute ticks.  */
    osek_alarm = (OSEK_ALARM *) AlarmID;
    if((osek_alarm == TX_NULL) || (osek_alarm->osek_alarm_id != OSEK_ALARM_ID))
    {
        exec_ErrorHook (E_OS_ID);

        /* Invalid AlarmID.  */
        return (E_OS_ID);
    }

    TX_DISABLE

    if ((osek_alarm->armed == FALSE) || (osek_alarm->occupied == TX_FALSE))
    {
        TX_RESTORE

        exec_ErrorHook(E_OS_NOFUNC);

        return (E_OS_NOFUNC);      /* Alarm expired or not in use.  */
    }

    counter_ptr = osek_alarm->cntr;

    status = GetCounterValue(counter_ptr, &current_ticks);
    if (status != E_OK)
    {
        TX_RESTORE

        exec_ErrorHook (E_OS_ID);
        return (E_OS_ID);
    }

    if (osek_alarm->expiration_count < current_ticks)
    {
        /* Alarm's expiration count is less than current counter value means */
        /* alarm will expire after the counter rolls back and reach alarm's expiration count.  */
        *tick_ptr = (osek_alarm->max_allowed_value - current_ticks + osek_alarm->expiration_count);
    }
    else
    {
        /* Alarm's expiration is more than counter's current value. */
        *tick_ptr = (osek_alarm->max_allowed_value - current_ticks);
    }

    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_initialize                                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up, configures and initializes all the           */
/*    "behind-the-scenes" data structures, tables, memory regions, etc.   */
/*    used by the OSEK at run-time.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    osek_memory                            OSEK pointer                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_create                      Create system manager thread  */
/*    tx_timer_create                       Create a system timer         */
/*    osek_memory_init                      Initialize OSEK memory        */
/*    tx_queue_create                       Create system manager queue   */
/*    osek_tcb_init                         Initialize OSEK tasks         */
/*    osek_resource_init                    Initialize OSEK resources     */
/*    osek_alarm_init                       Initialize OSEK alarms        */
/*    osek_counter_init                     Initialize OSEK counter       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/

UCHAR *osek_initialize(void *osek_memory, APPLICATION_INFO_PTR application1)
{

UCHAR       *pointer;
UINT         status;
UINT         i;
UINT         j;

    /* Initialization started, set init state.  */
    osek_init_state = OSEK_INIT;

    Application = application1;
    Application->osek_object_creation_error = 0u;
    /* Setup temporary memory pointer so we can start allocating
       space for the OSEK data structures.  The important thing to
       remember here is that the system threads stack, the region0
       memory, and the queue are allocated sequentially from the
       address specified by OSEK_memory.  */

    pointer =  (UCHAR *)osek_memory;

    /* Create the work item message queue.  */
    status = tx_queue_create(&osek_work_queue, "OSEK work queue", SYSMGR_QUEUE_MSG_LENGTH,
                                pointer, SYSMGR_QUEUE_DEPTH * SYSMGR_QUEUE_MSG_LENGTH);
    if(status != TX_SUCCESS)
    {
        Application->osek_object_creation_error++;
    }

    pointer = pointer + (SYSMGR_QUEUE_DEPTH * SYSMGR_QUEUE_MSG_LENGTH);

    /* Create the system manager thread.  */
    status = tx_thread_create(&osek_system_manager, "OSEK System Manager", osek_system_manager_entry,
                                 0, pointer, OSEK_SYSTEM_STACK_SIZE, SYSMGR_PRIORITY, SYSMGR_THRESHOLD,
                                 TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        /* Error creating the system manager thread.  */
        Application->osek_object_creation_error++;
    }

    pointer =  pointer + OSEK_SYSTEM_STACK_SIZE;

    /* Set up a memory "heap" or TX_REGION0_SIZE_IN_BYTES used internally
       by the osek.  */

    status = osek_memory_init(pointer);

    if (status != TX_SUCCESS)
    {
        Application->osek_object_creation_error++;
    }

    pointer =  pointer + TX_REGION0_SIZE_IN_BYTES;


    /* Create the system timer.  */
    status = tx_timer_create( &osek_system_timer, "OSEK SYSTEM TIMER", osek_system_timer_entry,
                           0,5,5,TX_NO_ACTIVATE);

    if (status != TX_SUCCESS)
    {
        Application->osek_object_creation_error++;
    }

    /* Check whether any error occurred during system object creation.  */
    if (Application->osek_object_creation_error != 0u)
    {
        osek_internal_error(THREADX_OBJECT_CREATION_ERROR);
    }

    /* Initialize static pool of task control blocks.  */
    osek_tcb_init();

    /* Set up a pool of resource used internally by the OSEK layer.  */
    osek_resource_init();

    /* Set up a pool of counter used internally by the OSEK layer.  */
    osek_counter_init();

    /* Set up a pool of alarm used internally by the OSEK layer.  */
    osek_alarm_init();

    /* Clear all events.  */
    global_events = 0u;
    global_event_count = 0u;

     /* Clear task table.  */

    for (i = 0u; i < (OSEK_ISR1_PRIORITY + 1u); i++)
    {
        for (j = 0u; j < TASK_QUEUE_DEPTH; j++)
        {
            task_table[i][j] = 0u;
        }
    }

    /* Change OSEK mode.  */
    osek_wrapper_operation_mode = INITSYSTEM_MODE;

    system_start = TX_TRUE;

    /* All done.  */
    return (pointer);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_cleanup                                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Cleans up after an OS shutdown. Used for testing.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    application1                          Application pointer.          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                            If successful.                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/

UINT osek_cleanup(APPLICATION_INFO_PTR application1)
{

UINT status;
UINT i;
OSEK_TCB *p_tcb;

    /* Delete the system manager thread.  */
    status = tx_thread_delete(&osek_system_manager);
    if (status != TX_SUCCESS)
    {
        /* System internal error.  */
        osek_internal_error(SYS_MGR_START_OS);
    }

    /* Delete the system manager work queue.  */
    status = tx_queue_delete(&osek_work_queue);
    if (status != TX_SUCCESS)
    {
        /* System internal error.  */
        osek_internal_error(SYS_MGR_START_OS);
    }

    /* Delete the system timer. */
    status = tx_timer_delete(&osek_system_timer);
    if (status != TX_SUCCESS)
    {
        /* System internal error.  */
        osek_internal_error(SYS_MGR_START_OS);
    }

    /* Delete the memory pool used for OSEK objects.  */
    status = tx_byte_pool_delete(&osek_region0_byte_pool);
    if (status != TX_SUCCESS)
    {
        /* System internal error.  */
        osek_internal_error(SYS_MGR_START_OS);
    }

    /* Terminate and delete all the threads.  */
    for(i = 0u; i < OSEK_MAX_TASKS; i++) {
        p_tcb = &osek_tcb_pool[i];
        if(p_tcb->task.tx_thread_entry != NULL) {
            status = tx_thread_terminate(&p_tcb->task);
            if (status != TX_SUCCESS)
            {
                /* System internal error.  */
                osek_internal_error(SYS_MGR_START_OS);
            }

            status = tx_thread_delete(&p_tcb->task);
            if (status != TX_SUCCESS)
            {
                /* System internal error.  */
                osek_internal_error(SYS_MGR_START_OS);
            }

            p_tcb->task.tx_thread_entry = NULL;
        }
    }

    /* Clear the memory pool to prevent errors on restart.  */
    memset(osek_tcb_pool, 0u, sizeof(osek_tcb_pool));

    /* OSEK cleaned up, change init state to not initialized.  */
    osek_init_state = OSEK_NOT_INIT;

    return TX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    process_ISR2                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    isrname                      ISR to process.                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    ActivateISR                  Puts ISR in the queue area             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Wrapper Internal code (Not available for Application)                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   process_ISR2(ISRType isrname)
{

   if (disable_ISR2 != 0u)
   {
       /* ISR2 disabled, do nothing.  */
       return;
   }

   ActivateISR(isrname);

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ShutdownOS                                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function shuts down the OS. If ShutdownHook is defined it is   */
/*    called.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    StatusType                     Returned error to the hook           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    ErrorHook                      If defined                           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application or OS                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void   ShutdownOS(StatusType error)
{
TX_INTERRUPT_SAVE_AREA
ULONG       request[SYSMGR_QUEUE_MSG_LENGTH];
UINT        status;

    TX_DISABLE

    /* Disable ISRs.  */
    disable_ISR2 = TX_TRUE;

   /* Check for any shutdown hook routine.  */
   if (Application->shutdown_hook_handler != TX_NULL)
   {
       /* Change operation mode for to shutdownhook mode. */
       osek_wrapper_operation_mode = SHUTDOWNHOOK_MODE;

       (Application->shutdown_hook_handler)(error);

   }

   /* Change to default operations mode.  */
   osek_wrapper_operation_mode = NORMAL_EXECUTION_MODE;

   /* Now send message to system thread to shut down the system.  */
   /* Build the request.  */
   request[0] = SYSMGR_SHUTDOWN_OS;         /* Request type.  */
   request[1] = 0u;                         /* Dummy.  */
   request[2] = 0u;                         /* Dummy.  */
   request[3] = 0u;                         /* Dummy.  */

   /* Now send a message to the SysMgr supervisor thread.  */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed.  */
   if (status != TX_SUCCESS)
   {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_START_OS);
   }

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    ActivateISR                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This call activates a task which eventually executes an ISR code    */
/*    This call puts the task in the ready queue.                         */
/*    The operating system ensures that the task code is being executed   */
/*    from the first statement. Rescheduling after this call depends on   */
/*    whether required resources for this ISR are free or not and no      */
/*    other higher priority are running.                                  */
/*    If E_OS_LIMIT is returned then activation is ignored as there is    */
/*    one request of same ISR is already pending.                         */
/*                                                                        */
/*    ** NOTE**                                                           */
/*    This is not a standard OSEK API call.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ISRId                                 ISR Name                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                  If successful                 */
/*    E_OS_ID                               Invalid ISR id                */
/*    E_OS_LIMIT                            One ISR already pending       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_queue_send                         Send message to Sys Manager   */
/*    osek_internal_error                   OSEK internal error           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK Wrapper Internal code (not available for Applications)         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType ActivateISR(ISRType ISRID)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
UINT            status;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];

   /* Get the OSEK TCB for the ISR to be activated.  */
   tcb_ptr = (OSEK_TCB *)ISRID;

   /* Check for a valid ISR id.  */
   if((tcb_ptr == TX_NULL) || (tcb_ptr->osek_task_id != OSEK_ISR_ID))
   {
       /* Return Error.  */
       return (E_OS_ID);
   }

   TX_DISABLE

   /* Check whether an ISR of same name is pending.  */
   if(tcb_ptr->current_active >= tcb_ptr->max_active)
   {
       TX_RESTORE
       /* Reached its max activation limit.  */
       return (E_OS_LIMIT);
   }

   /* Now send a message to the system manager to activate this ISR.  */
   /* Build the request.  */

   request[0] = SYSMGR_ACTIVATE_TASK;          /* Request type.                */
   request[1] = (ULONG)tcb_ptr;                /* ISR to Activate,             */
   request[2] = last_run_task;                 /* Running task if any.         */
   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority, this call   */
   /* will be preempted by SysMgr supervisor thread.                           */
   /* SysMgr will eventually call osek_do_task_activate().                     */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed.  */
   if (status != TX_SUCCESS)
   {
       /* System internal error.  */
       osek_internal_error(SYS_MGR_SEND_ACTIVATETASK);
   }

   /* Return status.  */
   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    TerminateISR                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This service terminates the calling ISR.                             */
/*   Only internal resources held by this ISR are released here. While    */
/*   it is assumed that any external resources occupied by the ISR must   */
/*   have been released before the call to TerminateISR. In case          */
/*   a resource is still occupied while calling this service, then the    */
/*   behaviour is undefined in STANDARD version of OSEK. In the EXTENDED  */
/*   version of OSEK, this service returns an error, which can be         */
/*   evaluated by the application.                                        */
/*   If successful, this call will causes rescheduling, this also means   */
/*   that upon success TerminateTask does not return to the call level.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                            If success                          */
/*    Error Code.                     If error                            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area      Check if we are in task context     */
/*    osek_thread2tcb                 Get TCB pointer for thread pointer  */
/*    tx_queue_send                   Send message to sys manager thread  */
/*    osek_internal_error             In case of any internal error       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK Wrapper Internal code (Not available for Applications)         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType  TerminateISR(void)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB       *tcb_ptr;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];
UINT            index;
UINT            status;
ULONG           area;
TX_THREAD      *p_thread;


   /* Check for task or ISR context */
   /* All ISR are treated as as a high priority tasks.  */
   area = osek_task_independent_area();
   if(area != TX_TRUE)
   {
       /* Return error.  */
       return (E_OS_CALLEVEL);
   }

   /* Get OSEK TCB of this TASK/ISR  */
   p_thread = tx_thread_identify();
   tcb_ptr = osek_thread2tcb(p_thread);
   if (tcb_ptr == NULL)
   {
       return (E_OS_CALLEVEL);
   }

   if ((tcb_ptr->osek_task_id != OSEK_ISR_ID))
   {
       /* This call is allowed only from ISR.  */
       return (E_OS_CALLEVEL);
   }


   /* Check operating mode.  */
   if ((osek_wrapper_operation_mode != ISR1_MODE) && (osek_wrapper_operation_mode != ISR2_MODE))
   {
       /* Hook toutines and alarm callbacks can't call this service.  */
       /* This explicit check is required because all hook routines are
      executed in task's context.  */
       return (E_OS_CALLEVEL);
   }

   TX_DISABLE

   /* Check if any resource is occupied. A task can not be terminated if it is holding any resource.  */
   if(tcb_ptr->res_ocp != 0u)
   {
       TX_RESTORE

      /* Return.  */
      return (E_OS_RESOURCE);
   }

   /* Release any internal resources held.  */
   if (tcb_ptr->internal_res != 0u)
   {
       for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
       {
           if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
           {
               break;
           }

           ((OSEK_RESOURCE *)(tcb_ptr->internal_resource_occuplied_list[index]))->taskid = 0u;
           tcb_ptr->internal_resource_occuplied_list[index] = 0u;

       }   /* End of for loop.  */
   }

   /* Now all set to terminate this task.  */
   /* Send a message to the System Manager to terminate this task.  */
   /* Build the request.  */

   request[0] = SYSMGR_TERMINATE_TASK;      /* Request type.  */
   request[1] = (ULONG)tcb_ptr;             /* ID of the task to kill.  */
   request[2] = 0u;
   request[3] = 0u;

   /* Since the SysMgr supervisor thread has the highest priority,        */
   /* this call will be preempted by SysMgr supervisor thread             */
   /* SysMgr will eventually call osek_do_task_terminate and reschedule.  */

   status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

   TX_RESTORE

   /* This should always succeed. and Sys Manager terminates this task    */
   /* This point will never be reached, as this thread itself will be     */
   /* deleted by the system manager!                                      */
   /* Still as a safety precaution enter system internal error loop.      */

   if (status != TX_SUCCESS)
   {
       osek_internal_error(SYS_MGR_SEND_TERMINATETASK);
   }

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_system_manager_entry                          PORTABLE C       */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This is the System Manager thread for the OSEK.                     */
/*    All OSEK service calls that may cause rescheduling, ends up in      */
/*    sending a message to a queue, and this highest priority thread      */
/*    'System Manager' reads this message and acts upon it.               */
/*    It then calls the actual task scheduler which selects the next ready*/
/*    task from a queue of READY tasks.                                   */
/*    After completing the request this routine returns back to read      */
/*    next message which suspends this System Manager Thread and the task */
/*    selected by the task scheduler starts running.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input                          Not used  (Don't care)               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_queue_receive               Get message from ThreadX queue       */
/*    osek_do_task_terminate         Terminate the OSEK task.             */
/*    osek_do_task_activate          Activate the task                    */
/*    osek_internal_error            OSEK internal error.                 */
/*    osek_do_delete_task            Delete the OSEK task                 */
/*    pop_task_from_table            Removes a task from the queue        */
/*    tx_thread_suspend              Suspends the calling task            */
/*    add_task_to_table              Adds a task to the TASK ready queue  */
/*    start_osek_tasks               Runs the next ready task from queue  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Wrapper Internal code (Not available to Application )               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  osek_system_manager_entry(ULONG input)
{
TX_THREAD      *this_thread;
OSEK_TCB       *tcb_ptr;
UINT            status;
StatusType      osek_status;
UINT            i;
UINT            j;
ULONG           request[SYSMGR_QUEUE_MSG_LENGTH];


   /* The input argument 'input' is not needed , hence not used */
   /* This statement is added just to avoid compiler warning.  */
   (void)&input;

   /* Loop forever, waiting for any new message in the work queue.  */
   for(;;)
   {
       /* Wait forever for the next work request.  */
       status = tx_queue_receive(&osek_work_queue, &request, TX_WAIT_FOREVER);

       /* Make sure we didn't encounter any trouble.  */
       if (status != TX_SUCCESS)
       {
           osek_internal_error(SYSMGR_FATAL_ERROR);
           continue;
       }

       /* Look at the first entry in the message for the request type.  */
       switch(request[0u])
       {
           case    SYSMGR_TERMINATE_TASK:

              /* ISR ends up in terminating itself so there is no separate switch case
                 for 'Return from ISR'.  */
              /* As the task is terminated, remove it from the queue.  */
              pop_task_from_table((OSEK_TCB *)request[1u]);
              /* Now Terminate the task.  */
              osek_status = osek_do_task_terminate((OSEK_TCB *)request[1u]);
              if(osek_status != E_OK) {
                  osek_internal_error(SYSMGR_FATAL_ERROR);
              }

              task_terminated = TX_TRUE;

              /* Enforce default operations mode.  */
              osek_wrapper_operation_mode = NORMAL_EXECUTION_MODE;

              break;


           case    SYSMGR_CHAIN_TASK:

               /* As calling task is terminated remove it from the queue.  */
               pop_task_from_table ((OSEK_TCB *)request[1u]);

               /* Now terminate the calling task.  */
               osek_status = osek_do_task_terminate((OSEK_TCB *)request[1u]);
               if(osek_status != E_OK) {
                   osek_internal_error(SYSMGR_FATAL_ERROR);
               }

               task_terminated = TX_TRUE;

               /* Now activate the task to be chained.  */
               osek_status = osek_do_activate_task((OSEK_TCB *)request[2u]);
               if(osek_status != E_OK) {
                   osek_internal_error(SYSMGR_FATAL_ERROR);
               }

               break;

           case    SYSMGR_START_OS:

               system_start = TX_TRUE;                    /* Indicates a fresh OS start.  */
               disable_ISR2 = TX_FALSE;
               suspend_ISR2 = TX_FALSE;

               /* OSEK started change init mode.  */
               osek_init_state = OSEK_STARTED;

               break;

           case    SYSMGR_ACTIVATE_TASK:

               /* First suspend the calling task but if an ISR is activating a task then */
               /* no need to suspend ISR Task as rescheduling (if required) is done only */
               /* after ISR completes. With ISR calling this service the request[2] is   */
               /* always a NULL.  */
               if (request[2] != 0u)
               {
                   this_thread = (TX_THREAD *)request[2];
                   status = tx_thread_suspend(this_thread);
                   if (status != TX_SUCCESS)
                   {
                       osek_internal_error(SYSMGR_FATAL_ERROR);
                   }
               }

               /* Now activate the requested task.  */
               osek_status = osek_do_activate_task((OSEK_TCB *)request[1]);
               if(osek_status != E_OK) {
                   osek_internal_error(SYSMGR_FATAL_ERROR);
               }

               break;

           case    SYSMGR_RELEASE_RESOURCE:

               tcb_ptr = (OSEK_TCB *)request[1];

               /* First remove the task from its current queue position.  */
               pop_task_from_table( tcb_ptr);

               /* Now change this task prio to a new one after release res.  */
               tcb_ptr->cur_threshold = (UINT)request[2];

               /* Now move this task to its new position.  */
               push_task_to_table(tcb_ptr);

               /* Now suspend the calling task.  */
               if (request[1u] != 0u)
               {
                   this_thread = (TX_THREAD *)request[1];
                   status = tx_thread_suspend(this_thread);
                   if (status != TX_SUCCESS)
                   {
                       osek_internal_error(SYSMGR_FATAL_ERROR);
                   }
               }
               break;

           case    SYSMGR_GET_RESOURCE:

               tcb_ptr = (OSEK_TCB *)request[1];

               /* First remove the task from its current queue position.  */
               pop_task_from_table ( tcb_ptr);

               /* Now change this task prio to a new one after release res.  */
               tcb_ptr->cur_threshold = (UINT)request[2];

               /* Now move this task to its new position.  */
               push_task_to_table(tcb_ptr);

               /* Now suspend the calling task.  */
               if (request[1u] != 0u)
               {
                    this_thread = (TX_THREAD *)request[1u];
                    status = tx_thread_suspend(this_thread);
                    if (status != TX_SUCCESS)
                    {
                        osek_internal_error(SYSMGR_FATAL_ERROR);
                    }
               }

               break;

           case    SYSMGR_SCHEDULE:

               /* Get TCB for the calling task.  */
               tcb_ptr = (OSEK_TCB *)request[1u];

               /* Now release any internal resources held
                  and move the task to its original priority queue.  */
               osek_status = release_internal_resource(tcb_ptr);
               if(osek_status != E_OK) {
                   osek_internal_error(SYSMGR_FATAL_ERROR);
               }

               /* Suspend the calling task.  */
               if (request[1u] != 0u)
               {
                   this_thread = (TX_THREAD *)request[1u];
                   status = tx_thread_suspend(this_thread);
                   if (status != TX_SUCCESS)
                   {
                       osek_internal_error(SYSMGR_FATAL_ERROR);
                   }
               }

               break;

           case    SYSMGR_SETEVENT:

               /* Add this out of wait state task to the ready queue as a newest member.  */
               ((OSEK_TCB *)request[2u])->cur_threshold = ((OSEK_TCB *)request[2u])->org_prio;

               add_task_to_table((OSEK_TCB *)request[2u]);

               /* Suspend the calling task.  */
               if (request[1u] != 0u)
               {
                   this_thread = (TX_THREAD *)request[1u];
                   status = tx_thread_suspend(this_thread);
                   if (status != TX_SUCCESS)
                   {
                       osek_internal_error(SYSMGR_FATAL_ERROR);
                   }
               }

               break;

           case    SYSMGR_WAITEVENT:

               /* Remove from the queue the task that went into waiting state.  */
               pop_task_from_table((OSEK_TCB *)request[2u]);

               /* Suspend it.  */
               this_thread = (TX_THREAD *)request[1u];

               status = tx_thread_suspend(this_thread);
               if (status != TX_SUCCESS)
               {
                   osek_internal_error(SYSMGR_FATAL_ERROR);
               }

               break;

           case    SYSMGR_SHUTDOWN_OS:
               /* System shut down, clear task queue.  */

               for (i = 0u; i < (OSEK_ISR1_PRIORITY + 1u); i++)
               {
                   for (j = 0u; j < TASK_QUEUE_DEPTH; j++)
                   {
                       if (task_table[i][j] != 0u)
                       {
                           status = tx_thread_terminate((TX_THREAD *)(task_table[i][j]));
                           if(status != TX_SUCCESS) {
                               osek_internal_error(SYSMGR_FATAL_ERROR);
                           }
                           task_table [i][j] = 0u;
                       }
                   }
               }

               return;

               break;

           case    SYSMGR_ERRORHOOK:
               this_thread = (TX_THREAD *)request[1u];

                   /* Although an ERRORHOOK won't call any API Service that may cause
                      rescheduling there could be an ISR logged while an Errorhook
                      was executing, so Errorhook call ends up in sending a message to
                      SYS manager to reschedule if necessary.  */

                   /* Suspend the task causing Error condition.  */
               status = tx_thread_suspend(this_thread);
               if (status != TX_SUCCESS)
               {
                   osek_internal_error(SYSMGR_FATAL_ERROR);
               }

                   break;

           default:

                   /* We should NEVER get here... call a fatal error.  */
                   /* System internal error.  */
                   osek_internal_error(SYSMGR_FATAL_ERROR);

                   break;

       } /* End of switch.  */

       /* Now call scheduler to check for any preemption and resume.  */
       start_osek_tasks();

   }  /* wile (1) System Manager forever loop.  */

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_memory_init                                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to create a ThreadX byte pool that will      */
/*    act as a "heap" for the OSEK dynamic internal objects               */
/*    memory needs.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    region0_ptr                            OSEK memory pointer          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_byte_pool_create                   Create region0 byte pool      */
/*    osek_internal_error                   Internal OSEK error           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static UINT osek_memory_init (void  *region0_ptr)
{
UINT    retval;

    /* Create a ThreadX byte pool that will provide memory
       needed by the OSEK.  */
    retval = tx_byte_pool_create((TX_BYTE_POOL *)&osek_region0_byte_pool,
                                 "OSEK REGION 0",
                                 region0_ptr,
                                 TX_REGION0_SIZE_IN_BYTES);

    return (retval);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_counter_init                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up, configures and initializes all the           */
/*    counter structures, which are defined at compile-time in order to   */
/*    ensure that there is sufficient memory.                             */
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
/*    osek_reset_counter                    Reset a Counters structure    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  osek_counter_init(void)
{

ULONG       index;

    /* Loop through array of semaphores and initialize each one.  */
    for (index = 0u; index < OSEK_MAX_COUNTERS; index++)
    {
        osek_reset_counter(&(osek_counter_pool[index]));
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_reset_counter                                  PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets a counter structure.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    counter_ptr                             Counter pointer             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_reset_counter(OSEK_COUNTER *counter_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT  index;

    TX_DISABLE

    /* This counter is now no longer in use.  */
    counter_ptr->cntr_in_use = TX_FALSE;

    /* Make any counter initial ticks to 0.  */
    counter_ptr->counter_value = 0u;

    /* Make max. allowable ticks to 0.  */
    counter_ptr->maxallowedvalue = 0u;

    /* Make no. of alarm repetition to 0.  */
    counter_ptr->mincycle = 0u;

    /* Make ticks per base = 1.  */
    counter_ptr->ticksperbase = 1u;

    /* Not attached yet to system timer.  */
    counter_ptr->system_timer = TX_FALSE;

    /* Clear alarm list.  */
    for (index = 0u; index < OSEK_MAX_ALARMS; index++)
    {
        counter_ptr->alarm_list[index] = 0u;
    }

    counter_ptr->osek_counter_id = 0u;

    TX_RESTORE

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_alarm_init                                   PORTABLE C        */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up, configures and initializes all the           */
/*    alarm structures, which we define at compile-time in order to       */
/*    ensure that there is sufficient memory.                             */
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
/*    osek_reset_alarm                                                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  osek_alarm_init(void)
{

ULONG       index;

    /* Loop through array of Semaphores and initialize each one.  */
    for(index = 0u; index < OSEK_MAX_ALARMS; index++)
    {
        osek_reset_alarm(&(osek_alarm_pool[index]));
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_reset_alarm                                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets an alarm structure to its default state.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    alarm_ptr                             Alarm pointer                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_reset_alarm(OSEK_ALARM *alarm_ptr)
{
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE

    /* First unarm the timer.  */
    alarm_ptr->armed = TX_FALSE;

    /* This alarm is now no longer in use.  */
    alarm_ptr->alarm_in_use = TX_FALSE;
    alarm_ptr->occupied = TX_FALSE;

    /* Make Call back function to TX_NULL.  */
    alarm_ptr->alarm_callback = TX_NULL;

    /* Make any counter reference to TX_NULL.  */
    alarm_ptr->cntr = TX_NULL;

    /* Make any task and event reference to TX_NULL.  */
    alarm_ptr->task = TX_NULL;
    alarm_ptr->events = 0u;
    alarm_ptr->action = 0u;

    /* Make its default value = 0.  */
    alarm_ptr->max_allowed_value = 0u;
    alarm_ptr->min_cyc = 0u;
    alarm_ptr->cycle = 0u;
    alarm_ptr->expiration_count = 0u;
    alarm_ptr->auto_start = 0u;

    alarm_ptr->osek_alarm_id = 0u;

    TX_RESTORE

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_tcb_init                                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up, configures and initializes all the           */
/*    task control blocks, which are defined at compile-time in order to  */
/*    ensure that there is sufficient memory.                             */
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
/*    osek_reset_tcb                         Reset a task control block   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  osek_tcb_init(void)
{

ULONG       index;

    /* Loop through array of TCBs and initialize each one.  */
    for(index = 0u; index < OSEK_MAX_TASKS; index++)
    {
        osek_reset_tcb(&(osek_tcb_pool[index]));
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_resource_init                                  PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up, configures and initializes all the           */
/*    resource structures, which are defined at compile-time in order to  */
/*    ensure that there is sufficient memory.                             */
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
/*    osek_reset_res                        Reset a semaphore structure   */
/*    CreateResource                        Create the resource in OSEK   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  osek_resource_init(void)
{

ULONG             index;
OSEK_RESOURCE     *res_ptr;


    /* Loop through array of resources and initialize each one.  */
    for(index = 0u; index < OSEK_MAX_RES; index++)
    {
        osek_reset_res(&(osek_res_pool[index]));
    }

    RES_SCHEDULER = CreateResource("RES_SCHEDULER", STANDARD, 0u);
    res_ptr = (OSEK_RESOURCE *)RES_SCHEDULER;
    res_ptr->c_priority = OSEK_NON_SCHEDULE_PRIORITY;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_reset_res                                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets a resource a structure to its default state.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    res_ptr                               Resource pointer              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_reset_res(OSEK_RESOURCE  *res_ptr)
{
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE

    /* Indicate this entry is not in use.  */
    res_ptr->res_in_use = TX_FALSE;

    TX_RESTORE

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_do_task_terminate                              PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function terminates the specified task.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcb_ptr                         Task to activate                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                            If successful                       */
/*    Error Code.                     If an error occurs                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_internal_error             Internal error                      */
/*    tx_thread_terminate             Terminate the thread                */
/*    tx_thread_create                Create the thread                   */
/*    osek_reset_tcb                  Free the task control block         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK only (internal)                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType osek_do_task_terminate(OSEK_TCB  *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT                  retval;
TX_THREAD            *thread_ptr;
UINT                  priority;


    /* Converts OSEK into ThreadX.  */
    priority = osek_remap_priority(tcb_ptr->org_prio);

    /* Make the current threshold as original priority.  */
    tcb_ptr->cur_threshold = tcb_ptr->org_prio;

    /* Get the ThreadX thread pointer.  */
    thread_ptr = (TX_THREAD *)tcb_ptr;

    /* Terminate the task's thread.  */
    retval = tx_thread_terminate(thread_ptr);

    /* See if every thing is fine.  */
    if (retval != TX_SUCCESS)
    {
        /* System internal error.  */
        osek_internal_error(THREADX_THREAD_TERMINATE_TERMINATETASK);

        /* Error will be returned whenever the ThreadX call fails.  */
        return (E_OS_SYSTEM);
    }

    TX_DISABLE

    /* Delete the ThreadX thread.  */
    retval = tx_thread_delete(thread_ptr);

    /* Check if everything is fine.  */
    if (retval != TX_SUCCESS)
    {
        /* System internal error.  */
        osek_internal_error(THREADX_THREAD_DELETE_TERMINATETASK);

        /* Error will be returned whenever the ThreadX call fails.  */
        return (E_OS_SYSTEM);
    }


    if (tcb_ptr->current_active != 0u)
    {
        /* Decrement the activation count.  */
        tcb_ptr->current_active--;
    }

    /* Now check if there is any activations pending?  */
    if (tcb_ptr->current_active == 0u)
    {
        /* If no activations pending then make this task SUSPENDED.  */
        tcb_ptr->suspended = TX_TRUE;
    }

    TX_RESTORE

    tcb_ptr->waiting = TX_FALSE;

    /* Create ThreadX thread.  */
    retval = tx_thread_create(thread_ptr,
                              (char *)tcb_ptr->name,
                              osek_task_wrapper,
                              (ULONG)tcb_ptr,
                              tcb_ptr->pStackBase,
                              tcb_ptr->stack_size,
                              priority,
                              priority,
                              TX_NO_TIME_SLICE,
                              TX_DONT_START);

    /* See if ThreadX encountered an error.  */
    if (retval != TX_SUCCESS)
    {
        /* Free the task TCB structure.  */
        osek_reset_tcb(tcb_ptr);

        /* Internal error */
        osek_internal_error(THREADX_OBJECT_CREATION_ERROR);

        /* Error will be returned whenever the ThreadX call fails.  */
        return (E_OS_SYSTEM);
    }

    /* Everything is OK. */
    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_do_activate_task                               PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function activates the specified task.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    *tcb_ptr                        task to activate                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                            If successful                       */
/*    Error Code.                     If an error occurs.                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_internal_error             Internal error                      */
/*    tx_thread_resume                Activate  the thread                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK only (internal)                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType osek_do_activate_task (OSEK_TCB  *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
    /*  Activate a task means making it READY and place into the TASK READY queue.  */

    TX_DISABLE
    /* Increment current multiple activation count.  */
    tcb_ptr->current_active++ ;

    /* Move this task from suspended state to ready state.  */
    tcb_ptr->suspended = TX_FALSE;

    /* And add to the queue as a newest member.  */
    add_task_to_table(tcb_ptr);

    TX_RESTORE;

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_thread2tcb                                      PORTABLE C       */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  This function returns a ThreadX thread pointer to an OSEK task        */
/*  pointer.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread                         Thread pointer                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    *tcb                           TCB pointer                          */
/*                                   On error a null pointer is returned  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Internal Code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static OSEK_TCB  *osek_thread2tcb(TX_THREAD *thread_ptr)
{

OSEK_TCB   *tcb;

    /* Make sure we were called from a thread.  */
    if (thread_ptr == TX_NULL)
    {
        /* Communicate error by means of a NULL pointer.  */
        return ((OSEK_TCB *)0u);
    }

    /* We can do this because the Thread information is intentionally
       located as the first field in the structure.  */
    tcb = (OSEK_TCB *)thread_ptr;

    /* All done.  */
    return (tcb);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_task_independent_area                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines if the system is currently in a thread     */
/*    context, i.e. not timer routine, not ISR, not idling,               */
/*    not in initialization phase.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TRUE                       If in task area                       */
/*    TX_FALSE                      If not in task area                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    osek internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static ULONG  osek_task_independent_area(void)
{

    if (osek_init_state != OSEK_STARTED)
    {

        /* We are calling from initialization, return FALSE.  */
        return (TX_FALSE);
    }
    else if ((_tx_thread_current_ptr == TX_NULL) ||         /* Not in a thread.  */
             (_tx_thread_system_state != 0u) ||             /* In an ISR.  */
             (_tx_thread_current_ptr == &_tx_timer_thread)) /* Timer routine.  */

    {
        /* We are NOT in thread (task) context.  */
        return (TX_FALSE);
    }
    else
    {
        /* We ARE in thread (task) context.  */
        return (TX_TRUE);
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_create_task                                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an OSEK task under ThreadX.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcb_ptr                               Task control block pointer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                                  If successful                 */
/*    E_OS_SYSTEM                           If failure                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_tcb2thread                       Convert TCB to thread         */
/*    tx_thread_create                      Create thread                 */
/*    osek_reset_tcb                        OSEK free task control block  */
/*    add_task_to_table                     Add this task to queue        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK only (internal)                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType osek_create_task(OSEK_TCB * tcb_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT                  retval;
UINT                  priority;

TX_DISABLE

   /* Converts OSEK priority into ThreadX. In ThreadX '0' is highest priority and '31' is the lowest
       while in OSEK it is exactly the opposite.  */

   priority = osek_remap_priority(tcb_ptr->org_prio);

   /* Create a ThreadX thread, osek_task_wrapper is the entry function which is common for all thread
      implementing an OSEK task to differentiate which task we supply 'task id' as an input for
      thread entry function.  */

    retval = tx_thread_create ( &(tcb_ptr->task),
                                 (char *)tcb_ptr->name,
                                 osek_task_wrapper,
                                 (ULONG)tcb_ptr,
                                 tcb_ptr->pStackBase,
                                 tcb_ptr->stack_size,
                                 priority,
                                 priority,
                                 TX_NO_TIME_SLICE,
                                 TX_DONT_START);       /* All threads implementing an OSEK TASK are created in SUSPENDED state.  */

    /* Check for any error.  */
    if (retval != TX_SUCCESS)
    {
        /* Free the task tcb structure.  */
        osek_reset_tcb(tcb_ptr);

        /* Internal error.  */
        osek_internal_error(THREADX_OBJECT_CREATION_ERROR);

        /* Error will be returned whenever the ThreadX call fails.  */
        retval = E_OS_SYSTEM;
    }
    else
    {
        /* Got the thread, now check AUTO START Specified for this task.  */
        /* If the task is not specified as 'AUTO RUN' then it is not in 'READY' State,
           so won't be added to task queue at this stage.  */

        if (tcb_ptr->task_autostart == TRUE)
        {
            /* With AUTO START the task will attain READY state
               the moment it is created, whether it will execute (RUN) will depend on Scheduler.  */

            tcb_ptr->suspended = TX_FALSE;

            /* As this task will be in Ready immediately after creation its
               Current activation counter must be increment to 1 from 0.  */
            tcb_ptr->current_active = 1u;

            /* Now add this task to Schedulers 'READY' task queue.  */
            add_task_to_table(tcb_ptr);
        }

        /* Everything is fine.  */
        retval = E_OK;
    }


TX_RESTORE

    return(retval);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_allocate_tcb                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to allocate memory for a task stack and an   */
/*    OSEK Thread Control Block (TCB).                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    stack_size                            Requested task stack size     */
/*    tcb_ptr                               Pointer to tcb pointer        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TRUE                               If successful.                */
/*    TX_FALSE                              If an error occurs.           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_memory_allocate                   Allocate task's stack        */
/*    osek_reset_tcb                         Free task control block      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static ULONG osek_allocate_tcb(ULONG stack_size, OSEK_TCB **tcb_ptr)
{
OSEK_TCB         * tcb;
ULONG             index;
ULONG             retval;

    /* Assume the worst.  */
    tcb = (OSEK_TCB *)TX_NULL;

    /* This next search is optimized for simplicity, not speed.  */
    tcb = osek_tcb_pool;
    for (index = 0u; index < OSEK_MAX_TASKS; index++)
    {
        /* Is thisTCB in use? If not, we can use it.  */
        if (tcb->tcb_in_use == TX_FALSE)
        {
            /* This TCB is now in use.  */
            tcb->tcb_in_use = TX_TRUE;

            /* Stop searching.  */
            break;
        }

        tcb++;
    } /* try next TCB.  */

    /* Did we search all TCBs and come up empty?  */
    if (index == OSEK_MAX_TASKS)
    {
        /* No more TCBs available - user configuration error.  */
        return(E_OS_SYS_STACK);
    }
    else
    {
        /* Found one.  */
        *tcb_ptr = tcb;
    }

    /* Reset stack pointer.  */
    tcb->pStackBase  = (CHAR *)TX_NULL;

    /* Allocate memory for the task stack.  */
    retval = osek_memory_allocate(stack_size,
                              ((void **)&(tcb->pStackBase)));

    /* Make sure we got the memory for the task stack.  */
    if ((retval == 0u) || (tcb->pStackBase == TX_NULL))
    {
        /* Failed - at least try to return the OSEK TCB memory.  */
        osek_reset_tcb(tcb);

        /* Indicate failure.  */
        return (E_OS_SYS_STACK);
    }

    /* Remember the size of the stack.  */
    tcb->stack_size  = stack_size;

    /* Return OSEK TCB to caller.  */
    return (TX_TRUE);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_memory_allocate                                PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to obtain the specified amount of memory     */
/*    from the OSEK heap.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    size                                  Number of bytes to allocate   */
/*    memory_ptr                            Pointer to the returned       */
/*                                          memory                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TRUE                               If successful                 */
/*    TX_FALSE                              If an error occurs            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_byte_allocate                      Allocate from the pool        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static UINT osek_memory_allocate(ULONG size, void  **memory_ptr)
{
ULONG   size_align;
UINT    retval;

   /* Initialize the pointer to NULL in case we fail.  */
   *memory_ptr = (void *)TX_NULL;

   /* Force all alignments to long word boundaries to be safe.  */
   size_align = size;
   if ((size_align) != 0u)
   {
       /* Bump size up to next 4 byte boundary.  */
       size_align = ((size_align + 0x03u) & ~0x03u);
   }

   /* Attempt to allocate the desired memory from the OSEK heap.  */
   /* Do not wait - if memory isn't available, flag an error.  */
   retval = tx_byte_allocate((TX_BYTE_POOL *)&osek_region0_byte_pool, memory_ptr,
           size_align, TX_NO_WAIT);

   /* Make sure the memory was obtained successfully.  */
   if(retval != TX_SUCCESS)
   {
       /* Error obtaining memory.  */
       return (TX_FALSE);
   }

   /* Return to caller.  */
   return (TX_TRUE);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_reset_tcb                                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets a task TCB to its default state.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcb                                   Task control block pointer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_reset_tcb(OSEK_TCB  *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA

UINT  index;

   /* Disable interrupt.  */
   TX_DISABLE

   /* Indicate this TCB is not in use.  */
   tcb_ptr->tcb_in_use = TX_FALSE;

   /* Make suspended and waiting to FALSE.  */
   tcb_ptr->suspended = TX_TRUE;
   tcb_ptr->waiting = TX_FALSE;

   /* Make this task as BASIC.  */
   tcb_ptr->task_type = BASIC;

   /* No AUTOSTART */
   tcb_ptr->task_autostart = 0u;

   /* Erase TCB id */
   tcb_ptr->osek_task_id = 0u;

   /* Make original priority of the task to 0 (the lowest possible).  */
   tcb_ptr->org_prio = 0u;

   /* Set the thread preemption threshold to default.  */
   tcb_ptr->cur_threshold = THREADX_LOWEST_PRIORITY;

   /* Make the stack size to 0.  */
   tcb_ptr->stack_size = 0u;

   /* No task to chain.  */
   tcb_ptr->task_to_chain = 0u;

   /* Since now event is not attached Make it to TX_NULL.  */
   tcb_ptr->events = 0u;

   /* Make task entry function to TX_NULL.  */
   tcb_ptr->task_entry = TX_NULL;

   /* Make maximum activation to 0.  */
   tcb_ptr->max_active = 0u;

   /* Make current activation to 0.  */
   tcb_ptr->current_active = 0u;

   /* Clear lists for external Resource attached and Resource occupied.  */
   for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
   {
       tcb_ptr->external_resource_list[index] = 0u;
       tcb_ptr->external_resource_occuplied_list[index] = 0u;
   }

   /* Clear lists for Internal Resource attached and resource occupied.  */
   for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
   {
       tcb_ptr->internal_resource_list[index] = 0u;
       tcb_ptr->internal_resource_occuplied_list[index] = 0u;
   }

   /* Since resource is not occupied make it to FALSE.  */
   tcb_ptr->res_ocp = 0u;

   /* All done. */

   /* Enable interrupt.  */
   TX_RESTORE

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_get_resource                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds if any resource is available in the pool.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcb                              Task control block pointer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    res_ptr                          Resource pointer OR null           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Start-up code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static ResourceType osek_get_resource(void)
{
TX_INTERRUPT_SAVE_AREA
ULONG             index;
OSEK_RESOURCE     *res_ptr;

    TX_DISABLE

   /* Start out pessimistic - assume we won't find a match.  */
   res_ptr = (OSEK_RESOURCE *)TX_NULL;

   /* Search the resource from resource pool.  */
   res_ptr = &(osek_res_pool[0u]);
   for (index = 0u; index < OSEK_MAX_RES; index++)
   {
       /* Make sure the resource is not already in use.  */
       if (res_ptr->res_in_use == TX_FALSE)
       {
           /* This Resource is now in use.  */
           res_ptr->res_in_use = TX_TRUE;
           break;
       }

       res_ptr++;
   }

   TX_RESTORE

   /* Did we search all OSEK RESOURCES and come up empty?  */
   if (index == OSEK_MAX_RES)
   {
       return ((ResourceType) TX_NULL);
   }

   return ((ResourceType) res_ptr);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_get_alarm                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets an alarm if it is available.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ALARM_ID             AlarmID if found                               */
/*    TX_FALSE             If not available                               */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*  API function.                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static UINT   osek_get_alarm(void)
{
TX_INTERRUPT_SAVE_AREA
OSEK_ALARM        *this_alarm;
UINT              index;

    TX_DISABLE

   /* Assume the worst.  */
   this_alarm = (OSEK_ALARM *)TX_NULL;

   /* Search for a free alarm.  */
   this_alarm = osek_alarm_pool;

   for (index = 0u;
        index < OSEK_MAX_ALARMS;
        index++)
   {
       /* Is this alarm in use? If not, we can use it.  */
       if (this_alarm->alarm_in_use == TX_FALSE)
       {
           /* This alarm is now in use.  */
           this_alarm->alarm_in_use = TX_TRUE;

           /* Stop searching.  */
           break;
       }

       this_alarm++;
   } /* check next if this one is in use.  */

   TX_RESTORE

   /* Did we search alarms all and come up empty?  */
   if (index == OSEK_MAX_ALARMS)
   {
       /* No more alarms available - user configuration error.  */
       return ((AlarmType)0u);
   }

   /* Found one.  */

   return ((AlarmType) this_alarm);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_get_events                                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets events if it is available in the pool.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    COUNTER_ID           CounterID if found                             */
/*    TX_FALSE             if not available                               */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    API function                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static EventMaskType  osek_get_event(void)
{
TX_INTERRUPT_SAVE_AREA
EventMaskType     event_mask;

    TX_DISABLE

   if (global_event_count >= OSEK_MAX_EVENTS)
   {
       TX_RESTORE

       /* Already alloted all possible events.  */
       return ((EventMaskType)0u);
   }

   /* Next event.  */
   event_mask = 1u;
   event_mask <<= global_event_count;
   global_event_count++;

   TX_RESTORE

   return (event_mask);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  osek_get_alarm                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets an alarm if it is available.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    COUNTER_ID           CounterID if found                             */
/*    TX_FALSE             If not available                               */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    API function                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static CounterType   osek_get_counter(void)
{
TX_INTERRUPT_SAVE_AREA
OSEK_COUNTER       *this_counter;
UINT               index;

    TX_DISABLE

   /* Assume the worst.  */
   this_counter = (OSEK_COUNTER *)TX_NULL;

   /* Search for a free counter.  */
   this_counter = osek_counter_pool;
   for ( index = 0u;
         index < OSEK_MAX_COUNTERS;
         index++)
   {
       /* Is this guy in use? If not, we can use it.  */
       if (this_counter->cntr_in_use == TX_FALSE)
       {
           /* This counter is now in use.  */
           this_counter->cntr_in_use = TX_TRUE;
           /* Stop searching.  */
           break;
       }

       this_counter++;
   } /* Try next counter.  */

   TX_RESTORE

   /* Did we search all OSEK COUNTERS and come up empty?  */
   if (index >= OSEK_MAX_COUNTERS)
   {
       /* No more Counters available - user configuration error.  */
       return ((CounterType) E_OS_SYS_STACK);
   }

   /* Found one.  */
   return ((CounterType)this_counter);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_system_timer_entry                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This wrapper acts as a system timer and maintains a system counter   */
/*   It is up to the user to assign any one of the OSEK counter as        */
/*   a system counter by defining it.                                     */
/*   This counter will then be updated by this system timer.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input                             Not used                          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK system timer expiry function                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_system_timer_entry(ULONG input)
{
TX_INTERRUPT_SAVE_AREA
OSEK_COUNTER      *this_counter;
UINT               index;
UINT               found;

    TX_DISABLE

    (void)&input; /* Prevent unused parameter warnings.  */

   found = TX_FALSE;
   /* Search for a counter acting as a system counter.  */
   this_counter = osek_counter_pool;
   for ( index = 0u;
         index < OSEK_MAX_COUNTERS;
         index++)
   {
       /* Is this guy in a system counter.  */
       if (this_counter->system_timer == TX_TRUE)
       {
           /* Got system counter.  */
           found = TX_TRUE;
           /* Stop searching.  */
           break;
       }

       this_counter++;
   } /* Try next counter.  */

   /* Did we search all OSEK COUNTERS and come up empty?  */
   if (found == TX_TRUE)
   {
       /* Got a system counter, update it.  */
       IncrCounter((CounterType)this_counter);
   }

   TX_RESTORE

   return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_remap_priority                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts a OSEK task priority into a ThreadX          */
/*    priority.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    osek_priority                       Priority of task                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ThreadX priority                    Converted priority              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static UINT osek_remap_priority(UINT osek_priority)
{

   /* Remap OSEK task priority to ThreadX thread priority.              */
   /* In OSEK 0 = lowest priority and in ThreadX, 0 = highest priority, */
   /* Means ThreadX priority of a thread acting as a OSEK Task          */
   /* would be ThreadX max priority - osek_priority.                    */

   /* Return the ThreadX priority.  */
   return (THREADX_MAX_PRIORITY - 1u - osek_priority);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_task_wrapper                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Every thread that is modeling a OSEK task uses this routine has     */
/*    its entry point. This routine calls the OSEK task entry function.   */
/*                                                                        */
/*    A task must end with either TerminateTask() or ChainTask(), if this */
/*    is not the way a task is ended, the control will come back to this  */
/*    osek_task_wrapper and call osek_internal_error() with an error code */
/*    'TASK_ENDING_WITHOUT_CHAIN_OR_TERMINATE'.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcb                                Task control block pointer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    task_entry                         OSEK task entry                  */
/*    osek_internal_error                                                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Internal code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_task_wrapper(ULONG tcb)
{
TX_INTERRUPT_SAVE_AREA
OSEK_TCB        *tcb_ptr;
StatusType      status;

    TX_DISABLE

   /* The input argument is really a pointer to the task's TCB.  */
   tcb_ptr = (OSEK_TCB *)tcb;
   /* Check whether it is a Task or an ISR.  */
   if (tcb_ptr->osek_task_id == OSEK_TASK_ID)
   {
       osek_wrapper_operation_mode = NORMAL_EXECUTION_MODE;

       /* last_run_task now holds this new task being run.  */
       last_run_task = (ULONG)((TX_THREAD*)tcb_ptr);

       /* Check this task's scheduling policy.  */
       if (tcb_ptr->policy == NON)
       {
           pop_task_from_table(tcb_ptr);   /* This routine remove task from table based on its cur_threshold.  */

           /* Store the new preemption threshold.  */
           tcb_ptr->cur_threshold = OSEK_NON_SCHEDULE_PRIORITY;

           /* Place this task to its proper priority queue based on Scheduling policy.  */
           push_task_to_table(tcb_ptr);       /* This routine pushes a task at the front of a queue based on its cur_threshold.  */
       }

   }
   else
   {
       if (tcb_ptr->task_type == CATEGORY1)
       {
               osek_wrapper_operation_mode = ISR1_MODE;
       }
       else
       {
               osek_wrapper_operation_mode = ISR2_MODE;
       }
   }

   TX_RESTORE

   /* Invoke the OSEK task entry point with appropriate arguments.  */
   (tcb_ptr->task_entry)();

   /* In case of a task we shouldn't be here- because tasks are always ended with either
      a TerminateTask() or ChainTask() but ISR will return here.  */
   /* If it is a TASK then seems to be ended without any ChainTask or TerminateTask call
      anyway, as the task is ended it is better to Terminate it so that next available ready
      task can be run.  */


   /* Terminate the task.  */
   if (tcb_ptr->osek_task_id == OSEK_TASK_ID)
   {
       /* Change to default operations mode.  */
       osek_wrapper_operation_mode = NORMAL_EXECUTION_MODE;

       status = TerminateTask();
       if(status != E_OK) {
           osek_internal_error(SYS_MGR_SEND_TERMINATETASK);
       }
   }
   else
   {
       status = TerminateISR();
       if(status != E_OK) {
           osek_internal_error(SYS_MGR_SEND_TERMINATETASK);
       }
   }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    osek_internal_error                                  PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is invoked whenever an error is encountered           */
/*    in the OSEK code. This is an endless loop.                          */
/*    Source of the error can be traced by the error code.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    error_code                            Error code                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void osek_internal_error(ULONG error_code)
{
   /* This just an end less loop, to trap error.  */
   for(;;)
   {
       ; /* Empty loop.  */
   }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    exec_ErrorHook                                       PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invokes ErroHook routine whenever an error is         */
/*    encountered, provided ErroHook is defined.                          */
/*    Hook routines are called by the operating system, in a special      */
/*    context have higher prior than all tasks, and not interrupted by    */
/*    category 2 interrupt routines.                                      */
/*    These functions are implemented by the user with user defined       */
/*    functionality. Usually hook routines are allowed to use only a      */
/*    subset of API functions. Whether to execute these routine or not is */
/*    user configurable via OIL.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    error_code                            Passed to ErrorHook           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Application->error_handler          That is ErrorHook (if defined)  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  exec_ErrorHook (StatusType error)
{

TX_THREAD     *thread_ptr;
UINT           save_op_mode;
UINT           status;
ULONG          request[SYSMGR_QUEUE_MSG_LENGTH];

   /* Check for any startup hook routine.  */
   if (Application->error_hook_handler != TX_NULL)
   {
       /* But check whether already in ErrorHook routine? There must not be nested ErrorHook calls.  */
       if (osek_wrapper_operation_mode == ERRORHOOK_MODE)
       {
           /* Already in ErroHook.  */
           return;
       }

       /* Save old operation mode.  */
       save_op_mode = osek_wrapper_operation_mode;

       /* Change to ErroHOOK mode.  */
       osek_wrapper_operation_mode = ERRORHOOK_MODE;

       /* Now execute user defined ErroHook routine.  */
       (Application->error_hook_handler)(error);

       /* Get the pointer to thread.  */
       thread_ptr = tx_thread_identify();

       /* Restore original operation mode.  */
       osek_wrapper_operation_mode = save_op_mode;

       /* If this ErrorHook is called by a task then check for any ISR came while executing this ErrorHook.  */
       if (osek_wrapper_operation_mode == NORMAL_EXECUTION_MODE)
       {
           /* Now send a message to the SysMgr supervisor thread to execute the Error Hook.  */
           /* Build the request. */
           request[0] = SYSMGR_ERRORHOOK;           /* Request type.            */
           request[1] = (ULONG)thread_ptr;          /* ptr of calling thread.   */
           request[2] = 0u;                         /* input to Error Hook.     */
           request[3] = 0u;

           /* Since the system manager supervisor thread is with the highest priority,  */
           /* this routine will be preempted by SysMgr supervisor thread when   */
           /* queue read is successful.                                         */

           status = tx_queue_send(&osek_work_queue, request, TX_NO_WAIT);

           /* This should always succeed.  */
           if (status != TX_SUCCESS)
           {
               /* System internal error.  */
               osek_internal_error(SYS_MGR_SEND_CHAINTASK);

               /* Return.  */
               return;
           }
       }  /* End if ( osek_wrapper_operation_mode ==.  */

   } /* end  if (Application->error_handler).  */

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    exec_PreTaskHook                                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invokes the PreTaskHook routine whenever a task       */
/*    starts provided PreTaskHook is defined.                             */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  exec_PreTaskHook(void)
{

UINT           save_op_mode;

   if (Application->pretask_hook_handler != TX_NULL)
   {
       /* Set up the mode.  */
       save_op_mode = osek_wrapper_operation_mode;
       osek_wrapper_operation_mode = PRETASKHOOK_MODE;

       /* Call the Pretask hook.  */
       (Application->pretask_hook_handler)();

       /* Restore mode.  */
       osek_wrapper_operation_mode = save_op_mode;

   }  /* end  if (Application->PretaskHook_handler).  */

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    exec_PostTaskHook                                    PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invokes PostTaskHook routine whenever a task          */
/*    terminates provided PostTaskHook is defined.                        */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  exec_PostTaskHook(void)
{

UINT           sav_op_mode;

   if (Application->posttask_hook_handler != TX_NULL)
   {
       /* Set up the mode.  */
       sav_op_mode = osek_wrapper_operation_mode;
       osek_wrapper_operation_mode = POSTTASKHOOK_MODE;

       /* Call the Posttask hook.  */
       (Application->posttask_hook_handler)();

       /* Restore mode.  */
       osek_wrapper_operation_mode = sav_op_mode;

    }  /* end  if (Application->PosttaskHook_handler).  */

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    add_task_to_table                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a ready task in the task queue.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    *tcb_ptr              Pointer to the task to be added to the queue  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  add_task_to_table(OSEK_TCB *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT         priority;
UINT         i;

    TX_DISABLE

   /* Get the priority of this task.  */
   priority = tcb_ptr-> org_prio;

   for (i = 0u; i < TASK_QUEUE_DEPTH; i++)
   {
       /* Add this task to the a queue assigned for this task's priority level.  */
       /* Find next free entry in the queue.  */
       /* Oldest activated task is at the front of the queue, so this new entry.  */
       /* will go at the end of the queue.  */

       if (task_table[priority][i] == 0u)
       {
           task_table[priority][i] = (TaskType)tcb_ptr;
           break;
       }
   }

   if (i >= TASK_QUEUE_DEPTH)
   {
       osek_internal_error(SYSMGR_FATAL_ERROR);
   }

   TX_RESTORE

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    push_task_to_table                                   PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function pushes a task at the front of the task queue.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    *tcb_ptr              Pointer to the task to be added to the queue  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    OSEK internal code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void   push_task_to_table(OSEK_TCB *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT    priority;
UINT    i;
UINT    k;

    TX_DISABLE

   priority = tcb_ptr->cur_threshold;

   i = (TASK_QUEUE_DEPTH - 2u);

   while (i != 0u)
   {
       /* Add this task to the queue (of for priority level supplied).  */
       /* Place the entry at the front of the queue and push back all entries by one */
       k = i + 1u;
       task_table[priority][k] = task_table[priority][i];
       i--;
   }

   task_table[priority][1u] = task_table[priority][0u];

   /* Now push the calling task's id  at the front of the queue.  */
   task_table[priority][0u] = (ULONG)tcb_ptr;

   TX_RESTORE
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    start_osek_tasks                                     PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function schedules the first task upon StatOS                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Error Code                                                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_resume                                                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    osek_system_manager_entry                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void start_osek_tasks(void)
{
TX_INTERRUPT_SAVE_AREA
TX_THREAD  *task_thread;
UINT       i;
UINT       j;
UINT       found;

    TX_DISABLE

   /* Start with the highest priority */
   j = OSEK_ISR1_PRIORITY;
   found = FALSE;

   while (found == FALSE)
   {
       for (i = 0u; i < TASK_QUEUE_DEPTH; i++)
       {
           /* Search for any ready but not waiting task within this priority level.  */
           if (( task_table[j][i] != 0u) && (((OSEK_TCB*)task_table[j][i] )->waiting == TX_FALSE))
           {
               /* Found it but check whether this task can be run,
                  This task must have higher priority than current task's ceiling priority
                  This task must have all needed Internal/External resources.  */
               task_thread = &((OSEK_TCB*)task_table[j][i])->task;
               found = check_task_to_run((OSEK_TCB*)task_table[j][i]);
               if (found == TRUE)
               {
                   break;
               }
           }

       }

       /* No ready task found for this priority level, check for next lower priority level
          but are we at the lowest priority level?  */
       if ((j == 0u) || (found == TRUE))
       {
           break;
       }

       j--;

   } /* End while.  */

   /* Reached here means either a ready Task is found or no task is READY.  */
   /* if found = 1, task is found.  */

   if (found == FALSE)
   {
       last_run_task = INVALID_TASK;
   }
   else
   {
       if (((OSEK_TCB*)task_thread)->osek_task_id == OSEK_ISR_ID)
       {
       /* If it is an ISR , set the op mode and no need to:
          call pre and post task hooks as well as change last_run_task data.  */
          osek_wrapper_operation_mode = ISR2_MODE;
       }
       else
       {
           osek_wrapper_operation_mode = NORMAL_EXECUTION_MODE;
           /* Check whether it is a fresh OS start, for fresh start no need to run pre & post task hooks.  */
           if (system_start == 0u)
           {
               /* If no preemption takes place , then no need to execute pre and post task hooks.  */
               if ((last_run_task != (ULONG)task_thread )|| (task_terminated == TX_TRUE))
               {
                   exec_PostTaskHook();

                   /* Update last run task data to reflect next task to run.  */
                   last_run_task = (ULONG)task_thread;

                   exec_PreTaskHook();
               }

           }

           last_run_task = (ULONG)task_thread;
       } /* End else. */

       tx_thread_resume(task_thread);

       system_start = TX_FALSE;

       task_terminated = TX_FALSE;

   } /* end of else part of if (!found).  */

   TX_RESTORE
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    check_task_to_run                                    PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function schedules the first task upon StatOS                  */
/*    This is called by system manager thread when StartOS is called.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Error Code                                                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_resume                                                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    osek_system_manager_entry                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static UINT check_task_to_run (OSEK_TCB *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
   UINT     status;

    TX_DISABLE

   if (tcb_ptr->osek_task_id != OSEK_TASK_ID)
   {
       /* if selection is an ISR,
          it can't be run if ISRs are suspended or any HOOK mode is ON.  */
       if ((suspend_ISR2 == TX_TRUE) ||
           (osek_wrapper_operation_mode == ERRORHOOK_MODE)      ||
           (osek_wrapper_operation_mode == PRETASKHOOK_MODE)    ||
           (osek_wrapper_operation_mode == POSTTASKHOOK_MODE)   ||
           (osek_wrapper_operation_mode == STARTUPHOOK_MODE)    ||
           (osek_wrapper_operation_mode == SHUTDOWNHOOK_MODE)   ||
           (osek_wrapper_operation_mode == ALARM_CALLBACK_MODE))
       {
           TX_RESTORE

           return (FALSE);
       }
   }

   if (tcb_ptr->internal_res != 0u)
   {
       status = get_internal_resource(tcb_ptr);

       if (status != E_OK)
       {
           TX_RESTORE

           return (FALSE);
       }
   }

   status = check_external_resource(tcb_ptr);
   if (status != E_OK)
   {
       TX_RESTORE

       return (FALSE);
   }

   /* Now check for NON scheduling policy.  */
   if (tcb_ptr->policy == NON)
   {
       /* First remove task from its current queue position.  */
       pop_task_from_table(tcb_ptr);

       tcb_ptr->cur_threshold = OSEK_NON_SCHEDULE_PRIORITY;

       push_task_to_table(tcb_ptr);
   }

   TX_RESTORE

   return (TRUE);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    pop_task_from_table                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*    Removes a task from the table of active tasks.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                           Id of the resource.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void  pop_task_from_table(OSEK_TCB *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT         priority;
UINT         i;
UINT         j;
UINT         k;

    TX_DISABLE

   /* Get the priority of this task.  */
   priority = tcb_ptr-> cur_threshold;
   j = (TASK_QUEUE_DEPTH - 2u);
   for (i = 0u; i < j; i++)
   {
       /* Move N+1 th element to Nth location.  */
       /* Oldest task is at the bottom of the queue.  */
       k = i + 1u;
       task_table[priority][i] = task_table[priority][k];
   }

   /* Make the top most element in the queue NULL as the oldest task is removed from the bottom.  */
   j = (TASK_QUEUE_DEPTH) - 1u;
   task_table[priority][j] = 0u;

   TX_RESTORE
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    check_linked_resources                              PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   Check if resources are linked                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static void        check_linked_resources(void)
{
TX_INTERRUPT_SAVE_AREA
ULONG              index;
OSEK_RESOURCE     *res_ptr;
OSEK_RESOURCE     *linked_res_ptr;

    TX_DISABLE

    /* Search the Resource from resource pool.  */
    for (index = 0u; index < OSEK_MAX_RES; index++)
    {

        res_ptr = &(osek_res_pool[index]);
        if ((res_ptr->res_in_use == TX_TRUE) && (res_ptr->type == LINKED))
        {
            /* Get the res linked to this resource.  */
            linked_res_ptr = (OSEK_RESOURCE *)(res_ptr->resolved_res);

            /* Make this resources's ceiling priority equal to the ceiling priority of linked resource.  */
            if ( res_ptr->c_priority < linked_res_ptr->c_priority)
            {
                res_ptr->c_priority = linked_res_ptr->c_priority;
            }
        }
    }

    TX_RESTORE
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    get_internal_resource                               PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call serves to enter critical sections in the code that are     */
/*   assigned to the resource referenced by <ResID>. A critical section   */
/*   must always be left using ReleaseResource. Nested resource           */
/*   occupation is only allowed if the inner critical sections are        */
/*   completely executed within the surrounding critical section.         */
/*   Nested occupation of one and the same resource is also forbidden.    */
/*   Corresponding calls to GetResource and ReleaseResource should appear */
/*   within the same function on the same function level.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                           Id of the resource.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If success                             */
/*    E_OS_CALLEVEL                Called from ISR                        */
/*    E_OS_ACCESS                  Attempt to get a resource which is     */
/*                                 already occupied by any task or ISR,   */
/*                                 or the statically assigned priority of */
/*                                 the calling task or interrupt routine  */
/*                                 is higher than the calculated ceiling  */
/*                                 priority,                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area   See if called from task independent    */
/*                                 area                                   */
/*    tx_thread_identify           Identify the current thread            */
/*    osek_internal_error          Osek internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType get_internal_resource(OSEK_TCB  *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
OSEK_RESOURCE  *osek_res;
UINT            res_prio;
UINT            index;


TX_DISABLE

   for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
   {

       if (tcb_ptr->internal_resource_list[index] == 0u)
       {
           break;   /* No Internal Resource in the list.  */
       }

       /* Get internal RES's control block.  */
       osek_res = (OSEK_RESOURCE *)tcb_ptr->internal_resource_list[index];

       /* Now check whether this resource is already occupied by other task.  */
       if (osek_res->taskid != 0u)
       {
           /* Some entry is there, is it occupied by any other task?  */
           if (osek_res->taskid != (TaskType)tcb_ptr)
           {
               TX_RESTORE

               return (E_OS_ACCESS);
           }
       }

       /* Take this resource.  */
       tcb_ptr->internal_resource_occuplied_list[index] = tcb_ptr->internal_resource_list[index];

       /* Save this task's id in the res's control block to indicate that this task is the owner of this res.  */
       osek_res->taskid = (TaskType)(tcb_ptr);

   }   /* End for (index..  */

   /* Now all internal resources are taken, this needs to change task's preemption
      to highest ceiling priority of internal resource occupied.  */

   /* First remove task from its current queue position.  */
   pop_task_from_table(tcb_ptr);

   /* Now need to change this task's preemption threshold to resource's ceiling priority.  */
   /* Assume task's original priority is the highest.  */
   if ((tcb_ptr->resource_scheduler != 0u) || (tcb_ptr->policy == NON))
   {
       tcb_ptr->cur_threshold = OSEK_NON_SCHEDULE_PRIORITY;
   }

   for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
   {
       if (tcb_ptr->internal_resource_occuplied_list[index] == 0u) {
           break;
       }

       res_prio = ((OSEK_RESOURCE *) (tcb_ptr->internal_resource_occuplied_list[index]))->c_priority;
       if  (tcb_ptr->cur_threshold < res_prio )
       {
           tcb_ptr->cur_threshold = res_prio;
       }
   }
   /* Now the task's current threshold reflect the highest of ceiling priority of the remaining
      resources held by this task, move this task to the front of that priority queue.  */
   push_task_to_table(tcb_ptr);

   TX_RESTORE

   return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    release_internal_resource                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call releases any internal resources held by the calling task.  */
/*   It also changes the calling task's priority to reflect either its    */
/*   original priority, NON SCHEDULE priority or maximum ceiling          */
/*   priority of any any external resources held by this task.            */
/*   At the end this task is then moved to a queue appropriate to its new */
/*   ceiling priority.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                           Task id.                               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         Always                                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   pop_task_from_table           Removes a task from its current queue  */
/*   push_task_to_table            Pushes a task at the front of a queue  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Wrapper internal code (not available for application)                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType  release_internal_resource(OSEK_TCB  *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
UINT           index;

    TX_DISABLE

    /* Release any internal resources held?  */
    for (index = 0u; index < OSEK_MAX_INTERNAL_RES; index++)
    {

        if (tcb_ptr->internal_resource_occuplied_list[index] == 0u)
        {
             break;
        }

        ((OSEK_RESOURCE *)(tcb_ptr->internal_resource_occuplied_list[index]))->taskid = 0u;

        tcb_ptr->internal_resource_occuplied_list[index] = 0u;
    }

    /* First remove task from its current queue position.  */
    pop_task_from_table(tcb_ptr);

    /* As all internal resources are released and no external resources are held
       this task must be reverted to its original design time priority.  */

    tcb_ptr->cur_threshold = tcb_ptr->org_prio;

    /* Now move this task to the front of that priority queue.  */
    push_task_to_table(tcb_ptr);

    TX_RESTORE

    return (E_OK);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    check_external_resource                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This call serves to enter critical sections in the code that are     */
/*   assigned to the resource referenced by <ResID>. A critical section   */
/*   must always be left using releaseResource. Nested resource           */
/*   occupation is only allowed if the inner critical sections are        */
/*   completely executed within the surrounding critical section.         */
/*   Nested occupation of one and the same resource is also forbidden.    */
/*   Corresponding calls to GetResource and ReleaseResource should appear */
/*   within the same function on the same function level.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                           Id of the resource.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    E_OK                         If success                             */
/*    E_OS_CALLEVEL                Called from ISR                        */
/*    E_OS_ACCESS                  Attempt to get a resource which is     */
/*                                 already occupied by any task or ISR,   */
/*                                 or the statically assigned priority of */
/*                                 the calling task or interrupt routine  */
/*                                 is higher than the calculated ceiling  */
/*                                 priority.                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    osek_task_independent_area   See if called from task independent    */
/*                                 area                                   */
/*    tx_thread_identify           Identify the current thread            */
/*    osek_internal_error          OSEK internal error                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application Code                                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
static StatusType      check_external_resource(OSEK_TCB *tcb_ptr)
{
TX_INTERRUPT_SAVE_AREA
OSEK_RESOURCE  *osek_res;
OSEK_RESOURCE  *osek_res1;
StatusType      status;
UINT            index;

   /* Check that all assigned external resources for this task are free.  */
   /* Here only availability of resources is checked, no resource will be taken.  */

    TX_DISABLE

   status = E_OK;       /* Assuming that everything is OK.  */
   for (index = 0u; index < OSEK_MAX_EXTERNAL_RES; index++)
   {

       if (tcb_ptr->external_resource_list[index] == 0u)
       {
           break;    /* No External Resource left in the list.  */
       }

       /* Get External RES's control block.  */
       osek_res = (OSEK_RESOURCE *)tcb_ptr->external_resource_list[index];

       /* Now check whether this resource is already occupied by other task.  */
       if (osek_res->taskid != 0u)
       {
           if (osek_res->taskid != (TaskType)tcb_ptr)
           {
               status = E_OS_ACCESS; /* Already occupied by any other task.  */
               break;
           }

       }

       if (osek_res->type == LINKED)
       {

           osek_res1 = (OSEK_RESOURCE *)(osek_res->resolved_res);

           if (osek_res1->taskid != 0u)
               {
               if (osek_res1->taskid != (TaskType)tcb_ptr)
               {
                   status = E_OS_ACCESS; /* Already occupied by any other task.  */
                   break;
               }
           }

       } /* End (osek_res->type == LINKED) */


   }   /* End for.  */

   TX_RESTORE

   return (status);
}


/***************************  END OF FILE ************************************/
