# Orientation Hold: 3D aerobatics for fixed wing

*(wiki-ready: this page is written so it can be pasted into the INAV
wiki as-is once the feature merges)*

Orientation Hold is a flight mode family that lets a fixed-wing model
hold ANY attitude: sustained inverted flight, knife edge (either side),
a prop hang with hands-free hover throttle, controlled flat spins, and
scripted aerobatic figures flown on a line. It is a quaternion
controller, so there is no gimbal lock and no special-casing at pitch
90 - a loop is just "pitch rotation, 360 degrees".

Status: bench-validated against a closed-loop JSBSim simulation
(deterministic lockstep, 50-case gust matrix, replay videos); first
hardware flights are upcoming. Treat everything here as experimental.

## Requirements

- Fixed wing (`platform_type = AIRPLANE`). Multirotors are untouched.
- A target built with `USE_ORIENTATION_HOLD` (F7/H7 class; F411 fits).
- Barometer required (altitude assist, floor, hover throttle).
- GPS optional but recommended: it gates crash detection in flight and
  hardens the altitude estimate during aerobatics.

## The modes

| Box | What it does |
| --- | --- |
| INVERT | holds sustained inverted level flight |
| KNIFE L / KNIFE R | holds a left / right knife edge |
| P-HANG | prop hang: nose vertical, hover throttle owns the altitude |
| FLAT SPIN | controlled spin about the earth vertical; combine with the holds |
| 3DLOCK | sticks centered = hold the current attitude, deflected = rate flying |
| FLOOR | altitude safety floor with automatic upright + climb recovery |
| F ROLL / F LOOP / F 4PT | one-switch figures: axial roll, loop, 4-point roll |
| F SEQ | flies a scripted figure sequence (programmed via MSP) |

INVERT, KNIFE L, KNIFE R and P-HANG are four separate boxes; the
natural mapping is one multi-position selector switch with one band per
hold, plus a separate switch for FLOOR and one for the figure bands.

### Holds (INVERT, KNIFE L/R, P-HANG)

Problem being solved: flying inverted or on the knife edge by hand
means holding constant corrective pressure, and any distraction ends
the maneuver. The hold takes over the attitude; you keep flying.

- Engaging a hold slews the target from your current attitude to the
  hold at `ohold_entry_rate` - no snap, no 180-degree surprise.
- Holds are heading-free: the rotation about the earth vertical stays
  yours. Rudder steers in level/inverted flight; at the prop hang the
  free axis is body roll (that is the torque-roll axis).
- Sticks carve held angle offsets from the hold (ANGLE semantics,
  `ohold_stick_angle`); releasing returns the target gently. Yaw is
  always a rate.
- Leaving a hold far from level hands over to ANGLE with a slew to the
  horizon, so a hover exit does not whip through nose down.
- Per-attitude pitch trims: `ohold_inverted_pitch_trim`,
  `ohold_knife_left_pitch_trim`, `ohold_knife_right_pitch_trim` (the
  sides are separate on purpose - prop effects are not symmetric).

### Throttle behavior per hold

Holding the attitude is half the job; each hold also declares what the
throttle means:

- **P-HANG**: a hover throttle PID owns the altitude. Your throttle
  stick commands a CLIMB RATE around the point where you engaged;
  slamming the stick low remains a hard cut (bailout). The hover base
  throttle is learned from your own throttle at engage - there is no
  per-model hover setting to find. Altitude ownership follows the
  ATTITUDE: pull a knife edge up into a harrier and the hover
  controller takes the altitude over seamlessly.
- **KNIFE / INVERT**: the base is your throttle scaled so the forward
  thrust component keeps the speed you chose; a slow vz-to-zero trim
  adds power while the hold sinks (`ohold_assist_thr_p/i`), and a speed
  feedforward puts more nose on the knife immediately when the throttle
  (the v-squared proxy without an airspeed sensor) is low
  (`ohold_knife_speed_ff`).
- **Stall reserve**: sustained control effort toward saturation raises
  power while the attitude still looks clean - the early warning. By
  the time an attitude degrades, the escalation chain has already gone
  through effort trend, then sinking, then oscillation.

### FLAT SPIN family

Problem being solved: a spin mode wired to body yaw is only correct in
a flat attitude - inverted or knife-edge spins would be impossible.

