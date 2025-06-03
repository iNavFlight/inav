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

### The VTOL functionality is achieved by switching/transitioning between two configurations stored in the FC. VTOL specific configurations are Mixer Profiles with associated PID profiles. One profile set is for fixed-wing(FW) mode, One is for multi-copter(MC) mode. Configuration/Settings other than Mixer/PID profiles are shared among two modes 
![Alt text](Screenshots/mixerprofile_flow.png)

0. **Find a DIFF ALL file for your model and start from there if possible**
   - Be aware that `MIXER PROFILE 2` RC mode setting introduced by diff file can get your stuck in a mixer_profile. remove or change channel to proceed
1. **Setup Profile 1:**
   - Configure it as a normal fixed-wing/multi-copter.

2. **Setup Profile 2:**
   - Configure it as a normal multi-copter/fixed-wing.

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


set nav_disarm_on_landing = OFF  #band-aid for false landing detection in NAV landing of multi-copter
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

profile 1 #pid profile
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

1. **Select the fisrt Mixer Profile and PID Profile:**
   - In the CLI, switch to the mixer_profile and pid_profile you wish to set first. You can also switch mixer_profile/pid_profile through gui if with aforementioned presets loaded.
     ```
     mixer_profile 1 #in this example, we set profile 1 first
     set mixer_pid_profile_linking = ON  # Let the mixer_profile handle the pid_profile switch on this mixer_profile
     set platform_type = AIRPLANE
     save
     ```

2. **Configure the fixed-wing/Multi-Copter:**
   - Configure your fixed-wing/Multi-Copter as you normally would, or you can copy and paste default settings to expedite the process.
   - Dshot esc protocol availability might be limited depends on outputs and fc board you are using. change the motor wiring or use oneshot/multishot esc protocol and calibrate throttle range.
   - You can use throttle = -1 as a placeholder for the motor you wish to stop if the motor isn't the last motor
   - Consider conducting a test flight to ensure that everything operates as expected. And tune the settings, trim the servos.

![Alt text](Screenshots/mixerprofile_fw_mixer.png)


# STEP 2: Configuring as a Multi-Copter in Profile 2

1. **Switch to Another Mixer Profile with PID Profile:**
   - In the CLI, switch to another mixer_profile along with the appropriate pid_profile. You can also switch mixer_profile/pid_profile through gui if with aforementioned presets loaded.
     ```
     mixer_profile 2
     set mixer_pid_profile_linking = ON
     set platform_type = MULTIROTOR/TRICOPTER
     save
     ```

2. **Configure the Multicopter/tricopter:**
   - Set up your multi-copter/fixed-wing as usual, this time for mixer_profile 2 and pid_profile 2.
   - Utilize the 'MAX' input in the servo mixer to tilt the motors without altering the servo midpoint.
   - At this stage, focus on configuring profile-specific settings. You can streamline this process by copying and pasting the default PID settings.
   - you can set -1 in motor mixer throttle as a place holder: disable that motor but will load following motor rules
   - compass is required to enable navigation modes for multi-rotor profile.
   - Consider conducting a test flight to ensure that everything operates as expected. And tune the settings.
   - It is advisable to have a certain degree of control surface (elevon / elevator) mapping for stabilization even in multi-copter mode. This helps improve control authority when airspeed is high. It might be unable to recover from a dive without them.

![Alt text](Screenshots/mixerprofile_mc_mixer.png)

5. **Tailsitters:planned for INAV 7.1**
   - Configure the fixed-wing mode/profile sets normally. Use MultiCopter platform type for tail_sitting flying mode/profile sets. 
   - The baseline board aliment is FW mode(ROLL axis is the trust axis). So set `tailsitter_orientation_offset = ON ` in the tail_sitting MC mode.
   - Configure mixer ROLL/YAW mixing according to tail_sitting orientation in the tail_sitting MC mode. YAW axis is the trust axis.
   - Conduct a bench test and see the orientation of the model changes in inav-configurator setup tab

