---
title: Low Power Utility
author: sclarson
ms.author: sclarson
description: This article is a summary of the Low Power utility APIs available in ThreadX.
ms.date: 03/02/2021
ms.topic: article
ms.service: rtos
---

# ThreadX Low Power Utilities

These low power utilities are intended to maintain ThreadX timers and the internal tick count while the processor is in a low-power/sleep state. The terms "low power" and "sleep" are used interchangeably throughout this document.

The low power utilities may be used with any ThreadX port except SMP.

## Installation of Low Power Utilities

The ThreadX low power utilities are comprised of two files:

 - [tx_low_power.c](tx_low_power.c)
 - [tx_low_power.h](tx_low_power.h)

These files can be built with the ThreadX library or built in the user application.

## Detailed Description

By default, ThreadX spins in an idle loop in the scheduler when there are no threads ready to run. It may be desireable to put the processor in a low power state while idle. Functions  ```tx_low_power_enter``` and ```tx_low_power_exit``` are designed to enter and exit low power mode, respectively.

The ```tx_low_power_enter``` and ```tx_low_power_exit``` functions manage the ThreadX timers and invoke optional macros that the user must define in order to configure the hardware for low power mode. These macros are discussed in a section below.

For convenience, the Cortex-M and RX ports have calls to the low power APIs already integrated into their schedulers. If symbol **TX_LOW_POWER** is defined, functions ```tx_low_power_enter``` and ```tx_low_power_exit``` are called in the scheduler idle loop. It is assumed that the processor will exit sleep mode and resume execution in the idle loop (or in an interrupt and then return to the idle loop). For processors that exhibit different behavior (such as waking from sleep at the reset vector), these low power functions may need to be called elsewhere.

### Low Power Timer and Tick-less Low Power Mode

If accurate timekeeping is not desired while in low power mode, macros **TX_LOW_POWER_TIMER_SETUP** and **TX_LOW_POWER_TICKLESS** do not need to be defined. The internal ThreadX tick count will not reflect the time spent in low power mode.

If **TX_LOW_POWER_TIMER_SETUP** is defined, there are two use cases for timekeeping in low power operation:
1. Keep track of elapsed time in low power mode only when ThreadX timers are active. If there are ThreadX timers active, the elapsed time while in low power mode must be measured, thus a low power hardware timer is required (this timer can be configured in **TX_LOW_POWER_TIMER_SETUP**). If there are no ThreadX timers active when entering low power mode, then no hardware timer needs to keep track of elapsed time. This is "tick-less" operation. To enable this feature, the symbol **TX_LOW_POWER_TICKLESS** must be defined. The internal ThreadX tick count will not reflect the time spent in low power mode when no ThreadX timers are active.
2. Always keep track of time in low power mode. This is necessary to keep the internal ThreadX tick count accurate. In this case, **TX_LOW_POWER_TICKLESS** must *not* be defined, as a hardware timer will always be needed in low power mode to measure elapsed time. The hardware timer is intended to be configured in **TX_LOW_POWER_TIMER_SETUP**.

> Example 1: No low power timer is in use (**TX_LOW_POWER_TIMER_SETUP** is *not* defined). There is a ThreadX timer with 50 ticks remaining. The internal ThreadX tick count is 1000. The processor goes into low power mode for some length of time. After exiting low power mode, the ThreadX timer still has 50 ticks remaining. The internal ThreadX tick count is 1000.

> Example 2: A low power timer is available (**TX_LOW_POWER_TIMER_SETUP** is defined). **TX_LOW_POWER_TICKLESS** is defined. There is a ThreadX timer A with 50 ticks remaining and another ThreadX timer B with 20 ticks remaining. The internal ThreadX tick count is 1000. The processor goes into low power mode for 20 ticks. Upon exiting low power mode, the ThreadX timer A will have 30 ticks remaining. The expiration function for ThreadX timer B will be executed. The internal ThreadX tick count is 1020.  
After executing for some time, the internal ThreadX tick count is 2000 and there are no ThreadX timers active. The processor goes into low power mode for some length of time. After exiting low power mode, the internal ThreadX tick count is 2000.

> Example 3: A low power timer is always used (**TX_LOW_POWER_TIMER_SETUP** is defined and **TX_LOW_POWER_TICKLESS** is *not* defined). The internal ThreadX tick count is 1000 and there are no ThreadX timers active. The processor goes into low power mode for some length of time. Upon exiting low power mode, it is determined that the processor was in low power mode for 20 ticks. The internal ThreadX tick count is updated to 1020.

## User-defined Macros

The following macros invoke functions that the user may want to define/implement.

### Summary of user-defined macros

 - ```TX_LOW_POWER_TIMER_SETUP``` - *set up low power timer*
 - ```TX_LOW_POWER_USER_ENTER``` - *configure processor to enter low power mode*
 - ```TX_LOW_POWER_USER_EXIT``` - *configure processor to exit low power mode*
 - ```TX_LOW_POWER_USER_TIMER_ADJUST``` - *return the number of ticks the processor was in low power mode*

---

