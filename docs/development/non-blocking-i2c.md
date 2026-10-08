# Non-blocking I2C transfers

How a driver starts an I2C transfer without waiting for it, how the platform
drivers carry it to the end, and what happens when it fails. This document
covers the bus layer, the barometer and the compass, the first sensors
converted to it. The other sensor layers (pitot, rangefinder) follow in
later changes.

## Why

An I2C register read of 6 bytes at 400 kHz keeps the wire busy for about
200 µs. Every `busReadBuf()` spins in the platform driver until the whole
transfer has finished, so a BARO task that needs a few microseconds of CPU
shows up as 100 to 300 µs in `tasks`, and the compass, pitot and
rangefinder tasks behave the same way.

With the calls below a task starts a transfer, returns, and picks up the
result on a later run. The wire time stays the same, the CPU time moves to a
handful of short interrupts. The model follows Betaflight: no bus queue, no
DMA, one outstanding transfer per bus, the owner polls for its completion.

## Platform drivers (`drivers/bus_i2c_*.c`)

Every platform implements the same `drivers/bus_i2c.h` API:

| Function | Behaviour |
|---|---|
| `i2cRead()`, `i2cWrite()`, `i2cWriteBuffer()` | Blocking, unchanged for callers. They first wait for a pending non-blocking transfer, so drivers that were not converted keep working on a shared bus. |
| `i2cReadStart()`, `i2cWriteStart()`, `i2cWriteBufferStart()` | Start a transfer and return at once. `false` means nothing was started (bus busy or start failed), try again later. |
| `i2cBusy(device, addr, &error)` | `true` while the given slave's non-blocking transfer runs. `error` reports the outcome of that slave's last completed transfer; the result is kept per address, so another device using the bus in between does not overwrite it. |

| Platform | Implementation |
|---|---|
| STM32F4 | Own interrupt-driven engine in `bus_i2c_stm32f40x.c`, ported from the Betaflight `bus_i2c_stm32f4xx.c` driver. Blocking calls use the same engine and wait for it. |
| STM32F7/H7 | `HAL_I2C_Mem_Read_IT()` and friends, completion through the `HAL_I2C_*CpltCallback()` / `HAL_I2C_ErrorCallback()` hooks. |
| AT32F43x | `i2c_memory_read_int()` and friends from `i2c_application`. The library sends the slave and register address synchronously (tens of µs, bounded by the library timeout), only the data phase runs in the interrupt. A NACK in that address phase is reported like any other failed transfer: the start call returns `true` and `i2cBusy()` reports the error at once. |
| RP2350, software I2C, SITL | Synchronous: the start call completes the transfer, `i2cBusy()` is always `false`. |

`i2cWriteStart()` copies the data byte into driver-owned storage, the caller
does not have to keep it alive. Buffers given to `i2cReadStart()` and
`i2cWriteBufferStart()` must stay valid until `i2cBusy()` reports idle, so
drivers use static buffers.

A write with a zero length sends the register byte alone (the HAL and AT32
blocking paths refuse zero-length transfers, the drivers wrap that).

## Bus layer (`drivers/bus.h`)

| Function | Behaviour |
|---|---|
| `busReadBufStart(dev, reg, buf, len)` | Non-blocking register read. On SPI the read completes inside the call. |
| `busWriteStart(dev, reg, data)` | Non-blocking single register write. |
| `busWriteBufStart(dev, reg, buf, len)` | Non-blocking multi-byte write, the buffer must stay valid until the bus is idle. A zero length sends the register byte alone. |
| `busIsBusy(dev, &error)` | Poll for completion of this device's transfer: `true` while it is on the bus, otherwise `error` carries its outcome. Another device's transfer on the same bus does not affect the result. SPI reports idle. |
| `busReadStepResult_e` | `BUS_READ_STEP_BUSY` (nothing started), `BUS_READ_STEP_NEXT` (intermediate transfer started), `BUS_READ_STEP_LAST` (final transfer started). Used by sensors whose sample needs several transfers. |

Raw access (`reg == 0xFF` with `DEVFLAGS_USE_RAW_REGISTERS`) works the same
way as for the blocking calls.

## Timeouts and recovery

A non-blocking transfer is abandoned when it makes no progress for
`I2C_TIMEOUT` (10 ms) while somebody looks at it: the peripheral is
reinitialised with `i2cInit()`, the error counter goes up and the failure is
recorded for the transfer's owner. Progress is the number of bytes still to
go (`XferCount` on the HAL, `pcount` on the AT32, byte index and subaddress
phase on the F4), not the time since the start:

* A gap between two looks longer than 1 ms counts as 1 ms. A main loop that
  was busy elsewhere for a while (a settings save) does not get a transfer
  that is merely late declared stuck, because the interrupts carried it on in
  the meantime and the next look sees the progress.
* A transfer that really is stuck still times out when looked at seldom:
  about ten looks at 1 ms intervals, or 10 ms of a blocking wait.

A NACK or a bus error ends a non-blocking transfer at once: the error is
recorded for its owner and the counter shown by `status` goes up. The
peripheral is not reinitialised for that, it is fine after a NACK. This
includes the AT32 address phase, which runs inside the start call: the call
returns `true` and `i2cBusy()` reports the failure, so an unplugged sensor
costs one short failed start per attempt and nothing more. The F4 driver
never reinitialised on a NACK for blocking calls either; the HAL and AT32
blocking calls keep reinitialising on failure, as before.

