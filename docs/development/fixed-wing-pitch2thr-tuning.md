# Fixed-Wing Pitch-to-Throttle: Implementation and Aerodynamic Background

This document explains the aerodynamics behind `nav_fw_pitch2thr` for
developers working on the fixed-wing navigation throttle code. For
user-facing tuning guidance, see `docs/Fixed Wing Pitch To Throttle
Tuning.md`.

## Current Implementation

**Function:** `fixedWingPitchToThrottleCorrection()` in
`src/main/navigation/navigation_fixedwing.c`, called from
`applyFixedWingPitchRollThrottleController()` in the same file.

**Core formula:**

```c
int16_t pitchToThrottle = currentBatteryProfile->nav.fw.pitch_to_throttle;
// ... pitch is low-pass filtered via a PT1 filter; the deadband decides
// whether the raw or filtered pitch is used:
return DECIDEGREES_TO_DEGREES(pitch) * pitchToThrottle;
```

The resulting `throttleCorrection` is:

1. Clamped to `[min_throttle - cruise_throttle, max_throttle - cruise_throttle]`
   (or `[min_throttle - cruise_throttle, 0]` during `NAV_CTL_LAND`, since
   landing should never raise throttle for a nose-up moment).
2. Added to `cruise_throttle`.
3. Clamped again to the battery profile's absolute `[min_throttle,
   max_throttle]`.

(In `NAV_CTL_POS` mode outside of landing, `applyFixedWingMinSpeedController()`'s
output is also added and the result re-clamped before this final step — a
separate minimum-airspeed mechanism, not part of `nav_fw_pitch2thr` itself.)

During `USE_FW_AUTOLAND` final approach, `pitchToThrottle` is additionally
scaled by `navFwAutolandConfig()->finalApproachPitchToThrottleMod / 100.0f`
(`nav_fw_land_final_approach_pitch2throttle_mod` in settings.yaml) when
diving, to cut power more aggressively on short final.

**Setting:** `nav_fw_pitch2thr` — `src/main/fc/settings.yaml`, field
`nav.fw.pitch_to_throttle`, `uint8_t`, range 0-100, default 10. Units are
PWM microseconds of throttle correction per degree of pitch. Related
settings: `nav_fw_pitch2thr_threshold` (deadband, decidegrees, default 50 =
5°) and `nav_fw_pitch2thr_smoothing` (PT1 filter strength, default 6).

This is a **local linear gain**, not a lookup table or envelope mapping —
for every degree of pitch change, throttle changes by a fixed number of
microseconds, clamped by the battery profile's throttle range.

## Why a Linear Approximation Is Used

The physical relationship between flight path angle and required power
involves `sin(γ)`, which is nonlinear. For small angles, `sin(γ) ≈ γ` (γ in
radians):

| γ | sin(γ) | γ (rad) | Error |
|---|---|---|---|
| 5° | 0.0872 | 0.0873 | 0.11% |
| 10° | 0.1736 | 0.1745 | 0.52% |
| 15° | 0.2588 | 0.2618 | 1.16% |

(Houghton & Carpenter, *Aerodynamics for Engineering Students*, 5th ed.)

Below about 15° the linear approximation is accurate to ~1%, which is well
inside the accuracy of the attitude estimate itself — so a simple
multiplication is an adequate model for the pitch range fixed-wing
navigation actually flies at (most GPS-assisted maneuvering stays within
±20°). The real-world limiting factor isn't the linear approximation — it's
that the aircraft's throttle range (typically ~1000 μs of usable span) is
too narrow to fully compensate for the power change theory calls for during
a sustained steep climb; clamping, not curve-fitting error, is what
dominates behavior at the edges of the envelope.

## Three Theoretical Estimates for the Gain — and Why They Disagree

Three independent, individually-valid simplified aerodynamic models can be
used to estimate a "correct" value for `nav_fw_pitch2thr` from an aircraft's
L/D and thrust-to-weight (T/W) ratio. **They give substantially different
answers for the same aircraft**, because each linearizes around a different
flight regime. None of the three has been implemented, flight-tested, or
reviewed as a proposed default change — they're presented here as
aerodynamic reasoning aids, not as recommendations.

### Model 1: Climb/descent power ratio

Power required to hold constant airspeed at flight path angle γ:

```
P_climb   = P_level × (1 + sin(γ) × L/D)
P_descent = P_level × (1 - sin(γ) × L/D)
```

(Force balance: thrust = drag ± W·sin(γ) along the flight path; power =
thrust × V. Houghton & Carpenter pp. 26-44.) Approximating throttle as
proportional to power and using the small-angle substitution:

```
nav_fw_pitch2thr ≈ throttle_cruise × 0.01745 × L/D
```

For `throttle_cruise = 1500 μs`, `L/D = 10`: **≈262 μs/°** theoretical — but
this saturates the throttle range at only ~2° of pitch, which isn't usable.
Backing off to fit a realistic ±10-20° working range before saturation
(`(max_throttle - cruise_throttle) / max_practical_climb_angle`) gives
**~25-50 μs/°** depending on the assumed working range.

### Model 2: Local force-derivative at level flight

Force balance at any pitch angle γ: `T = W[cos(γ)/(L/D) + sin(γ)]`. Taking
the derivative with respect to γ and evaluating at γ = 0 (level flight):

```
dT/dγ = W / 57.3   (per degree)
```

Converting to a throttle-fraction gain using T/W = α (thrust-to-weight at
full throttle) and INAV's 1000 μs range:

```
nav_fw_pitch2thr ≈ 1000 / (57.3 × α)  ≈  17.5 / α   μs/°
```

For α = 1.0 (roughly 1:1 thrust-to-weight, typical of many RC aircraft):
**≈17.5 μs/°**. This model is L/D-independent at γ = 0 — only T/W enters at
the linearization point — and predicts that *higher* T/W aircraft need a
*lower* gain, since the same absolute thrust change is a smaller fraction
of a larger available thrust.

### Model 3: Gravity-only vertical flight (90°)

At exactly 90° pitch, drag depends only on airspeed and cancels when
comparing a steady vertical climb to a steady vertical descent at the same
airspeed, leaving a pure thrust-vs-weight balance. For α = T/W = 1 (climb is
hover-only, V=0), spreading the full 1000 μs range over the ±90° envelope
gives:

```
nav_fw_pitch2thr = 1000 μs / 90° ≈ 11.1 μs/°
```

This happens to be close to the current default of 10. Whether that's
actually why 10 was chosen isn't verifiable — the setting predates the
earliest history available in this repository (a single "commit
everything" import), so there's no changelog or commit message to check.
Treat the closeness as a coincidence worth noting, not a confirmed
explanation.

### Why the three disagree

Each model linearizes the same underlying force/power balance around a
different point or regime — Model 1 integrates power over a realistic climb
angle range, Model 2 takes a derivative strictly at level flight, and Model
3 only considers the 90° extremes where drag cancels by construction. None
of them accounts for the aspects the "Limitations" sections below list
(propeller thrust curve nonlinearity, motor efficiency vs. load, propwash,
dynamic/transient effects), which is why empirical, flight-tested tuning
(see the user-facing guide) is more reliable than any single formula here.

## Known Limitations of the Current (Linear) Model

- **No adaptation to cruise throttle.** The same gain applies regardless of
  current cruise throttle percentage, which itself may vary with battery
  voltage sag or payload.
- **No adaptation to L/D regime changes** (flaps, airspeed-dependent drag
  polar).
- **Assumes throttle ∝ power ∝ thrust**, none of which is exactly true for
  a propeller: thrust and power both vary nonlinearly with airspeed and RPM,
  and motor/ESC efficiency varies with load.
- **No propwash modeling** — tractor-configuration aircraft get extra wing
  lift from propwash during climb, which isn't represented.
- **Steady-state only** — pitch rate and acceleration transients aren't
  modeled; the PT1 filter and deadband exist specifically to avoid reacting
  to short-duration attitude noise rather than sustained pitch changes.

None of these are currently addressed in `fixedWingPitchToThrottleCorrection()`
— they're listed here as context for anyone considering changes to the
throttle-correction model, not as a to-do list.

## References

**Source:** `src/main/navigation/navigation_fixedwing.c`
(`fixedWingPitchToThrottleCorrection`,
`applyFixedWingPitchRollThrottleController`), `src/main/fc/settings.yaml`
(`nav_fw_pitch2thr` and related settings).

**Aerodynamic theory:** Houghton, E.L. and Carpenter, P.W., *Aerodynamics
for Engineering Students*, 5th ed., Butterworth-Heinemann — pp. 26-44 (force
equilibrium, drag, climb/descent power), pp. 62-67 (performance
calculations, small-angle validity).

**User-facing guide:** `docs/Fixed Wing Pitch To Throttle Tuning.md`.
