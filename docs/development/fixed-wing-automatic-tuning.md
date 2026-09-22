# Automatic fixed-wing tuning: implementation proposal for INAV 10.x

**Status: design proposal, not an implemented flight mode.** This document
specifies an automatic commissioning sequence for conventional fixed-wing
aircraft, including two-servo flying wings. It does not enable excitation,
change gains, or declare a vehicle safe to fly.

## Objective and boundaries

After takeoff and establishing stable flight, the pilot starts one sequence
with one switch. The firmware coordinates servo trim, roll/pitch identification,
rate-controller tuning, attitude-controller tuning, level trim, and validation.
Optional extensions cover gain scheduling and navigation altitude/throttle
control. The pilot must not have to switch between AUTOTUNE, AUTOTRIM and
AUTOLEVELTRIM to advance the sequence.

The initial supported mixer is a conventional fixed-wing mixer with independent
roll and pitch authority. A flying wing does not require a yaw actuator: the
default axis mask is roll/pitch, and yaw identification is not attempted.
VTOL transitions, tailsitters, rudder-only roll control, flaps, and changing
mixer geometry are outside the first implementation's supported envelope.

The aircraft must already fly and stabilize with its initial settings. This
is not a first-launch recovery system. Sensor calibration, control direction,
mechanical travel, mixer configuration, and centre of gravity remain preflight
work. No stall search, maximum-speed discovery, battery-capacity estimation,
power-consumption survey, or range-estimator tuning is part of this proposal.

## Existing implementation and related work

The baseline is `maintenance-10.x`, inspected at
`86a0441c0003466d54a7711e7240d8297c8d067f`. Integrations must be checked again
against the target branch when code is submitted.

| Component | Existing behaviour | Required new work |
| --- | --- | --- |
| `flight/pid_autotune.c` | Learns FF and, depending on mode/settings, maximum rates from pilot manoeuvres. Does not tune P/I/D. | Automated excitation, model/measurement acceptance, controller synthesis and independent validation. |
| `flight/servos.c` | Continuous servo trim can move neutral positions using stable I-term contributions; it saves on disarm. | Phase gating, bounded candidate trims and a transaction-aware persistence path. |
| `flight/pid.c` | Auto-level trim adjusts the pitch datum from vertical velocity when altitude control is inactive. | Eligibility/convergence reporting and explicit commit/rollback of its runtime state. |
| `flight/pid.c` / `fc/control_profile.*` | APA/TPA already scale gains; the level controller has a shared P gain and a setpoint filter. | Reuse scheduling; validate both axes before changing the shared level gain. |
| `navigation/navigation_fixedwing.c` | Existing altitude, speed and pitch-to-throttle controllers. | A separately validated navigation-tuning stage, not an import of another autopilot's energy controller. |
| `fc/rc_modes.c` | Gives existing tuning modes priority: AUTOTUNE, then AUTOTRIM, then AUTOLEVELTRIM. | One owner for the automatic session, without fabricating receiver mode bits. |

Related INAV work:

