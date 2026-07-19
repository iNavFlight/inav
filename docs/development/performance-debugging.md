# Performance Debugging

Techniques for investigating PID loop and task performance issues.

## Key Insight: Excessive Update Rates

Functions in high-frequency loops may not need every-cycle updates. If a function only checks slowly-changing state (arming eligibility, sensor calibration status, etc.), calling it at the full scheduler rate wastes cycles that could go toward the PID loop.

**Real example already in the codebase:** `taskMainPidLoop()` in `fc_core.c` calls `updateArmingStatus()` through a divider rather than every tick:

```c
static uint8_t armingStatusDivider = 0;
if (++armingStatusDivider >= 10) {
    armingStatusDivider = 0;
    updateArmingStatus();
}
```

`updateArmingStatus()` only checks slowly-changing state (calibration status, arming-blocking flags), so it doesn't need to run at the full PID-loop rate — a divider like this trades a small amount of arming-status latency for reduced per-tick work.

**Lesson:** When profiling shows a high-frequency loop spending time on a function that only reads slowly-changing state, this divider pattern is the fix — audit for other functions in the scheduler's hot path that could use the same treatment.

## Measuring Task Performance

Use the CLI `tasks` command to see execution times:

```
tasks
```

Output shows:
- Task name
- Rate (Hz)
- Max execution time (us)
- Average execution time (us)

## GPIO Constraints: Backup Domain Pins

Some MCU backup-domain GPIO pins (used for things like the RTC/VBAT-backed circuitry) have lower current/speed limits than the chip's other GPIOs — check your MCU's datasheet electrical characteristics for backup-domain pin limits before configuring one for a high-drive-strength use.

**Lesson:** Don't reach for `GPIO_DRIVE_STRENGTH_STRONGER` on a backup domain pin without checking the datasheet limit first.

**Symptoms if exceeded:** LEDs or other outputs on that pin don't behave correctly, unexpected GPIO behavior.

**Fix:** Use `GPIO_DRIVE_STRENGTH_MODERATE` or weaker on backup-domain pins.

## Serial Printf Debugging

For detailed printf debugging techniques, see [serial_printf_debugging.md](serial_printf_debugging.md).

Quick reference:
```c
#include "build/debug.h"

// SITL-only debug output
SD(fprintf(stderr, "[DEBUG] value=%d\n", value));
```

## Build with Debug Logging

```bash
make CPPFLAGS="-DUSE_BOOTLOG=1024 -DUSE_LOG" TARGETNAME
```

This enables `LOG_DEBUG` output over serial — see [serial_printf_debugging.md](serial_printf_debugging.md) for the full logging facility and [bootlog-debugging.md](bootlog-debugging.md) for capturing output from before serial init.

## Related

- GCC preprocessing techniques: [gcc-preprocessing-techniques.md](gcc-preprocessing-techniques.md)