The spin command is a rotation about the EARTH VERTICAL - exactly the
axis the holds leave free - distributed onto the body axes from the
current tilt. The identical mode therefore does:

- FLAT SPIN alone: the classic upright flat spin,
- FLAT SPIN + INVERT: inverted flat spin,
- FLAT SPIN + KNIFE L/R: knife-edge spin,
- FLAT SPIN + P-HANG: torque roll.

Your rudder commands the rotation rate, with aircraft-referenced sense:
right rudder spins the airframe right when upright AND when inverted -
seen from above, an inverted spin reverses, like a real aircraft.
Releasing the rudder stops the rotation with the attitude still held;
releasing the box recovers.

### 3DLOCK

Sticks centered: the current attitude is captured and held. Sticks
deflected: pure rate flying, and the lock follows - it freezes on
whatever attitude you had when the sticks came back to center. Think of
it as "hold whatever I'm doing" for improvised 3D.

### FLOOR (altitude safety floor)

Problem being solved: practicing low 3D means a mistake reaches the
ground before you do.

- Set the floor with `alt_floor_altitude` (meters above home). The
  floor ARMS only after you have climbed above floor + margin once, so
  switching it on before takeoff never grabs the aircraft.
- A predicted breach (sink rate looked ahead a few seconds) engages an
  automatic upright + climb recovery that OVERRIDES the selected mode.
  It catches out of a dive with the elevator still held, and out of a
  spin.
- The recovery brings its own energy: a throttle floor of cruise +
  pitch compensation, the motor keeps running through a panic-chopped
  stick, and held roll/pitch sticks are ignored (they used to drag the
  recovery target down). Yaw stays live for steering.
- The climb ends at floor + `alt_floor_margin`. To take over earlier:
  center the sticks once, then any fresh roll/pitch input hands control
  back immediately. Switching the box off always ends it.

### Figures (F ROLL, F LOOP, F 4PT, F SEQ)

One-switch figures fly an axial roll, a loop (`fig_loop_rate` - radius
is rate and speed: R = v / omega) or a 4-point roll. F SEQ flies a
scripted sequence of segments (roll / pitch / hold / wait-altitude /
wait-time / impulse / wait-position / spin), programmed over MSP;
community tooling can turn a written routine into such a script.

Figures fly ON A LINE: the heading captured at figure start anchors the
trajectory, and the full attitude error is regulated - a slow roll
stays on its string instead of walking off course. An altitude assist
holds the entry altitude through the figure. After the last segment the
sequencer is done - switch back to your normal mode; there is no
automatic level-off yet.

### Crash detection (`crash_g_threshold`)

Problem being solved: after an unscheduled arrival the prop keeps
churning until you walk over and disarm.

An impact spike above the threshold, followed by the airframe lying
still (no rotation, resting 1 g, frozen raw baro, and - with a GPS
fix - zero ground speed) CUTS the motor while staying armed. Moving the
throttle to zero and up again re-allows it deliberately: short motor
bursts are the most reliable way to find a plane in high grass or corn.
Hand-launch safe (it arms only once clearly flying). Opt-in, 0 = off.
Without GPS, keep the threshold above your figures' g load - a smooth
level line right after a hard pull is indistinguishable from lying
still on IMU + baro alone.

## Learned gains

Normal-flight gains are the reference. Each hold regime (hover;
inverted/knife/figures) learns its own damping scale from its own limit
cycles and persists it on disarm - fly a figure repeatedly and it gets
better. No per-regime hand tuning.

## Setup order

1. Trim the airframe physically first (level trim, CG via the 45-degree
   inverted test, per-side knife-edge coupling, thrust line, aileron
   differential). Never paper over a bad CG with software trim.
2. Set the per-attitude pitch trims from those flights.
3. Let the hover gain learner converge (a few prop hangs).
4. Then figure rates and the altitude assist.

All `ohold_*`, `alt_floor_*`, `fig_*` and `crash_g_threshold` settings
are documented in [Settings.md](Settings.md).

## Simulation

Everything above can be flown against a JSBSim plant through
`MSP_SIMULATOR`. SITL gained a deterministic lockstep mode
(`--lockstep`): the sim clock advances exactly 1 ms per injected frame,
so the same input produces the same flight bit for bit, host load does
not matter, and many SITL instances can run in parallel. Replay videos
of every mode and several scripted routines live in the companion bench
repository, all flown on one configuration.
