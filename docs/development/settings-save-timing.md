# Settings Save Timing and ESC Safety During the Write

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

## Saves Are Also Discouraged While Armed — But That's Not the ESC-Safety Mechanism

Separately from the DMA fix above, most save paths also avoid triggering while armed, though not via one single uniform check:

- Calling `saveConfigAndNotify()` or `saveConfig()` (`src/main/fc/config.c`) doesn't write to flash immediately — both only set a `saveState` flag. The actual write (`processDelayedSave()` → `writeEEPROM()`) is picked up from the main flight loop (`src/main/fc/fc_core.c`), directly gated on `!ARMING_FLAG(ARMED)`, with a 0.5s delay after disarm.
- The OSD CMS menu's "SAVE + REBOOT" action (`src/main/cms/cms.c`) calls `processDelayedSave()` immediately, with no direct armed-check at that call site — instead, opening the CMS menu sets `ARMING_DISABLED_CMS_MENU`, which blocks *new* arm attempts while the menu is open.
- Entering the CLI (`src/main/fc/cli.c`) similarly sets `ARMING_DISABLED_CLI` before `cliSave()`/`cliDefaults()` can run their own direct `writeEEPROM()` calls — again a "block new arming" flag, not a check at the write call site itself.
- MSP's `MSP_RESET_CONF` and `MSP_EEPROM_WRITE` handlers (`src/main/fc/fc_msp.c`) do check directly: `if (!ARMING_FLAG(ARMED)) { ...write... } else return MSP_RESULT_ERROR`.
- `setConfigProfileAndWriteEEPROM()`, `setConfigBatteryProfileAndWriteEEPROM()`, and `setConfigMixerProfileAndWriteEEPROM()` (`src/main/fc/config.c`) call `writeEEPROM()` directly with **no internal armed-check at all** — they rely entirely on whichever caller reaches them: an RC stick command (`src/main/fc/rc_controls.c`, gated by that subsystem's own `disableStickCommands` check), a CLI profile-switch command (gated only by CLI's blanket `ARMING_DISABLED_CLI`), or an MSP profile-select command (`fc_msp.c`, each with its own explicit `!ARMING_FLAG(ARMED)` check).

The CMS/CLI guards prevent *becoming* armed while active — they don't force a disarm of a craft that was already armed before the menu/CLI was entered, so they're weaker than the flight-loop and MSP checks. None of this is the reason ESCs stay safe during a write, though: the circular-DMA mechanism above runs unconditionally on every `writeConfigToEEPROM()` call, regardless of which path triggered it or whether the craft happens to be armed. It's the one guarantee that doesn't depend on getting every save-trigger's arm-gating right.

(Boot-time EEPROM writes — `ensureEEPROMContainsValidData()` and the sensor-autodetect update in `src/main/sensors/initialisation.c` — are out of scope above since they only run from `init()`, before arming is possible.)
