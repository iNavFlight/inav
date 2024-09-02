               Azure RTOS' OSEK compatibility layer for ThreadX

1. Installation

The OSEK compatibility layer for ThreadX is comprised of two files tx_osek.c and os.h
which should be copied at a suitable location for inclusion into a ThreadX project. Refer
to the ThreadX readme file for installation instructions for ThreadX.


2. Building

Building the OSEK layer should be as simple as including the tx_osek.c and os.h file to 
an existing ThreadX project, making sure that the location of the os.h header is in the
include paths. The OSEK layer requires that the ThreadX headers such as tx_api.h are 
reachable in the include paths as well.


3. Initialization

The OSEK layer initialization can be performed either from the tx_application_define() 
during the ThreadX initialization or from a running ThreadX thread. It is strongly 
recommended to initialize and start the OSEK layer as soon as possible. Once started 
other ThreadX tasks and API call should not be used to prevent resource conflicts with 
OSEK.

The OSEK initialization has three phases. First the osek internal initialization which 
must be performed before calling any other OSEK API functions, by calling 
osek_initialize() passing it a pointer to the OSEK memory and the application structure.
The size of the memory region passed to osek_initialize() must be set at compile time
by adjusting the OSEK_MEMORY_SIZE define in osek_uart.h.

After having initialized the OSEK internally, the application can now create OSEK objects 
and link or assigned them as needed. See below for a list of object creation functions.

Finally, after all the objects are created and configured the OSEK layer can be started 
using StartOS(). Once started it is no longer possible to create or change any OSEK 
objects.


4. OSEK shutdown and restart

The OSEK layer can be shutdown using the standard OSEK API ShutdownOS(). As an extension 
to the OSEK layer offers an osek_cleanup() function which can be used to cleanup and 
reset the OSEK layer allowing a subsequent restart without having to reset the CPU. This 
is primarily intended for testing.


5. Hooks

The various hook routines available within OSEK can be set during initialization by 
setting the various handler members of the APPLICATION_INFO structure passed to 
osek_initialize(). See the OSEK documentation for the signature of those hook functions.

For example:

    app.error_hook_handler = ErrorHook;
    app.startup_hook_handler = StartupHook;
    app.shutdown_hook_handler = ShutdownHook;
    app.pretask_hook_handler = PreTaskHook;
    app.posttask_hook_handler = PostTaskHook;


6. Interrupts

As per the OSEK specification, category 1 ISRs are not affected by the OSEK layer 
execution and are not allowed to call any of the OSEK API. Those ISR are configured and
processed just like any other interrupts under ThreadX. Category 2 ISR have to be 
created using CreateISR() as well as being registered and enable like a category 1 ISR.
In the body of the low level ISR process_ISR2() must be called with the return value of
the corresponding CreateISR() in argument. This will instruct the OSEK layer to schedule 
the category 2 ISR as soon as possible.

A category 2 ISR is made of two handlers, the one that process the hardware interrupt 
and the OSEK ISR body.

To define a category 2 ISR body use the standard ISR() macro as follows:


ISRType DemoISR; /* ISR declaration. */

/* ISR body definition. */
ISR(DemoISR)
{
    /* ISR body. */
}


This ISR should be created during system initialization:


DemoISR = CreateISR("Demo ISR", ISREntry(DemoISR), CATEGORY2, 1024);


Once properly initialized the ISR can be triggered by calling process_ISR2 with the ISR
name in argument from within the hardware ISR handler.


void demo_isr_hardware_handler(void)
{
    /* Call OSEK to process the ISR. */
    process_ISR2(DemoISR);
}


7. Implementation specific information

Since OSEK requires a static allocation methodology, the number of available OSEK object 
has to be limited at compile time. By default the following limits apply:

Maximum number of tasks: 32
Maximum number of internal resources: 8
Maximum number of external resources: 16
Maximum number of alarms: 16
Maximum number of counters: 16
Maximum number of events: 32
Maximum number of category 2 ISRs: 8
Minimum OSEK task priority: 0
Maximum OSEK task priority: 23
Maximum alarm counter value: 0x7FFFFFFFUL
Maximum task activation count: 8

8. Supported OSEK API

The ThreadX OSEK layer supports all the mandatory APIs specified in version 2.2.3 of 
the OSEK/VDK Operating System Specification.

Summary of the supported API, see the OSEK specification and tx_osek.c for the full 
details of each API.

TASK MANAGEMENT

DeclareTask
ActivateTask
TerminateTask
ChainTask
Schedule
GetTaskID
GetTaskState

INTERRUPT HANDLING

EnableAllInterrupts
DisableAllInterrupts
ResumeAllInterrupts
SuspendAllInterrupts
ResumeOSInterrupts
SuspendOSInterrupts

RESOURCE MANAGEMENT

DeclareResource
GetResource
ReleaseResource

EVENT CONTROL

DeclareEvent
SetEvent
ClearEvent
GetEvent
WaitEvent

ALARMS

DeclareAlarm
GetAlarmBase
GetAlarm
SetRelAlarm
SetAbsAlarm
CancelAlarm

EXECUTION CONTROL

GetActiveApplicationMode
StartOS
ShutdownOS

Hook Routines

ErrorHook
PreTaskHook
PostTaskHook
StartupHook
ShutdownHook

9. Object creation API

The various object creation and registration functions are as follows. See tx_osek.c for a
detailed description of each function.

----
CreateTask – Creates an OSEK task, the task is returned if successful.

    TaskType CreateTask(CHAR *name, 
                        void(*entry_function)(), 
                        UINT priority, 
                        UINT max_activation, 
                        ULONG stack_size, 
                        SCHEDULE policy, 
                        AUTOSTART start, 
                        UINT type, 
                        AppModeType mode);


----
CreateResource - Creates an OSEK resource, the resource is returned if successful.

    ResourceType CreateResource(const CHAR *name, 
                                StatusType type, 
                                ResourceType linked_res);


----
RegisterTasktoResource - Registers a task to a resource. The resource will be accessible
by the registered task.

    StatusType RegisterTasktoResource(ResourceType Resource, 
                                      TaskType TaskID);


----
CreateEvent - Creates an event, the created event is returned if successful. Note that 
per the OSEK specification an absolute maximum of 32 events can be created.

    EventMaskType CreateEvent(void);


----
RegisterEventtoTask - Register an event to a task. The event is now usable from that 
task. Note that an event can only be registered to a single task.

    StatusType RegisterEventtoTask(EventType eventid, 
                                   TaskType TaskID);


----
CreateISR - Creates an ISR.

    ISRType CreateISR(const CHAR *name, 
                      void(*entry_function)(), 
                      UINT category, 
                      ULONG stack_size);


----
RegisterISRtoResource - Register an ISR to a resource. Note that ISR cannot be registered 
to category 1 ISRS.

    StatusType RegisterISRtoResource(ResourceType Resource, 
                                     ISRType ISRID);


----
CreateCounter - Creates a new counter.

    CounterType CreateCounter(CHAR *name, 
                              TickType max_allowed_value, 
                              TickType ticks_per_base, 
                              TickType min_cycle, 
                              TickType start_value);

----
DefineSystemCounter - Assign a counter to be used as the system counter.

    StatusType DefineSystemCounter(CounterType cntr);

---
CreateAlarm - Creates an alarm.

AlarmType CreateAlarm(CHAR *name, 
                      CounterType cntr, 
                      UINT action, 
                      ULONG events, 
                      TaskType task, 
                      void (*callback)(), 
                      UINT Startup, TickType Alarmtime, 
                      TickType Cycle);
