var rtx5_impl =
[
    [ "Create an RTX5 Project", "cre_rtx_proj.html", [
      [ "Additional requirements for RTX on Cortex-A", "cre_rtx_proj.html#cre_rtx_cortexa", null ],
      [ "Using Interrupts on Cortex-M", "cre_rtx_proj.html#cre_UsingIRQs", null ],
      [ "Add support for RTX specific functions", "cre_rtx_proj.html#cre_rtx_proj_specifics", null ],
      [ "Add Event Recorder Visibility", "cre_rtx_proj.html#cre_rtx_proj_er", null ]
    ] ],
    [ "Theory of Operation", "theory_of_operation.html", [
      [ "System Startup", "theory_of_operation.html#SystemStartup", null ],
      [ "Scheduler", "theory_of_operation.html#Scheduler", null ],
      [ "Memory Allocation", "theory_of_operation.html#MemoryAllocation", [
        [ "Global Memory Pool", "theory_of_operation.html#GlobalMemoryPool", null ],
        [ "Object-specific Memory Pools", "theory_of_operation.html#ObjectMemoryPool", null ],
        [ "Static Object Memory", "theory_of_operation.html#StaticObjectMemory", null ]
      ] ],
      [ "Thread Stack Management", "theory_of_operation.html#ThreadStack", null ],
      [ "Low-Power Operation", "theory_of_operation.html#lowPower", null ],
      [ "RTX Kernel Timer Tick", "theory_of_operation.html#kernelTimer", [
        [ "Tick-less Low-Power Operation", "theory_of_operation.html#TickLess", null ]
      ] ],
      [ "RTX5 Header File", "theory_of_operation.html#rtx_os_h", null ],
      [ "Timeout Value", "theory_of_operation.html#CMSIS_RTOS_TimeOutValue", null ],
      [ "Calls from Interrupt Service Routines", "theory_of_operation.html#CMSIS_RTOS_ISR_Calls", null ],
      [ "SVC Functions", "theory_of_operation.html#CMSIS_RTOS_svcFunctions", null ],
      [ "Arm C library multi-threading protection", "theory_of_operation.html#cre_rtx_proj_clib_arm", null ]
    ] ],
    [ "Configure RTX v5", "config_rtx5.html", [
      [ "System Configuration", "config_rtx5.html#systemConfig", [
        [ "Global Dynamic Memory size [bytes]", "config_rtx5.html#systemConfig_glob_mem", null ],
        [ "Round-Robin Thread Switching", "config_rtx5.html#systemConfig_rr", null ],
        [ "ISR FIFO Queue", "config_rtx5.html#systemConfig_isr_fifo", null ],
        [ "Object Memory Usage Counters", "config_rtx5.html#systemConfig_usage_counters", null ]
      ] ],
      [ "Thread Configuration", "config_rtx5.html#threadConfig", [
        [ "Configuration of Thread Count and Stack Space", "config_rtx5.html#threadConfig_countstack", null ],
        [ "Stack Overflow Checking", "config_rtx5.html#threadConfig_ovfcheck", null ],
        [ "Stack Usage Watermark", "config_rtx5.html#threadConfig_watermark", null ],
        [ "Processor Mode for Thread Execution", "config_rtx5.html#threadConfig_procmode", null ]
      ] ],
      [ "Timer Configuration", "config_rtx5.html#timerConfig", [
        [ "Object-specific memory allocation", "config_rtx5.html#timerConfig_obj", null ],
        [ "User Timer Thread", "config_rtx5.html#timerConfig_user", null ]
      ] ],
      [ "Event Flags Configuration", "config_rtx5.html#eventFlagsConfig", [
        [ "Object-specific memory allocation", "config_rtx5.html#eventFlagsConfig_obj", null ]
      ] ],
      [ "Mutex Configuration", "config_rtx5.html#mutexConfig", [
        [ "Object-specific Memory Allocation", "config_rtx5.html#mutexConfig_obj", null ]
      ] ],
      [ "Semaphore Configuration", "config_rtx5.html#semaphoreConfig", [
        [ "Object-specific memory allocation", "config_rtx5.html#semaphoreConfig_obj", null ]
      ] ],
      [ "Memory Pool Configuration", "config_rtx5.html#memPoolConfig", [
        [ "Object-specific memory allocation", "config_rtx5.html#memPoolConfig_obj", null ]
      ] ],
      [ "Message Queue Configuration", "config_rtx5.html#msgQueueConfig", [
        [ "Object-specific memory allocation", "config_rtx5.html#msgQueueConfig_obj", null ]
      ] ],
      [ "Event Recorder Configuration", "config_rtx5.html#evtrecConfig", [
        [ "Global Configuration", "config_rtx5.html#evtrecConfigGlobIni", null ],
        [ "RTOS Event Generation", "config_rtx5.html#evtrecConfigEvtGen", null ],
        [ "Manual event configuration", "config_rtx5.html#systemConfig_event_recording", null ]
      ] ]
    ] ],
    [ "Building the RTX5 Library", "creating_RTX5_LIB.html", null ],
    [ "Tutorial", "rtos2_tutorial.html", [
      [ "Prerequisites", "rtos2_tutorial.html#rtos2_tutorial_pre", null ],
      [ "First Steps with Keil RTX5", "rtos2_tutorial.html#rtos2_tutorial_first_steps", null ],
      [ "Accessing the CMSIS-RTOS2 API", "rtos2_tutorial.html#rtos2_tutorial_access", null ],
      [ "Threads", "rtos2_tutorial.html#rtos2_tutorial_threads", null ],
      [ "Starting the RTOS", "rtos2_tutorial.html#rtos2_tutorial_start", [
        [ "Exercise 1 - A First CMSIS-RTOS2 Project", "rtos2_tutorial.html#rtos2_tutorial_ex1", null ]
      ] ],
      [ "Creating Threads", "rtos2_tutorial.html#rtos2_tutorial_thread_create", [
        [ "Exercise 2 - Creating and Managing Threads", "rtos2_tutorial.html#rtos2_tutorial_ex2", null ]
      ] ],
      [ "Thread Management and Priority", "rtos2_tutorial.html#rtos2_tutorial_thread_mgmt", null ],
      [ "Memory Management", "rtos2_tutorial.html#rtos2_tutorial_ex2_mem_mgmt", [
        [ "Exercise 3 - Memory Model", "rtos2_tutorial.html#rtos2_tutorial_ex3", null ]
      ] ],
      [ "Multiple Instances", "rtos2_tutorial.html#rtos2_tutorial_multi_inst", [
        [ "Exercise 4 - Multiple Instances", "rtos2_tutorial.html#rtos2_tutorial_multi_inst_ex4", null ]
      ] ],
      [ "Joinable Threads", "rtos2_tutorial.html#rtos2_tutorial_thread_join", [
        [ "Exercise 5 - Joinable Threads", "rtos2_tutorial.html#rtos2_tutorial_ex4", null ]
      ] ],
      [ "Time Management", "rtos2_tutorial.html#rtos2_tutorial_time_mgmt", [
        [ "Time Delay", "rtos2_tutorial.html#rtos2_tutorial_time_delay", null ],
        [ "Absolute Time Delay", "rtos2_tutorial.html#rtos2_tutorial_abs_time_delay", null ],
        [ "Exercise 6 - Time Management", "rtos2_tutorial.html#rtos2_tutorial_ex6", null ],
        [ "Virtual Timers", "rtos2_tutorial.html#rtos2_tutorial_virtual_timers", null ],
        [ "Exercise 7 - Virtual Timer", "rtos2_tutorial.html#rtos2_tutorial_ex7", null ],
        [ "Idle Thread", "rtos2_tutorial.html#rtos2_tutorial_idle_thread", null ],
        [ "Exercise 8 - Idle Thread", "rtos2_tutorial.html#rtos2_tutorial_ex8", null ]
      ] ],
      [ "Inter-thread Communication", "rtos2_tutorial.html#rtos2_tutorial_interthread_com", [
        [ "Thread Flags", "rtos2_tutorial.html#rtos2_tutorial_thread_flags", [
          [ "Exercise 9 - Thread Flags", "rtos2_tutorial.html#rtos2_tutorial_ex9", null ]
        ] ],
        [ "Event Flags", "rtos2_tutorial.html#rtos2_tutorial_event_flags", [
          [ "Exercise 10 - Event Flags", "rtos2_tutorial.html#rtos2_tutorial_ex10", null ]
        ] ],
        [ "Semaphores", "rtos2_tutorial.html#rtos2_tutorial_semaphores", [
          [ "Using Semaphores", "rtos2_tutorial.html#rtos2_tutorial_sem_usage", null ],
          [ "Signalling", "rtos2_tutorial.html#rtos2_tutorial_sem_sig", null ],
          [ "Exercise 11 - Semaphore Signalling", "rtos2_tutorial.html#rtos2_tutorial_ex11", null ],
          [ "Multiplex", "rtos2_tutorial.html#rtos2_tutorial_sem_multi", null ],
          [ "Exercise 12 - Multiplex", "rtos2_tutorial.html#rtos2_tutorial_ex12", null ],
          [ "Rendezvous", "rtos2_tutorial.html#rtos2_tutorial_sem_rend", null ],
          [ "Exercise 13 - Rendezvous", "rtos2_tutorial.html#rtos2_tutorial_sem_rend_ex13", null ],
          [ "Barrier Turnstile", "rtos2_tutorial.html#rtos2_tutorial_sem_barr_turn", null ],
          [ "Exercise 14 - Semaphore Barrier", "rtos2_tutorial.html#rtos2_tutorial_ex14", null ],
          [ "Semaphore Caveats", "rtos2_tutorial.html#rtos2_tutorial_sem_caveats", null ]
        ] ],
        [ "Mutex", "rtos2_tutorial.html#rtos2_tutorial_mutex", [
          [ "Exercise 15 - Mutex", "rtos2_tutorial.html#rtos2_tutorial_ex15", null ],
          [ "Mutex Caveats", "rtos2_tutorial.html#rtos2_tutorial_mutex_caveats", null ]
        ] ],
        [ "Data Exchange", "rtos2_tutorial.html#rtos2_tutorial_data_exchange", [
          [ "Message Queue", "rtos2_tutorial.html#rtos2_tutorial_msg_queue", null ],
          [ "Exercise 16 - Message Queue", "rtos2_tutorial.html#rtos2_tutorial_ex16", null ],
          [ "Extended Message Queue", "rtos2_tutorial.html#rtos2_tutorial_ext_msg_queue", null ],
          [ "Exercise 17 - Message Queue", "rtos2_tutorial.html#rtos2_tutorial_ex17", null ],
          [ "Memory Pool", "rtos2_tutorial.html#rtos2_tutorial_mem_pool", null ],
          [ "Exercise 18 - Zero Copy Mailbox", "rtos2_tutorial.html#rtos2_tutorial_ex18", null ]
        ] ]
      ] ],
      [ "Configuration", "rtos2_tutorial.html#rtos2_tutorial_config", [
        [ "System Configuration", "rtos2_tutorial.html#rtos2_tutorial_config_sys", null ],
        [ "Thread Configuration", "rtos2_tutorial.html#rtos2_tutorial_config_thread", null ],
        [ "System Timer Configuration", "rtos2_tutorial.html#rtos2_tutorial_config_sys_timer", null ]
      ] ],
      [ "Conclusion", "rtos2_tutorial.html#rtos2_tutorial_conclusion", null ]
    ] ],
    [ "Technical Data", "technicalData5.html", "technicalData5" ],
    [ "MISRA C:2012 Compliance", "misraCompliance5.html", [
      [ "[MISRA Note 1]: Return statements for parameter checking", "misraCompliance5.html#MISRA_1", null ],
      [ "[MISRA Note 2]: Object identifiers are void pointers", "misraCompliance5.html#MISRA_2", null ],
      [ "[MISRA Note 3]: Conversion to unified object control blocks", "misraCompliance5.html#MISRA_3", null ],
      [ "[MISRA Note 4]: Conversion from unified object control blocks", "misraCompliance5.html#MISRA_4", null ],
      [ "[MISRA Note 5]: Conversion to object types", "misraCompliance5.html#MISRA_5", null ],
      [ "[MISRA Note 6]: Conversion from user provided storage", "misraCompliance5.html#MISRA_6", null ],
      [ "[MISRA Note 7]: Check for proper pointer alignment", "misraCompliance5.html#MISRA_7", null ],
      [ "[MISRA Note 8]: Memory allocation management", "misraCompliance5.html#MISRA_8", null ],
      [ "[MISRA Note 9]: Pointer conversions for register access", "misraCompliance5.html#MISRA_9", null ],
      [ "[MISRA Note 10]: SVC calls use function-like macros", "misraCompliance5.html#MISRA_10", null ],
      [ "[MISRA Note 11]: SVC calls use assembly code", "misraCompliance5.html#MISRA_11", null ],
      [ "[MISRA Note 12]: Usage of exclusive access instructions", "misraCompliance5.html#MISRA_12", null ],
      [ "[MISRA Note 13]: Usage of Event Recorder", "misraCompliance5.html#MISRA_13", null ]
    ] ]
];