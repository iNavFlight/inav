# Experimental multirotor chirp measurement

Chirp generates a repeatable, single-axis rate excitation for Blackbox analysis.
It is a measurement tool: it does **not** select or change PID gains, filters,
rates or EZ-Tune settings. The initial implementation is intended for developer
validation; automated recommendations and a Configurator workflow are separate
work. It has not been validated in real flight.

## Relationship to existing work

[Q-Tune #8546](https://github.com/iNavFlight/inav/pull/8546) detects oscillations
from flight samples. This feature supplies a known excitation signal for
identification experiments instead. It neither copies nor replaces Q-Tune's
detector. Both relate to [the multirotor autotuning request
#10433](https://github.com/iNavFlight/inav/issues/10433); this measurement feature
alone does not resolve that request.

## Scope and prerequisites

- Conventional multirotor mixer and multirotor PID controller; no tricopters,
  helicopters, VTOL transitions, fixed wings or reversible motors.
- Manual-throttle ANGLE flight only. Navigation modes, heading hold, horizon,
  failsafe and programming overrides inhibit the test.
- A working, already flyable tune. This is not a way to make an unstable aircraft
  flyable. Maintain visual control and space to recover from the excitation.
- Blackbox must actually be recording with `debug_mode = CHIRP`, at least
  600 samples/second. The PID loop must also run at least at 600 Hz. For example,
  a 1 kHz PID loop with `blackbox_rate_num = 1` and `blackbox_rate_denom = 1`
  meets this requirement. A 500 Hz log does not.
- A dedicated receiver switch channel between **5 and 12** (5 = AUX1), with no
  other function assigned to it. Channels 13-32 can be overlaid by MSP, so are
  deliberately excluded. The selected channel must exist on the receiver.

The feature is built when `USE_CHIRP` and `USE_BLACKBOX` are available, and is
disabled by default (`chirp_axis = OFF`). It uses a receiver channel directly,
without adding or reusing a flight-mode ID or changing MSP status payloads.

## Configuration

Save a `diff all` backup first. Set up and verify normal Blackbox recording before
enabling the test. For example, to prepare a roll measurement using CH8 (AUX4):

```text
set chirp_axis = ROLL
set chirp_trigger_channel = 8
set chirp_amplitude = 10
set debug_mode = CHIRP
save
```

`chirp_amplitude` is the peak rate perturbation in degrees/second (1-30, default
10). It describes the commanded excitation, not the maximum aircraft response.
Use small excitation appropriate to the airframe. Change the axis to `PITCH` or
`YAW` while disarmed to measure the other axes. Do not change controller settings
or profiles during a run.

## Measurement sequence

1. Leave the trigger low, take off normally, and establish a level hover in
   ANGLE with manual throttle. This implementation does not hold position or
   altitude for the pilot.
2. With roll/pitch/yaw sticks centred and recording running, the test observes
   the low trigger (900-1300). Switching high (1700-2100) starts a two-second
   settling period followed by one 20-second logarithmic sweep from 2 to 60 Hz.
   The amplitude fades in and out over one second.
3. Keep the aircraft under control. Moving a stick, moving the trigger out of
   its high range, or changing to an incompatible mode stops excitation. After
   an abort the switch must be observed low again under valid conditions before
   another high transition can start a test. A held-high switch never repeats
   a completed run and cannot start a run at arming or after receiver recovery.
4. Switch low, land and inspect the log. Repeat for another axis if needed.
5. Disable the experiment afterwards with `set chirp_axis = OFF` and `save`.

During settling and excitation, any of the following stops the test:

- Disarm, loss of valid receiver data, failsafe, wrong controller/platform/mode,
  landing detection, mixer transition, MSP RC override or programming override.
- Roll/pitch/yaw channel deviation above 50 from centre, processed stick command
  above 50, processed throttle outside (1200, 1800), or throttle change greater
  than 100 from its value at test start.
- Roll or pitch exceeding 20 degrees, measured gyro rate above 200 degrees/sec
  on any axis, non-finite gyro data or motor mix range at/above 0.9.
- Blackbox stopping/pausing, inadequate logging rate, missing CHIRP debug mode,
  changed chirp configuration/control profile/mixer profile, or a PID update gap
  exceeding 10 ms.

These limits are conservative experimental checks, not a guarantee of safe
flight. Receiver values are processed by INAV's normal RX path. Mixer saturation
feedback is from the preceding mixer update, and storage-full/write-error
detection follows the normal Blackbox state machine. A short output clipping
event or dropped log record can still occur; inspect the recorded output and
timestamps and discard invalid runs. There is no automatic airborne detection
beyond the normal landing flag: deliberately operating the switch while armed
on the ground can start the test if the other conditions are met.

## Blackbox contract

The new `CHIRP` debug enum is appended; existing debug IDs are unchanged. All
eight debug channels are populated on each PID iteration. Phase values are:
0 idle/disabled, 1 ready, 2 settling, 3 running, 4 complete, 5 aborted.

| Field | Meaning | Scale |
| --- | --- | --- |
| `debug[0]` | Phase | enum above |
| `debug[1]` | Selected axis | 0 off, 1 roll, 2 pitch, 3 yaw |
| `debug[2]` | Injected rate perturbation | degrees/sec × 100 |
| `debug[3]` | Instantaneous sweep frequency | Hz × 100 |
| `debug[4]` | Inhibit/abort bitmask | below |
| `debug[5]` | Actual selected-axis rate-controller setpoint, including chirp | degrees/sec × 100 |
| `debug[6]` | Selected-axis gyro feedback used by the controller | degrees/sec × 100 |
| `debug[7]` | Selected-axis limited PID sum before mixing | PID output × 100 |

Inhibit bits: 1 flight conditions, 2 pilot/override input, 4 logging, 8 motion,
16 mixer saturation, 32 configuration/profile, 64 timing, 128 trigger state.
Several bits may be present. Aborted runs retain their last reason until another
inhibit occurs, a valid low trigger resets them or the feature is disabled.

Excitation is added **after** angle control, rate acceleration limiting and
programming overrides, immediately before the multicopter rate controller. The
normal P, I, D and CD paths all receive the perturbed setpoint. The standard
Blackbox setpoint is updated accordingly. `debug[6]` is the actual controller
feedback; it can differ from the standard `gyroADC` field when the Smith predictor
is enabled. No changes are made to the predictor or filters for this test.

Use `debug[2]` as the known external excitation, and retain the actual setpoint,
feedback and actuator output. ANGLE feedback and the CD path remain active, so
the gyro/excitation ratio alone is **not** the open-loop plant or the isolated
rate-controller response. Do not derive PID recommendations from that ratio
without modelling the closed loop and validating the estimate. A completed
sweep is not a quality or stability certificate.

## Developer validation

`chirp_unittest` exercises the production generator, single-shot state machine,
inhibits, switch reset, amplitude limits, sweep frequency, timing gaps and the
32-bit microsecond wrap. `chirp_flight_unittest` links the production firmware
adapter with stubbed flight inputs and exercises axis selection, logging,
controller readiness, invalid channels and the flight abort paths.

Before release, validate the full control loop in a flight-dynamics simulator,
measure CPU cost and flash/RAM use on supported boards, verify Blackbox storage
throughput, and perform staged flight tests. Unit tests and a successful SITL
build do not substitute for those checks.
