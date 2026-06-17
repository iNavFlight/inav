# Welcome to INAV VTOL

Thank you for trying the INAV VTOL. Read every line in this tutorial. Your patience can save both time and potential repair costs for the model.

## Who Should Use This Tutorial?

This tutorial is designed for individuals who 
- have prior experience with **both INAV multi-rotor and INAV fixed-wing configurations/operations.** 
- know how to create a custom mixer for their model. 
- know basic physics of vtol operation

## Firmware Status

The firmware is in a flyable state, but it hasn't undergone extensive testing yet. This means there may be potential issues that have not yet been discovered.

## Future Changes

Please be aware that both the setup procedure and firmware may change in response to user feedback and testing results. 
## Your Feedback Matters

We highly value your feedback as it plays a crucial role in the development and refinement of INAV VTOL capabilities. Please share your experiences, suggestions, and any issues you encounter during testing. Your insights are invaluable in making INAV VTOL better for everyone.

# VTOL Configuration Steps

### The VTOL functionality is achieved by switching/transitioning between two configurations stored in the FC. VTOL specific configurations are Mixer Profiles with associated control profiles. One profile set is for fixed-wing(FW) mode, One is for multi-copter(MC) mode. Configuration/Settings other than Mixer/control profiles are shared among two modes

This guide uses the long-standing VTOL setup order:
- Profile 1 = fixed-wing (FW)
- Profile 2 = multicopter (MC)

The firmware can work with the profiles swapped, but keeping one order in the guide makes the setup steps easier to follow.
![Alt text](Screenshots/mixerprofile_flow.png)

0. **Find a DIFF ALL file for your model and start from there if possible**
   - Be aware that `MIXER PROFILE 2` RC mode setting introduced by diff file can get your stuck in a mixer_profile. remove or change channel to proceed
1. **Setup Profile 1:**
   - Configure it as your normal fixed-wing setup.

2. **Setup Profile 2:**
   - Configure it as your normal multicopter setup.

3. **Mode Tab Settings:**
   - Set up switching in the mode tab.

4. *(Recommended)* **Transition Mixing (Multi-Rotor Profile):**
   - Configure transition mixing to gain airspeed in the multi-rotor profile.

5. *(Optional)* **Automated Switching (RTH):**
   - Optionally, set up automated switching in case of failsafe.

# STEP 0: Load parameter preset/templates
Find a working diff file if you can and start from there. If not, select keep current settings and apply following parameter in cli but read description about which one to apply.

```
set small_angle = 180
set gyro_main_lpf_hz = 80
set dynamic_gyro_notch_min_hz = 50
set dynamic_gyro_notch_mode = 3D
set motor_pwm_protocol = DSHOT300 #Try dshot first and see if it works
set airmode_type = STICK_CENTER_ONCE


set nav_disarm_on_landing = OFF  #Recommended for first VTOL tests; enable only after validating landing detection on your airframe
set nav_rth_allow_landing = FS_ONLY
set nav_wp_max_safe_distance = 500
set nav_fw_control_smoothness = 2
set nav_fw_launch_max_altitude = 5000

set servo_pwm_rate = 160 #If model using servo for stabilization in MC mode and servo can tolerate it 
set servo_lpf_hz = 30 #If model using servo for stabilization in MC mode


## profile 1 as airplane and profile 2 as multi rotor
mixer_profile 1

set platform_type = AIRPLANE
set model_preview_type = 26
set motorstop_on_low = ON
set mixer_pid_profile_linking = ON

mixer_profile 2

set platform_type = TRICOPTER
set model_preview_type = 1
set mixer_pid_profile_linking = ON

profile 1 #control profile
set dterm_lpf_hz = 10
set d_boost_min =  1.000
set d_boost_max =  1.000
set fw_level_pitch_trim =  5.000
set roll_rate = 18
set pitch_rate = 9
set yaw_rate = 3
set fw_turn_assist_pitch_gain = 0.4
set max_angle_inclination_rll = 450
set fw_ff_pitch = 80
set fw_ff_roll = 50
set fw_p_pitch = 15
set fw_p_roll = 15

profile 2
set dterm_lpf_hz = 60
set dterm_lpf_type = PT3
set d_boost_min =  0.800
set d_boost_max =  1.200
set d_boost_gyro_delta_lpf_hz = 60
set antigravity_gain =  2.000
set antigravity_accelerator =  5.000
set smith_predictor_delay =  1.500
set tpa_rate = 20
set tpa_breakpoint = 1200
set tpa_on_yaw = ON #If model using control surface/tilt mechanism for stabilization in MC mode
set roll_rate = 18
set pitch_rate = 18
set yaw_rate = 9
set mc_iterm_relax = RPY

save
```

# STEP 1: Configuring as a normal fixed-wing in Profile 1

1. **Select the first Mixer Profile and Control Profile:**
   - In the CLI, switch to the mixer_profile and control_profile you wish to set first. You can also switch
     mixer_profile/control_profile through gui with aforementioned presets loaded.
     ```
     mixer_profile 1 #in this example, we set profile 1 first
     set mixer_pid_profile_linking = ON  # Let the mixer_profile handle the control profile (formerly pid_profile) switch on this mixer_profile
     set platform_type = AIRPLANE
     save
     ```

2. **Configure the fixed-wing:**
   - Configure your fixed-wing as you normally would, or you can copy and paste default settings to speed things up.
   - Dshot esc protocol availability might be limited depends on outputs and fc board you are using. change the motor wiring or use oneshot/multishot esc protocol and calibrate throttle range.
   - You can use throttle = -1 as a placeholder for the motor you wish to stop if the motor isn't the last motor
   - Consider conducting a test flight to ensure that everything operates as expected. And tune the settings, trim the servos.

![Alt text](Screenshots/mixerprofile_fw_mixer.png)

You must also assign the tilting servos values using the Fixed Value values (formerly called "MAX").  If you don't do this the motors will point in the direction assigned by the transition mode.

# STEP 2: Configuring as a Multi-Copter in Profile 2

1. **Switch to Another Mixer Profile with Control Profile:**
   - In the CLI, switch to another mixer_profile along with the appropriate control profile. You can also switch
     mixer_profile/control_profile through gui with aforementioned presets loaded.
     ```
     mixer_profile 2
     set mixer_pid_profile_linking = ON
     set platform_type = MULTIROTOR/TRICOPTER
     save
     ```

2. **Configure the multicopter/tricopter:**
   - Set up your multicopter/tricopter as usual, this time for mixer_profile 2 and control_profile 2.
   - Utilize the Fixed Value input (formerly called "MAX") in the servo mixer to tilt the motors without altering the servo midpoint.
   - At this stage, focus on configuring profile-specific settings. You can streamline this process by copying and pasting the default PID settings.
   - you can set -1 in motor mixer throttle as a place holder: this will disable that motor but will load following the motor rules
   - compass is required to enable navigation modes for multi-rotor profile.
   - Consider conducting a test flight to ensure that everything operates as expected. And tune the settings.
   - It is advisable to have a certain degree of control surface (elevon / elevator) mapping for stabilization even in multi-copter mode. This helps improve control authority when airspeed is high. It might be unable to recover from a dive without them.

![Alt text](Screenshots/mixerprofile_mc_mixer.png)

5. **Tailsitters:planned for INAV 7.1**
   - Configure the fixed-wing mode/profile sets normally. Use MultiCopter platform type for tail_sitting flying mode/profile sets. 
   - The baseline board aliment is FW mode (ROLL axis is the thrust axis). Set `tailsitter_orientation_offset = ON ` in the tail_sitting MC mode.
   - Configure mixer ROLL/YAW mixing according to tail_sitting orientation in the tail_sitting MC mode. YAW axis is the thrust axis.
   - Conduct a bench test and see the orientation of the model changes in inav-configurator setup tab


