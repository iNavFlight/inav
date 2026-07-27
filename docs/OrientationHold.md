# Orientation Hold: 3D aerobatics for fixed wing

*(wiki-ready: this page is written so it can be pasted into the INAV
wiki as-is once the feature merges)*

Orientation Hold is a flight mode family that lets a fixed-wing model
hold ANY attitude: sustained inverted flight, knife edge (either side),
a prop hang with hands-free hover throttle, controlled flat spins, and
scripted aerobatic figures flown on a line. It is a quaternion
controller, so there is no gimbal lock and no special-casing at pitch
90 - a loop is just "pitch rotation, 360 degrees".

Status: implemented and flown in a closed-loop JSBSim SITL simulation
(deterministic lockstep, replay videos); first hardware flights are
upcoming, so treat everything here as experimental. Be honest about what
the simulation proves, because it is uneven. The actively driven
behaviours - the flat-spin family, the scripted figures, and the floor
recovery - command real, measured control authority in the bench. The
static holds (inverted, knife edge, prop hang) do NOT yet: the SITL
aerobatic airframes are near-symmetric and self-trim at those attitudes,
and the bench disturbance is a uniform vertical gust that shifts angle of
attack without producing a rolling or pitching moment, so the controller
is never forced to work to hold them. The holds are implemented but
unproven until a hardware flight - or a deliberately destabilised bench
model - makes the airframe actually want to leave the attitude.

## Requirements

- Fixed wing (`platform_type = AIRPLANE`). Multirotors are untouched.
- A target with more than 512 KB flash (F405, F765, H743, ...): the
  feature is excluded on F722/F411 builds to preserve their flash space.
- Barometer required (altitude assist, floor, hover throttle).
- GPS optional but recommended: it gates crash detection in flight and
  hardens the altitude estimate during aerobatics.
- Thrust-vectoring models: assign the servo mixer inputs TVC ROLL /
  PITCH / YAW (61-63) to the vane servos. `tvc_gain` scales them,
  `tvc_thrust_comp` raises vane deflection as thrust drops so the loop
  gain stays constant (vane torque follows thrust).

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

Platform capability is enforced: a knife edge is held on the yaw
effector (rudder or a TVC yaw vane), so on a mixer without one - a
flying wing - the KNIFE boxes are not offered, and a stale
configuration that still maps them is ignored in flight. The laws of
aerodynamics outrank the switch; a wing can still prop-hang, fly
inverted and use every figure. What thrust cannot deliver, no mode can
promise: expect roughly half the work from excess power and half from
the airframe.

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
- Big 3D throws and a smooth fast tune coexist: above the airframe's
  cruise throttle the commanded hold authority scales back with thrust
  (the airflow proxy - surface moment goes with airflow squared), so
  the same throws that hover the model do not over-deflect at speed.
  The hover regime always keeps the full throw. No setting; the closed
  rate loop refines the guess.

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
  adds power while the hold sinks (its gains derive from your own
  operating point - no settings), and a speed feedforward puts more
  nose on the knife immediately when the throttle (the v-squared proxy
  without an airspeed sensor) is low (`ohold_knife_speed_ff`).
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

Full rudder commands at most half a turn per second (a display spin,
not a tumble - independent of your ACRO yaw rate), and the load
governor backs the command off with the measured load (see below). A
stalled airframe can still autorotate faster than commanded; at idle
power the rudder has little authority to hold it back - that is
physics, not a tune.

### 3DLOCK

Sticks centered: the current attitude is captured and held. Sticks
deflected: pure rate flying, and the lock follows - it freezes on
whatever attitude you had when the sticks came back to center. Think of
it as "hold whatever I'm doing" for improvised 3D.

### FLOOR (altitude safety floor)

Problem being solved: this is the PANIC net. Practicing low 3D, a bad
moment ends at the ground - the aircraft dives and the pilot, in the
panic, forgets to flip the aerobatic mode switch back to normal. The
floor is what saves the airframe: whatever mode is selected - any hold,
any figure, a flat spin - sinking through the floor line switches the
aerobatic mode OUT and flies a normal, stable, upright attitude that
pulls up and away from the ground. A stable flight attitude is the thing
that prevents the crash; the floor just makes the aircraft take one.

- Set the floor with `alt_floor_altitude` (meters above home). The
  floor ARMS only after you have climbed above floor + margin once, so
  switching it on before takeoff never grabs the aircraft.
- Sinking THROUGH the floor line is the trigger - no prediction, the
  crossing fires it - and engages an automatic upright + climb recovery
  that OVERRIDES the selected aerobatic mode. It catches out of a dive
  with the elevator still held, and out of a spin.
