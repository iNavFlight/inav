# RP2350_PICO Port — TODO / Pending Work

## Status
- M4 COMPLETE: INAV Configurator connects successfully and shows board name, firmware
  version, sensor status.
- USB VCP (TinyUSB CDC) working; MSP protocol functional over USB.

---

## Task Timings

### Baseline (before optimizations, with dynamic notch enabled, 500 Hz)

```
Task list         rate/hz  max/us  avg/us maxload avgload     total/ms
 0 -       SYSTEM       9     378       1    0.8%    0.5%       204
 1 -          PID     405    2337    1464   95.1%   59.7%     18521
 2 -         GYRO     408     459      37   19.2%    2.0%       444
 3 -           RX      48     884     414    4.7%    2.4%       670
 4 -       SERIAL      94   1285653      83 12085.6%    1.2%      3318
 6 -  TEMPERATURE      99     390      35    4.3%    0.8%        81
Total (excluding SERIAL)                   135.2%   69.7%
```

### After optimizations (FAST_CODE callees, pidSumLimit precompute, isRXDataNew cache, 1 kHz)

```
Task list         rate/hz  max/us  avg/us maxload avgload     total/ms
 1 -          PID     993    1398     690  139.3%   69.0%     70605
 2 -         GYRO     993     343      15   34.5%    1.9%      1394
 3 -           RX      49     747     389    4.1%    2.4%      2588
Total (excluding SERIAL)                   190.8%   78.6%
```

Root cause of remaining PID cost: dominant functions still run from QSPI flash (XIP cache).

### PID loop profiling breakdown (early-return, 1 kHz, no GPS)

| Profiling point | PID avg | Delta | Code section |
|---|---|---|---|
| after `gyroFilter()` | 9 µs | — | Gyro filter chain |
| after `imuUpdateAttitude()` | 317 µs | **+308 µs** | **IMU (Mahony AHRS + attitude) ← dominant** |
| after `processPilotAndFailSafe()` | 404 µs | +87 µs | RC processing + failsafe |
| after `updatePositionEstimator()` | 519 µs | +115 µs | Navigation / position estimator |
| after `pidController()` | 653 µs | +134 µs | PID computation |
| after `mixTable()` | 665 µs | +12 µs | Mixer + output |

IMU accounts for 46% of PID loop time (308/665 µs). This is the primary target for
TODO 5 (RP2350_FAST_CODE on `imuMahonyAHRSupdate`, `imuCalculateEstimatedAttitude`).

### Drill-down profiling (multi-file early-return, 1 kHz, no GPS, no angle mode)

**IMU block (+308 µs) breakdown:**

| Function | Cost | Notes |
|---|---|---|
| `imuUpdateAccelerometer()` | ~21 µs | Fast; just scales ADC samples |
| `imuUpdateAttitude()` (Mahony AHRS) | ~287 µs | **All of the IMU cost; runs from QSPI flash** |

Sub-breakdown of `imuUpdateAttitude` (287 µs total):

| Section | Cost | Notes |
|---|---|---|
| `gyroGetMeasuredRotationRate()` | ~1 µs | Trivial |
| `accGetMeasuredAcceleration()` + `imuCheckVibrationLevels()` | ~14 µs | Data reads + scaling |
| `imuCalculateFilters()` | ~5 µs | 9× `pt1FilterApply4` (already FAST_CODE, callee is in SRAM) |
| GPS/weight-calc overhead | ~0 µs | All GPS branches skip; weight math tiny |
| **`imuMahonyAHRSupdate()`** | **~205 µs** | **Quaternion integrator, ~2 KB, runs from QSPI flash — primary TODO 5 target** |
| **`imuUpdateEulerAngles()`** | **~62 µs** | **Calls `atan2_approx` × 2 + `acos_approx` × 1 from flash — also needs RP2350_FAST_CODE** |