### TX_LOW_POWER_TIMER_SETUP
```c
VOID TX_LOW_POWER_TIMER_SETUP(ULONG tx_low_power_next_expiration);
```

### Input parameters

- *tx_low_power_next_expiration* - the number of ticks to configure the low power timer.

### Return values

- *none*

**TX_LOW_POWER_TIMER_SETUP** is a macro invoking user-defined function that sets up a low power timer. To set up a low power timer or operate ticklessly in low power mode, this symbol must be defined. This macro is called in ```tx_low_power_enter```. If **TX_LOW_POWER_TICKLESS** is not defined and there are no timers active, the *tx_low_power_next_expiration* parameter will be set to 0xFFFFFFFF.

> Note: The number of ThreadX ticks is the input to this function. The frequency of a ThreadX timer tick is defined in **TX_TIMER_TICKS_PER_SECOND** (typically defined in file tx_api.h, tx_user.h, or tx_port.h).

> Note: do not put the processor to sleep in this macro.

### Optional Define

- **TX_LOW_POWER_TICKLESS** - an optional define to operate ticklessly in low power mode only if no ThreadX timers are active. With symbol **TX_LOW_POWER_TICKLESS** defined, if there are no ThreadX timers active, **TX_LOW_POWER_TIMER_SETUP** will not be called in ```tx_low_power_enter```. ThreadX will not maintain/update the internal tick count during or after exiting low power mode. Symbol **TX_LOW_POWER_TIMER_SETUP** must also be defined if defining **TX_LOW_POWER_TICKLESS**.

### Example

```c
/* Low power timer function prototype. */
void low_power_timer_config(ULONG ticks);

/* Define the TX_LOW_POWER_TIMER_SETUP macro. */
#define TX_LOW_POWER_TIMER_SETUP low_power_timer_config

void low_power_timer_config(ULONG ticks)
{
    /* Insert code here to configure a hardware timer
       to wake the processor from sleep after
       ticks/TX_TIMER_TICKS_PER_SECOND seconds. */
}
```

---

### TX_LOW_POWER_USER_ENTER
A macro invoking a user-defined function that configures the processor for entering low power mode (e.g. turn off peripherals and select a sleep mode). This macro is called in ```tx_low_power_enter```.

### Input parameters

- *none*

### Return values

- *none*

### Example

```c
/* Low power enter function prototype. */
void low_power_enter(void);

/* Define the TX_LOW_POWER_USER_ENTER macro. */
#define TX_LOW_POWER_USER_ENTER low_power_enter

void low_power_enter(void)
{
    /* Insert code here to configure the processor to enter low power mode. */
}
```
---

### TX_LOW_POWER_USER_EXIT
A macro invoking a user-defined function that configures the processor for exiting low power mode (e.g. turn on peripherals). This is called in ```tx_low_power_exit```.

### Input parameters

- *none*

### Return values

- *none*

### Example

```c
/* Low power exit function prototype. */
void low_power_exit(void);

/* Define the TX_LOW_POWER_USER_EXIT macro. */
#define TX_LOW_POWER_USER_EXIT low_power_exit

void low_power_exit(void)
{
    /* Insert code here to configure the processor to exit low power mode. */
}
```

---

### TX_LOW_POWER_USER_TIMER_ADJUST

```c
ULONG TX_LOW_POWER_USER_TIMER_ADJUST(VOID);
```

A macro invoking a user-defined function to determine how much time has elapsed while in low power mode (in units of ThreadX ticks). This is called in ```tx_low_power_exit``` and returns the number of ticks needed to adjust the ThreadX timers.

When exiting low power mode, there are two possibilities:
 1. The processor slept for the entire time the timer was configured to sleep.
 2. The processor was awakened early.

### Input parameters

- *none*

### Return values

- *tx_low_power_adjust_ticks*

### Example

```c
/* Low power timer adjust function prototype. */
ULONG low_power_timer_adjust(void);

/* Define the TX_LOW_POWER_USER_TIMER_ADJUST macro. */
#define TX_LOW_POWER_USER_TIMER_ADJUST low_power_timer_adjust

ULONG low_power_timer_adjust(void)
{
    ULONG actual_ticks_slept;
    ULONG elapsed_time_in_ms;

    /* Insert code here to read timer registers to determine
       how long the processor actually slept. */
    elapsed_time_in_ms = read_timer_register();

    /* Convert elapsed time to ThreadX ticks. */
    actual_ticks_slept = elapsed_time_in_ms / (1000/TX_TIMER_TICKS_PER_SECOND);

    return(actual_ticks_slept);
}
```

---

## Summary of Low Power APIs

- ```tx_low_power_enter``` - *Enter low power mode*
- ```tx_low_power_exit``` - *Exit low power mode*
- ```tx_time_increment``` - *Increment ThreadX timers by specific amount*
- ```tx_timer_get_next``` - *Get next ThreadX timer expiration*

---

## tx_low_power_enter

Enter low power mode.

### Prototype

```c
VOID tx_low_power_enter(VOID);
```

### Description

