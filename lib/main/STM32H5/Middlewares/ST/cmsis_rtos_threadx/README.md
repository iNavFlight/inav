# CMSIS-RTOS v2 wrapper implementation

The CMSIS-RTOS v2 (CMSIS-RTOS2) provides generic RTOS interfaces for Arm® Cortex® processor-based devices. It provides a standardized API for software components that require RTOS functionality and gives therefore serious benefits to the users and the software industry:

 - CMSIS-RTOS2 provides basic features that are required in many applications.
 - The unified feature set of the CMSIS-RTOS2 reduces learning efforts and simplifies sharing of software components.
 - Middleware components that use the CMSIS-RTOS2 are RTOS agnostic and are easier to adapt.
 - Standard project templates of the CMSIS-RTOS2 may be shipped with freely available CMSIS-RTOS2 implementations.

This CMSIS-RTOS v2 represents a wrapper layer for CMSIS RTOS v2 APIs implementation based on threadX RTOS APIs.
The list of features supported by the current implementation are as below:

| Feature                       |  Supported |       Short Description      |
|-------------------------------|------------|------------------------------|
|Kernel Information and Control |      Y     | It provide version/system information and starts/controls the RTOS Kernel. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html) |
|Thread Management              |      Y     | It define, create, and control thread functions.. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html) |
|Thread Flags                   |      N     | It synchronize threads using flags. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadFlagsMgmt.html) |
|Event Flags                    |      Y     | It synchronize threads using event flags. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html) |
|Generic Wait Functions         |      Y     | It wait for a certain period of time..  [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Wait.html) |
|Timer Management               |      Y     | It create and control timer and timer callback functions.  [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html) |
|Mutex Management               |      Y     | It synchronize resource access using Mutual Exclusion (Mutex). [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html) |
|Semaphores                     |      Y     | It access shared resources simultaneously from different threads. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html) |
|Memory Pool                    |      Y     | It manage thread-safe fixed-size blocks of dynamic memory. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html) |
|Message Queue                  |      Y     | It exchange messages between threads in a FIFO-like operation. [More...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html) |

For more information about CMSIS-RTOS v2 APIs, please refer to the ARM manual: [CMSIS-RTOS API v2](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS.html)

# CMSIS-RTOS v2 design

### Kernel Initialize and Start

In threadX RTOS only one interface is used to start the kernel (tx_kernel_enter). This function will:
 - _tx_initialize_low_level: invoke the low-level initialization to handle all processor specific initialization issues
 - _tx_initialize_high_level: invoke the high-level initialization to exercise all of the ThreadX components and the application's initialization function
 - tx_application_define: call the application provided initialization function.  Pass the first available memory address to it
 - _tx_thread_schedule: enter the scheduling loop to start executing threads.

For ARM CMSIS solution, its mandatory to separate the kernel initialization from the kernel start to allow the user to create threads, timers, semaphores,... in between.
For that we design the CMSIS-RTOS2 wrapper as below:
 - osKernelInitialize: will initialize the low level and high level layers by calling the function "_tx_initialize_kernel_setup"
 - osKernelGetState: will call the application provided initialization function start the kernel.

### Dynamic Memory Management

CMSIS-RTOS v2 APIs such as osThreadNew, give the possibilities to the user, when implementing his application, to:
 - Pass the block memory and stack address (declared or allocated in the application level)
 - Let the low layers (wrapper or RTOS) allocate the memory

However, the dynamic memory allocation is not a supported feature for threadX RTOS. In fact, its mandatory to implement it in CMSIS RTOS v2 wrapper level.

The global idea for dynamic memory allocation solution is to use two threadX BytePools:
 - HeapBytePool: used for thread, timer, mutex, semaphore, event flags and message queue object block memory allocation
 - StackBytePool: used for thread and message queue stack memory allocation.

In fact, three internal functions are added as following:

| Function Name |       Short Description      |
|---------------|------------------------------|
|MemInit        | creates the HeapBytePool and StackBytePool BytePools |
|MemAlloc       | allocate the needed memory for object block or stack | 
|MemFree        | free the memory for object block or stack            |