The F4 handlers never reinitialise the peripheral themselves. The I2C
interrupts sit above the motor timers and an unstick clocks the bus for up
to a few ms, so a transfer the handler has to give up on (a bus error with a
START still pending, a repeated START that does not go out within 1 ms) only
flags the bus: the next `i2cBusy()` or start call on that bus finishes the
cycle with a STOP and reinitialises the peripheral from task context.

A bus that refuses every start as busy with no transfer of ours in flight
(`HAL_BUSY` on F7/H7, `BUSYF` on the AT32) is a fault the stuck detection
cannot see, since nothing is in progress. The drivers count the refused
starts the same way as the stuck looks and reinitialise the peripheral after
`I2C_TIMEOUT` of them; the error counter goes up and the failure is recorded
for the address that asked, so the owner sees it on its next `busIsBusy()`.
The F4 engine does not check the busy flag before a start, a held bus shows
up there as a stuck transfer.

A blocking call issued while a non-blocking transfer is on the bus waits for
it with the same stuck detection, then runs as before.

## Barometer (`sensors/barometer.c`, hooks in `drivers/barometer/barometer.h`)

```
start_ut -> (ut_delay) -> read_ut -> get_ut -> start_up -> (up_delay) -> read_up -> get_up -> calculate
```

* `start_*` triggers a measurement, usually a `busWriteStart()`.
* `read_*` starts the data transfer with `busReadBufStart()` and returns
  `false` when the bus is busy.
* `get_*` only parses the static buffer and returns `false` when the sample
  is unusable (the phase is restarted).
* `combined_read = true` skips the temperature phase for sensors that deliver
  both values in one read.
* Drivers without `read_*` hooks keep their blocking `get_*` and run exactly
  as before.

While a transfer is on the bus the task asks to be called again after 1 ms,
otherwise it returns to the sensor's delays. A delay of 0 keeps the current
task period. The position estimator is fed only when a new sample was
produced, the 1 ms steps bring it nothing.

Converted so far:

* **DPS310 / SPL07-003**: the sensor runs in continuous mode. The temperature
  phase reads `MEAS_CFG` every `ut_delay` and `get_ut` checks `PRS_RDY`; a
  clear flag restarts the phase, a set flag moves on to the pressure phase,
  which reads the six result bytes one scheduler step later. Same sample
  rate as before, the sensor's 32 per second.
* **SPL06-001**: command mode, two phases. `start_ut` fires the temperature
  command, the temperature bytes are read after `ut_delay`, then the same for
  the pressure. A sample takes `ut_delay + up_delay` plus two 1 ms steps,
  practically the previous cadence.
* **BMP280 / BME280**: forced mode. `start_up` fires the one-byte trigger
  write, the pressure phase reads the six-byte frame after `up_delay`, and
  the next trigger follows right after the parse. Note the cadence: a sample
  now takes `up_delay` plus one 1 ms step instead of `2 * up_delay`, because
  the blocking cycle spent its second half idle. For the default
  oversampling that is about 24 ms per sample instead of 46 ms.

The remaining barometer drivers are untouched and still block.

## Compass (`sensors/compass.c`, hooks in `drivers/compass/compass.h`)

`readStart(mag, firstStep)` returns a `busReadStepResult_e`. It is called with
`firstStep` set for the first transfer of a sample and again after every
completed transfer until it returns `BUS_READ_STEP_LAST`; `read()` then only
parses. `compassUpdate()` returns the delay until its next call: 1 ms while a
transfer is on the bus, the regular 100 ms period once the sample was
handled. Drivers without `readStart` keep their blocking `read()`.

Converted so far: QMC5883L and QMC5883P, two transfers per sample. The status
byte is read first; the six data bytes only when `DRDY` confirms a fresh
sample, otherwise `read()` reports the miss as before. A sample costs three
task runs about 1 ms apart, then the task sleeps for the rest of the period.

## Converting a driver

1. Replace the bus access in the periodic measurement path with a static
   buffer and a `busReadBufStart()` / `busWriteStart()` call. Return "busy"
   to the caller when the start call refuses, and try again on the next run.
2. Pick up the result on a later run: `busIsBusy(dev, &error)` is `false`
   once the transfer is over, `error` says whether the buffer is valid.
3. Keep the parse step free of bus access.
4. Leave initialisation and detection on the blocking calls; they wait for a
   pending transfer first, so they are safe on a shared bus.
5. One outstanding transfer per device. A sample made of several transfers
   (status, then data) takes several runs.

Drivers for other buses (SPI gyros, MSP, fake sensors) need no change.

## Debugging

`set debug_mode = I2C`, then `debug` in the CLI:

| Field | Meaning |
|---|---|
| `debug[0]` | interrupts during the last transfer (about 10 for a 6 byte read on F4) |
| `debug[1]` | error interrupts since boot |
| `debug[2]` | duration of the last transfer in µs (about 200 for 6 bytes at 400 kHz) |
| `debug[3]` | longest single interrupt handler run in µs (F4 only) |
| `debug[4]` | longest wait for a START / STOP bit release in µs (F4 only) |
| `debug[5]` | I2C error counter, also shown by `status` |
| `debug[6]` | interrupt enables found at transfer start (F4 only, 1024 is normal) |
| `debug[7]` | time spent in the start call in µs, should be a few µs on F4/F7/H7 |

## Known limitations

* No bus arbitration queue: a device that finds the bus busy retries later.
  Fine at the 10 to 60 Hz rates the sensors use.
* On the AT32 the start call blocks for the address phase of the transfer.
* The RP2350 driver is still blocking.
* SPI stays synchronous.
