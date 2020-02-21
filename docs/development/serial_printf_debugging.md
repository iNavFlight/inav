# Serial printf style debugging

## Overview

iNav offers a function to use serial `printf` style debugging.

This provides a simple and intuitive debugging facility. This facility is only available after the serial sub-system has been initialised, which should be adequate for all but the most hard-core debugging requirements.

## CLI settings

It is necessary to set a serial port for serial logging using the function mask `FUNCTION_LOG`, 32768. For convenience this may be shared with MSP (mask 1), but no other function.
For example, on a VCP port.

```
serial 20 32769 115200 115200 0 115200
```

If the port is shared, it will be resused with extant settings; if the port is not shared it is opened at 921600 baud.

There are two run time settings that control the verbosity, the most verbose settings being:

```
log_level = DEBUG
Allowed values: ERROR, WARNING, INFO, VERBOSE, DEBUG

log_topics = 0
Allowed range: 0 - 4294967295

```

The use of level and topics is described in the following sections.

## LOG LEVELS

Log levels are defined in `src/main/common/log.h`, at the time of writing these include (in ascending order):

* ERROR
* WARNING
* INFO
* VERBOSE
* DEBUG

These are used at both compile time and run time.

At compile time, a maximum level may be defined. As of iNav 2.3, for F3 targets the maximum level is ERROR, for F4/F7 the maximum level is DEBUG.

At run time, the level defines the level that will be displayed, so for a F4 or F7 target that has compile time suport for all log levels, if the CLI sets
```
log_level = INFO
```
then only `ERROR`, `WARNING` and `INFO` levels will be output.

## Log Topic

Log levels are defined in `src/main/common/log.h`, at the time of writing:

* SYSTEM
* GYRO
* BARO
* PITOT
* PWM
* TIMER
* IMU
* TEMPERATURE
* POS_ESTIMATOR
* VTX
* OSD

Topics are stored as masks (SYSTEM=1 ... OSD=1024) and may be used to unconditionally display log messages.

If the CLI `log_topics` is non-zero, then all topics matching the mask will be displayed regardless of `log_level`. Setting `log_topics` to 4294967295 (all bits set) will display all log messages regardless of run time level (but still constrained by compile time settings), so F3 will still only display ERROR level messages.

## Code usage

A set of macros `LOG_S()` (log system) through `LOG_D()` (log debug) may be used, subject to compile time log level constraints. These provide `printf` style logging for a given topic.

```
//  LOG_D(topic, fmt, ...)
LOG_D(SYSTEM, "This is %s topic debug message, value %d", "system", 42);
```

It is also possible to dump a hex representation of arbitrary  data:

```
// LOG_BUF_D(topic, buf, size)

struct {...} tstruct;
...
LOG_BUF_D(TEMPERATURE, &tstruct, sizeof(tstruct));

```

## Output Support

Log messages are transmitted through the `FUNCTION_LOG` serial port as MSP messages (`MSP_DEBUGMSG`). It is possible to use any serial terminal to display these messages, however it is advisable to use an application that understands `MSP_DEBUGMSG` in order to maintain readability (in a raw serial terminal the MSP message envelope may result in the display of strange characters). `MSP_DEBUGMSG` aware applications include:

* msp-tool https://github.com/fiam/msp-tool
* mwp https://github.com/stronnag/mwptools
* iNav Configurator

For example, with the final lines of `src/main/fc/fc_init.c` set to:

```
 LOG_E(SYSTEM, "Init is complete");
 systemState |= SYSTEM_STATE_READY;
```

and the following CLI settings:

```
serial 20 32769 115200 115200 0 115200
set log_level = DEBUG
set log_topics = 4294967295
```

The output will be formatted as follows:

```
# msp-tool
[DEBUG] [     3.967] Init is complete
# mwp (stderr log file)
2020-02-02T19:09:02+0000 DEBUG:[     3.968] Init is complete
```

The numeric value in square brackets is the FC uptime in seconds.