# STEP 3: Mode Tab Settings:
### We recommend using an 3-pos switch on you radio to activate these modes, So pilot can jump in or bell out at any moment.
### Here is a example, in the bottom of inav-configurator Modes tab:
![Alt text](Screenshots/mixer_profile.png)
|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| Profile1(FW) with transition off |  Profile2(MC) with transition on  | Profile2(MC) with transition off |

- Profile file switching becomes available after completing the runtime sensor calibration(15-30s after booting). And It is **not available** when a navigation mode or position hold is active.

- By default, `mixer_profile 1` is used. `mixer_profile 2` is used when the `MIXER PROFILE 2` mode is activate. Once configured successfully, you will notice that the profiles and model preview changes accordingly when you refresh the relevant INAV Configurator tabs. 

- Use the `MIXER TRANSITION` mode to gain airspeed in MC profile, set `MIXER TRANSITION` accordingly.

Conduct a bench test on the model (without props attached). The model can now switch between fixed-wing and multi-copter modes while armed. Furthermore, it is capable of mid-air switching, resulting in an immediate stall upon entering fixed-wing profile

# STEP 4: Tilting Servo Setup (Recommended)
### Setting up the tilting servos to operate correclty is crucial for correct yaw control of the craft. Using the default setup works, but will most likely result in your craft crawling forward with evey yaw input.
The steps below describe how you can fine-tune the tilting servos such as to obtian the desired result.

1. **Set the tilt servos at 45 degrees:**
   - Connect and power the tilting servos with your flight controller.
   - Enter transition mode (your switch should be in the mid-position).
   - Check from the Outputs tab output that the tilt servo channels are exactly at 1500μs.
   - In this mode, your tilt servos should be at the 45-degree position and you can now mount the motor and prop to your tilt servo such that the angle of the motor mounting plate is at 45 degrees upwards.
   - NOTE1: If you have dedicated tilt servos, you may have engraved indeces on the servos and tilting motor sassembly to help you with this step. If the servos don't end up exactly at 45 degrees due to the teeth on the servo and the control arm/plate, don't worry, this will be automatically adjusted after completing the other steps below.
   - NOTE2: If you are using control rods to adjust the tilt of the servos, adjust the lenth of your control rod and the position of the control arm position the control arm as close as possible to the mid position. It will depend on the oriatation of the servo, but generally speaking, the control arm of the servo should be pointed perpendicular to the fuselage when the motor mounts are at the 45 degree setting.

2. **Switch to Multicopter/Tricopter:**
   - Assuming that you have set up your mixer similar to STEP1 and STEP2, you can now switch to the tricopter/multicopter mode and your servos should be tilting the motors upwards. If this is not the case, reverse the servo(s) in the Outputs tab such that the servo(s) is/are pointed upwards.
   - It is OK for the servos not to point exactly 90 degrees upwards, but they should be as close as possible to that position.
   - Also, ensure that your MAX values in the Mixer tab are at 100 and -100, so that your servo will move to the maximum position, as shown in the screenshots in STEP1 and STEP2.

3. **Adjust the maximum throws for the Multicopter/Tricopter mode:**
   - While in tricopter mode, go to the Outputs tab and adjust the endpoints MIN and MAX values such that when your motors are pointed slightly backwards.
   - Rotate the prop such that it is pointed backwards towards the wing/motor mount and ensure that the gap is the same on both sides by adjusting the MIN and MAX values for the tilt servo channels.
   - NOTE: You can check the distance with calipers or gauge blocks. Alternatively, you can adjust the MIN and MAX for your tilting servos such that the props are just touching the top of the wing or motor mount, and then you can increase/degrease the MIN and MAX values for each channel by the same ammount for both servos. This should ensure that you have the same gap between the tip of the prop and the wing or motor mount for both sides.

