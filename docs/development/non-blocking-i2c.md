# Non-blocking I2C sensor access

How the barometer, compass and pitot tasks read their sensors without
blocking the scheduler, and how to convert a driver.

## Why

An I2C register read of 6 bytes at 400 kHz keeps the wire busy for about
200 µs. Before this change every `busReadBuf()` spun in the platform driver
until the whole transfer had finished, so a BARO task that only needed a few
microseconds of CPU showed up as ~200 µs in `tasks`, and the compass, pitot
and rangefinder tasks behaved the same way.

Now the sensor task starts a transfer, returns, and parses the result on a
later run. The wire time stays the same, the CPU time moves to a handful of
short interrupts. The model follows Betaflight: no bus queue, no DMA, every
driver owns one outstanding transfer and polls for its completion.

## Layers

### Platform drivers (`drivers/bus_i2c_*.c`)

Every platform implements the same `drivers/bus_i2c.h` API:

| Function | Behaviour |
|---|---|
| `i2cRead()`, `i2cWrite()`, `i2cWriteBuffer()` | Blocking, unchanged for callers. They first wait for a pending non-blocking transfer, so drivers that were not converted keep working on a shared bus. |
| `i2cReadStart()`, `i2cWriteStart()` | Start a transfer and return at once. `false` means nothing was started (bus busy or start failed), try again later. |
| `i2cBusy(device, &error)` | `true` while a non-blocking transfer runs. `error` reports the outcome of the last transfer. A transfer that never completes is reset after `I2C_TIMEOUT`. |

| Platform | Implementation |
|---|---|
| STM32F4 | Own interrupt-driven engine in `bus_i2c_stm32f40x.c`, ported from the Betaflight `bus_i2c_stm32f4xx.c` driver. Fully asynchronous. |
| STM32F7/H7 | `HAL_I2C_Mem_Read_IT()` and friends, completion through the `HAL_I2C_*CpltCallback()` / `HAL_I2C_ErrorCallback()` hooks. Fully asynchronous. |
| AT32F43x | `i2c_memory_read_int()` and friends from `i2c_application`. The library waits for the bus to be free and sends the slave and register address synchronously (tens of µs, bounded by the library timeout), only the data phase runs in the interrupt. |
| RP2350 | Commands are queued into the controller FIFO and finished by the `STOP_DET` / `TX_ABRT` interrupt, ported from the Betaflight Pico driver. Blocking calls still use the Pico SDK functions. |
| Software I2C, SITL | Synchronous: the start call completes the transfer, `i2cBusy()` is always `false`. |

`i2cWriteStart()` copies the data byte into driver-owned storage, the caller
does not have to keep it alive. Buffers given to `i2cReadStart()` must stay
valid until `i2cBusy()` reports idle, so drivers use static buffers.

### Bus layer (`drivers/bus.h`)

| Function | Behaviour |
|---|---|
| `busReadBufStart(dev, reg, buf, len)` | Non-blocking register read. On SPI the read completes inside the call. |
| `busWriteStart(dev, reg, data)` | Non-blocking single register write. |
| `busWriteBufStart(dev, reg, buf, len)` | Non-blocking multi-byte write, the buffer must stay valid until the bus is idle. A zero length sends the register byte alone. |
| `busIsBusy(dev, &error)` | Poll for completion. The error flag is per bus, not per device: if another device's transfer failed in between, a driver may discard one good sample. |
| `busReadStepResult_e` | `BUS_READ_STEP_BUSY` (nothing started), `BUS_READ_STEP_NEXT` (intermediate transfer started), `BUS_READ_STEP_LAST` (final transfer started). Used by sensors whose sample needs several transfers. |

Raw access (`reg == 0xFF` with `DEVFLAGS_USE_RAW_REGISTERS`) works the same
way as for the blocking calls.

### Sensor layers

Each sensor subsystem drives its own small state machine and reschedules
its task with the returned delay. While a transfer is on the bus the task
asks to be called again after 1 ms, otherwise it returns to its regular
period.

**Barometer** (`sensors/barometer.c`, hooks in `drivers/barometer/barometer.h`)

```
start_ut -> (ut_delay) -> read_ut -> get_ut -> start_up -> (up_delay) -> read_up -> get_up -> calculate
```

* `start_*` triggers a measurement, usually a `busWriteStart()`.
* `read_*` starts the data transfer with `busReadBufStart()` and returns
  `false` when the bus is busy.
* `get_*` only parses the static buffer and returns `false` when the sample
  is unusable (phase is restarted).
* `combined_read = true` skips the temperature phase for sensors that deliver
  both values in one read.
* Drivers without `read_*` hooks keep their blocking `get_*` and run exactly
  as before.