This service enters low power mode. The macros **TX_LOW_POWER_TIMER_SETUP** and **TX_LOW_POWER_USER_ENTER** are called in this function to allow the user to configure a low power timer and configure the hardware for low power mode.

For keeping track of time while in low power mode, there are two possibilities:

1. A ThreadX timer is active. Function ```tx_timer_get_next``` returns **TX_TRUE**. Note that in this situation, a low power clock must be used in order to wake up the CPU for the next ThreadX timer expiration. Therefore a low power timer/clock must be programmed. Program the hardware timer source such that the next timer interrupt is equal to: *tx_low_power_next_expiration \* tick_frequency*. The *tick_frequency* is application-specific and typically set up in ```tx_low_level_initialize```.

2. There are no ThreadX timers active. Function ```tx_timer_get_next``` returns *TX_FALSE*.
    1. The application may choose **not** to keep the ThreadX internal
   tick count updated (define **TX_LOW_POWER_TICKLESS**), therefore there is no need to set up a low power clock.
    2. The application still needs to keep the ThreadX tick up-to-date. In this case a low power clock needs to be configured.

### Input parameters

- *none*

### Return values

- *none*

### Allowed from

Internal ThreadX code, application

### Example

ARM Cortex-M assembly
```c
#ifdef TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_enter                      // Enter low power mode
    POP     {r0-r3}
#endif

#ifdef TX_ENABLE_WFI
    DSB                                             // Ensure no outstanding memory transactions
    WFI                                             // Wait for interrupt
    ISB                                             // Ensure pipeline is flushed
#endif

#ifdef TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_exit                       // Exit low power mode
    POP     {r0-r3}
#endif
```
### See also

- tx_low_power_exit

---

## tx_low_power_exit

Exit low power mode.

### Prototype

```c
VOID tx_low_power_exit(VOID);

```

### Description

This service exits low power mode. Macro **TX_LOW_POWER_USER_EXIT** is called in this function to allow the user to configure the hardware to exit low power mode. Macro **TX_LOW_POWER_USER_TIMER_ADJUST** is called in this function to determine how long the processor actually slept.

### Input parameters

- *none*

### Return values

- *none*

### Allowed from

Internal ThreadX code, application

### Example

```c
#ifdef TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_enter                      // Enter low power mode
    POP     {r0-r3}
#endif

#ifdef TX_ENABLE_WFI
    DSB                                             // Ensure no outstanding memory transactions
    WFI                                             // Wait for interrupt
    ISB                                             // Ensure pipeline is flushed
#endif

#ifdef TX_LOW_POWER
    PUSH    {r0-r3}
    BL      tx_low_power_exit                       // Exit low power mode
    POP     {r0-r3}
#endif
```

### See also

- tx_low_power_enter

---

## tx_time_increment

This function increments the current time by a specified value. The value was derived by the application by calling the ```tx_timer_get_next``` function prior to this call, which was right before the processor was put in low power mode.

### Prototype

```c
VOID tx_time_increment(ULONG time_increment);
```

### Description

This function increments the current time by a specified value. The value was derived by the application by calling the ```tx_timer_get_next``` function prior to this call, which was right before the processor was put in low power mode.

### Input parameters

- *time_increment* - Number of ThreadX ticks to increment time and timers.

### Return values

- *none*

### Allowed from

Internal ThreadX code, application

### Example

From ```tx_low_power_exit```:

```c
        /* Call the low-power timer driver code to obtain the amount of time (in ticks)
        the system has been in low power mode. */
#ifdef TX_LOW_POWER_TIMER_ADJUST
        tx_low_power_adjust_ticks = TX_LOW_POWER_USER_TIMER_ADJUST;
#else
        tx_low_power_adjust_ticks = (ULONG) 0;
#endif

        /* Determine if the ThreadX timer needs incrementing.  */
        if (tx_low_power_adjust_ticks)
        {
            /* Yes, the ThreadX time must be incremented.  */
            tx_time_increment(tx_low_power_adjust_ticks);
        }
```

### See also

- tx_timer_get_next

---

## tx_timer_get_next

Get next ThreadX timer expiration

### Prototype

```c
ULONG tx_timer_get_next(ULONG *next_timer_tick_ptr);
```

### Description

This service gets the next ThreadX timer expiration, in ticks.

### Input parameters

- *next_timer_tick_ptr* - pointer to hold number of ticks

### Return values

- *TX_TRUE* (1) At least one timer is active.
- *TX_FALSE* (0) No timers are currently active.

### Allowed from

Internal ThreadX code, application

### Example

From ```tx_low_power_enter```:

```c
ULONG   tx_low_power_next_expiration;   /* The next timer expiration (units of ThreadX timer ticks). */
ULONG   timers_active;

    /*  At this point, we want to enter low power mode, since nothing
        meaningful is going on in the system. However, in order to keep
        the ThreadX timer services accurate, we must first determine the
        next ThreadX timer expiration in terms of ticks. This is
        accomplished via the tx_timer_get_next API.  */
    timers_active =  tx_timer_get_next(&tx_low_power_next_expiration);
```

### See also

- tx_time_increment