→ `imuMahonyAHRSupdate` (205 µs) + `imuUpdateEulerAngles` (62 µs) = 267 µs = 92% of the IMU block.
  Both run from QSPI flash. Moving them to SRAM (TODO 5) should cut IMU cost to ~20 µs.
  Estimated new PID avg: 683 - 267 ≈ **416 µs** (39% faster).

→ `imuUpdateAttitude` alone is ~42% of the entire PID loop. TODO 5 is confirmed #1 priority.

**Nav block (+115 µs) breakdown — inside `updatePositionEstimator()`:**

| Function | Cost | Notes |
|---|---|---|
| `updateIMUTopic()` | ~64 µs | IMU pre-processing for nav; runs from flash |
| `updateEstimatedTopic()` | ~15 µs | State estimation step |
| `publishEstimatedTopic()` | ~32 µs | Write nav state to consumers |
| `applyWaypointNavigationAndAltitudeHold()` | ~0 µs | Essentially free without GPS |

→ `updateIMUTopic` is the dominant nav cost. Possible second-core candidate: it runs
  at PID rate but is independent of the RC/PID pipeline; it produces results consumed by
  `updateEstimatedTopic`. Would need a double-buffer and spin-lock. Note for later.

**PID computation block (+134 µs) breakdown — inside `pidController()`:**

| Section | Cost | Notes |
|---|---|---|
| Rate-target setup loop (3 axes) | ~115 µs | `pidController` is FAST_CODE but calls some non-FAST_CODE helpers |
| Angle/horizon loop + turn assist | ~0 µs | Mode not active in bench test |
| Core PID loop (`pidControllerApplyFn` × 3) | ~35 µs | Callee fns are FAST_CODE; relatively cheap |

→ Rate-target loop is expensive (~115 µs) despite `pidController` being in SRAM.
  Likely culprits: `pidHeadingHold`, `adaptiveFilterPushRate`, `gyroKalmanUpdateSetpoint`
  (conditional on target features) — these are not FAST_CODE and cause XIP cache pressure.
  Check with `grep -r "USE_ADAPTIVE_FILTER\|USE_GYRO_KALMAN" src/main/target/RP2350_PICO/`.

---

## TODO 1 — Map FAST_CODE to RAM (.time_critical) for RP2350

**Why:** On STM32F7/H7, `FAST_CODE` places hot functions in ITCM RAM (zero wait state).
On RP2350, `FAST_CODE` is currently a no-op (common.h only defines it for F7/H7).
The RP2350 executes all code from QSPI flash via a 16 KB XIP cache; cache-miss latency
on the large PID+scheduler+gyro code path is responsible for PID's 1464 µs avg.

**Linker script note:** `rp2350_flash.ld` has `*(.time_critical*)` in TWO places:
- Line 74: inside `.text` section → FLASH only (bad — symbols stay in flash)
- Line 169: inside `.data` section (`> RAM AT> FLASH`) → copied to SRAM at boot (good)

GNU ld matches each input section once, so the `.text` pattern at line 74 wins and
`.time_critical*` ends up in flash. **Fix:** remove `*(.time_critical*)` from `.text`
(line 74) so the `.data` rule at line 169 takes effect and copies them to SRAM.

**Implementation steps:**
1. `rp2350_flash.ld` line 74: remove `*(.time_critical*)` from `.text` section.
2. `target.h` for RP2350: add after all other defines:
   ```c
   // FAST_CODE: place hot functions in SRAM (copied from flash at boot) to avoid
   // XIP cache pressure. Overrides the no-op definition in common.h.
   #undef FAST_CODE
   #define FAST_CODE __attribute__((section(".time_critical.inav")))
   ```
   NOTE: common.h defines FAST_CODE without `#ifndef` guard so `#undef` first is needed.
   target.h is included after common.h in the build chain.
3. Build and check map file: `grep time_critical build_rp2350/bin/RP2350_PICO.elf.map`
   — should show scheduler.c, gyro.c, pid.c symbols in `.data` (RAM) not `.text`.
