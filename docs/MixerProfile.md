# MixerProfile

A MixerProfile is a set of motor mixer, servo-mixer and platform type configuration settings. It is designed for experienced inav users.

### For a tutorial of vtol setup, Read https://github.com/iNavFlight/inav/blob/master/docs/VTOL.md

Not limited to VTOL. air/land/sea mixed vehicle is also achievable with this feature. Model behaves according to current mixer_profile's platform_type and configured custom motor/servo mixer

Currently two profiles are supported on targets other than F411(due to resource constraints on F411). i.e VTOL transition is not available on F411.

For VTOL setup. one mixer_profile is used for multi-rotor(MR) and the other is used for fixed-wing(FW)
By default, switching between profiles requires reboot to take affect. However, using the RC mode: `MIXER PROFILE 2` will allow in flight switching for things like VTOL operation
. And will re-initialize pid and navigation controllers for current MC or FW flying mode.

For consistency, this document uses the long-standing VTOL order:
- Profile 1 = fixed-wing (FW)
- Profile 2 = multicopter (MC)

The firmware can work with the profiles swapped, but the examples below keep this order so the switch logic is easier to follow.

Please note that this is an emerging / experimental capability that will require some effort by the pilot to implement.

## Mixer Transition input

Typically, 'transition input' will be useful in MR mode to gain airspeed.
The associated motor or servo will then move accordingly when transition mode is activated.
Transition input is disabled when navigation mode is activate

The use of Transition Mode is recommended to enable further features and future developments like fail-safe support. Mapping motor to servo output, or servo with logic conditions is **not** recommended

`MIXER TRANSITION` now behaves as a transition trigger/request (edge-triggered), not a continuous blend hold:

- A rising edge starts one transition (MC->FW or FW->MC depending on current profile).
- The transition state machine runs automatically to completion.
- Keeping the mode ON does not repeatedly restart transitions.
- A new transition requires mode OFF then ON again.
- If switched OFF before hot-switch completes, the manual transition request is aborted.
- If valid pitot is present and MC->FW threshold is configured, direct manual profile hot-switch to FW is blocked until threshold is reached.
- Optional FW protection: `vtol_fw_to_mc_auto_switch_airspeed_cm_s` can auto-request FW->MC transition when valid pitot airspeed drops to/below the configured value (`0` disables).

This edge-triggered behavior is enabled by `mixer_vtol_manualswitch_autotransition_controller`.
Set `mixer_vtol_manualswitch_autotransition_controller = ON` in both mixer profiles (MC and FW) used for switching to keep manual transition semantics consistent after profile hot-switch.
When `mixer_vtol_manualswitch_autotransition_controller = OFF`, manual transition keeps legacy behavior.
With manual auto-transition enabled, Active Modes `MIXER TRANSITION` now indicates that the internal transition controller/mixing is actually active, not merely that the RC `MIXER TRANSITION` switch is active.
Active Modes `MIXER PROFILE 2` indicates the currently active mixer profile.

Important path split:
- `MIXER PROFILE 2` remains a direct manual profile-switch path.
- Smooth VTOL transition state-machine behavior is triggered by `MIXER TRANSITION` when `mixer_vtol_manualswitch_autotransition_controller = ON`.

Recommended switch topology example (clear 3-position setup):
- This example assumes the usual VTOL order used in this document:
  - Profile 1 = FW
  - Profile 2 = MC
- Use a dedicated 3-position mapping:
  - Pos1 = FW (`MIXER PROFILE 2` OFF, `MIXER TRANSITION` OFF)
  - Pos2 = Transition trigger (`MIXER PROFILE 2` OFF, `MIXER TRANSITION` ON)
  - Pos3 = MC (`MIXER PROFILE 2` ON, `MIXER TRANSITION` OFF)
- Keep `mixer_vtol_manualswitch_autotransition_controller` ON in both profiles used by this mapping.
- Avoid overlapping FW selection and transition trigger in the same position.
- Avoid 2-position setups where one position activates both `MIXER PROFILE 2` and `MIXER TRANSITION`.
- Overlapping mode activation can produce order-dependent behavior (direct profile switch path vs transition-controller path), which is unpredictable and not recommended.
- If you intentionally swap the profile order, the same idea still works; just swap the FW and MC end positions.

