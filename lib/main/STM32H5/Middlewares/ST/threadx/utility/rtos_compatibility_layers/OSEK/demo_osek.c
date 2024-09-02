
#include <os.h>

/************************************************/
/*    Azure RTOS Implementation Specific        */
/************************************************/

/* Osek application definition.  */
APPLICATION_INFO    Application1;

/* Task definition.  */
TaskType      Task1;

/* Alarm definition.  */
AlarmType     Alarm1;

/* Resource definition.  */
ResourceType  Resource1;

/* Event definition.  */
EventMaskType       Event1;

/* Counter definition.  */
CounterType       SystemTimer;

/* Demo ISR definition.  */
ISRType DemoISR;

/* Task body declaration.  */
DeclareTask(Task1);

/* Demo ISR body declaration.  */
DeclareISR(DemoISR);

/* User hooks declarations.  */
static void ShutdownHook(StatusType Error);

static void PreTaskHook(void);

static void PostTaskHook(void);

static void StartupHook(void);

static void ErrorHook(StatusType Error);


/* ThreadX timer for demo ISR.  */
TX_TIMER demo_isr_timer;

/* Entry function for the ThreadX timer.  */
VOID demo_isr_timer_entry(ULONG arg);


/* Main function.  */  
int main()
{

    tx_kernel_enter();

}
ULONG free_memory[64*1024 / sizeof(ULONG)];

VOID tx_application_define(VOID * first_unused_memory)
{

CHAR    * pointer;

    /* Put the first available address into character pointer.  */
    pointer = (CHAR * )free_memory;
     
    /* Setup hook pointers (optional). */  
    Application1.shutdown_hook_handler = ShutdownHook;
    Application1.pretask_hook_handler = PreTaskHook;
    Application1.posttask_hook_handler = PostTaskHook;
    Application1.startup_hook_handler = StartupHook;
    Application1.error_hook_handler = ErrorHook;

    /* Initialize a pointer.  */
    osek_initialize(pointer,&Application1);
    
    /* Create the system counter  */
    SystemTimer = CreateCounter("SystemTimer", 0x7FFFFFFF, 2, 2, 0);
    DefineSystemCounter(SystemTimer);

    /* Create the first Task.  */
    Task1 = CreateTask("Task1", TaskEntry(Task1), 3, 1, 1024, NON, TRUE, EXTENDED, 0);

    /* Create an event. */
    Event1 = CreateEvent();
    
    /* Register Event1 to Task1.  */
    RegisterEventtoTask(Event1 , Task1);

    /* Create a resource.  */
    Resource1 = CreateResource("Resource1", STANDARD, 0);
    
    /* Register Resource1 to Task1.  */
    RegisterTasktoResource(Resource1, Task1);

    /* Create a demo ISR triggered by a ThreadX timer. */
    DemoISR = CreateISR("Demo ISR", ISREntry(DemoISR), CATEGORY2, 1024);

    /* Create a ThreadX timer to simulate an ISR.  */
    tx_timer_create(&demo_isr_timer, "Demo ISR timer", demo_isr_timer_entry, DemoISR,
                     1000, 1000, TX_AUTO_ACTIVATE);
    
    /* Start OSEK  */
    StartOS(OSDEFAULTAPPMODE);
 
}

/* Task body.  */
TASK(Task1)
{
    /* Task body. */
    while(1)
    {
    }
}

/* Demo category 2 ISR function body.  */
ISR(DemoISR)
{
    /* ISR body. */
}

static void ShutdownHook(StatusType Error)
{
    /* Hook body.  */
}

static void PreTaskHook(void)
{
    /* Hook body.  */
}

static void PostTaskHook(void)
{
    /* Hook body.  */
}

static void StartupHook(void)
{
    /* Hook body.  */
}

static void ErrorHook(StatusType Error)
{
    /* Hook body.  */
}

/* ThreadX timer handler to simulate an ISR.  */
VOID demo_isr_timer_entry(ULONG arg)
{
    /* Call OSEK to process the ISR.  */
    process_ISR2(arg);
}
