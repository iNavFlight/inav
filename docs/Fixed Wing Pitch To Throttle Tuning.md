# Fixed-Wing Pitch-to-Throttle Tuning (`nav_fw_pitch2thr`)

## What This Setting Does

`nav_fw_pitch2thr` adjusts throttle in response to pitch angle, to help maintain
constant airspeed as the aircraft climbs or descends in GPS-assisted modes
(Position Hold, RTH, Waypoint Mission, Cruise).

**Formula (from the setting's own description):**

```
throttle = nav_fw_cruise_throttle - (nav_fw_pitch2thr * pitch_angle)
```

`pitch_angle` is in degrees, negative when climbing and positive when diving,
so this adds throttle in a climb and removes it in a dive. The result is
clamped between `nav_fw_min_thr` and `nav_fw_max_thr`.

**Units:** PWM microseconds per degree of pitch (μs/°)

**Range:** 0-100. **Default:** 10.

This setting only affects GPS-assisted navigation modes — manual/Acro/Angle
flight is unaffected, so it's safe to test in Position Hold without touching
the way the aircraft flies under direct pilot control.

Two related settings shape how the correction is applied:

- `nav_fw_pitch2thr_threshold`: a deadband (in decidegrees) around the
  average pitch. Momentary pitch excursions inside the deadband get a
  smoothed (low-pass filtered) correction instead of the instantaneous one,
  to avoid jerky throttle response to small attitude noise.
- `nav_fw_pitch2thr_smoothing`: controls the strength of that low-pass filter.

During the final approach phase of a fixed-wing autoland,
`nav_fw_land_final_approach_pitch2throttle_mod` scales `nav_fw_pitch2thr` up
(as a percentage) to cut power more aggressively when the nose points down —
see `Fixed Wing Landing.md`.

## Why It Matters

- **Too low** → airspeed bleeds off during climbs, builds up during descents.
- **Correct** → airspeed stays roughly constant through altitude changes.
- **Too high** → throttle over-compensates; airspeed changes in the opposite
  direction, and throttle may saturate at gentle pitch angles.

The default value of 10 μs/° is conservative: a 10° climb only adds 100 μs of
throttle, and throttle only reaches maximum at 50° of pitch — an angle most
fixed-wing aircraft never reach. In practice this means many aircraft see
some airspeed loss in sustained climbs and some airspeed gain in sustained
descents with the default value. Whether that matters for your aircraft, and
how much to increase the setting, is best determined by flight testing your
specific airframe rather than by a single number — aircraft vary widely in
lift-to-drag ratio and thrust-to-weight ratio, both of which affect the ideal
value (see `development/fixed-wing-pitch2thr-tuning.md` in the source tree
for the underlying aerodynamics, and why they don't converge on one
"correct" number even for the same setting).

## Flight Testing & Tuning Procedure

### Step 1: Baseline Test

1. Fly in Position Hold or Loiter mode (GPS stabilized).
2. Note the current `nav_fw_pitch2thr` value.
3. Command a climb (e.g. +50 m altitude change) and observe airspeed:
   - Decreases noticeably → setting too low.
   - Stays roughly constant → setting about right.
   - Increases → setting too high.
4. Repeat for a descent (-50 m) — same logic, opposite direction (increasing
   airspeed on descent means the setting is too low; decreasing means too
   high).

### Step 2: Adjust

- Airspeed drops in climbs, or builds up in descents → the setting is
  under-compensating. Increase `nav_fw_pitch2thr` by roughly 10-15 μs/° and
  retest.
- Airspeed increases in climbs, or throttle saturates (hits min/max) at
  gentle pitch angles (well under 10°) → the setting is over-compensating.
  Decrease by roughly 5-10 μs/° and retest.

### Step 3: Iterate

Repeat the climb/descent tests after each adjustment until airspeed stays
within about ±1 m/s through altitude changes in Position Hold.

### Step 4: Edge Cases

- A steeper climb test (if safe) should approach maximum throttle at a
  realistic pitch angle for your aircraft — if it doesn't, the setting may
  still be conservative.
- A moderate descent test should reduce throttle noticeably; if airspeed
  keeps building up in descent, increase the setting further.

## Signs of Incorrect Tuning

**Too low:**
- Airspeed drops during climbs, builds up during descents.
- Altitude overshoots as the altitude controller pitches harder to
  compensate for a climb that's losing speed.
- Speed variations during RTH altitude changes.
- Possible low-speed alarms during climbs.

**Too high:**
- Airspeed increases during climbs, decreases during descents.
- Throttle hits min/max at gentle pitch angles.
- Altitude "hunting" from over-correction.

**About right:**
- Airspeed stays within roughly ±1 m/s through altitude changes.
- Smooth altitude transitions, no speed alarms, predictable behavior in
  Position Hold/RTH.

## Starting Points by Aircraft Type

These are starting points for the flight-test procedure above, not
guaranteed-correct values — treat them the same way you'd treat any other
PID starting point in this manual, as something to verify by flying, not
something to set and forget:

| Aircraft Type | Typical Starting Range |
|---|---|
| Glider / sailplane (high L/D, low power) | 15-25 μs/° |
| Trainer / cruiser | 25-35 μs/° |
| Sport / FPV wing | 25-40 μs/° |
| Aerobatic / high power-to-weight | 15-30 μs/° |

If you don't know which category fits, start around 25 μs/° and use the
flight-test procedure to refine it.

## Related Documentation

- `Fixed Wing Landing.md` — `nav_fw_land_final_approach_pitch2throttle_mod`
  and other landing-phase throttle behavior.
- `Battery.md`, `Inflight Adjustments.md` — battery-profile scoping and
  in-flight adjustment of this and related FW throttle settings.
- `development/fixed-wing-pitch2thr-tuning.md` — aerodynamic background on
  why this setting behaves the way it does, for developers working on the
  navigation throttle code.