# STEP 3: Mode Tab Settings:
### We recommend using an 3-pos switch on you radio to activate these modes, So the pilot can jump in or bail out at any moment.

### Here is a example, in the bottom of inav-configurator Modes tab:
![Alt text](Screenshots/mixer_profile.png)
|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| Profile1(FW) with transition off |  Profile2(MC) with transition on  | Profile2(MC) with transition off |

- This is one supported mapping, where one switch position turns ON both `MIXER PROFILE 2` and `MIXER TRANSITION`.
- With `mixer_vtol_manualswitch_autotransition_controller = OFF`, `MIXER TRANSITION` is used as a live transition input.
- With `mixer_vtol_manualswitch_autotransition_controller = ON`, that same overlap position is used as a controller-owned transition position.
- While both are ON, the smooth transition controller runs and direct `MIXER PROFILE 2` switching is deferred.
- When `MIXER TRANSITION` turns OFF again, `MIXER PROFILE 2` once more decides which stable mixer profile should be active.

- Profile file switching becomes available after completing the runtime sensor calibration (15-30s after booting). And It is **not available** when a navigation mode or position hold is active.

- By default, `mixer_profile 1` is used. `mixer_profile 2` is used when the `MIXER PROFILE 2` mode is activate. Once configured successfully, you will notice that the profiles and model preview changes accordingly when you refresh the relevant INAV Configurator tabs. 

- Use the `MIXER TRANSITION` mode to gain airspeed in MC profile, set `MIXER TRANSITION` accordingly.

Conduct a bench test on the model (without props attached). The model can now switch between fixed-wing and multi-copter modes while armed. Furthermore, it is capable of mid-air switching, resulting in an immediate stall upon entering fixed-wing profile


# STEP 4: Tilting Servo Setup (Recommended)
### Setting up the tilting servos to operate correctly is crucial for correct yaw control of the craft. Using the default setup works, but will most likely result in your craft crawling forward with evey yaw input.
The steps below describe how you can fine-tune the tilting servos to obtian the optimum result.


1. **Set the tilt servos at 45 degrees:**
   - Connect and power the tilting servos with your flight controller.
   - Enter transition mode (your switch should be in the mid-position).
   - Check the Outputs tab and make sure that the tilt servo channels are exactly at 1500μs.
   - In this mode, your tilt servos should be at the 45-degree position and you can now mount the motor and prop to your tilt servo such that the angle of the motor mounting plate is at 45 degrees upwards.
   - NOTE 1: If you have dedicated tilt servos, you may have engraved indices on the servos and tilting motor assembly to help you with this step. If the servos don't end up exactly at 45 degrees due to the teeth on the servo and the control arm/plate, don't worry, this will be automatically adjusted after completing the other steps below.
   - NOTE 2: If you are using control rods to adjust the tilt of the servos, adjust the lenth of your control rod and the position of the control arm to position the control arm as close as possible to the mid position. It will depend on the oriatation of the servo, but generally speaking, the control arm of the servo should be pointed perpendicular to the fuselage when the motor mounts are at the 45 degree setting.

2. **Switch to Multicopter/Tricopter:**
   - Assuming that you have set up your mixer similar to STEP1 and STEP2, you can now switch to the tricopter/multicopter mode and your servos should be tilting the motors upwards. If this is not the case, reverse the servo(s) in the Outputs tab such that the servo(s) is/are pointed upwards.
   - It is OK for the servos not to point exactly 90 degrees upwards, but they should be as close as possible to that position.
   - Also, ensure that your Fixed Value values (formerly called "MAX") values in the Mixer tab are at 100 and -100, so that your servo will move to the maximum position, as shown in the screenshots in STEP1 and STEP2.

3. **Adjust the maximum throws for the Multicopter/Tricopter mode:**
   - While in tricopter mode, navigate to the Outputs tab and adjust the MIN and MAX endpoint values to position the motors slightly backward.
   - Rotate the prop such that it is pointed backwards towards the wing/motor mount and ensure that the gap is the same on both sides by adjusting the MIN and MAX values for the tilt servo channels.
   - NOTE: You can check the distance with calipers or gauge blocks. Alternatively, you can adjust the MIN and MAX for your tilting servos such that the props are just touching the top of the wing or motor mount, and then you can increase/decrease the MIN and MAX values for each channel by the same ammount for both servos. This should ensure that you have the same gap between the tip of the prop and the wing or motor mount for both sides.

4. **Adjust the minimum position for the Fixed-wing mode:**
   - Repeat the same step as point 3 with the model in fixed-wing mode, where the servos are tilted forwards.
   - For this step, you just have to make sure that the motors are pointed exactly forwards.
   - You can do this by adjusting the respective MIN and MAX values in the Output tab for the tilt servo channels while in fixed-wing mode.
   - NOTE: Ensuring that your servos are tilted exactly forward is a crucial step as it can cause the plane to roll slightly if that it is not the case. However, ensuring the exact aligment will depend on your specific setup. If you are using dedicated tilting motor servos rather than standard servos with control arms and pushrods, you can make sure that you are exact by measuring the distance between the front edge of the tilting servo and the of the motor mounting plate. If the disances are the uniform across each mount and same on both motors, your servos are pointed forwads correctly.

5. **Adjsut the vertival position of the tilt servos:**
   - Switch back to multicopter/tricopter mode and open the Mixer tab.
   - Start adjusting the `Fixed Value` mixer lines from STEP2 such that the servos are pointed exactly upwards. In other words, start reducing the values of 100 and -100 to something like 80 and -80 until the motors are are pointed exaxctly upwards.
   - You will have to `Save & reboot` for adjustement for the changes to take effect, so be patient, take your time and don't forget to `Save & reboot`.
   - Move the YAW stick to either extreme position and ensure that the servos are tilting the motors both forwards and backwards.
   - NOTE: When yawing fully left, the left motor should tilt backwards and the right motor should tilt forwards. 
  
6. **Adjsut the throws of the tilt servos:**
   - The final step is to adjust the throws of the servos such that they are the same in both directions.
   - To do this, move back to the Mixer tab while in multicopter/tricopter mode and start adjusting the previously set up 50 and -50 values from the Stabilised Yaw lines.
   - You can try with lower values of about 30 and -30 and then increase the values until you reach the maximum travel point.
   - NOTE: The maximum is reached when both servos are moving the same ammount in oposite directions and one servo does not continue to move after the other has stopped. 

7. **Check correct operation and direction:**
   - Cycle back and forth between the plane, transition and tricopter modes to make sure that your servos are maintaining the same setting.
   - For the fixed wing setting, your tilt servos should point the motors exactly forwards.
   - For the multicoper/tricopter mode, the tilt servos should point the motors exactly upwards and when moving the yaw stick, both servos should tilt the motors the same ammount in opposite directions.

Optional Setup Step for Tilt Servos:

8. **Reversing tilt servos and mixer signs:**
If you have set up the mixer as suggested in STEP1 and STEP2, you may have to deal with negative values for the mixer. You may wish to reverese a servo so that you don't have to deal with the negative signs. In that case, you may have to adjust the MIN and MAX values from point 4 again, so that your tilt servos are operating correctly. Check the operation of the servos once again for the YAW control in multicopter/tricipter mode as well as the horizontal position of the tilt servos in fixed-wing mode.


# STEP 5: Transition Mixing (Multi-Rotor Profile)(Recommended)
### Transition Mixing is typically useful in multi-copter profile to gain airspeed in prior to entering the fixed-wing profile. When the `MIXER TRANSITION` mode is activated, the associated motor or servo will move according to your configured Transition Mixing. 

Please note that manual transition input is disabled when a navigation mode is active. Mission-authorized VTOL transition (via configured waypoint User Action) still works through the automated transition state.
## Servo 'Transition Mixing': Tilting rotor configuration.
Add new servo mixer rules, and select 'Mixer Transition' in input. Set the weight/rate according to your desired angle. This will allow tilting the motor for tilting rotor model.