4. Measure TASK_PID avg/max before vs after via Configurator task tab or:
   ```
   python3 debug/openocd_helper.py flash   # flash new build
   ```
   Expected: PID avg drops from ~1464 µs to ~300–500 µs (3–5× improvement).

**Affected functions marked FAST_CODE:** scheduler(), taskGyro(), pidController(),
gyroUpdate(), and anything in scheduler.c/gyro.c/pid.c tagged FAST_CODE or NOINLINE.
`grep -r "FAST_CODE" src/main/` to get the full list.

---

## TODO 2 — Test removing the gyro rate override in fc_tasks.c

**Background:** An `#ifdef RP2350` block in `fc_tasks.c` hard-codes GYRO and PID task
periods to 1000 µs (1 kHz):

```c
#ifdef RP2350
    rescheduleTask(TASK_PID, 1000);
    rescheduleTask(TASK_GYRO, 1000);
#else
    rescheduleTask(TASK_PID, getLooptime());
    rescheduleTask(TASK_GYRO, getGyroLooptime());
#endif
```

**Why it was added:** GYRO max execution time exceeded its period at the default 250 µs,
causing `forcedRealTimeTask=true` in every scheduler iteration, starving all other tasks.
Reduced to 1000 µs; GYRO avg is now 15 µs with occasional max of ~343 µs.

**Why it might not be needed after FAST_CODE fix:** Once FAST_CODE → .time_critical
puts the gyro filter pipeline in SRAM, GYRO avg should drop well below 250 µs. The
standard `getGyroLooptime()` path (which returns 250 µs) would then work correctly.

**Test plan:**
1. Apply FAST_CODE fix (TODO 1) first.
2. Remove the `#ifdef RP2350` block from `fc_tasks.c`.
3. Build and flash.
4. Check Configurator task tab: if GYRO/PID still running and TASK_SERIAL/SYSTEM still
   running (not starved), the override is no longer needed.
5. If starvation returns (TASK_SERIAL avgload 0%, averageSystemLoadPercent = 0), revert
   and keep the override.

**Location:** `src/main/fc/fc_tasks.c`, function `fcTasksInit()`, lines ~352–362.

---

## TODO 3 — Loop rate 1 kHz via targetConfiguration (clean-up)

The current `#ifdef RP2350` hack in fc_tasks.c bypasses `getGyroLooptime()`. The proper
INAV pattern (matching other fixed-wing targets) is:
1. Guard `TASK_GYRO_LOOPTIME` in `config.h` with `#ifndef` so target.h can override it.
2. Define `#define TASK_GYRO_LOOPTIME 1000` in target.h.
3. Add `gyroConfigMutable()->looptime = 1000;` in `config.c` → `targetConfiguration()`.
4. Remove the `#ifdef RP2350` block from fc_tasks.c entirely.

Only worth doing after TODO 2 confirms whether the rate override is still needed at all.

---

## TODO 4 — Flash script convenience

Permanent flash script: `debug/flash.sh` (already created).
Usage: `./debug/flash.sh` from the RP2350_PICO target directory.
Flashes `build_rp2350/bin/RP2350_PICO.elf` via SWD (no BOOTSEL required).

---

## TODO 5 — RP2350_FAST_CODE macro for large hot functions

**Why:** `FAST_CODE` on all targets shares the F7 ITCM budget (16 KB, ~5 KB was free
before the RP2350 port). Large functions like `imuMahonyAHRSupdate` (~2 KB) and
`imuCalculateEstimatedAttitude` (~1 KB) would overflow F7 ITCM if marked `FAST_CODE`.
Those functions were intentionally left without `FAST_CODE` to keep the all-targets
budget intact, but they are the dominant XIP cache-pressure source on RP2350.

**Solution:** define a `RP2350_FAST_CODE` macro that maps to `FAST_CODE` on RP2350
only (and is a no-op everywhere else). Depends on TODO 1 (FAST_CODE → .time_critical).