**Notes:**
 - The sizes of HeapBytePool and StackBytePool are user defined using respectively the macro defines RTOS2_BYTE_POOL_HEAP_SIZE and RTOS2_BYTE_POOL_STACK_SIZE
 - The minimum size of HeapBytePool and StackBytePool is defined by the threadX macro define TX_BYTE_POOL_MIN
 - The HeapBytePool and StackBytePool are allocated from the first free memory area defined by the threadX variable _tx_initialize_unused_memory.

### Static Memory Management

CMSIS-RTOS v2 gives the possibilities to the user, when implementing his application, to statically allocate the memory.
In fact, static buffers will be created and allocated based on user defines RTOS2_BYTE_POOL_HEAP_SIZE and RTOS2_BYTE_POOL_STACK_SIZE.
The minimum size of HeapBytePool and StackBytePool is defined by the threadX macro define TX_BYTE_POOL_MIN.
 
# CMSIS-RTOS v2 Modules description

### Kernel Information and Control

The kernel Information and Control function group allows to:

 - Obtain information about the system and the underlying kernel.
 - Obtain version information about the CMSIS-RTOS API.
 - Initialize of the RTOS kernel for creating objects.
 - Start the RTOS kernel and thread switching.
 - Check the execution status of the RTOS kernel.

| API Name                | Supported |       Short Description      |       Limitation             |
|-------------------------|-----------|------------------------------|------------------------------|
|osKernelInitialize       |     Y     | [Kernel Initialize...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#gae818f6611d25ba3140bede410a52d659)         | No limitations |
|osKernelGetInfo          |     Y     | [Kernel Get Info...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga6f7764e7250c5c5364c00c45a5d1d199)           | No limitations |
|osKernelGetState         |     Y     | [Kernel Get State...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga48b69b81012fce051f639be288b243ba)          | Only osKernelInactive, osKernelReady and osKernelRunning states are supported |
|osKernelStart            |     Y     | [Kernel Start...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga9ae2cc00f0d89d7b6a307bba942b5221)              | No limitations |
|osKernelLock             |     N     | [Kernel Lock...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga948609ee930d9b38336b9e1c2a4dfe12)               | This API is not supported due to threadX limitation |
|osKernelUnlock           |     N     | [Kernel Unlock...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#gaf401728b4657456198c33fe75f8d6720)             | This API is not supported due to threadX limitation |
|osKernelRestoreLock      |     N     | [Kernel Restore Lock...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#gae7d0a71b9586cbbb49fcbdf6a04f0289)       | This API is not supported due to threadX limitation |
|osKernelSuspend          |     N     | [Kernel Suspend...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#gae26683e1606ec633354a2876c68f0c1f)            | This API is not supported due to threadX limitation |
|osKernelResume           |     N     | [Kernel Resume...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga8c4b4d7ed34cab73c001665d9176aced)             | This API is not supported due to threadX limitation |
|osKernelGetTickCount     |     Y     | [Kernel Get Tick Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga84bcdbf2fb76b10c8df4e439f0c7e11b)     | No limitations |
|osKernelGetTickFreq      |     Y     | [Kernel Get Tick Freq...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga7a8d7bd927eaaa58999f91d7d6310cee)      | No limitations |
|osKernelGetSysTimerCount |     Y     | [Kernel Get SysTimer Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#gae0fcaff6cecfb4013bb556c87afcd7d2) | No limitations |
|osKernelGetSysTimerFreq  |     Y     | [Kernel Get SysTimer Freq...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__KernelCtrl.html#ga4d69215a93220f72be3684cad582f16a)  | No limitations |

**Notes:**
 - Due to threadX RTOS limitation (no lock or suspend feature are supported), all kernel lock, suspend and resume APIs are not supported.

### Thread Management

The Thread Management function group allows defining, creating, and controlling thread functions in the system.

| API Name             | Supported |       Short Description      |       Limitation             |
|----------------------|-----------|------------------------------|------------------------------|
|osThreadNew           |     Y     | [Thread New...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga48d68b8666d99d28fa646ee1d2182b8f)            | If argument is given as input this will be considered as entry_input for threadX |
|osThreadGetName       |     Y     | [Thread Get Name...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gac3230f3a55a297514b013ebf38f27e0a)       | No limitations |
|osThreadGetId         |     Y     | [Thread Get Id...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga8df03548e89fbc56402a5cd584a505da)         | No limitations |
|osThreadGetState      |     Y     | [Thread Get State...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gacc0a98b42f0a5928e12dc91dc76866b9)      | Only osThreadRunning, osThreadReady, osThreadTerminated and osThreadBlocked thread states are supported |
|osThreadSetPriority   |     Y     | [Thread Set Priority...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga861a420fb2d643115b06622903fb3bfb)   | No limitations |
|osThreadGetPriority   |     Y     | [Thread Get Priority...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga0aeaf349604f456e68e78f9d3b42e44b)   | No limitations |
|osThreadYield         |     Y     | [Thread Yield...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gad01c7ec26535b1de6b018bb9466720e2)          | No limitations |
|osThreadSuspend       |     Y     | [Thread Suspend...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gaa9de419d0152bf77e9bbcd1f369fb990)        | No limitations |
|osThreadResume        |     Y     | [Thread Resume...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga3dbad90eff394b02de76a452c84c5d80)         | No limitations |
|osThreadDetach        |     Y     | [Thread Detach...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gaaad14cd9547341ea8109dc4e8540f1dc)         | TX_THREAD_USER_EXTENSION must be defined |
|osThreadJoin          |     Y     | [Thread Join...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga3fca90fb0679afeb968aa8c3d5874487)           | TX_THREAD_USER_EXTENSION must be defined |
|osThreadExit          |     Y     | [Thread Exit...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gaddaa452dd7610e4096647a566d3556fc)           | No limitations |
|osThreadTerminate     |     Y     | [Thread Terminate...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gaddaa452dd7610e4096647a566d3556fc)      | No limitations |
|osThreadGetStackSize  |     Y     | [Thread Get Stack Size...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gab9f8bd715d671c6ee27644867bc1bf65) | No limitations |
|osThreadGetStackSpace |     Y     | [Thread Get Stack Space...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga9c83bd5dd8de329701775d6ef7012720)| No limitations |
|osThreadGetCount      |     Y     | [Thread Get Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga495b3f812224e7301f23a691793765db)      | No limitations |
|osThreadEnumerate     |     Y     | [Thread Enumerate...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga5606604d56e21ece1a654664be877439)      | No limitations |

**Notes:**
 - Thread management functions cannot be called from Interrupt Service Routines.
 - To support osThreadDetach and osThreadJoin APIs, the TX_THREAD_USER_EXTENSION must be defined as tx_thread_detached_joinable (ULONG) in tx_user.h file

### Thread Flags

Thread Flags are a more specialized version of the Event Flags. See Event Flags. While Event Flags can be used to globally signal a number of threads, thread flags are only send to a single specific thread. Every thread instance can receive thread flags without any additional allocation of a thread flags object.

| API Name          | Supported |       Short Description      |       Limitation             |
|-------------------|-----------|------------------------------|------------------------------|
|osThreadFlagsSet   |     N     | [Thread Flags Set...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadFlagsMgmt.html#ga6f89ef9caded1d9963c7b12b0f6412c9)   | Not yet implemented |
|osThreadFlagsClear |     N     | [Thread Flags Clear...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadFlagsMgmt.html#ga656abc1c862c5b9a2b13584c42cc0bfa) | Not yet implemented |
|osThreadFlagsGet   |     N     | [Thread Flags Get...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadFlagsMgmt.html#ga85c8d2c89466e25abbcb545d9ddd71ba)   | Not yet implemented |
|osThreadFlagsWait  |     N     | [Thread Flags Wait...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadFlagsMgmt.html#gac11542ad6300b600f872fc96e340ec2b)  | Not yet implemented |
                    
**Notes:**
 - Thread flag management functions cannot be called from Interrupt Service Routines, except for osThreadFlagsSet.
 - The Thread flag management functions are not supported in the current CMSIS RTOS v2 implementation and will be implemented in the future version.

### Event Flags

The event flags management functions in CMSIS-RTOS allow you to control or wait for event flags. Each signal has up to 31 event flags.

A thread :
 - Can wait for event flags to be set (using osEventFlagsWait). Using this function, it enters the BLOCKED state.
 - May set one or more flags in any other given thread (using osEventFlagsSet).
 - May clear its own signals or the signals of other threads (using osEventFlagsClear).
 - When a thread wakes up and resumes execution, its signal flags are automatically cleared (unless event flags option osFlagsNoClear is specified).

| API Name           | Supported |       Short Description      |       Limitation             |
|--------------------|-----------|------------------------------|------------------------------|
|osEventFlagsNew     |     Y     | [Thread Flags Set...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#gab14b1caeb12ffa42cce1bfe889cd07df)   | No limitations |
|osEventFlagsSet     |     Y     | [Thread Flags Clear...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#ga33b71d14cecf90b4e72639dd19f23a5e) | No limitations |
|osEventFlagsClear   |     Y     | [Thread Flags Get...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#ga93bf258ca0007c6641fbe8e4f2b8a1e5)   | No limitations |
|osEventFlagsGet     |     Y     | [Thread Flags Wait...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#ga8bda3185f46bfd278cea8a6cf357677d)  | No limitations |
|osEventFlagsWait    |     Y     | [Thread Flags Clear...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#ga52acb34a8322e58020227344fe662b4e) | No limitations |
|osEventFlagsDelete  |     Y     | [Thread Flags Get...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#ga7c4acf2fb0d506ec82905dee53fb5435)   | No limitations |
|osEventFlagsGetName |     Y     | [Thread Flags Wait...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__EventFlags.html#ga59f4ddf0ee8c395b1672bb978d1cfc88   | No limitations |

**Notes:**
 - The functions osEventFlagsSet, osEventFlagsClear, osEventFlagsGet, and osEventFlagsWait can be called from Interrupt Service Routines
 - If a thread is blocked on waiting an event flag, the osEventFlagsSet will unblock it.

### Generic Wait Functions

The generic wait functions provide means for a time delay.

| API Name    | Supported |       Short Description      |       Limitation             |
|-------------|-----------|------------------------------|------------------------------|
|osDelay      |     Y     | [Delay...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Wait.html#gaf6055a51390ef65b6b6edc28bf47322e)       | No limitations |
|osDelayUntil |     Y     | [Delay Until...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Wait.html#ga3c807924c2d6d43bc2ffb49da3f7f3a1) | No limitations |

**Notes:**
 - Generic wait functions cannot be called from Interrupt Service Routines.

### Timer Management

In addition to the Generic Wait Functions CMSIS-RTOS also supports virtual timer objects. These timer objects can trigger the execution of a function (not threads).
When a timer expires, a callback function is executed to run associated code with the timer. Each timer can be configured as a one-shot or a periodic timer.
A periodic timer repeats its operation until it is deleted or stopped. All timers can be started, restarted, or stopped.

| API Name        | Supported |       Short Description      |       Limitation             |
|-----------------|-----------|------------------------------|------------------------------|
|osTimerNew       |     Y     | [Timer New...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#gad4e7f785c5f700a509f55a3bf6a62bec)        | No limitations |
|osTimerGetName   |     Y     | [Timer Get Name...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#ga4f82a98eee4d9ea79507e44340d3d319)   | No limitations |
|osTimerStart     |     Y     | [Timer Start...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#gab6ee2859ea657641b7adfac599b8121d)      | No limitations |
|osTimerStop      |     Y     | [Timer Stop...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#gabd7a89356da7717293eb0bc5d87b8ac9)       | No limitations |
|osTimerIsRunning |     Y     | [Timer Is Running...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#ga69d3589f54194022c30dd01e45ec6741) | No limitations |
|osTimerDelete    |     Y     | [Timer Delete...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#gad0001dd74721ab461789324806db2453)     | No limitations |

**Notes:**
 - Timer management functions cannot be called from Interrupt Service Routines.

### Mutex Management

Mutual exclusion (widely known as Mutex) is used in various operating systems for resource management.
Many resources in a microcontroller device can be used repeatedly, but only by one thread at a time (for example communication channels, memory, and files).
Mutexes are used to protect access to a shared resource. A mutex is created and then passed between the threads (they can acquire and release the mutex).

| API Name        | Supported |       Short Description      |       Limitation             |
|-----------------|-----------|------------------------------|------------------------------|
|osMutexNew       |     Y     | [Mutex New...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html#gad4e7f785c5f700a509f55a3bf6a62bec)       | osMutexRobust and osMutexRecursive mutex types are not supported |
|osMutexGetName   |     Y     | [Mutex Get Name...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html#ga00b5e58cd247a412d1afd18732d8b752)  | No limitations |
|osMutexAcquire   |     Y     | [Mutex Acquire...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html#gabc54686ea0fc281823b1763422d2a924)   | No limitations |
|osMutexRelease   |     Y     | [Mutex Release...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html#gaea629705703580ff58776bf73c8db915)   | No limitations |
|osMutexGetOwner  |     Y     | [Mutex Get Owner...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html#ga7f9a7666df0978738cd570cb700b83fb) | No limitations |
|osMutexDelete    |     Y     | [Mutex Delete...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__MutexMgmt.html#gabee73ad227ba4587d3db12ef9bd582bc)    | No limitations |

**Notes:**
 - Mutex management functions cannot be called from Interrupt Service Routines (ISR), unlike a binary semaphore that can be released from an ISR.
 - If a thread is blocked on acquiring or releasing the mutex, the osMutexAcquire and osMutexRelease will unblock it. Therefore, acquiring or releasing the mutex will not change the mutex status except from the number of thread blocked on it.

### Semaphores

Semaphores are used to manage and protect access to shared resources. Semaphores are very similar to Mutexes.
Whereas a Mutex permits just one thread to access a shared resource at a time, a semaphore can be used to permit a fixed number of threads/ISRs to access a pool of shared resources.
Using semaphores, access to a group of identical peripherals can be managed (for example multiple DMA channels).

| API Name           | Supported |       Short Description      |       Limitation             |
|--------------------|-----------|------------------------------|------------------------------|
|osSemaphoreNew      |     Y     | [Semaphore New...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga2a39806ace781a0008a4374ca701b14a)       | The parameter max_count is not supported |
|osSemaphoreGetName  |     Y     | [Semaphore Get Name...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga9586952051f00285f1482dbe6695bbc4)  | No limitations |
|osSemaphoreAcquire  |     Y     | [Semaphore Acquire...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga7e94c8b242a0c81f2cc79ec22895c87b)   | No limitations |
|osSemaphoreRelease  |     Y     | [Semaphore Release...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga0abcee1b5449d7a6928fb9248c690bb6)   | No limitations |
|osSemaphoreGetCount |     N     | [Semaphore Get Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga7559d4dff3cda9992fc5ab5de3e74c70) | This API is not supported due to max_count limitation |
|osSemaphoreDelete   |     Y     | [Semaphore Delete...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga81258ce9c67fa89f07cc49d2e136cd88)    | No limitations |

**Notes:**
 - The functions osSemaphoreAcquire, osSemaphoreGetCount, and osSemaphoreRelease can be called from Interrupt Service Routines.
 - If a thread is blocked on acquiring or releasing the semaphore, the osSemaphoreAcquire and osSemaphoreRelease will unblock it. Therefore, acquiring or releasing the semaphore will not change the semaphore status except from the number of thread blocked on it
 - Due to max_count limitation, the binary semaphore is not supported.

### Memory Pool

Memory Pools are fixed-size blocks of memory that are thread-safe. They operate much faster than the dynamically allocated heap and do not suffer from fragmentation. Being thread-safe, they can be accessed from threads and ISRs alike.
A Memory Pool can be seen as a linked list of available (unused) memory blocks of fixed and equal size.
Allocating memory from a pool (using osMemoryPoolAlloc) simply unchains a block from the list and hands over control to the user.
Freeing memory to the pool (using osMemoryPoolFree) simply rechains the block into the list.

| API Name                | Supported |       Short Description      |       Limitation             |
|-------------------------|-----------|------------------------------|------------------------------|
|osMemoryPoolNew          |     Y     | [MemoryPool New...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#ga497ced5d72dc5cd405c4c418516220dc)            | No limitations |
|osMemoryPoolGetName      |     Y     | [MemoryPool Get Name...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#gab414a1e138205a55820acfa277c8f386)       | No limitations |
|osMemoryPoolAlloc        |     Y     | [MemoryPool Alloc...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#ga8ead54e99ccb8f112356c88f99d38fbe)          | No limitations |
|osMemoryPoolFree         |     Y     | [MemoryPool Free...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#gabb4f4560daa6d1f8c8789082ee186d16)           | No limitations |
|osMemoryPoolGetCapacity  |     Y     | [MemoryPool Get Capacity...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#gad696e94bfbe28f0b6613f9303fdf6a37)   | No limitations |
|osMemoryPoolGetBlockSize |     Y     | [MemoryPool Get Block Size...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#gab2bf059b7fa7679c3cccdaeec60b6c0e) | No limitations |
|osMemoryPoolGetCount     |     Y     | [MemoryPool Get Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#ga958a9449bff8c95ce213de98eef5739d)      | No limitations |
|osMemoryPoolGetSpace     |     Y     | [MemoryPool Get Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#ga0394cffa9479a7994e3b03c79c1cb909)      | No limitations |
|osMemoryPoolDelete       |     Y     | [MemoryPool Delete...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__PoolMgmt.html#ga8c39e7e5cd2b9eda907466808e59d62e)         | No limitations |

**Notes:**
 - The functions osMemoryPoolAlloc, osMemoryPoolFree, osMemoryPoolGetCapacity, osMemoryPoolGetBlockSize, osMemoryPoolGetCount, osMemoryPoolGetSpace can be called from Interrupt Service Routines.

### Message Queue

Message passing is another basic communication model between threads.
In the message passing model, one thread sends data explicitly, while another thread receives it.
The operation is more like some kind of I/O rather than a direct access to information to be shared. In CMSIS-RTOS, this mechanism is called s message queue.
The data is passed from one thread to another in a FIFO-like operation. Using message queue functions, you can control, send, receive, or wait for messages. The data to be passed can be of integer or pointer type.

| API Name                 | Supported |       Short Description      |       Limitation             |
|--------------------------|-----------|------------------------------|------------------------------|
|osMessageQueueNew         |     Y     | [MessageQueue New...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#ga24e895a00f9d484db33aaf784c57bfed)          | No limitations |
|osMessageQueueGetName     |     Y     | [MessageQueue Get Name...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gae7cf7bf2b97a5ae481fb60fcce99247a)     | No limitations |
|osMessageQueuePut         |     Y     | [MessageQueue Put...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gaa515fc8b956f721a8f72b2c505813bfc)          | Message priority is not supported |
|osMessageQueueGet         |     Y     | [MessageQueue Get...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gad90d4959466a7a65105061da8256ab9e)          | Message priority is not supported |
|osMessageQueueGetCapacity |     Y     | [MessageQueue Get Capacity...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gac24f87d4f395e9e9c900c320e45ade8a) | No limitations |
|osMessageQueueGetMsgSize  |     Y     | [MessageQueue Get Msg Size...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#ga96d3d84069b20359de48109e28a1a89e) | No limitations |
|osMessageQueueGetCount    |     Y     | [MessageQueue Get Count...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#ga6a32ac394fcff568b251c160cc3014b2)    | No limitations |
|osMessageQueueGetSpace    |     Y     | [MessageQueue Get Space...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gaddf0904427436dd3880d46263c2dc9fa)    | No limitations |
|osMessageQueueReset       |     Y     | [MessageQueue Reset...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gac6dce7f9ad132d266292c2e979d861b4)        | No limitations |
|osMessageQueueDelete      |     Y     | [MessageQueue Delete...](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__Message.html#gaba987f665444e0d83fa6a3a68bc72abe)       | No limitations |


**Notes:**
 - The functions osMessageQueuePut, osMessageQueueGet, osMessageQueueGetCapacity, osMessageQueueGetMsgSize, osMessageQueueGetCount, osMessageQueueGetSpace can be called from Interrupt Service Routines.
 - If a thread is blocked on getting or putting a message on the queue, the osMessageQueuePut and osMessageQueueGet will unblock it. Therefore, putting or getting a message will not change the queue status except from the number of thread blocked on it.