- The recovery brings its own energy: a throttle floor of cruise +
  pitch compensation, the motor keeps running through a panic-chopped
  stick, and held roll/pitch sticks are ignored (they used to drag the
  recovery target down). Yaw stays live for steering.
- The catch LATCHES OUT whatever aerobatic mode it interrupted - any
  hold, any figure, a flat spin, all the same. The pilot flies again the
  instant they touch the sticks, but the interrupted figure does NOT
  restart on its own: it stays suppressed until the pilot switches its
  mode OFF and back ON. This is the fix for the fly-up / fall-back loop -
  where the figure re-engaged the moment the recovery released and dived
  straight back into the floor, over and over. A forgotten or held-on
  switch cannot drop the aircraft back into the ground.
- The climb ends at floor + `alt_floor_margin` and the aircraft loiters
  there, waiting. Touching the sticks hands manual control back (in your
  base ANGLE/ACRO mode); the latched figure still will not restart until
  you cycle its switch. To land, switch the FLOOR box OFF - only then
  does the aircraft descend through the line instead of being caught.

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
sequencer holds LEVEL at the entry altitude (assist active) until you
release the box; switching the box off at any time aborts instantly.

### Load governor (`ohold_load_limit`)

Problem being solved: a figure flown "fast AND tight" is bounded by one
number - the load. Centripetal load is speed times rotation rate
(radius r = v^2 / a), so an aggressive loop rate at full power reads
double-digit g at the exit pull.

`ohold_load_limit` [g x 10, default 40 = 4 g] is a fact about your
airframe: what it may pull. While the measured load sits above it, the
governor slows the figure's rotation, the target slew (the catch-up
pull toward a distant target is the hardest load of a maneuver, not the
rotation) and - only while a figure or spin flies - bleeds throttle,
because a governed rotation at full power just converts into speed and
keeps the load. Plain holds at 1 g and your normal flying are never
touched. Set it to your airframe's structural rating; 0 disables the
governor entirely.

### Crash detection (`crash_detection`)

*Note: crash detection is a standalone feature (`USE_CRASH_DETECTION`),
independent of the orientation-hold modes and intended for any platform
including multirotors. It is described here for completeness but is being
moved to its own pull request.*

Problem being solved: after an unscheduled arrival the prop keeps
churning until you walk over and disarm.

A crash has one signature: a sharp acceleration spike, then NOTHING. A
spike near the accelerometer's full-scale, followed by the airframe
lying still (no rotation, resting 1 g, frozen raw baro, and - with a
GPS fix - zero ground speed) CUTS the motor while staying armed. Moving
the throttle to zero and up again re-allows it deliberately: short
motor bursts are the most reliable way to find a plane in high grass or
corn. Hand-launch safe (it arms only once clearly flying).

There is no threshold to tune: the impact level is derived from the
detected accelerometer (15% below its full-scale - ~13.6 g on a 16 g
IMU), which even the hardest 3D figure stays clear of, and the
stillness that must follow is what tells a crash from a hard maneuver.
`crash_detection` is ON by default; set it to OFF to disable.

## Learned gains - do not hand-tune these

Four settings look like gains but are **written by the firmware, not by
you**: `ohold_hover_gain`, `ohold_inverted_gain`, `ohold_knife_gain`,
`ohold_figure_gain`. Each is a per-regime damping scale (in %) relative
to your normal-flight PIDs. A limit-cycle detector watches for a buzz in
that regime and backs the scale off, recovering slowly when the buzz is
gone; the value is saved on disarm. Fly a hold or figure a few times and
it settles itself. You only touch these to RESET them (set to 100) if
you changed props/airframe and want the learner to start over. Leave
them alone otherwise.

Everything else below is yours to set.

## First flights and tuning

Do this in order. Each step depends on the one before it being right;
skipping ahead just moves the symptom.

**Step 0 - bench, props off.** Run the level-1 MSP check (bench repo)
and confirm on the ground: each hold box drives the surfaces the right
way (roll the model by hand in INVERT, the ailerons should fight back to
inverted), and the FLOOR box, when you fake a low altitude, commands
nose-up. Wrong sign here is a reversed servo or a wrong mode range, not
a gain.

**Step 1 - trim the airframe physically. This is not optional.** In the
order of the trimming checklist (see the bench repo quick guide):
level trim; CG via the 45-degree inverted test (only a breath of down
elevator should hold the line - move the battery, never the software);
per-side knife-edge coupling; thrust line; aileron differential. Every
later step assumes a trimmed airframe. A hold buzzing or a figure
drifting almost always traces back to a trim you skipped here.