**Implementation:**
1. In `src/main/target/RP2350_PICO/target.h`, after the `FAST_CODE` override:
   ```c
   #define RP2350_FAST_CODE FAST_CODE
   ```
2. In all other target builds (or common.h), add:
   ```c
   #ifndef RP2350_FAST_CODE
   #define RP2350_FAST_CODE
   #endif
   ```
3. Mark the following functions `RP2350_FAST_CODE` in their definitions:

   **Primary targets (confirmed by drill-down profiling):**
   - `imuMahonyAHRSupdate` (~2,080 B) — **205 µs**, quaternion integrator, called every PID cycle
   - `imuUpdateEulerAngles` — **62 µs**, calls `atan2_approx`×2 + `acos_approx`×1 from flash
   - `imuCalculateEstimatedAttitude` (~988 B) — wrapper for both above, called every PID cycle

   **Callee chain for `imuUpdateEulerAngles` (also need RP2350_FAST_CODE):**
   - `atan2_approx`, `acos_approx` in `maths.c` — currently no-op (were stripped of FAST_CODE);
     once `imuUpdateEulerAngles` is RP2350_FAST_CODE they become SRAM→flash veneer calls

   **Lower priority (skipped without GPS on bench test):**
   - `imuCalculateGPSacceleration`, `imuCalculateTurnRateacceleration` (~360 B, ~244 B)
   - `imuCalculateMcCogWeight`, `imuCalculateMcCogAccWeight` (~168 B, ~108 B)
   - `processPilotAndFailSafeActions` in fc_core.c (~900 B, called every PID cycle)

4. Build for both RP2350_PICO and an F7 target (e.g., JHEMCUF405); confirm F7 ITCM
   size does not increase (map file `.itcm` / `d_itcm` section unchanged).

**RP2350_FAST_CODE vs #ifdef FAST_CODE:** Use the macro. `#ifdef RP2350` in function
signatures is ugly and doesn't scale; a `RP2350_FAST_CODE` macro keeps the intent
visible at the definition site and degrades gracefully on all other targets to a no-op.

**Measured result (commit 49ad7209):**
- PID avg: 710 µs → 575 µs  (-135 µs, 19% improvement)
- PID max: 1401 µs → 1093 µs
- Smaller than the theoretical 267 µs; remaining gap likely due to callee functions
  not yet in SRAM (see rate-target loop, `imuCheckAndResetOrientationQuaternion`, etc.)

**Follow-up (see TODO 9):** `processPilotAndFailSafeActions`, `throttleStickMixedValue`,
`rcLookupThrottle` subsequently marked `RP2350_FAST_CODE` → 575 µs → 546 µs (-29 µs).

**Status: DONE** ✓ — `RP2350_FAST_CODE` macro implemented; all listed functions marked.

---

## TODO 10 — Set default clock to 192 MHz

**Why:** The RP2350 supports up to 200+ MHz with default SoC configuration. At 192 MHz
vs 150 MHz default that is a 28% raw clock increase, which directly reduces the µs cost
of every instruction. Code that is already in SRAM benefits fully (no XIP factor).
XIP-resident code also benefits because the QSPI clock scales proportionally.

**Implementation:** Call `set_sys_clock_khz(192000, true)` at the top of `systemInit()`
in `system_rp2350.c`, before `tusb_init()` and `stdio_init_all()`. Update the initial
`SystemCoreClock` constant from 150000000 to 192000000.

The USB PLL is independent (clk_usb stays at 48 MHz from pll_usb) so TinyUSB CDC is
unaffected. The hardware timer (clk_timer, from clk_ref at 12 MHz) is also independent,
so `micros()` stays accurate.

**Measured result:**
- PID avg: 503 µs → 360 µs  (-143 µs, -28%)
- PID max: 1181 µs → 833 µs
- Better than theoretical 503 × (150/192) = 393 µs; SRAM-resident code benefits
  extra since it has zero cache-miss penalty at any clock speed.