![Alt text](Screenshots/mixerprofile_servo_transition_mix.png)

## Motor 'Transition Mixing': Dedicated forward motor configuration
In motor mixer set:
- -2.0 <= throttle <= -1.05: The motor will spin regardless of the radio's throttle position at a speed of `abs(throttle) - 1` multiplied by throttle range only when Mixer Transition is activated. Use exactly `-1.000` only as a placeholder, not as a transition helper motor.
- Airmode type should be set to "STICK_CENTER". Airmode type must NOT be set to "THROTTLE_THRESHOLD". If set to throttle threshold the (-) motor will spin until the throttle threshold is passed.
  
![Alt text](Screenshots/mixerprofile_4puls1_mix.png)

## TailSitter 'Transition Mixing': 
No additional settings needed, 45 deg offset will be added to target pitch angle for angle mode in the firmware.

### With aforementioned settings, your model should be able to enter fixed-wing profile without stalling.

# Smooth VTOL Auto-Transition Setup

This section describes the new VTOL auto-transition features as a practical setup path. It is written as a sequence of small steps:

1. Manual switch auto transition
2. Manual switch auto transition with dynamic scaling
3. Automated mission transition
4. Adding VTOL MC stabilisation protection
5. Landing detection setup

The examples assume:

- mixer profile 1 is the fixed-wing profile (`FW`, usually `platform_type = AIRPLANE`)
- mixer profile 2 is the multicopter profile (`MC`, for example `MULTIROTOR` or `TRICOPTER`)
- `MIXER PROFILE 2` mode selects the MC profile when you fly manually
- the model has been bench-tested without propellers before flight

The smooth auto-transition controller is available only on targets with more than 512 KB flash. Smaller targets keep the older VTOL transition behavior and do not include these new settings.

## 1. Manual switch auto transition

Manual switch auto transition means that `MIXER TRANSITION` starts one complete transition. You no longer need to manually time the exact profile switch point. INAV starts the transition, waits for the configured speed or timer condition, then changes to the target mixer profile.

This does not remove the older behavior. If `mixer_vtol_manualswitch_autotransition_controller = OFF`, manual transition stays as close as possible to the older INAV behavior.

### Recommended first setup

Configure both mixer profiles. This makes MC -> FW and FW -> MC behave consistently.

Per-mixer-profile settings:

```
mixer_profile 1
set mixer_vtol_manualswitch_autotransition_controller = ON
set mixer_vtol_transition_dynamic_mixer = OFF
set mixer_switch_trans_timer = 50
set mixer_vtol_transition_airspeed_timeout_ms = 0
set mixer_vtol_transition_scale_ramp_time_ms = 0

mixer_profile 2
set mixer_vtol_manualswitch_autotransition_controller = ON
set mixer_vtol_transition_dynamic_mixer = OFF
set mixer_switch_trans_timer = 50
set mixer_vtol_transition_airspeed_timeout_ms = 0
set mixer_vtol_transition_scale_ramp_time_ms = 0
```

Global settings:

```
set vtol_transition_to_fw_min_airspeed_cm_s = 0
set vtol_transition_to_mc_max_airspeed_cm_s = 0
set nav_vtol_mission_transition_user_action = OFF
save
```

What this setup does:

- `MIXER TRANSITION` starts one transition each time it moves from OFF to ON.
- Leaving `MIXER TRANSITION` ON does not restart the transition repeatedly.
- To request another transition, turn `MIXER TRANSITION` OFF, then ON again.
- If you turn `MIXER TRANSITION` OFF before the profile switch happens, INAV cancels that transition request.
- `mixer_vtol_transition_dynamic_mixer = OFF` keeps transition motor/servo behavior close to the older setup while the new controller only manages the timing of the profile switch.
- `mixer_switch_trans_timer = 50` means `5.0s`, because this setting is in deciseconds.

Value direction notes:

- Lower `mixer_switch_trans_timer`: timer-based transitions complete sooner, but may switch before the aircraft has enough speed.
- Higher `mixer_switch_trans_timer`: gives more time to accelerate or slow down, but keeps the aircraft in transition longer when pitot is not used.
- `mixer_vtol_transition_airspeed_timeout_ms = 0`: disables airspeed timeout aborts. Higher values wait longer before aborting an airspeed-controlled attempt; lower values abort sooner.
- `mixer_vtol_transition_scale_ramp_time_ms = 0`: no extra smooth motor/servo movement in this legacy-compatible baseline. Higher values are useful only when dynamic scaling is ON.
- `vtol_transition_to_fw_min_airspeed_cm_s = 0` and `vtol_transition_to_mc_max_airspeed_cm_s = 0`: use timer completion. Higher non-zero values enable pitot-based completion and make the speed condition stricter.
- `nav_vtol_mission_transition_user_action = OFF`: mission transition is disabled. Set it to `USER1`..`USER4` only after manual transition is validated.

Typical 3-position switch layout:

- Position 1: FW (`MIXER PROFILE 2` OFF, `MIXER TRANSITION` OFF)
- Position 2: transition request (`MIXER PROFILE 2` ON, `MIXER TRANSITION` ON)
- Position 3: MC (`MIXER PROFILE 2` ON, `MIXER TRANSITION` OFF)

Workflow:

- Start in MC.
- Move to the transition position.
- INAV starts MC -> FW and changes profile after the configured condition is met.
- Move the switch to the FW position after the transition completes.
- Reverse the process for FW -> MC.

In the standard layout above, the transition position deliberately has both `MIXER PROFILE 2` and `MIXER TRANSITION` ON. While the auto-transition controller is enabled, the controller owns the profile switching until `MIXER TRANSITION` is turned OFF again.

### Timer, pitot, and timeout examples

`mixer_switch_trans_timer` is the backup completion timer. It is used when pitot airspeed is not configured, not trusted, or not available.

Examples:

- `mixer_switch_trans_timer = 30`: transition completes by timer after `3.0s` if pitot is not used.
- `mixer_switch_trans_timer = 50`: transition completes by timer after `5.0s` if pitot is not used.
- `vtol_transition_to_fw_min_airspeed_cm_s = 1300`: MC -> FW waits for `13 m/s` pitot airspeed when pitot is trusted.
- `vtol_transition_to_mc_max_airspeed_cm_s = 850`: FW -> MC waits until pitot airspeed falls to `8.5 m/s` or lower when pitot is trusted.
- `mixer_vtol_transition_airspeed_timeout_ms = 6500`: if pitot remains trusted but the requested airspeed is not reached within `6.5s`, that airspeed-controlled transition attempt is aborted.

Important pitot behavior:

- If pitot is trusted and a non-zero airspeed threshold is configured, INAV prefers airspeed for transition completion.
- The configured airspeed must be reached before `mixer_vtol_transition_airspeed_timeout_ms` expires. For example, with `vtol_transition_to_fw_min_airspeed_cm_s = 1300` and `mixer_vtol_transition_airspeed_timeout_ms = 6500`, MC -> FW must reach `13 m/s` within `6.5s`.
- If that timeout expires during a manual transition, the transition attempt is aborted and INAV does not force the target profile switch from that timeout.
- If that timeout expires during a mission transition, mission retry/failure handling is used: retry can run if `nav_vtol_transition_retry_on_airspeed_timeout = ON`; otherwise the configured mission fail action is used.
- If pitot becomes unavailable during the transition, INAV falls back to `mixer_switch_trans_timer`.
- Ground speed is not used to decide transition completion.
- `mixer_vtol_transition_airspeed_timeout_ms` does not complete a transition. It only stops an airspeed-controlled attempt that is taking too long.

Optional low-speed protection:

```
set vtol_fw_to_mc_auto_switch_airspeed_cm_s = 750
```

With this set, fixed-wing flight automatically starts FW -> MC when trusted pitot airspeed drops to `7.5 m/s` or lower. After this protection switches to MC, INAV stays in MC until you deliberately command another manual profile change. Set it to `0` to disable this protection.