A delay of 0 keeps the current task period. The estimator is fed only when a
new sample was produced.

**Compass** (`sensors/compass.c`, hooks in `drivers/compass/compass.h`)

`readStart(mag, firstStep)` returns a `busReadStepResult_e`. It is called with
`firstStep` set for the first transfer of a sample and again after every
completed transfer until it returns `BUS_READ_STEP_LAST`; `read()` then parses.
All I2C compass drivers are converted: one step for sensors in continuous
mode (HMC5883L, IST8308, IST8310, VCM5883, MLX90393), status then data for
sensors with a data-ready flag (QMC5883L/P, LIS2MDL, LIS3MDL, MAG3110,
RM3100), and status, data and the trigger of the next single measurement for
the AK8963 (three steps) and AK8975 (four steps, STATUS2 is read as well).

**Pitot** (`sensors/pitotmeter.c`, hooks in `drivers/pitotmeter/pitotmeter.h`)

Same `readStart(pitot, firstStep)` contract inside the existing protothread.
`start()` may be non-blocking as well. The MS4525 reads two frames per
sample (`NEXT`, `LAST`), MS5525 and DLVR one.

**Rangefinder** (`sensors/rangefinder.c`, hooks in `drivers/rangefinder/rangefinder.h`)

`update()` returns the delay until its next call, 0 meaning the regular
`delayMs`. Each I2C driver keeps a small step machine: it starts one
transfer per call, returns `RANGEFINDER_UPDATE_RETRY_US` while the transfer
is on the bus and evaluates the result on the next call. The VL53L0X start
sequence of eight register writes takes eight calls, the VL53L1X sends the
16 bit register index and reads the data in separate steps, the interrupt
polarity it needs is read once at init.

**Others**

* IRLock: `read()` returns `IRLOCK_READ_PENDING` while the sync scan or the
  frame read is on the bus, `irlockUpdate()` reschedules the task.
* INA226: `ina226Update()` is polled from the battery task and alternates
  bus and shunt voltage reads in the background; the read functions return
  the last completed values, so each quantity refreshes every other battery
  task cycle.
* LM75: `readStart()` plus `read()`, the protothread waits for the bus with
  `ptWait()`. The result lands in the device scratchpad.
* DS2482 1-Wire bridge: commands are fired with `busWriteStart()` (blocking
  fallback when the bus is busy), `owBusReady()` starts a status read on one
  poll and evaluates it on the next. Reading the data register stays
  blocking, two short transfers per byte.
* PCF8574: the output write is fired and forgotten, with the blocking write
  as fallback.
* OLED (`display_ug2864hsweg01.c`): bytes are queued and `i2c_OLED_pump()`
  sends runs of up to 15 commands or data bytes as one transfer. The
  dashboard task calls the pump every millisecond while the queue holds
  data, the CMS display port reports the queue through
  `isTransferInProgress`. A full queue and the initialisation drain
  synchronously.

## Converting a driver

1. Replace every bus access in the measurement path with a static buffer
   and a `busReadBufStart()` / `busWriteStart()` call that returns the
   `BUSY` / started result to the sensor layer.
2. Keep `get()` / `read()` free of bus access: parse the buffer, validate,
   return `false` for an unusable sample.
3. Leave initialisation and detection on the blocking calls.
4. If the sample needs several transfers, keep the step counter in the
   driver, reset it on `firstStep`, and return `NEXT` until the last transfer.
5. Check the cadence: forced-mode sensors now sample every
   `delay + 1 ms` instead of `2 * delay`, because the next measurement is
   triggered right after the previous one was read.

Drivers for other buses (SPI gyros, MSP, fake sensors) need no change.

Every driver that was converted keeps its blocking init path; only the
periodic measurement path changed.

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
| `debug[7]` | time spent in the start call in µs, should be a few µs on F4/F7/H7/RP2350 |

In `tasks` the BARO, COMPASS and PITOT rates alternate between the regular
rate and 1000 Hz, because a cycle consists of a long wait and a 1 ms step.
The average execution time is what matters.

## Known limitations

* No bus arbitration queue: a device that finds the bus busy retries 1 ms
  later. Fine at the 10 to 60 Hz rates these sensors use.
* `busIsBusy()` reports the error of the last transfer on the bus, not of a
  particular device.
* Initialisation and detection of every driver, the DS2482 data register
  read and the VL53L1X register helpers used at init still block. They are
  safe because the blocking calls wait for a pending transfer first.
* The MPU9250 internal compass is read through the gyro's SPI slave interface
  and keeps its own polling state machine.
* On the RP2350 a non-blocking write must fit the controller FIFO: the
  register byte plus 15 data bytes.
* SPI stays synchronous.