**Step 2 - per-attitude pitch trims.** From those trim flights, set
`ohold_inverted_pitch_trim`, and `ohold_knife_left_pitch_trim` /
`ohold_knife_right_pitch_trim` separately (the sides are not symmetric).
Symptom of too little: the hold sinks or the nose drops in that
attitude. Too much: it balloons/climbs. Aim for a hold that neither
climbs nor sinks with the sticks centered.

**Step 3 - entry feel.** `ohold_entry_rate` (deg/s) is how fast the
target rolls into a hold when you flip the box. Too slow feels mushy and
lags your intent; too fast snaps and can overshoot on a heavy model.
Start at the default and adjust to taste.

**Step 4 - hover.** Hold a prop hang. `ohold_hover_thr_min` is the
throttle floor that keeps prop-wash authority in updrafts - raise it if
the model feels rudderless/limp at the top of the hover, lower it if it
climbs when you back off. There is no hover PID to tune: the hover base
throttle is learned online (from your own stick at engage), and the
altitude-loop gains derive from that learned point at runtime - the
throttle-to-thrust slope is the one airframe fact they all share.
`ohold_hover_baro_weight` raises the baro share of the altitude estimate
in the hover regime and normally stays put. The throttle stick is a
climb-rate command while hovering; a slammed-low stick is still a hard
cut.

**Step 5 - knife edge energy.** `ohold_knife_speed_ff` adds nose-up
angle as throttle (the speed proxy) drops, so the edge holds height at
low speed. Symptom of too little: the knife sinks as you slow down.
Too much: the nose climbs and it balloons off the line. `ohold_stick_angle`
is how far a full roll/pitch stick carves the held attitude off the
preset - taste, larger = more authority to reshape the line by hand;
`ohold_stick_return_rate` is how fast the target eases back to the
preset after you let go.

**Step 6 - figures.** `fig_roll_rate`, `fig_loop_rate`,
`fig_point_dwell` set the one-switch figure speeds. Loop radius follows
from rate and speed (R = v / omega): halve `fig_loop_rate` for double
the radius. The rate settings are the CEILING - the load governor
(`ohold_load_limit`, see above) slows the figure and bleeds throttle
whenever the measured load exceeds the budget, so an aggressive rate is
safe to program: at the budget the figure flies as fast and as tight as
the load allows. The altitude assist (`fig_assist_z_gain`,
`fig_assist_vz_gain`, `fig_assist_max`) holds the entry altitude through
a figure - raise the gains if figures drift down, lower them if the
model pumps altitude during a slow roll.

**Step 7 - the safety floor.** Only once the above is trusted, set
`alt_floor_altitude` (m above home) and `alt_floor_margin`. Test it high:
climb above floor + margin, then push over and HOLD the down elevator -
the floor must catch and level against the held stick.
`alt_floor_climb_pitch` is the recovery climb angle.

## Troubleshooting - symptom to setting

| Symptom | Look at |
| --- | --- |
| Hold buzzes / oscillates in one attitude | first check trim (step 1); to reset a learned gain set the matching `ohold_*_gain` to 100 |
| Inverted / knife sinks with sticks centered | that attitude's pitch trim too low; knife also `ohold_knife_speed_ff` |
| Hold balloons / climbs | pitch trim too high |
| Hover feels limp / rudderless up high | raise `ohold_hover_thr_min` |
| Knife edge drops as it slows | raise `ohold_knife_speed_ff` |
| Entry into a hold snaps / overshoots | lower `ohold_entry_rate` |
| Loop too tight / too wide | `fig_loop_rate` (radius = speed / rate) |
| Figure slower / wider than the rate says, throttle dips in it | the load governor at work - raise `ohold_load_limit` if the airframe is rated for more, or accept the wider line |
| Figure drifts down | raise `fig_assist_z_gain` / `fig_assist_max` |
| Wrong surface direction in a hold | reversed servo or wrong mode range, not a gain (step 0) |
| Floor does not catch | `alt_floor_altitude`/`margin`, and confirm it armed (climb above floor+margin once) |

All settings with their exact ranges are in [Settings.md](Settings.md).

## Simulation

Everything above can be flown against a JSBSim plant through
`MSP_SIMULATOR`. SITL gained a deterministic lockstep mode
(`--lockstep`): the sim clock advances exactly 1 ms per injected frame,
so the same input produces the same flight bit for bit, host load does
not matter, and many SITL instances can run in parallel. Replay videos
of every mode and several scripted routines live in the companion bench
repository, all flown on one configuration.