## 2. Manual switch auto transition with dynamic scaling

Dynamic scaling is the optional smooth part of the new transition system. It lets INAV change motor power and stabilisation strength gradually instead of making one large step at the profile switch.

Enable it in both VTOL profiles:

Per-mixer-profile settings:

```
mixer_profile 1
set mixer_vtol_manualswitch_autotransition_controller = ON
set mixer_vtol_transition_dynamic_mixer = ON
set mixer_switch_trans_timer = 50
set mixer_vtol_transition_airspeed_timeout_ms = 6500
set mixer_vtol_transition_scale_ramp_time_ms = 1200

mixer_profile 2
set mixer_vtol_manualswitch_autotransition_controller = ON
set mixer_vtol_transition_dynamic_mixer = ON
set mixer_switch_trans_timer = 50
set mixer_vtol_transition_airspeed_timeout_ms = 6500
set mixer_vtol_transition_scale_ramp_time_ms = 1200
```

Global settings:

```
set vtol_transition_to_fw_min_airspeed_cm_s = 1300
set vtol_transition_to_mc_max_airspeed_cm_s = 850
set vtol_transition_lift_min_percent = 30
set vtol_transition_mc_authority_min_percent = 20
set vtol_transition_fw_authority_min_percent = 20
save
```

What dynamic scaling changes:

- MC -> FW can smoothly bring in the forward motor before the profile switch.
- MC -> FW can reduce lift motor power and MC motor stabilisation while fixed-wing control is increased.
- FW -> MC can remove the forward motor while lift motors and MC motor stabilisation come back.
- FW -> MC can reduce fixed-wing control as MC control comes back.
- Transition-linked servos continue from their current output if the profile switch would otherwise cause a step.

What it does not change:

- It does not decide by itself when the transition is complete.
- Completion still uses trusted pitot airspeed first, or `mixer_switch_trans_timer` when pitot is not used.
- With `mixer_vtol_transition_dynamic_mixer = OFF`, the old transition input behavior is preserved and the auto-controller only manages the profile switch timing.

Value direction notes:

- Lower `mixer_vtol_transition_scale_ramp_time_ms`: pusher, lift return, and tilt servo movement happen faster. Too low can still look abrupt.
- Higher `mixer_vtol_transition_scale_ramp_time_ms`: movement is gentler, but the aircraft spends longer with partial pusher/lift/servo authority.
- Lower `vtol_transition_lift_min_percent`: lift motors reduce more during MC -> FW. This can reduce drag/power use but gives less lift reserve.
- Higher `vtol_transition_lift_min_percent`: more lift is kept through transition. `100` keeps full lift power.
- Lower `vtol_transition_mc_authority_min_percent`: MC motor stabilisation is reduced more during MC -> FW. Use carefully on large VTOLs.
- Higher `vtol_transition_mc_authority_min_percent`: more MC stabilisation stays available. `100` keeps full MC stabilisation.
- Lower `vtol_transition_fw_authority_min_percent`: fixed-wing control starts more gently. Higher values bring fixed-wing control in more strongly from the start.
- Lower airspeed thresholds complete sooner; higher thresholds wait for more airspeed before switching profile.

### `mixer_vtol_transition_scale_ramp_time_ms`

This setting controls the time-based smooth movement for:

- MC -> FW forward motor power increase
- FW -> MC forward motor power removal
- FW -> MC lift motor and MC motor stabilisation return
- `INPUT_MIXER_TRANSITION` movement when `mixer_vtol_transition_dynamic_mixer = ON`
- servo output continuation after a direct profile switch or transition abort changes the active mixer output

Example:

- `mixer_vtol_transition_scale_ramp_time_ms = 1200`
- `mixer_switch_trans_timer = 50`

Result:

- The forward motor moves from idle to requested power over `1.2s` during MC -> FW.
- A tilt servo using `INPUT_MIXER_TRANSITION` moves over `1.2s` while dynamic scaling is ON.
- If the profile switch changes the final servo output, the servo continues from its current output and moves toward the new output over a fresh `1.2s`.
- If pitot is not used, the profile switch still happens after `5.0s`.
- If pitot is trusted, the profile switch waits for the configured pitot threshold instead of the `5.0s` timer.

This separation is intentional. Tilt servos should not wait for airspeed to build before they start moving; otherwise the aircraft may need tilt to gain speed but also need speed before it is allowed to tilt. The tilt servo source uses `mixer_vtol_transition_scale_ramp_time_ms` when dynamic scaling is ON.

### Percentage settings

These settings are active only when `mixer_vtol_transition_dynamic_mixer = ON`.

`vtol_transition_lift_min_percent`

- Current purpose: sets the lowest lift motor power used during transition.
- MC -> FW: lift power is reduced toward this value.
- FW -> MC: lift power starts from this value and returns to full power.
- `100` keeps full lift power for the whole transition.

Example:

- `vtol_transition_lift_min_percent = 30`
- At halfway through the lift-power change, lift power is about `65%`.
- At the lowest point, lift power is `30%`, not zero.

`vtol_transition_mc_authority_min_percent`

- Current purpose: sets the lowest MC motor stabilisation strength during transition.
- MC -> FW: active MC motor stabilisation is reduced toward this value.
- FW -> MC: target MC motor stabilisation starts from this value and returns to full strength.
- During FW -> MC, this target MC stabilisation comes from the MC mixer profile and target MC PID preview, not from the active FW controller.

Example:

- `vtol_transition_mc_authority_min_percent = 20`
- MC motor stabilisation never goes below `20%` during the transition.

`vtol_transition_fw_authority_min_percent`

- Current purpose: sets the lowest fixed-wing stabilisation strength during transition.
- MC -> FW: fixed-wing control starts from this value and increases to full strength.
- FW -> MC: fixed-wing control is reduced toward this value.
- When `INPUT_AUTOTRANSITION_TARGET_STABILIZED_*` servo rules are configured in the MC profile, this setting also controls how strongly those target fixed-wing servo corrections are applied during MC -> FW.

Example:

- `vtol_transition_fw_authority_min_percent = 20`
- Fixed-wing control starts gently, then increases as the transition progresses.
- `100` means fixed-wing control is full strength for the whole transition.

### Pusher configuration example

For a pusher VTOL, the preferred smooth setup is:

- In the FW profile, configure the pusher as a normal positive-throttle motor.
- In the MC profile, reserve the same motor index with a placeholder rule if that motor is not used in MC flight.
- Use `throttle = -1.000`, `roll = 0`, `pitch = 0`, `yaw = 0` for the placeholder if Configurator removes zero-throttle motor rules.

Example shape:

```
# MC profile: reserve motor 5 as a placeholder
mixer_profile 2
mmix 4 -1.000 0.000 0.000 0.000

# FW profile: motor 5 is the real forward motor
mixer_profile 1
mmix 4  1.000 0.000 0.000 0.000
```

What happens with dynamic scaling ON:

- MC -> FW: motor 5 starts from idle and smoothly reaches the requested FW throttle over `mixer_vtol_transition_scale_ramp_time_ms`.
- FW -> MC: motor 5 smoothly moves back to idle over `mixer_vtol_transition_scale_ramp_time_ms`.
- If motor 5 is not used by the destination profile after the switch, INAV keeps moving it toward idle instead of stopping it in one step.

The older helper rule still works:

```
mmix 4 -1.200 0.000 0.000 0.000
```

That older style spins the pusher at a fixed helper power only while `MIXER TRANSITION` is active. It can still be useful for legacy setups, but it is not the recommended setup for the smooth auto-transition controller.

### Tilt servo configuration example

Tilt servos commonly use:

- `INPUT_MAX` (`29`) as a fixed offset
- `INPUT_MIXER_TRANSITION` (`38`) as the transition movement
- `INPUT_STABILIZED_YAW` (`2`) for tricopter/tiltmotor yaw correction