4. **Adjust the minimum position for the Fixed-wing mode:**
   - Repeat the same step as point 3 with the model in fixed-wing mode, where the servos are tilted forwards.
   - For this step, you just have to make sure that the motors are pointed exactly forwards.
   - You can do this by adjusting the respective MIN and MAX values in the Output tab for the tilt servo channels while in fixed-wing mode.
   - NOTE: Ensuring that your servos are tilted exactly forward is a crucial step as it can cause the plane to roll slightly if that it is not the case. However, ensuring the exact aligment will depend on your specific setup. If you are using dedicated tilting motor servos rather than standard servos with control arms and pushrods, you can make sure that you are exact by measuring the distance between the front edge of the tilting servo and the of the motor mounting plate. If the disances are the uniform across each mount and same on both motors, your servos are pointed forwads correctly.

5. **Adjsut the vertival position of the tilt servos:**
   - Switch back to multicopter/tricopter mode and open the Mixer tab.
   - Start adjusting the `MAX` mixer lines from STEP2 such that the servos are pointed exactly upwards. In other words, start reducing the values of 100 and -100 to something like 80 and -80 until the motors are are pointed exaxctly upwards.
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
If you have set up the mixer as suggested in STEP1 and STEP2, you may have to deal with negative values for the mixer. You may wish to reverese a servo so that you don't have to deal with the negative signs. In that case, you may have to adjust the MIN and MAX values from point 4 again, so that your tilt servos are operating correctly. Check the operation of the servos once again for the YAW control in multicopter/tricipter mode as well as the horixontal position of the tilt servos in fixed-wing mode.


# STEP 5: Transition Mixing (Multi-Rotor Profile)(Recommended)
### Transition Mixing is typically useful in multi-copter profile to gain airspeed in prior to entering the fixed-wing profile. When the `MIXER TRANSITION` mode is activated, the associated motor or servo will move according to your configured Transition Mixing. 

Please note that transition input is disabled when a navigation mode is activated. The use of Transition Mixing is necessary to enable additional features such as VTOL RTH with out stalling.
## Servo 'Transition Mixing': Tilting rotor configuration.
Add new servo mixer rules, and select 'Mixer Transition' in input. Set the weight/rate according to your desired angle. This will allow tilting the motor for tilting rotor model.

![Alt text](Screenshots/mixerprofile_servo_transition_mix.png)

## Motor 'Transition Mixing': Dedicated forward motor configuration
In motor mixer set:
- -2.0 < throttle < -1.0: The motor will spin regardless of the radio's throttle position at a speed of `abs(throttle) - 1` multiplied by throttle range only when Mixer Transition is activated.
- Airmode type should be set to "STICK_CENTER". Airmode type must NOT be set to "THROTTLE_THRESHOLD". If set to throttle threshold the (-) motor will spin till throttle threshold is passed.
  
![Alt text](Screenshots/mixerprofile_4puls1_mix.png)

## TailSitter 'Transition Mixing': 
No additional settings needed, 45deg off set will be added to target pitch angle for angle mode in the firmware.

### With aforementioned settings, your model should be able to enter fixed-wing profile without stalling.

# Automated Switching (RTH) (Optional):
### This is one of the least tested features. This feature is primarily designed for Return to Home (RTH) in the event of a failsafe. 
When configured correctly, the model will use the Fixed-Wing (FW) mode to efficiently return home and then transition to Multi-Copter (MC) mode for easier landing.

To enable this feature, type following command in cli

1. In your MC mode mixer profile (e.g., mixer_profile 2), set `mixer_automated_switch` to `ON`. leave it to `OFF` if burning remaining battery capacity on the way home is acceptable.
```
mixer_profile 2or1
set mixer_automated_switch= ON
```

2. Set `mixer_switch_trans_timer` ds in cli in the MC mode mixer profile to specify the time required for your model to gain sufficient airspeed before transitioning to FW mode. 
```
mixer_profile 2or1
set mixer_switch_trans_timer = 30 # 3s, 3000ms
```
3. In your FW mode mixer profile (e.g., mixer_profile 1), also set `mixer_automated_switch` to `ON`. leave it to `OFF` if automated landing in fixed-wing is acceptable.
```
mixer_profile 1or2
set mixer_automated_switch = ON
```
4. Save your settings. type `save` in cli. 

If you set `mixer_automated_switch` to `OFF` for all mixer profiles (the default setting), the model will not perform automated transitions. You can always enable navigation modes after performing a manual transition.


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
