# Serial printf style debugging

## Overview

INAV offers a function to use serial `printf` style debugging.

This provides a simple and intuitive debugging facility. This facility is only available after the serial sub-system has been initialised, which should be adequate for all but the most hard-core debugging requirements.

In order to use this feature, the source file must include `common/log.h`.

## CLI settings

It is necessary to set a serial port for serial logging using the function mask `FUNCTION_LOG`, 32768. For convenience this may be shared with MSP (mask 1), but no other function.
For example, on a VCP port.

```
serial 20 32769 115200 115200 0 115200
```

If the port is shared, it will be reused with extant baud rate settings; if the port is not shared it is opened at 921600 baud.

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

* LOG_LEVEL_ERROR
* LOG_LEVEL_WARNING
* LOG_LEVEL_INFO
* LOG_LEVEL_VERBOSE
* LOG_LEVEL_DEBUG

These are used at both compile time and run time.

At compile time, a maximum level may be defined. As of INAV 2.3, for F3 targets the maximum level is ERROR, for F4/F7 the maximum level is DEBUG.

At run time, the level defines the level that will be displayed, so for a F4 or F7 target that has compile time suport for all log levels, if the CLI sets
```
log_level = INFO
```
then only `ERROR`, `WARNING` and `INFO` levels will be output.

## Log Topic

Log topics are defined in `src/main/common/log.h`, at the time of writing:

* LOG_TOPIC_SYSTEM
* LOG_TOPIC_GYRO
* LOG_TOPIC_BARO
* LOG_TOPIC_PITOT
* LOG_TOPIC_PWM
* LOG_TOPIC_TIMER
* LOG_TOPIC_IMU
* LOG_TOPIC_TEMPERATURE
* LOG_TOPIC_POS_ESTIMATOR
* LOG_TOPIC_VTX
* LOG_TOPIC_OSD

Topics are stored as masks (SYSTEM=1 ... OSD=1024) and may be used to unconditionally display log messages.

If the CLI `log_topics` is non-zero, then all topics matching the mask will be displayed regardless of `log_level`. Setting `log_topics` to 4294967295 (all bits set) will display all log messages regardless of run time level (but still constrained by compile time settings), so F3 will still only display ERROR level messages.

## Code usage

A set of macros `LOG_ERROR()` (log error) through `LOG_DEBUG()` (log debug) may be used, subject to compile time log level constraints. These provide `printf` style logging for a given topic.

```
//  LOG_DEBUG(topic, fmt, ...)
LOG_DEBUG(LOG_TOPIC_SYSTEM, "This is %s topic debug message, value %d", "system", 42);
```

It is also possible to dump a hex representation of arbitrary  data, using functions named variously `LOG_BUFFER_` (`ERROR`) and `LOG_BUF_` (anything else, alas) e.g.:

```
// LOG_BUFFER_ERROR(topic, buf, size)
// LOG_BUF_DEBUG(topic, buf, size)

struct {...} tstruct;
...
LOG_BUF_DEBUG(LOG_TOPIC_TEMPERATURE, &tstruct, sizeof(tstruct));
```

## Output Support

Log messages are transmitted through the `FUNCTION_LOG` serial port as MSP messages (`MSP_DEBUGMSG`). It is possible to use any serial terminal to display these messages, however it is advisable to use an application that understands `MSP_DEBUGMSG` in order to maintain readability (in a raw serial terminal the MSP message envelope may result in the display of strange characters). `MSP_DEBUGMSG` aware applications include:

* [msp-tool](https://github.com/fiam/msp-tool)
* [mwp](https://github.com/stronnag/mwptools)
* [dbg-tool](https://github.com/stronnag/mwptools)
* [INAV Configurator](https://github.com/iNavFlight/inav-configurator)

For example, with the final lines of `src/main/fc/fc_init.c` set to:

```
 LOG_ERROR(LOG_TOPIC_SYSTEM, "Init is complete");
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