Example based on a tricopter/tiltmotor style setup:

```
mixer_profile 2
set platform_type = TRICOPTER

smix reset
smix 0 4 2  -50 0 -1   # yaw correction on servo 4
smix 1 5 2  -50 0 -1   # yaw correction on servo 5
smix 2 4 29  87 0 -1   # fixed tilt offset on servo 4
smix 3 5 29 -87 0 -1   # fixed tilt offset on servo 5
smix 4 4 38 -45 0 -1   # transition tilt movement on servo 4
smix 5 5 38  45 0 -1   # transition tilt movement on servo 5
```

Behavior:

- `INPUT_MAX` stays constant.
- With dynamic scaling OFF, `INPUT_MIXER_TRANSITION` keeps the older fixed transition value while transition mode is active.
- With dynamic scaling ON, `INPUT_MIXER_TRANSITION` moves from `0` to `500` over `mixer_vtol_transition_scale_ramp_time_ms`.
- During MC -> FW, `INPUT_MIXER_TRANSITION` must not move backwards when the FW profile is selected.
- During FW -> MC, it must also continue smoothly if the transition is aborted or reversed.
- If the destination profile does not own the same tilt servo output, INAV keeps the current servo output and moves it toward the destination output instead of briefly jumping through a middle/default position.

Example:

- MC tilt position: `90 degrees`
- transition tilt position from source 38: `45 degrees`
- FW profile final position: `0 degrees`
- `mixer_vtol_transition_scale_ramp_time_ms = 1200`

Expected movement:

- The servo moves from `90` toward `45` over about `1.2s`.
- If the profile switch then changes the final target from `45` to `0`, the servo continues from its current output and moves toward `0` over a fresh `1.2s`.

### Optional fixed-wing control preview on MC profile

If your aircraft has control surfaces that should start helping before the profile switch, add `INPUT_AUTOTRANSITION_TARGET_STABILIZED_*` rules in the MC mixer profile.

Useful inputs:

- `INPUT_AUTOTRANSITION_TARGET_STABILIZED_ROLL` (`61`)
- `INPUT_AUTOTRANSITION_TARGET_STABILIZED_PITCH` (`62`)
- `INPUT_AUTOTRANSITION_TARGET_STABILIZED_YAW` (`63`)
- positive-only and negative-only variants `64..69`

During MC -> FW, these rules let the MC profile apply a preview of the target fixed-wing stabilisation. The preview uses the target fixed-wing PID bank, rates, angle limits, heading-hold limits, and turn-assist gains. During FW -> MC, the same MC profile rules mark which FW servo outputs should reduce their fixed-wing correction while MC motor control comes back.

If you do not configure these rules, pusher and tilt transition can still work. This preview is optional and mainly helps aircraft where control surfaces should start contributing before the final profile switch.

## 3. Automated mission transition (fully autonomous flight)

Mission transition lets a waypoint request MC or FW mode. The mission pauses while INAV performs the transition, then resumes after the transition completes.

### Required setup

Enable navigation-requested profile changes in profiles where NAV is allowed to change mode:

```
mixer_profile 1
set mixer_automated_switch = ON

mixer_profile 2
set mixer_automated_switch = ON
save
```

Enable the waypoint USER flag that will select the VTOL mode:

```
set nav_vtol_mission_transition_user_action = USER1
set nav_vtol_mission_transition_min_altitude_cm = 1200
set nav_vtol_transition_retry_on_airspeed_timeout = ON
set nav_vtol_transition_fail_action_mc_to_fw = POSH
set nav_vtol_transition_fail_action_fw_to_mc = LOITER
save
```

How waypoint USER selection works:

- selected USER bit clear (`0`) means target MC profile
- selected USER bit set (`1`) means target FW profile
- every navigable waypoint should intentionally set or clear that USER bit
- if the aircraft is already in the requested profile, INAV continues without starting a transition

Example mission:

- WP1: USER1 not set, climb in MC near the takeoff area
- WP2: USER1 set, request MC -> FW transition
- WP3: USER1 set, continue fixed-wing navigation
- LAND or final approach waypoint: USER1 not set, request FW -> MC before landing

For MC -> FW mission transition, INAV uses a straight acceleration segment. It does not try to loiter to build airspeed. Normal waypoint advancement is paused until the transition is finished.

### Altitude settings that matter in missions

`nav_vtol_mission_transition_min_altitude_cm`

- Current purpose: minimum altitude before a mission-requested VTOL transition may start.
- New transition use: prevents a mission from starting MC -> FW too low.
- `0` disables this gate.

Example:

- `nav_vtol_mission_transition_min_altitude_cm = 1200`
- Mission MC -> FW transition waits until current altitude is at least `12m` above the navigation reference.

`nav_wp_enforce_altitude`

- Existing purpose: controls whether a waypoint is considered reached only by horizontal distance, or also by altitude.
- New VTOL mission use: helps prevent the aircraft from accepting a transition waypoint horizontally while still far below the requested waypoint altitude.

Examples:

- `nav_wp_enforce_altitude = 0`: waypoint can be accepted by horizontal radius even if altitude is still not reached. This is faster, but can be risky for VTOL transition missions.
- Example with `nav_wp_enforce_altitude = 0`: if WP1 is directly above home at `120m`, the aircraft can be inside the horizontal waypoint radius immediately after takeoff. The waypoint may be accepted before the aircraft climbs to `120m`, so the mission can continue to the next action too early.
- `nav_wp_enforce_altitude = 100`: waypoint must be within about `1m` of the target altitude before it is considered reached.
- `nav_wp_enforce_altitude = 300`: waypoint must be within about `3m`; this is more relaxed and can be useful when altitude estimates are noisy.

`nav_wp_radius`

- Existing purpose: horizontal radius used to decide whether a normal waypoint has been reached.
- VTOL mission impact: if this is very large, a waypoint can be accepted early. Use `nav_wp_enforce_altitude` and `nav_vtol_mission_transition_min_altitude_cm` to avoid starting a transition before the aircraft is high enough.
- Landing safety use: VTOL MC landing settle uses a stricter internal landing capture radius based on `min(nav_wp_radius, 100cm)`.

Examples:

- `nav_wp_radius = 200`: waypoint accepted within `2m` horizontally.
- `nav_wp_radius = 1000`: waypoint accepted within `10m`; useful for fast FW navigation, but too loose by itself for deciding that a VTOL has settled at a landing point.

### Mission transition failure behavior

If transition cannot start because of temporary runtime conditions, INAV waits instead of immediately failing the mission. Temporary conditions include sensor calibration still in progress, navigation position temporarily unusable, heading temporarily unusable, or another transition already active.

Hard safety/configuration problems are treated as errors or use the configured fail action where applicable. Examples include disarmed state, failsafe, missing mixer-profile mode setup, or no valid profile switch path.

MC -> FW fail action:

- `IDLE`: stop mission navigation and leave the pilot to recover
- `POSH`: enter position hold if possible
- `RTH`: return to home
- `EMERGENCY_LANDING`: start emergency landing behavior

FW -> MC fail action:

- `IDLE`: stop mission navigation and leave the pilot to recover
- `LOITER`: loiter in fixed-wing if possible
- `RTH`: return to home
- `EMERGENCY_LANDING`: start emergency landing behavior
- `FORCE_SWITCH`: force the target mixer switch even though the normal transition condition failed

Use `FORCE_SWITCH` only when you have tested the airframe carefully. It is intended as a last-resort choice for setups where staying in FW is more dangerous than switching.

`nav_vtol_transition_retry_on_airspeed_timeout = ON` allows one retry after an airspeed timeout. If the retry also fails, the configured fail action is used.

## 4. Adding VTOL stabilisation

