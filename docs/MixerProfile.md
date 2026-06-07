# MixerProfile

A MixerProfile is a set of motor mixer, servo-mixer and platform type configuration settings. It is designed for experienced inav users.

### For a tutorial of vtol setup, Read https://github.com/iNavFlight/inav/blob/master/docs/VTOL.md

Not limited to VTOL. air/land/sea mixed vehicle is also achievable with this feature. Model behaves according to current mixer_profile's platform_type and configured custom motor/servo mixer

Two mixer profiles and smooth VTOL auto-transition are available only on targets with enough flash space.
In standard INAV builds this means targets with more than 512 KB flash, compiled with `USE_AUTO_TRANSITION`.
Targets with 512 KB flash keep the older single-profile / legacy transition behavior and do not include the smooth auto-transition settings.

For VTOL setup. one mixer_profile is used for multi-rotor(MR) and the other is used for fixed-wing(FW)
By default, switching between profiles requires reboot to take affect. However, using the RC mode: `MIXER PROFILE 2` will allow in flight switching for things like VTOL operation
. And will re-initialize pid and navigation controllers for current MC or FW flying mode.

For consistency, this document uses the long-standing VTOL order:
- Profile 1 = fixed-wing (FW)
- Profile 2 = multicopter (MC)

The firmware can work with the profiles swapped, but the examples below keep this order so the switch logic is easier to follow.

Please note that this is an emerging / experimental capability that will require some effort by the pilot to implement.

## Mixer Transition input

`MIXER TRANSITION` is mainly used while the model is still in multicopter mode, so a forward motor or tilt servo can help the aircraft build forward speed before the switch to fixed-wing.

This feature is recommended for VTOL setups. It is normally blocked while navigation modes are active.
Mapping a motor to a servo output, or using servo logic conditions for this feature, is **not** recommended.

If `mixer_vtol_manualswitch_autotransition_controller = ON`, `MIXER TRANSITION` works like a start switch for one transition:

- Each time you move `MIXER TRANSITION` from OFF to ON, iNAV starts one transition.
- The same switch can be used in both directions:
  - MC -> transition -> FW
  - FW -> transition -> MC
- After it starts, the transition keeps running until the speed target or timer target is reached.
- Leaving the switch ON does not keep restarting the transition.
- To start another transition, turn the switch OFF and then ON again.
- If you turn the switch OFF before the profile change happens, that transition request is cancelled.
- Optional extra protection: `vtol_fw_to_mc_auto_switch_airspeed_cm_s` can automatically start FW->MC if airspeed gets too low. After that switch, iNAV stays in MC until you deliberately command another manual profile change.

This behavior is controlled by `mixer_vtol_manualswitch_autotransition_controller`.
Turn it ON in both mixer profiles if you want the same switch behavior in both directions.
If it is OFF, manual transition keeps the older behavior.

In Active Modes:

- `MIXER TRANSITION` shows that the internal transition logic is actually running.
- `MIXER PROFILE 2` shows that mixer profile 2 is currently active.

There are two separate manual paths:

- `MIXER PROFILE 2` is still a direct manual profile switch when `MIXER TRANSITION` is OFF.
- `MIXER TRANSITION` starts the smooth automatic transition sequence when `mixer_vtol_manualswitch_autotransition_controller = ON`.
- If both are ON together while the automatic transition controller is enabled, the controller temporarily owns the profile switching. When `MIXER TRANSITION` turns OFF again, direct `MIXER PROFILE 2` switching becomes active again.
- If low-speed protection switches the model from FW to MC, iNAV keeps the MC profile until you deliberately command another manual profile change.

3-position switch example:

- This example assumes the usual VTOL order used in this document:
  - Profile 1 = FW
  - Profile 2 = MC
- One supported mapping is:
  - Pos1 = FW (`MIXER PROFILE 2` OFF, `MIXER TRANSITION` OFF)
  - Pos2 = Transition request (`MIXER PROFILE 2` OFF, `MIXER TRANSITION` ON)
  - Pos3 = MC (`MIXER PROFILE 2` ON, `MIXER TRANSITION` OFF)