**Cumulative from 710 µs baseline: 360 µs (-350 µs, -49%)**

**Status: DONE** ✓

---

## TODO 11 — RP2350_FAST_CODE for updatePositionEstimator hot path

**Why:** `updatePositionEstimator` runs every 1 kHz PID cycle from QSPI flash.
Drill-down profiling showed it costs ~115 µs total:
  - `updateIMUTopic` (~65 µs): body frame → earth frame accel transform + calibration
  - `updateEstimatedTopic` (~20 µs): EKF-style state estimation step
  - `publishEstimatedTopic` (~30 µs of slot, but timer-gated at ~50 Hz — not full cost)

**Implementation:** Mark `RP2350_FAST_CODE`:
- `updatePositionEstimator` in `navigation_pos_estimator.c` — tiny wrapper, 76 B
- `updateIMUTopic` — dominant cost, 692 B
- `updateIMUEstimationWeight` — callee of above, 236 B
- `updateEstimatedTopic` — second-largest nav cost, 1324 B
- `imuTransformVectorBodyToEarth` in `imu.c` — hot callee of updateIMUTopic, 48 B

Note: `publishEstimatedTopic` (992 B) was skipped — its body only runs at 50 Hz
(timer-gated), so XIP pressure from it is negligible at 1 kHz.

**Measured result:**
- PID avg: 546 µs → 503 µs  (-43 µs, -8%)
- PID max: 1055 µs → 1181 µs  (max is noisy; avg is the reliable metric)

**Status: DONE** ✓

---

## TODO 9 — RP2350_FAST_CODE for pilot/throttle hot path

**Why:** `processPilotAndFailSafeActions` runs every PID cycle (1 kHz) from QSPI flash.
Its hot callees `throttleStickMixedValue` and `rcLookupThrottle` are also in flash.
At 575 µs baseline, these account for an estimated 30–40 µs of cache-miss overhead.

**Implementation:** Mark the following `RP2350_FAST_CODE`:
- `processPilotAndFailSafeActions` in `fc_core.c` — the per-cycle wrapper
- `throttleStickMixedValue` in `rc_controls.c` — throttle mix, called every cycle
- `rcLookupThrottle` in `rc_curves.c` — table lookup callee of `throttleStickMixedValue`

Note: `applyRateDynamics` and `getAxisRcCommand` are already `FAST_CODE` (in SRAM).

**Measured result:**
- PID avg: 575 µs → 546 µs  (-29 µs)
- PID max: 1093 µs → 1055 µs

**Status: DONE** ✓

---

## TODO 6 — Eliminate unnecessary sqrt in calc_length_pythagorean_3D call sites

`sqrt()` is among the most expensive floating-point operations. When the result of
`calc_length_pythagorean_3D` is only compared against another length (especially another
`calc_length_pythagorean_3D` result), both sides can be squared — eliminating two sqrt
calls and replacing them with a comparison of the squared norms (which is already
available from `vectorNormSquared`).

**Call sites to audit** (`grep -rn calc_length_pythagorean_3D src/main`):

| File | Line | Use | Sqrt eliminable? |
|------|------|-----|-----------------|
| `wind_estimator.c` | 177 | `windLength < prevWindLength + 4000` | Partial — offset of 4000 complicates squaring. Could rewrite as `sq(windLength) < sq(prevWindLength + 4000)` which expands to comparing squares plus cross-terms, OR just check `windLength - prevWindLength < 4000` which avoids comparing two sqrts |
| `wind_estimator.c` | 155 | `calc_length_pythagorean_3D(...) / fast_fsqrtf(diffLengthSq)` | No — ratio needs actual value |
| `imu.c` | 671 | `GPS3Dspeed` stored and filtered | No — value used directly |
| `pid.c` | 408 | `getTotalRateTarget()` → pt1 filter | No — value used directly |
| `acceleration.c` | 613 | `gforce > acc.maxG` | Yes — store `maxGSq` and compare `(x²+y²+z²) > maxGSq` |
| `pitotmeter*.c` | various | airspeed reading | No — value used directly |