## Servo

`Mixer Transition` is the input source for transition input; use this to tilt motor to gain airspeed.

Example: Increase servo 1 output by +45 with speed of 150 when transition mode is activated for tilted motor setup:

```
# rule no; servo index; input source; rate; speed; activate logic function number
smix 6 1 38 45 150 -1
```

## Motor

The default `mmix` throttle value is 0.0, It will not show in `diff` command when throttle value is 0.0 (unused);

- 0.0<throttle<=1.0 : normal mapping
- -1.0<throttle<=0.0 : motor stop, default value 0, set to -1 to use a place holder for subsequent motor rules
- -2.0<throttle<-1.0 : spin regardless of throttle position at speed `abs(throttle)-1` when Mixer Transition is activated.

Airmode type should be set to "STICK_CENTER". Airmode type must NOT be set to "THROTTLE_THRESHOLD". If set to throttle threshold the (-) motor will spin till throttle threshold is passed. 

Example: This will spin motor number 5 (counting from 1) at 20%, in transition mode only, to gain speed for a "4 rotor 1 pusher" setup:

```
# motor number; throttle; roll; pitch; yaw
mmix 4 -1.200  0.000  0.000  0.000
```

## RC mode settings

It is recommend that the pilot uses a RC mode switch to activate modes or switch profiles.
Profile files Switching is not available until the runtime sensor calibration is done. Switching is NOT available when navigation mode is activate.

`mixer_profile` 1 will be used as default, `mixer_profile` 2 will be used when the `MIXER PROFILE 2` mode box is activated. 
Set `MIXER TRANSITION` accordingly when you want to use `MIXER TRANSITION` input for motors and servos. Here is sample of using these RC modes:

![Alt text](Screenshots/mixer_profile.png)

|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| FW(profile1) with transition off |  MC(profile2) with transition on  | MC(profile2) with transition off |

It is also possible to set it as 4 state switch by adding FW(profile1) with transition on.