- Keep `mixer_vtol_manualswitch_autotransition_controller` ON in both profiles used by this mapping.
- If you intentionally swap the profile order, keep the same idea and swap the FW and MC end positions.

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
Set `MIXER TRANSITION` accordingly when you want to use `MIXER TRANSITION` input for motors and servos.

Another supported mapping is where one switch position turns ON both `MIXER PROFILE 2` and `MIXER TRANSITION`.
With `mixer_vtol_manualswitch_autotransition_controller = OFF`, `MIXER TRANSITION` is used as a live transition input.
With `mixer_vtol_manualswitch_autotransition_controller = ON`, that same overlap position is used as a controller-owned transition position.

![Alt text](Screenshots/mixer_profile.png)

|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| FW(profile1) with transition off |  MC(profile2) with transition on  | MC(profile2) with transition off |

It is also possible to set it as 4 state switch by adding FW(profile1) with transition on.

If `mixer_vtol_manualswitch_autotransition_controller = ON` and this overlap position is active:
- the smooth transition controller runs while `MIXER TRANSITION` stays ON
- direct `MIXER PROFILE 2` switching is deferred during that time
- when `MIXER TRANSITION` turns OFF, `MIXER PROFILE 2` again decides which stable mixer profile should be active

This overlap style is supported too.

## Automated Transition
This feature is mainly used for RTH and failsafe.
When set up correctly, the aircraft can use fixed-wing for the efficient flight home and then return to multicopter for easier landing.

If `mixer_automated_switch = ON` in a mixer profile, iNAV is allowed to run an automated transition when the navigation logic asks for it.

- Use `mixer_automated_switch = ON` in the MC mixer profile if you want automated MC->FW transition during the head-home part of RTH.
- Use `mixer_automated_switch = ON` in the FW mixer profile if you want automated FW->MC transition for the landing part.
- Set `mixer_switch_trans_timer` in each profile to a sensible backup time for that direction.

If `mixer_automated_switch = OFF` in all mixer profiles, automated VTOL transition is disabled.

### Unified VTOL transition controller

Manual `MIXER TRANSITION` and mission-requested VTOL transition both use the same internal transition controller.
That means the same airspeed checks, timer fallback, and smooth power changes are reused in both cases.

This section applies only to targets with more than 512 KB flash, compiled with `USE_AUTO_TRANSITION`.

Direct manual `MIXER PROFILE 2` switching is still a separate path when you want an immediate profile change.

### Airspeed-first completion

When valid pitot airspeed is available, iNAV uses airspeed to decide when the transition is complete:

- `vtol_transition_to_fw_min_airspeed_cm_s` for MC->FW
- `vtol_transition_to_mc_max_airspeed_cm_s` for FW->MC

If pitot is not available, not healthy, or the threshold is set to `0`, iNAV uses `mixer_switch_trans_timer` instead.
Ground speed is not used for transition completion/progress.

The three timer settings do different jobs:

- `mixer_switch_trans_timer` is the original VTOL transition timer. It is still the backup completion timer when trusted pitot airspeed is not being used.
- `mixer_vtol_transition_airspeed_timeout_ms` is only a maximum wait time for the required airspeed while pitot is still usable. It does not complete the transition by itself; it aborts that airspeed-controlled attempt.
- If pitot becomes unavailable during transition, iNAV stops using the airspeed timeout and falls back to `mixer_switch_trans_timer`.
- For pitot-based setups, use a non-zero `mixer_switch_trans_timer` as a sensible backup time, typically `40..60` (`4..6s`).

### Smooth power changes during transition (optional)

When `mixer_vtol_transition_dynamic_mixer = ON`, iNAV can smoothly change:

- forward motor power,
- lift motor power,
- multicopter stabilisation strength,
- fixed-wing control strength.