A VTOL in MC mode is not always equivalent to a normal multicopter. Large wings can still create lift and drag while the aircraft is braking, yawing, descending, or correcting altitude. That can turn normal MC commands into unexpected climb, sink, roll, pitch, or yaw coupling.

The VTOL MC protection feature is disabled by default:

```
set vtol_mc_protection_mode = OFF
```

This means:

- normal multicopters do not change
- fixed-wing mode does not change
- existing VTOL behavior does not change unless you enable the feature

### Protection modes

`vtol_mc_protection_mode = OFF`

- Current behavior: no new VTOL MC protection.
- Use this when comparing against older behavior.

`vtol_mc_protection_mode = NAV`

- Adds VTOL MC protections during navigation and automatic-throttle operation.
- Applies only when INAV detects an active multicopter-like profile and another configured profile is fixed-wing-like.
- Does not affect fixed-wing flight.
- Does not affect normal non-VTOL multicopters.

With `NAV`, INAV can:

- reserve throttle range for attitude stabilisation before altitude controller anti-windup limits are applied
- keep hover throttle inside the safe range, shrinking the reserve if necessary
- damp horizontal speed before fully settling position-hold behavior
- relax altitude target capture while the aircraft is bleeding speed or transitioning in MC
- require a more stable landing condition before starting/describing landing as complete
- use a conservative recovery path if attitude becomes excessive during automatic-throttle VTOL MC navigation

`vtol_mc_protection_mode = NAV_AND_STABILIZED`

- Includes everything from `NAV`.
- Also shapes pilot roll, pitch, and yaw commands in ANGLE and HORIZON mode when the aircraft is armed, in VTOL MC mode, not fixed-wing, velocity estimate is trusted, and horizontal speed is high enough.
- This is the mode to use when large wings make manual ANGLE/HORIZON MC flight feel too aggressive at forward speed.

### Roll, pitch, and yaw command shaping

The amount of shaping changes continuously with horizontal speed:

- below about `300 cm/s`, commands are unchanged
- between about `300 cm/s` and `800 cm/s`, commands are gradually reduced
- above about `800 cm/s`, commands are limited to about 50% of the original command

Example:

- At `250 cm/s`, yaw/roll/pitch stick response is normal.
- At `550 cm/s`, the same stick movement commands a smaller rate/angle than usual.
- At `900 cm/s`, the same stick movement commands about half of the normal response.

This is intended to reduce wing-driven surprises during fast MC-mode flight, not to make the aircraft unresponsive. If velocity estimate is not trusted, this shaping is not applied.

### Throttle reserve for attitude authority

`vtol_mc_thr_reserve_percent`

- Current purpose: reserves throttle range so altitude control cannot consume all motor authority.
- Applies when VTOL MC protection is active.
- The reserve is applied before altitude controller anti-windup bounds, not only as a final output clamp.
- Hover throttle is kept inside the safe range. If the configured reserve would exclude hover throttle, INAV shrinks the reserve and sets a debug flag.

Example:

- `nav_mc_hover_thr = 1500`
- `vtol_mc_thr_reserve_percent = 15`
- altitude control is not allowed to command all the way to minimum throttle, because very low base throttle can leave roll, pitch, and yaw corrections with little effective motor authority
- altitude control is not allowed to command all the way to maximum throttle, because saturated motors have no upward headroom left for attitude correction
- roll, pitch, and yaw still have headroom for correction

If `vtol_mc_thr_reserve_percent = 0`, the throttle range is not narrowed, but the other enabled VTOL MC protections can still run.

### Altitude tolerance while braking or transitioning

During VTOL MC braking, wing lift can make the aircraft climb even when the pilot or NAV only intended to slow down. If altitude hold reacts too aggressively, it can reduce throttle too far, then later command too much throttle, leaving less room for attitude stabilisation.

With VTOL MC protection enabled, INAV is more tolerant while the aircraft is still settling:

- NAV capture damps velocity first instead of immediately forcing a hard final hold point.
- Soft altitude capture follows current altitude more gently while horizontal speed is being reduced.
- During auto transition while the active side is MC, the same protection can reduce altitude-control aggression.
- Once the aircraft is stable, normal altitude/position behavior resumes.

This is meant to cover the case where the wings affect MC behavior during braking, yawing, and transition.

## 5. Landing detection setup

Landing detection is safety-critical for VTOL. A large VTOL wing can make the aircraft bounce in ground effect, and barometric altitude alone cannot prove that the aircraft is on the ground.

The new VTOL MC landing logic keeps the normal INAV landing detector, but adds extra checks before VTOL MC can report `LANDING_DETECTED`.

### Basic landing settings

`nav_rth_allow_landing`

- Existing purpose: controls whether RTH is allowed to land.
- Values are `NEVER`, `ALWAYS`, and `FS_ONLY`.
- If landing is not allowed, RTH will not intentionally descend to land.

`nav_disarm_on_landing`

- Existing purpose: allows automatic disarm after landing detection.
- If OFF, landing may be detected but the FC will not automatically disarm because of landing.

`nav_auto_disarm_delay`

- Existing purpose: delay after landing detection before automatic disarm.
- VTOL MC use: delay still applies, but VTOL MC must first pass its additional landing confirmation checks.

`nav_land_detect_sensitivity`

- Existing purpose: scales the generic landing detector velocity and gyro thresholds.
- At default `5`, MC landing detection uses about `100 cm/s` horizontal speed, `100 cm/s` vertical speed, and `4 deg/s` average pitch/roll gyro threshold.
- Higher values make detection easier and earlier, but increase false-detect risk.
- VTOL MC use: this setting can create a landing candidate, but it cannot bypass VTOL MC vertical-speed and throttle-probe confirmation.

Examples:

- `nav_land_detect_sensitivity = 5`: conservative default.
- `nav_land_detect_sensitivity = 7`: detects more easily, useful if the craft bounces on touchdown, but test carefully.
- `nav_land_detect_sensitivity = 10`: much more relaxed; not recommended for initial VTOL testing because it can create landing candidates while still airborne.

`nav_landing_bump_detection`

- Existing purpose: allows a touchdown acceleration bump to become a landing candidate.
- VTOL MC use: a bump is not an immediate disarm shortcut. Trusted high AGL blocks it, and accepted candidates must still pass throttle-probe confirmation.

### Descent speed and final landing behavior

`nav_land_maxalt_vspd`

- Existing purpose: requested vertical descent speed above `nav_land_slowdown_maxalt` during RTH landing.

`nav_land_minalt_vspd`

- Existing purpose: requested vertical descent speed under `nav_land_slowdown_minalt` during RTH landing.
- VTOL MC use: also provides the conservative vertical-speed reference for final landing settle checks, capped internally so it cannot become too loose.
- This is used because it represents the descent speed you already consider acceptable near the ground.

`nav_land_slowdown_maxalt`

- Existing purpose: altitude where RTH landing starts slowing from `nav_land_maxalt_vspd` toward `nav_land_minalt_vspd`.

`nav_land_slowdown_minalt`

- Existing purpose: altitude where RTH landing should already be using `nav_land_minalt_vspd`.

Examples:

- `nav_land_minalt_vspd = 50`: final descent target is `0.5 m/s`; VTOL final landing checks stay conservative.
- `nav_land_minalt_vspd = 100`: final descent target is `1.0 m/s`; VTOL final landing checks allow a faster near-ground descent, but remain internally capped.
- `nav_land_maxalt_vspd = 200`: descent above the slowdown window can be `2.0 m/s`.
- `nav_land_slowdown_maxalt = 2000` and `nav_land_slowdown_minalt = 500`: descent slows between `20m` and `5m` above the landing reference.

### VTOL MC throttle-probe confirmation

VTOL MC landing candidates must pass a short lift-throttle confirmation.

In plain language:

- INAV sees a possible landing.
- Instead of immediately reporting landed, it gently reduces lift throttle for a short confirmation window.
- If the aircraft starts falling away, AGL drops, low-G/unloading is detected, or vertical speed changes toward a stronger descent, the landing candidate is rejected.
- If that small throttle reduction does not produce airborne-response evidence during the confirmation window, landing can be reported.

This avoids a common false positive: the aircraft is still high in the air, vertical speed is temporarily low, and gyro rates are calm, so the generic detector thinks it might be landed. A real airborne VTOL should react to a small lift-throttle reduction; a landed aircraft should not fall away like an airborne one.

The confirmation is not meant to reject every bounce, rocking motion, or pitch/roll wobble by itself. A VTOL can bounce in ground effect shortly before real touchdown. The important question for this check is whether reducing lift throttle causes the aircraft to continue descending like it is still flying. If it does, INAV rejects the candidate and waits for another landing opportunity.

The confirmation does not rely on barometric altitude as proof of AGL. If a trusted surface/AGL sensor is available, it is used as an additional safety input. Without trusted AGL, vertical motion and acceleration are more important than baro altitude.

`nav_mc_hover_thr` matters here, but the probe does not assume that landing throttle is equal to hover throttle. It starts from the current adjusted throttle at the moment of the landing candidate, which during descent may already be below `nav_mc_hover_thr`. `nav_mc_hover_thr` is used to size a small bounded throttle reduction relative to idle/hover range. If the current landing throttle is already below that probe limit, INAV does not raise it just to run the probe. Tune hover throttle before relying on automatic VTOL landing because it still affects throttle reserve and the probe reduction size.

### Landing settle before descent

VTOL MC landing should not start descent just because the aircraft briefly touches a large waypoint radius.

For VTOL MC landing settle:

- INAV uses a tighter landing capture radius based on `min(nav_wp_radius, 100cm)`.
- Horizontal speed must be low.
- Vertical speed must be low when a vertical estimate is available.
- Roll/pitch attitude must be within a safe range.
- Conditions must stay stable long enough before landing descent/landing detection is allowed to proceed.

Examples:

- `nav_wp_radius = 500`: normal waypoint acceptance is `5m`, but VTOL landing settle uses `1m`.
- `nav_wp_radius = 80`: VTOL landing settle uses `80cm`.
- `nav_wp_radius = 1500`: normal waypoint acceptance is `15m`, but VTOL landing settle still uses `1m`.

This lets fixed-wing missions keep a larger waypoint radius while preventing VTOL landing from starting just because the aircraft briefly crossed a loose radius.

### Practical landing setup

Conservative starting point:

```
set nav_rth_allow_landing = ALWAYS
set nav_disarm_on_landing = ON
set nav_auto_disarm_delay = 1000
set nav_land_detect_sensitivity = 5
set nav_landing_bump_detection = ON
set nav_land_minalt_vspd = 50
set nav_land_maxalt_vspd = 150
set nav_land_slowdown_maxalt = 2000
set nav_land_slowdown_minalt = 500
set vtol_mc_protection_mode = NAV
save
```

If the aircraft bounces on landing:

- Do not immediately raise `nav_land_detect_sensitivity` a lot.
- First reduce final descent energy: lower `nav_land_minalt_vspd` if possible, tune hover throttle, and verify `nav_mc_hover_thr`.
- Keep `nav_landing_bump_detection = ON` so a real touchdown bump can help create a candidate.
- Increase `nav_land_detect_sensitivity` only in small steps, and verify with blackbox logs.

## Debugging and setting scope

Useful debug modes:

- `set debug_mode = VTOL_TRANSITION`: transition phase, transition direction, progress, motor scaling, servo transition progress, and profile-switch smoothing state.
- `set debug_mode = VTOL_MC_PROTECT`: VTOL MC protection flags, safe throttle min/max, protected throttle, speed, attitude, and command-shaping/settle progress.
- `set debug_mode = LANDING`: normal landing detector path and landing candidate state.

`VTOL_TRANSITION` debug channels:

- `debug[0]`: transition phase (`0=IDLE`, `1=TRANSITION_INITIALIZE`, `2=TRANSITIONING`, `3=after-switch smoothing`).
- `debug[1]`: active transition request, with direction packed in the high byte.
- `debug[2]`: packed flags. Important bits include direction, controller active, transition input active, airspeed path active, profile switch done, abort state, current/next profile index, mission active, failsafe active, direct profile switch active, target preview mode, after-switch smoothing active, and legacy manual session active.
- `debug[3]`: main transition progress x1000 (`0..1000`).
- `debug[4]`: pusher/forward motor scale x1000 (`0..1000`).
- `debug[5]`: lift motor scale x1000 (`0..1000`).
- `debug[6]`: packed MC/FW stabilisation scales. Low 16 bits are MC stabilisation scale x1000, high 16 bits are FW control scale x1000.
- `debug[7]`: packed progress values. Bits `0..9` are airspeed-linked scaling progress, bits `10..19` are motor ramp progress, and bits `20..29` are after-switch smoothing progress.

`VTOL_MC_PROTECT` debug channels:

- `debug[0]`: flags bitmask. Bits show protection configured, VTOL MC detected, NAV protection active, ANGLE/HORIZON protection active, NAV capture active, landing settle active, bailout active, throttle reserve shrunk, soft altitude capture active, roll/pitch/yaw command shaped, and velocity fallback used.
- `debug[1]`: safe throttle minimum.
- `debug[2]`: safe throttle maximum.
- `debug[3]`: protected throttle.
- `debug[4]`: horizontal speed [cm/s].
- `debug[5]`: vertical speed [cm/s].
- `debug[6]`: max absolute roll/pitch attitude [deci-degrees].
- `debug[7]`: capture/landing/bailout settle elapsed time [ms], or command scale x1000 when command shaping is active, otherwise `1000`.

Per-mixer-profile settings:

- `mixer_automated_switch`: existing RTH use is to allow NAV to switch between MC and FW for return/landing. New mission use is to allow waypoint USER actions to request MC/FW transitions.
- `mixer_switch_trans_timer`: existing legacy transition timer in deciseconds. New auto-transition use is the fallback completion timer when pitot airspeed is not used or not trusted.
- `mixer_vtol_transition_dynamic_mixer`: new optional smooth power/control scaling. OFF keeps old transition motor/servo behavior; ON allows smooth pusher, lift, MC stabilisation, FW control, and transition-servo movement.
- `mixer_vtol_manualswitch_autotransition_controller`: new manual switch controller. OFF keeps older manual behavior; ON makes `MIXER TRANSITION` start one complete transition.
- `mixer_vtol_transition_airspeed_timeout_ms`: new airspeed wait limit. It does not complete transition; it aborts an airspeed-controlled attempt that takes too long while pitot remains trusted.
- `mixer_vtol_transition_scale_ramp_time_ms`: new time used for smooth pusher/lift/transition-servo movement. It also controls how long transition-linked servo outputs continue from their current position after a direct switch or abort.

Global VTOL transition settings:

- `vtol_transition_to_fw_min_airspeed_cm_s`: new preferred MC -> FW completion threshold when pitot is trusted. `0` uses the timer path.
- `vtol_transition_to_mc_max_airspeed_cm_s`: new preferred FW -> MC completion threshold when pitot is trusted. `0` uses the timer path.
- `vtol_fw_to_mc_auto_switch_airspeed_cm_s`: new low-speed fixed-wing protection. When non-zero, FW can automatically start FW -> MC if pitot airspeed falls too low.
- `vtol_transition_lift_min_percent`: new lowest lift motor power during dynamic transition scaling. `100` keeps full lift power.
- `vtol_transition_mc_authority_min_percent`: new lowest MC motor stabilisation strength during dynamic transition scaling. `100` keeps full MC stabilisation.
- `vtol_transition_fw_authority_min_percent`: new lowest FW control strength during dynamic transition scaling. It also scales optional target fixed-wing servo preview rules.
- `nav_vtol_mission_transition_user_action`: new mission selector. OFF disables mission transition; USER1..USER4 chooses which waypoint USER bit requests MC or FW.
- `nav_vtol_mission_transition_min_altitude_cm`: new minimum-altitude gate before mission-requested transition may start. `0` disables this gate.
- `nav_vtol_transition_retry_on_airspeed_timeout`: new mission retry option after an airspeed timeout. If retry also fails, the configured fail action is used.
- `nav_vtol_transition_fail_action_mc_to_fw`: new MC -> FW mission failure action after transition cannot complete safely.
- `nav_vtol_transition_fail_action_fw_to_mc`: new FW -> MC mission failure action after transition cannot complete safely.