## Automated Transition
This feature is mainly for RTH in a failsafe event. When set properly, model will use the FW mode to fly home efficiently, And land in the MC mode for easier landing.
`ON` for a mixer_profile\`s `mixer_automated_switch` means to schedule a Automated Transition when RTH head home(applies for MC mixer_profile) or RTH Land(applies for FW mixer_profile) is requested by navigation controller.
Set `mixer_automated_switch` to `ON` in mixer_profile for MC mode. Set `mixer_switch_trans_timer` in mixer_profile for MC mode for the time required to gain airspeed for your model before entering to FW mode.
When `mixer_automated_switch`:`OFF` is set for all mixer_profiles(defaults). Model will not perform automated transition at all.

### Unified VTOL transition controller

Manual `MIXER TRANSITION` and mission-authorized VTOL transition both use the same internal transition controller.
This controller always computes transition progress/completion and performs its own profile hot-switch only inside the authorized transition state.
Direct manual `MIXER PROFILE 2` switching remains a separate path when no transition controller path is active.
When `mixer_vtol_transition_dynamic_mixer = ON`, pusher/lift/authority scaling is enabled and is driven by:
- transition progress (default), or
- `mixer_vtol_transition_scale_ramp_time_ms` when configured (>0).

### Airspeed-first completion

When pitot airspeed is healthy and available, transition completion uses pitot thresholds:

- `vtol_transition_to_fw_min_airspeed_cm_s` for MC->FW
- `vtol_transition_to_mc_max_airspeed_cm_s` for FW->MC

If pitot is unavailable/unhealthy (or threshold is `0`), timer fallback is used (`mixer_switch_trans_timer`).
Ground speed is not used for transition completion/progress.

Optional safety timeout:

- `mixer_vtol_transition_airspeed_timeout_ms` can abort transition if airspeed condition is not met in time.
- This timeout is only active while transition completion is using trusted pitot airspeed.
- If pitot is unavailable/unhealthy, transition completion falls back to `mixer_switch_trans_timer`; timeout does not force abort in that fallback path.
- For airspeed-first setups, configure non-zero `mixer_switch_trans_timer` fallback (typical `40..60`, i.e. `4..6s`) so pitot-loss fallback does not complete immediately.

### Dynamic scaling (optional)

When `mixer_vtol_transition_dynamic_mixer = ON`, transition progress scales:

- pusher contribution (`-2.0 < throttle < -1.0` motors) from configured max toward 0/100% depending on direction,
- lift motor throttle contribution (`vtol_transition_lift_end_percent`),
- MC stabilization authority (`vtol_transition_mc_authority_end_percent`),
- FW authority start level (`vtol_transition_fw_authority_start_percent`, servo transition input blend).

Default is OFF to preserve existing behavior.
With dynamic scaling enabled, `vtol_transition_fw_authority_start_percent = 100` preserves legacy FW authority handoff; lower values provide smoother ramp-in.

Optional scaling ramp timer:

- trusted pitot available/healthy: scaling follows airspeed-based transition progress.
- `mixer_vtol_transition_scale_ramp_time_ms > 0`: if trusted pitot becomes unavailable/unhealthy, scaling falls back to this timer.
- `mixer_vtol_transition_scale_ramp_time_ms = 0` (default): if trusted pitot is unavailable/unhealthy, scaling falls back to transition progress/timer behavior.

Example:

- `mixer_switch_trans_timer = 50` (5s fallback completion timer)
- `mixer_vtol_transition_scale_ramp_time_ms = 1200`

Result:
- when trusted pitot is healthy, scaling still follows airspeed progress,
- if trusted pitot becomes unavailable/unhealthy, scaling reaches target levels in ~1.2s,
- transition completion still follows airspeed threshold when pitot is healthy,
- timer fallback completion still uses 5s when pitot is unavailable/unhealthy.

### Mission-authorized VTOL transition (waypoint User Action)

INAV supports mission-requested VTOL transitions through the existing automated transition path. This is configured with:

- `nav_vtol_mission_transition_user_action` (`OFF`, `USER1`, `USER2`, `USER3`, `USER4`)
- `nav_vtol_mission_transition_min_altitude_cm` (optional, `0` disables minimum-altitude check)
- `vtol_transition_to_fw_min_airspeed_cm_s` (preferred MC->FW threshold)

Scope note:

- Per-mixer-profile settings:
  - `mixer_automated_switch`
  - `mixer_switch_trans_timer`
  - `mixer_vtol_transition_dynamic_mixer`
  - `mixer_vtol_manualswitch_autotransition_controller`
  - `mixer_vtol_transition_airspeed_timeout_ms`
  - `mixer_vtol_transition_scale_ramp_time_ms`
- Global settings:
  - `vtol_transition_to_fw_min_airspeed_cm_s`
  - `vtol_transition_to_mc_max_airspeed_cm_s`
  - `vtol_fw_to_mc_auto_switch_airspeed_cm_s`
  - `vtol_transition_lift_end_percent`
  - `vtol_transition_mc_authority_end_percent`
  - `vtol_transition_fw_authority_start_percent`
  - `nav_vtol_mission_transition_user_action`
  - `nav_vtol_mission_transition_min_altitude_cm`
  - `nav_vtol_mission_transition_track_distance_cm`

On each navigable mission waypoint (`WAYPOINT`, `POSHOLD_TIME`, `LAND`), the configured USER action bit is used as absolute target selector:

- selected USER bit = `0` -> transition to MC / MULTIROTOR profile
- selected USER bit = `1` -> transition to FW / AIRPLANE profile
- When `nav_vtol_mission_transition_user_action != OFF`, each navigable waypoint encodes a target state via that selected bit.
- This is a per-waypoint target-state declaration (not an event trigger). Users should intentionally set/clear the selected USER bit on each navigable waypoint.
- This is **not** a toggle command.
- If already in the requested profile type, the action is treated as complete (idempotent).

The mission pauses while transition is in progress and resumes after completion.

For MC -> FW mission transitions, navigation uses a straight acceleration segment (no loiter) to build speed before hot-switch.
Mission path uses the same controller and completion logic as manual transition (airspeed-first, timer fallback).

Manual RC switching (`MIXER PROFILE 2`, `MIXER TRANSITION`) remains blocked during normal active navigation. Mission VTOL transition does not bypass the hot-switch safety guard; it only authorizes switching inside the automated transition state.
Mission VTOL transition still relies on normal profile-switch infrastructure: configure two mixer profiles and a valid `MIXER PROFILE 2` mode activation condition.

### Example test presets (VTOL ~1.0m wingspan, ~1720g AUW)

These are practical starting points for first validation flights. They are examples, not universal defaults.

#### Test 1 - Legacy-compatible baseline (manual transition check)

CLI:
- `set mixer_vtol_manualswitch_autotransition_controller = ON`
- `set mixer_vtol_transition_dynamic_mixer = OFF`
- `set mixer_switch_trans_timer = 45`
- `set vtol_transition_to_fw_min_airspeed_cm_s = 0`
- `set vtol_transition_to_mc_max_airspeed_cm_s = 900`
- `set mixer_vtol_transition_airspeed_timeout_ms = 0`
- `set mixer_vtol_transition_scale_ramp_time_ms = 0`
- `set nav_vtol_mission_transition_user_action = OFF`

Behavior:
- Preserves legacy-style transition mixing while still using the new controller path.
- Useful as a known-safe baseline before enabling dynamic scaling.

#### Test 2 - Airspeed-first + dynamic scaling (manual tuning)

CLI:
- `set mixer_vtol_manualswitch_autotransition_controller = ON`
- `set mixer_vtol_transition_dynamic_mixer = ON`
- `set vtol_transition_to_fw_min_airspeed_cm_s = 1300`
- `set vtol_transition_to_mc_max_airspeed_cm_s = 850`
- `set mixer_switch_trans_timer = 50`
- `set mixer_vtol_transition_airspeed_timeout_ms = 6500`
- `set mixer_vtol_transition_scale_ramp_time_ms = 1200`
- `set vtol_transition_lift_end_percent = 30`
- `set vtol_transition_mc_authority_end_percent = 20`
- `set vtol_transition_fw_authority_start_percent = 20`
- `set nav_vtol_mission_transition_user_action = OFF`

Behavior:
- Uses pitot-first completion logic with timer fallback only when pitot is unavailable/unhealthy.
- Adds fast, smooth pusher/lift/authority ramping to reduce abrupt transitions.

#### Test 3 - Mission-authorized transition (mission integration)

CLI:
- `set mixer_vtol_manualswitch_autotransition_controller = ON`
- `set mixer_vtol_transition_dynamic_mixer = ON`
- `set vtol_transition_to_fw_min_airspeed_cm_s = 1300`
- `set vtol_transition_to_mc_max_airspeed_cm_s = 850`
- `set mixer_switch_trans_timer = 50`
- `set mixer_vtol_transition_airspeed_timeout_ms = 6500`
- `set mixer_vtol_transition_scale_ramp_time_ms = 1200`
- `set vtol_transition_lift_end_percent = 30`
- `set vtol_transition_mc_authority_end_percent = 20`
- `set vtol_transition_fw_authority_start_percent = 20`
- `set nav_vtol_mission_transition_user_action = USER1`
- `set nav_vtol_mission_transition_min_altitude_cm = 1200`
- `set nav_vtol_mission_transition_track_distance_cm = 4000`

Behavior:
- Uses USER1 as per-waypoint absolute target selector (clear=MC, set=FW).
- Pauses mission progression during transition and resumes only after transition completion.

### Validation Matrix (PR / SITL / HITL)

- MC->FW manual, pitot healthy/available.
- MC->FW manual, no pitot (timer fallback).
- FW->MC manual, pitot healthy/available.
- FW->MC manual, no pitot (timer fallback).
- `MIXER TRANSITION` held ON after completion (no repeated starts).
- `MIXER TRANSITION` OFF before hot-switch (safe abort).
- Mission transition with selected USER bit = `1` (TO_FW).
- Mission transition with selected USER bit = `0` (TO_MC).
- Failsafe/disarm during active transition (abort and no blind mission resume).

### VTOL transition debug mode (Blackbox / OSD debug)

For transition troubleshooting, use:

- `set debug_mode = VTOL_TRANSITION`
- `save`

Debug channels:

- `debug[0]` = transition phase (`0=IDLE`, `1=TRANSITION_INITIALIZE`, `2=TRANSITIONING`)
- `debug[1]` = active request (`MIXERAT_REQUEST_*` enum value)
- `debug[2]` = packed transition flags:
    - bits 0-1: transition direction (`0=NONE`, `1=TO_FW`, `2=TO_MC`)
    - bit2: auto-transition controller active
    - bit3: transition mixing output active (`isMixerTransitionMixing`)
    - bit4: RC `MIXERTRANSITION` mode active
    - bit5: airspeed-controlled path in use
    - bit6: hot-switch done
    - bit7: transition aborted
    - bit8: manual VTOL auto-transition controller enabled in current mixer config
    - bit9: dynamic transition mixer enabled in current mixer config
    - bits 10-11: current mixer profile index
    - bits 12-13: next mixer profile index
    - bit14: manual transition currently allowed by navigation state
    - bit15: mission mode active
    - bit16: transition mixing requested (`isMixerTransitionMixing_requested`)
    - bit17: failsafe mode active
    - bit18: manual VTOL auto-transition controller effective after mission gating
    - bit19: RC `MIXERPROFILE` mode active
- `debug[3]` = progress x1000 (`0..1000`)
- `debug[4]` = pusher scale x1000 (`0..1000`)
- `debug[5]` = lift scale x1000 (`0..1000`)
- `debug[6]` = MC authority scale x1000 (`0..1000`)
- `debug[7]` = current mixer profile pitch transition PID multiplier (`transition_PID_mmix_multiplier_pitch`)

## TailSitter (planned for INAV 7.1)
TailSitter is supported by add a 90deg offset to the board alignment. Set the board aliment normally in the mixer_profile for FW mode(`set platform_type = AIRPLANE`), The motor trust axis should be same direction as the airplane nose. Then, in the mixer_profile for takeoff and landing set `tailsitter_orientation_offset = ON ` to apply orientation offset. orientation offset will also add a 45deg orientation offset.

## Parameter list (Partial List)
####  Please be aware of what parameter is shared among FW/MC modes and what isn't. 
### Shared Parameters

- **Timer Overrides**
- **Outputs [Servo]:**
  - Servo min-point, mid-point, max-point settings
- **Motor Configuration:**
  - motor_pwm_protocol
  - motor_poles
- **Servo Configuration:**
  - servo_protocol
  - servo_pwm_rate
- **Board Alignment**
- ·······
### Profile-Specific Parameters in VTOL
- **Mixer Profile**
    - **Mixer Configuration:**
        - platform_type
        - motor_stop_on_low
        - tailsitter_orientation_offset
        - motor_direction_inverted, and more·······
    - **Motor Mixing (mmix)**
    - **Servo Mixing (smix)**
- **Control Profile**
  - PIDs for Roll, Pitch, Yaw
  - PIDs for Navigation Modes
  - TPA (Throttle PID Attenuation) Settings
  - Rate Settings
  - ·······

### TailSitter support
TailSitter is supported by add a 90deg offset to the board alignment. Set the board aliment normally in the mixer_profile for FW mode(`set platform_type = AIRPLANE`), The motor trust axis should be same direction as the airplane nose. Then, in the mixer_profile for takeoff and landing `set platform_type = TAILSITTER`. The `TAILSITTER` platform type is same as `MULTIROTOR` platform type, expect for a 90 deg board alignment offset. In `TAILSITTER` mixer_profile, when motor trust/airplane nose is pointing to the sky, 'airplane bottom'/'multi rotor front' should facing forward in model preview. Set the motor/servo mixer according to multirotor orientation, Model should roll around geography's longitudinal axis, the roll axis of `TAILSITTER` will be yaw axis of `AIRPLANE`. In addition, When `MIXER TRANSITION` input is activated, a 45deg offset will be add to the target angle for angle mode.

## Happy flying

Remember that this is currently an emerging capability:

* Test every thing on bench first.
* Try MR or FW mode separately see if there are any problems.
* Use the INAV Discord for help and setup questions; use the Github Issues for reporting bugs and unexpected behaviors. For reporting on Github, a CLI `diff all`, a DVR and a Blackbox log of the incident will assist investigation.
