# BOOTLOG Debugging in INAV

How to capture early-boot log messages from firmware using the INAV bootlog facility.

See also [serial_printf_debugging.md](serial_printf_debugging.md) for the general `LOG_*` logging facility this builds on.

---

## The Critical Requirement: Log Port Must Be Configured

**BOOTLOG will not capture anything unless a serial port is configured for the `Log` function.**

This is the most common pitfall. The `logIsEnabled()` function gates ALL log output — including bootlog buffer writes — behind `logHasOutput()`:

```c
// src/main/common/log.c
static bool logIsEnabled(logTopic_e topic, unsigned level)
{
    return logHasOutput() && (level <= logConfig()->level || ...);
}
```

`logHasOutput()` returns true only when `logPort != NULL` (a UART opened for logging) or `mspLogPort != NULL` (MSP shared with log). Without a configured log port the bootlog buffer stays at 0 bytes even though `USE_BOOTLOG` is compiled in and the `bootlog` CLI command shows the correct buffer size.

---

## Setup

### Step 1 — Enable BOOTLOG in the target

Add to the target's `target.h`:

```c
#define USE_BOOTLOG 4096   // bytes of RAM reserved for boot log buffer
```

Then rebuild. The `bootlog` CLI command only appears when `USE_BOOTLOG` is defined.

### Step 2 — Configure a log serial port via CLI

The VCP port (port 20) shared with MSP is the most convenient choice since it requires no extra wiring:

```
serial 20 32769 115200 115200 0 115200
save
```

`32769` = `FUNCTION_MSP (1) | FUNCTION_LOG (32768)`. When shared with MSP the port reuses the existing baud rate; when used standalone it opens at 921600.

### Step 3 — Set the log level

```
set log_level = DEBUG
save
```

Log levels in order: `ERROR < WARNING < INFO < VERBOSE < DEBUG`. The default is `ERROR`. Setting `DEBUG` captures everything.

Reboot after saving.

### Step 4 — Read the log

After reboot, in the CLI:

```
bootlog
```

Output format:
```
# bootlog
log size written: 312 of 4096 bytes reserved
[     0.012] OLED: Bus device found, running I2C probe and detection...
[     0.013] OLED: I2C probe — 0x3C:NAK 0x3D:ACK
[     0.013] OLED: Failed to read status register at 0x3C
```

The number in brackets is the FC uptime in seconds at the time the message was logged.

---

## Using BOOTLOG for I2C / OLED Debugging

When an I2C device (e.g. OLED display, compass) isn't responding, bootlog reveals exactly where init fails.

### Example: OLED display blank

Messages to look for and what they mean:

| Bootlog line | Diagnosis |
|---|---|
| No `OLED:` lines at all | `dashboardInit()` never called — `feature DASHBOARD` off, or `USE_OLED_UG2864` compiled out (check `USE_I2C` for target) |
| `Bus device found` + `0x3C:ACK` | Device found, address correct — look for later failure |
| `Bus device found` + `0x3C:NAK 0x3D:NAK` | Nothing on I2C — check wiring, power, pullups |
| `Bus device found` + `0x3C:NAK 0x3D:ACK` | Module has SA0 high — INAV uses 0x3C, module needs address fix in `common_hardware.c` |
| `Failed to send display OFF command` | I2C ACKs address but rejects commands — possible wrong I2C bus (I2C1 vs I2C2) |

### Checking DASHBOARD feature is on

After any full-erase flash, all settings are reset. Always verify:

```
feature
```

If `DASHBOARD` is absent:

```
feature DASHBOARD
save
```

Then reboot. The OLED only initializes when this feature is enabled (`dashboardInit()` is gated by `feature(FEATURE_DASHBOARD)` in `fc_init.c`).

---

## Writing Log Messages

Include `common/log.h` in the source file. Topic is specified without the `LOG_TOPIC_` prefix:

```c
LOG_ERROR(SYSTEM, "Init failed, code=%d", errorCode);  // always shown at ERROR level
LOG_DEBUG(SYSTEM, "Status register = 0x%02X", val);    // only shown at DEBUG level
```

For I2C address probing, use `i2cRead` with `reg=0xFF` and `allowRawAccess=true` for a raw read (no register-write phase before reading):

```c
uint8_t buf = 0;
bool found = i2cRead(busDev->busdev.i2c.i2cBus, 0x3C, 0xFF, 1, &buf, true);
LOG_ERROR(SYSTEM, "0x3C: %s (status=0x%02X)", found ? "ACK" : "NAK", buf);
```

The device must be registered with `DEVFLAGS_USE_RAW_REGISTERS` in `common_hardware.c` for `busRead(dev, 0xFF, ...)` to work via the normal bus API. Without that flag, pass `reg=0x00` through `busRead` and the bus layer sends `0x00` as a register-address byte before reading — which can confuse OLED controllers that interpret the first byte as a command control byte.

---

## Pitfalls Summary

| Pitfall | Symptom | Fix |
|---|---|---|
| No log port configured | `bootlog` shows `0 of N bytes reserved` | `serial 20 32769 115200 115200 0 115200` |
| Log level too low | Bootlog has some lines but missing DEBUG | `set log_level = DEBUG` |
| Full-erase flash wiped settings | Features/log port reset to default | Re-apply settings after flash; prefer selective erase |
| `busRead(dev, 0x00, ...)` on OLED | OLED I2C state machine confused, subsequent cmds fail | Use `DEVFLAGS_USE_RAW_REGISTERS` + `busRead(dev, 0xFF, ...)` |
| DASHBOARD feature off | OLED code never runs, no log output | `feature DASHBOARD`, `save`, reboot |