- [#7461](https://github.com/iNavFlight/inav/pull/7461) removed P/I/D values
  derived from FF because they gave poor results, particularly on flying wings.
  The new tuner must not reintroduce that method.
- [#7056](https://github.com/iNavFlight/inav/pull/7056) proposed discovering
  fixed-wing acceleration limits. Recheck its implementation and status before
  adding acceleration discovery; it is not a prerequisite of this sequence.
- [#11042](https://github.com/iNavFlight/inav/pull/11042) and
  [#11222](https://github.com/iNavFlight/inav/pull/11222) cover airspeed-dependent
  attenuation and validation. Do not add a competing speed-scaling path.
- [#12010](https://github.com/iNavFlight/inav/pull/12010) is a multirotor
  measurement proposal. Its excitation frequencies, controller assumptions,
  and flight adapter are not suitable fixed-wing defaults.

The open PR search on 2026-09-22 did not find an equivalent combined automatic
fixed-wing tuning sequence. This is a search result, not a claim that every
external branch or experiment has been examined.

## Pilot workflow

1. Configure the aircraft normally and select a documented tuning envelope.
   The configurator should explain missing prerequisites before flight.
2. Take off and establish stable flight inside that envelope.
3. Activate the sequence switch. The OSD shows the current phase, progress,
   and the reason for any wait, failure, or unavailable optional stage.
4. The sequence runs its tests and checks the candidate settings. Stick input,
   switching the sequence off, or a higher-priority flight action ends the
   experiment immediately.
5. A successful sequence retains candidates temporarily for the remainder of
   the flight. After landing, the pilot explicitly accepts or discards them.
   Accepting is one action for the entire result; it is not a series of
   separate mode changes.

Suggested OSD messages are `TUNE: SERVO TRIM`, `TUNE: ROLL`, `TUNE: PITCH`,
`TUNE: ANGLE`, `TUNE: LEVEL`, `TUNE: CHECK`, and `TUNE: RESULT READY`.
The existing settings-save gesture must not accidentally commit an incomplete
session. A timeout means failure or an incomplete stage, never success.

An outcome must distinguish completed required stages, skipped optional stages,
and failed stages. An altitude-tuning stage that was not run must not be
reported as a fully completed tune.

## Session ownership and integration

Use an explicit session state machine rather than toggling existing RC modes
on a timer. The session owns its candidate parameters and temporary excitation;
normal navigation, failsafe and pilot control retain higher priority.

Conceptual states:

```
IDLE -> PREFLIGHT_CHECK -> CAPTURE_BASELINE -> ESTABLISH_FLIGHT
     -> SERVO_TRIM -> IDENTIFY_ROLL -> IDENTIFY_PITCH
     -> DESIGN_RATE_GAINS -> CHECK_RATE_GAINS
     -> TUNE_ATTITUDE -> LEVEL_TRIM -> RECHECK_SERVO_TRIM
     -> OPTIONAL_SPEED_CHECK -> OPTIONAL_NAV_TUNE -> FINAL_CHECK
     -> RESULT_READY -> LAND_AND_ACCEPT -> COMMITTED

Any active state -> ABORT -> RESTORE -> IDLE (after switch reset)
```

Each measurement phase can return to ESTABLISH_FLIGHT between test blocks.
There must be a finite retry/session limit. Rechecking servo trim after level
trim is necessary because the equilibrium angle and neutral surface positions
are coupled. Cap the number of passes and reject non-convergence.

Separate the following responsibilities so they can be tested independently:

- A numerical estimator and controller-design module with no parameter writes,
  receiver reads, motor writes, or implicit clock calls.
- A sequence module that accepts timestamped observations and returns bounded
  test requests, phase/status, and candidate updates.
- A flight adapter that verifies eligibility, arbitrates requests, gathers
  actual applied signals, and cancels excitation before control output.
- A parameter transaction that owns snapshot, trial, restore and commit.

The adapter must use actual controller/mixer signals, not assume that a
requested test reached the aircraft unchanged. Integration points must expose
controller saturation, servo headroom, rate/angle limiting, and the effective
APA/TPA factors. Mixer sums and asymmetric servo travel can saturate a flying
wing even when each axis PID output is individually below its limit.

Changing control, battery or mixer profiles, live adjustments, CMS/MSP writes,
or programming overrides during an active session must abort or be rejected
through an explicit ownership mechanism. Do not silently overwrite unrelated
settings during rollback.

## Flight path and eligibility

There are two different kinds of phase:

- Identification and gain-validation phases can run with qualified navigation
  containment, provided that its commands are accounted for in the signal path.
- Servo and level-trim phases need qualified straight flight. In particular,
  the level-trim learner must not compete with altitude hold.

The navigation adapter must therefore provide bounded straight measurement
legs and recovery/turn segments. It must stop a measurement and recover before
the configured altitude or flight-area boundary is reached. Do not merely
disable altitude control and allow an unbounded straight run. The particular
navigation interface and available margin must be demonstrated in closed-loop
simulation before enabling an autonomous sequence.

Eligibility includes valid attitude and gyro observations, valid navigation
position/velocity, adequate altitude and containment margin, the intended
fixed-wing mixer, usable control headroom, valid receiver input, and stable
flight conditions. Test amplitude, bank/pitch limits and minimum/maximum
airspeed must be explicit parts of the approved test envelope. They are not
values to discover by deliberately approaching loss of control.

Airspeed validity needs special treatment. Ground speed alone is not proof of
adequate airspeed. A real validated pitot sensor is preferred. A sensorless
path requires a demonstrated wind/airspeed estimate with freshness and quality
checks. Until such a path is validated, the adapter must reject unsupported
speed-dependent stages rather than silently use GPS speed as airspeed.

Pilot takeover, receiver loss, failsafe/RTH/landing, navigation validity loss,
containment loss, excessive attitude/rate, output saturation, observation
gaps, or non-finite numbers must remove excitation on the first control update
that observes the condition. Aborting must not suppress the requested failsafe
or pilot action. A switch held high must not restart the session after abort,
completion, disarm/rearm, or a temporary loss of eligibility.

## Rate-controller identification and synthesis

Roll and pitch are identified separately with bounded, smooth excitation.
Choose the frequency band and amplitude from servo/airframe response and
simulation results; do not reuse a multirotor sweep. Excitation must produce
enough information without exhausting attitude or actuator headroom.

The measured plant input is the actual applied control signal, including
relevant mixing/filtering/limiting effects; the output is the matching gyro
rate with known timing. The estimator must account for trim offsets, sample
timing, actuator delay, closed-loop measurement bias, and airspeed changes.
Commanded rate divided by measured rate is not a plant transfer function.

Before producing candidate gains, require:

- sufficient independent excitation and signal-to-noise ratio;
- finite, numerically well-conditioned model coefficients;
- plausible sign, gain, delay and dynamics;
- prediction performance on measurements not used to fit the model;
- rejection of saturation, flexible-airframe modes and unmodelled coupling
  that invalidate the chosen model class;
- controller stability/robustness checks using the actual INAV signal path.

Controller synthesis must respect INAV's additive FF, P, I and derivative-on-
measurement structure, gain units, filters, I-term lock, integrator limiting,
and gain scheduling. A proven algorithm from another flight stack is a useful
reference, but its coefficients are not directly transferable.

Candidate gains must be bounded, introduced without an output discontinuity,
and evaluated against the baseline using comparable held-out manoeuvres.
Reject a candidate that fails tracking, damping, actuator-use or robustness
criteria. A four-second quiet segment alone is not proof of a good tune.
Do not increase configured maximum rates merely because a small-signal model
predicts that the aircraft could rotate faster.

## Attitude-controller tuning

Auto-level trim and attitude tuning solve different problems. Trim finds the
level-flight datum; attitude tuning sets how the aircraft approaches a target
angle.

In INAV, `fw_p_level` is shared by roll and pitch. `fw_i_level` is a filter
cutoff, and `fw_d_level` controls the HORIZON transition; these are not three
ordinary attitude PID gains. Keep the filter and HORIZON behaviour fixed in
the first implementation. Derive a candidate shared P from both accepted rate
responses and validate it on both axes with bounded angle commands. The slower
axis must constrain the result. Never tune it from roll alone and assume the
pitch response will also be acceptable.

## Scheduling and oscillation monitoring

Keep the existing APA/TPA machinery as the single gain-scheduling mechanism.
A first tune at one operating point does not identify a speed schedule. An
optional extension can validate the result at more than one already-approved
airspeed. Automatic changes to schedule parameters require repeatable results
at those operating points and a validated speed source.

Oscillation monitoring is a distinct runtime feature, not another trim phase.
Its detector must distinguish commanded excitation, turbulence, sensor noise,
and sustained control-loop oscillation. Specify which terms are reduced, the
minimum gain, release/recovery behaviour, and its interaction with I-term lock
and integrator state. It must never raise gains above the nominal tune.

During a tuning trial, activation of compression invalidates the trial unless
the estimator explicitly models that intervention. A tune must not be marked
successful because compression concealed unstable nominal gains. Recovery to
the baseline and routine pilot/failsafe takeover still take precedence.

## Optional altitude and throttle tuning

This is a separate stage after rate and attitude validation. INAV's navigation
controller is not ArduPilot TECS, so porting TECS parameter rules is not valid.

Test bounded altitude/climb-rate changes within the approved flight envelope.
Evaluate pitch response, height/vertical-speed tracking, speed preservation,
throttle response and saturation. Candidate altitude gains and pitch-to-
throttle feedforward must be identified separately enough to avoid one
controller hiding errors in the other. A pitch trim measurement must never run
concurrently with these tests.

No automatic search for stall speed or maximum climb capability is required.
The stage may only operate inside previously configured limits. Sensorless
airspeed support and behaviour with voltage sag, wind, delayed barometric
measurements and weak propulsion require explicit validation before support
is claimed. Cruise power and electrical consumption remain out of scope.

## Parameter persistence

Capture the affected settings and the identity of their profiles at session
start. Keep candidate results distinct from persistent settings. Cover servo
midpoints, runtime level trim and its integrator, rate gains, shared level gain,
and any enabled navigation/scheduling candidates.

Existing servo-trim saves on disarm and level-trim updates on disarm must be
made transaction-aware. Handle every configuration-save path: an unrelated
save must not persist trial gains. A power loss or reset before acceptance must
load the last accepted configuration.

Rollback restores the affected settings to their original profiles and resets
or transfers controller state in a tested way. Restoring coefficients alone
does not remove a contaminated integrator or filter state. On final acceptance,
commit the result using the existing configuration persistence mechanism,
without introducing an in-flight flash write.

## Validation and release gates

The following are requirements, not results already obtained:

| Layer | Required evidence |
| --- | --- |
| Numerical tests | Known plants, independent validation data, noise, offsets, sample jitter, delay, weak excitation, ill-conditioning, reversed response, non-finite inputs and unstable/inadequate models. |
| Controller tests | Actual gain units and filtering, I-term lock, APA/TPA, derivative convention, integrator limits, slew limits and saturation; compare baseline and candidate under identical disturbances. |
| Sequence/adapter tests | Every transition, timeout and retry; pilot/failsafe preemption; timestamp wrap; receiver/startup edge cases; sensor loss; mixer/profile changes; no yaw command on a two-servo wing. |
| Persistence tests | Successful accept, discard, abort, disarm, reset/power loss, unrelated save and changed-profile cases; no partial candidate settings after restart. |
| Closed-loop simulation | At least a flying wing and conventional plane; different servo speeds, asymmetric travel, coupled pitch/roll, wind/turbulence, actuator saturation, navigation containment and sensor failures. Use independent flight dynamics, not only the estimator's own model equations. |
| Target builds | Representative F4, small-flash F7, H7 and AT32 targets; flash/RAM/ITCM and worst-case execution cost. Unsupported targets must explicitly omit the feature. |
| Hardware/flight validation | Bench output/preemption tests, then staged identification-only and tuning trials with logs and baseline comparison. Simulator success is not a physical flight test. |

No phase may be reported complete solely because its timer expired. Define
and publish measurable acceptance thresholds alongside the implementing code,
with evidence for those thresholds. Thresholds and excitation envelopes are
intentionally not presented here as flight-proven constants.

## Implementation series

Keep code submissions focused and independently reviewable:

1. Measurement/estimator and replay tests, including rejected-model cases.
2. Controller synthesis and validation using the actual INAV rate/angle loops.
3. Session ownership, reversible trim/gain transactions, and persistence tests.
4. Flight-sequence/navigation integration, OSD status and one-switch workflow.
5. Optional speed-envelope validation and scheduling changes.
6. Optional navigation altitude/throttle tuning.
7. Independently tested oscillation monitoring/compression.

The first usable automatic sequence requires items 1-4 together and the
applicable validation gates. Merging an estimator alone must not expose a
mode that claims to complete the full tune. Optional stages need explicit
support/status reporting, not placeholder success paths.

## External references

- [PX4 fixed-wing autotune](https://docs.px4.io/main/en/config/autotune_fw):
  automatic excitation, selectable axes, model-based tuning and trial rollback.
- [PX4 fixed-wing implementation](https://github.com/PX4/PX4-Autopilot/tree/main/src/modules/fw_autotune_attitude_control)
  and [controller synthesis](https://github.com/PX4/PX4-Autopilot/tree/main/src/lib/pid_design):
  useful algorithm references; the examined fixed-wing adapter applies PI/FF,
  not a directly transferable INAV PID parameter set.
- [ArduPilot Plane autotune](https://ardupilot.org/plane/docs/automatic-tuning-with-autotune.html):
  FF/P/I/D tuning, configurable response and airspeed scaling.
- [PX4 gain compression](https://docs.px4.io/main/en/features_fw/gain_compression)
  and [ArduPilot limit-cycle detection](https://ardupilot.org/plane/docs/common-servo-limit-cycle-detection.html):
  runtime oscillation handling.
- [ArduPilot TECS tuning](https://ardupilot.org/plane/docs/tecs-total-energy-control-system-for-speed-height-tuning-guide.html):
  a reference for separating attitude-loop tuning from altitude/speed control,
  not evidence that TECS parameters map to INAV navigation gains.

Any adapted source must preserve its required copyright and licence notices.