Default is OFF to preserve existing behavior.
With it ON, you can configure `INPUT_AUTOTRANSITION_TARGET_STABILIZED_*` servo rules in the MC mixer profile.
During MC->FW they drive the selected servo outputs from the target FW controller before the hot-switch.
During FW->MC the same MC mixer rules mark which FW servo outputs should fade down as fixed-wing authority is reduced and motor stabilisation comes back in.
These inputs are active only while the smooth autotransition controller is running. If `mixer_vtol_transition_dynamic_mixer = OFF`, they stay at full authority while the controller is active. If `mixer_vtol_transition_dynamic_mixer = ON`, they follow the normal fixed-wing authority scaling.
`INPUT_MIXER_TRANSITION` remains available for transition-progress servo movement such as tilt or helper servos.

How `mixer_vtol_transition_scale_ramp_time_ms` works:

- MC->FW pusher:
  - `> 0`: forward motor power ramps from `0 -> 100%` over this time, even when pitot is working normally.
  - `= 0` (default): forward motor power goes to `100%` immediately.
- This timer does not decide when the transition completes.
- Lift motor power, MC stabilisation, and FW control:
  - with valid pitot airspeed, they still follow transition progress based on airspeed.
  - if pitot stops being usable and this setting is `> 0`, they use this same timer as a backup ramp.
  - if pitot stops being usable and this setting is `0`, they fall back to the normal transition timer/progress behavior.
- FW->MC keeps the existing style of smooth handover.

Example:

- `mixer_switch_trans_timer = 50` (5s fallback completion timer)
- `mixer_vtol_transition_scale_ramp_time_ms = 1200`

Result:
- in MC->FW, the forward motor reaches full power in about `1.2s`,
- when pitot is working, lift power and control handover still follow airspeed,
- if pitot stops being usable, the same handover reaches its target in about `1.2s`,
- transition completion still uses airspeed when pitot is working,
- backup completion time is still `5s` if pitot is not usable.

### Mission-authorized VTOL transition (waypoint User Action)

INAV can also change between MC and FW from the mission itself.
This is configured with:

- `nav_vtol_mission_transition_user_action` (`OFF`, `USER1`, `USER2`, `USER3`, `USER4`)
- `nav_vtol_mission_transition_min_altitude_cm` (optional, `0` disables minimum-altitude check)
- `vtol_transition_to_fw_min_airspeed_cm_s` (preferred MC->FW threshold)

Setting scope:

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

On each navigable mission waypoint (`WAYPOINT`, `POSHOLD_TIME`, `LAND`), the chosen USER flag tells iNAV which flight mode should be active there:

- selected USER flag = `0` -> target MC / MULTIROTOR profile
- selected USER flag = `1` -> target FW / AIRPLANE profile
- This is set per waypoint. It is **not** a toggle command.
- If the aircraft is already in the requested mode, iNAV does nothing and continues.

The mission pauses while transition is in progress and resumes after completion.

For MC->FW mission transitions, navigation uses a built-in straight acceleration run to build speed before the switch to fixed-wing.
Mission transition uses the same transition logic as manual transition: airspeed first, timer as backup.

Manual RC switching (`MIXER PROFILE 2`, `MIXER TRANSITION`) is still blocked during normal active navigation.
Mission VTOL transition still relies on the normal two-profile setup, so you must configure both mixer profiles and a valid `MIXER PROFILE 2` mode condition.

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
- Keeps behavior close to the older transition setup.
- Good as a known-safe starting point before enabling the smoother power changes.

#### Test 2 - Airspeed-first + smooth power changes (manual tuning)

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
- Uses pitot airspeed first, with timer fallback only if pitot is not usable.
- Adds smoother forward-motor, lift-motor, and control handover to reduce abrupt transitions.

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

Behavior:
- Uses USER1 as the per-waypoint target selector (`clear = MC`, `set = FW`).
- Pauses mission progression during transition and resumes only after transition completion.

### Validation Matrix (PR / SITL / HITL)

- MC->FW manual, pitot healthy/available.
- MC->FW manual, no pitot (timer fallback).
- FW->MC manual, pitot healthy/available.
- FW->MC manual, no pitot (timer fallback).
- `MIXER TRANSITION` held ON after completion (no repeated starts).
- `MIXER TRANSITION` OFF before profile change (safe abort).
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
    - bit6: profile change done
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
- `debug[6]` = MC stabilisation scale x1000 (`0..1000`)
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
