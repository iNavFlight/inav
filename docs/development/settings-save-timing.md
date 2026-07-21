# Avoiding ESC Signal Loss During a Settings Save

How INAV writes settings to flash without letting DShot ESCs lose signal (and potentially spin up or reboot) during the write.

## The Problem: Flash Writes Block the CPU

Writing settings to flash is a blocking operation — the comment in `writeConfigToEEPROM()` (`src/main/config/config_eeprom.c`) states it can block the CPU for 20-200ms. Normal DShot output is CPU-driven: if the CPU is blocked that long, ESCs stop receiving packets. Losing signal like that is not just "hold current throttle" for every ESC — some interpret sustained signal loss as an error condition and may spin up or reboot in response.

## The Fix: Circular DMA Repeats a Zero-Throttle Packet During the Write

`writeConfigToEEPROM()` wraps the actual flash write with `pwmSetMotorDMACircular()` (`src/main/drivers/pwm_output.c`):

```c
void writeConfigToEEPROM(void)
{
#if !defined(SITL_BUILD) && defined(USE_DSHOT)
    // Enable circular DMA so hardware keeps repeating zero-throttle DShot
    // packets during flash writes (which block the CPU for 20-200ms).
    // Without this, ESCs lose signal and may spin up or reboot.
    pwmSetMotorDMACircular(true);
#endif

    // ... the blocking flash write happens here ...

#if !defined(SITL_BUILD) && defined(USE_DSHOT)
    pwmSetMotorDMACircular(false);
#endif
    ...
}
```

Before the write starts, `pwmSetMotorDMACircular(true)` loads a zero-throttle DShot packet directly into the motor DMA buffers and switches those DMA streams to circular mode, so the DMA hardware keeps re-transmitting that packet continuously — with no CPU involvement — for as long as the CPU is blocked erasing/writing flash. ESCs keep seeing valid DShot frames throughout the write instead of going silent. Once the write finishes, `pwmSetMotorDMACircular(false)` switches DMA back to normal (non-circular) operation so regular motor updates resume.

This only applies when motors are actually running the DShot protocol (`pwmSetMotorDMACircular()` checks `isMotorProtocolDshot()` and no-ops otherwise) — the function itself is gated on `USE_DSHOT`, not on any MCU family, and has no chip-specific branching. It delegates the actual DMA register manipulation to `impl_pwmBurstDMASetCircular()`/`impl_timerPWMSetDMACircular()`, which do have separate per-platform implementations providing the same circular-mode behavior: StdPeriph-style direct register access for F4 (`drivers/timer_impl_stdperiph.c`), HAL/LL for F7 and H7 (`drivers/timer_impl_hal.c`), and AT32's register API for AT32 (`drivers/timer_impl_stdperiph_at32.c`) — selected per target family at build time in `cmake/`.

## This Protection Doesn't Depend on Arm State or Trigger Path

Various code paths (the flight loop's deferred-save queue, the CLI, the CMS menu, MSP handlers) also generally try to avoid triggering a save while armed, with varying degrees of strictness. That's incidental, not the reason ESCs stay safe during the write: `pwmSetMotorDMACircular()` runs unconditionally inside `writeConfigToEEPROM()` itself, on every call, regardless of which code path triggered the save or whether the craft happens to be armed. It doesn't depend on every save-triggering call site getting its own arm-state check right.