Global VTOL MC protection and landing settings:

- `vtol_mc_protection_mode`: new master switch for VTOL MC protection. OFF changes nothing; NAV protects navigation/altitude behavior; NAV_AND_STABILIZED also shapes ANGLE/HORIZON roll, pitch, and yaw commands at speed.
- `vtol_mc_thr_reserve_percent`: new throttle reserve for attitude authority while altitude/NAV owns throttle. Applied before altitude anti-windup bounds.
- `nav_mc_hover_thr`: existing MC hover throttle hint. New VTOL protection use is to keep hover throttle inside the protected range and to make landing throttle confirmation more accurate.
- `nav_wp_radius`: existing normal waypoint acceptance radius. New VTOL landing use is capped internally for landing settle so a large waypoint radius cannot by itself start landing too early.
- `nav_wp_enforce_altitude`: existing waypoint altitude acceptance tolerance. New VTOL mission use is to keep transition waypoints from being accepted horizontally while still far from target altitude.
- `nav_rth_allow_landing`: existing RTH landing permission. VTOL landing logic only matters when navigation is actually allowed to land.
- `nav_disarm_on_landing`: existing automatic disarm permission. New VTOL MC landing confirmation must pass before this can lead to disarm.
- `nav_auto_disarm_delay`: existing delay after landing detection. VTOL MC adds its own confirmation before landing is reported.
- `nav_land_detect_sensitivity`: existing generic landing detector sensitivity. New VTOL MC use allows it to create candidates, but not to bypass vertical-speed and throttle confirmation.
- `nav_landing_bump_detection`: existing touchdown-bump candidate detection. New VTOL MC use treats bumps as candidates only, not immediate proof of landing.
- `nav_land_minalt_vspd`: existing final RTH landing descent speed. New VTOL MC use is a conservative vertical-speed reference for final settle/landing checks.
- `nav_land_maxalt_vspd`: existing higher-altitude RTH landing descent speed. It controls descent energy before the final slowdown region.
- `nav_land_slowdown_minalt`: existing lower boundary of the RTH landing slowdown window. Below this, RTH landing should be using `nav_land_minalt_vspd`.
- `nav_land_slowdown_maxalt`: existing upper boundary of the RTH landing slowdown window. Above this, RTH landing may use `nav_land_maxalt_vspd`.

Mission profile-switch dependency:

- Mission VTOL transition uses the existing profile-change path.
- Configure two valid mixer profiles.
- Configure a valid `MIXER PROFILE 2` mode activation condition.
- Enable `mixer_automated_switch` in profiles where NAV is allowed to request VTOL profile changes.

# Notes and Experiences 
## General
- VTOL model operating in multi-copter (MC) mode may encounter challenges in windy conditions. Please exercise caution when testing in such conditions.
- Make sure you can recover from a complete stall before trying the mid air transition
- It will be much safer if you can understand every line in diff all, read your diff all before maiden

## Tilting-rotor
- In some tilting motor models, you may experience roll/yaw coupled oscillations when `MIXER TRANSITION` is activated. To address this issue, you can try the following:
    1. Use prop blade meets at top/rear prop direction for tilting motors to balance the effects of torque and P-factor. 
    2. In addition to 1. Add a little yaw mixing(about 0.2) in tilt motors.
- There will be a time window that tilting motors is providing up lift but rear motor isn't. Result in a sudden pitch raise on the entering of the mode. Use the max speed or faster speed in tiling servo to reduce the time window. OR lower the throttle on the entering of the FW mode to mitigate the effect.
## Dedicated forward motor 
- Easiest way to setup a vtol. and efficiency can be improved by using different motor/prop for hover and forward flight

## Pitot-based transition logic (reference)

When pitot is healthy/available, transition progress is airspeed-driven (not timer-driven).

- MC -> FW:
  - progress = `constrain(airspeed / to_fw_threshold, 0..1)`
  - completion condition = `airspeed >= to_fw_threshold`

- FW -> MC:
  - capture `startAirspeed` when transition starts
  - progress = `constrain((startAirspeed - airspeed) / (startAirspeed - to_mc_threshold), 0..1)`
  - completion condition = `airspeed <= to_mc_threshold`

Smooth transition power changes (`mixer_vtol_transition_dynamic_mixer = ON`) use this progress:

- MC -> FW:
  - forward motor power ramps `0 -> 1`
  - lift motor power ramps `1 -> vtol_transition_lift_min_percent`
  - MC stabilisation ramps `1 -> vtol_transition_mc_authority_min_percent`
  - FW control ramps `vtol_transition_fw_authority_min_percent -> 1`

- FW -> MC:
  - forward motor power ramps `1 -> 0`
  - lift motor power ramps `vtol_transition_lift_min_percent -> 1`
  - MC stabilisation ramps `vtol_transition_mc_authority_min_percent -> 1`
  - FW control ramps `1 -> vtol_transition_fw_authority_min_percent`

After the profile switch, INAV keeps only the old propulsion output alive for a short smooth shutdown:

- MC -> FW: lift motors that are not used by the FW profile move to idle.
- FW -> MC: a forward motor that is not used by the MC profile keeps moving to idle.
- This after-switch smoothing uses `mixer_vtol_transition_scale_ramp_time_ms` and does not keep the old PID/controller active.

Forward motor power increase, transition servo movement, and airspeed-linked control scaling are related but separate.
For MC->FW, forward motor power uses `mixer_vtol_transition_scale_ramp_time_ms`; if this is `0`, the motor goes to full power immediately.
For FW->MC, the same timer ramps the forward motor down to idle while lift power and MC stabilisation rise back from their configured minimums; if this is `0`, those changes happen immediately.
For `INPUT_MIXER_TRANSITION`, the same timer is used only when `mixer_vtol_transition_dynamic_mixer = ON`. With dynamic mixer OFF, source 38 keeps the older fixed transition endpoint behavior.
If a profile switch or direct switch affects servos that use transition-linked inputs, INAV captures the current servo output and moves from that captured output toward the new profile output using a fresh `mixer_vtol_transition_scale_ramp_time_ms` window.
This timer does not decide when the transition completes.
In MC->FW, lift power reduction, MC stabilisation reduction, and FW control increase still prefer pitot-based transition progress whenever pitot is working.
In FW->MC, FW control reduction still prefers pitot-based transition progress, while forward motor removal, lift power return, and MC motor stabilisation return use the time-based motor ramp.
If pitot is not usable, the airspeed-linked changes fall back to the normal transition timer/progress behavior (`mixer_switch_trans_timer`).

For legacy/helper transition motors (`-2.0 <= throttle <= -1.05`), output is interpolated from idle to target:

`motor = idle + (target - idle) * pusherScale`

where:
- `target = -mixerThrottle * 1000`
- `idle = throttleRangeMin`

If pitot is unavailable/unhealthy, timer fallback is used (`mixer_switch_trans_timer`).

For smooth auto-transition, the preferred forward motor setup is a normal positive-throttle rule in the FW mixer profile, with a placeholder on the same motor index in the MC mixer profile. Use `throttle = -1.000` for that placeholder if Configurator removes zero-throttle motor rules. If a helper such as `throttle = -1.200` is used, INAV moves from that helper output to the real FW mixer output after the profile switch.