**Priority candidates:**
1. `wind_estimator.c:177-181`: If the intent is just "don't update if wind jumped too much",
   `windLength - prevWindLength < 4000` avoids computing prevWindLength's sqrt entirely.
   Or replace both with `vectorNormSquared` and a squared threshold.
2. `acceleration.c:613-614`: Change `acc.maxG` to `acc.maxGSq`; compute
   `gforceSq = x²+y²+z²`; compare `gforceSq > acc.maxGSq`. No sqrt needed.

**Note:** `vectorNormSquared(v)` already exists in `vector.h` and returns `v·v` — use it
instead of `calc_length_pythagorean_3D` wherever only the squared magnitude is needed.

---

## TODO 8 — Mark imuComputeRotationMatrix RP2350_FAST_CODE

**Why:** `imuMahonyAHRSupdate` (TODO 5 target) calls `imuComputeRotationMatrix()` at
the end of every PID cycle (line ~569 in imu.c). Once `imuMahonyAHRSupdate` is placed
in SRAM by TODO 5, every call to the still-in-flash `imuComputeRotationMatrix` incurs
a SRAM→flash veneer + XIP cache lookup. The function is ~25 instructions (~48 B) and
called 1000×/s, so even a warm-cache hit costs 3–4 cycles per access; a cold miss costs
~100 cycles.

**Fix:** Add `RP2350_FAST_CODE` to `imuComputeRotationMatrix` in imu.c (alongside the
TODO 5 changes). Only mark it on RP2350 targets; the function is already too large for
F7 ITCM to absorb without overflow.

**Scope:** Single function definition in `src/main/flight/imu.c`.
Depends on TODO 5 (RP2350_FAST_CODE macro must exist before this can be applied).

---

## TODO 7 — Architecture / Code Review

Run the INAV code review agent over all RP2350-specific changes made so far.

**Scope:** All files added or modified for this port:
- `src/main/target/RP2350_PICO/` (entire directory)
- `src/main/fc/fc_tasks.c` (`#ifdef RP2350` block)
- `src/main/target/link/rp2350_flash.ld` (once FAST_CODE fix is applied)

**What to look for:**

*Small-scale (line-level):*
- Style consistency with surrounding INAV code
- Any undefined behaviour, off-by-one, or type issues
- Missing or incorrect `#ifdef` guards

*Big-picture (architecture):*
- Are there simpler / more direct ways to achieve the same result with fewer changes
  to existing INAV code?
- Does the `#ifdef RP2350` in `fc_tasks.c` fit INAV's patterns, or is there a cleaner
  hook (e.g. `targetConfiguration()`, a `TARGET_GYRO_LOOPTIME` define, etc.)?
- Is the `tud_task()` repeating-timer approach in `system_rp2350.c` the right place,
  or should it be driven differently (e.g. from `taskSerial` or a dedicated USB task)?
- Could any of the "fake sensor" / stub approach be done with fewer custom files by
  reusing existing SITL stubs more directly?
- Any other patterns that deviate unnecessarily from how other INAV targets work?

---

## Completed

- [x] USB VCP driver (`serial_usb_vcp_rp2350.c`) — TinyUSB CDC
- [x] `tusb_init()` + 1 ms repeating timer for `tud_task()` in `system_rp2350.c`
- [x] `USE_VCP`, `SERIAL_PORT_COUNT=3` in target.h
- [x] Scheduler starvation root cause found (GYRO exec > period → forcedRealTimeTask)
- [x] Gyro/PID rate override in fc_tasks.c (2000 µs / 500 Hz)
- [x] Dynamic notch filter disabled in targetConfiguration() (config.c)
- [x] M4: Configurator connects, shows board name + firmware version + sensor status
