# Welcome to INAV VTOL Testing

Thank you for participating in the INAV VTOL testing phase.

## Who Should Use This Tutorial?

This tutorial is designed for individuals who have prior experience with both INAV multi-rotor and INAV wing configurations/operations. If you're not familiar with them, this tutorial might be not for you.

## Firmware Status

The firmware is in a flyable state, but it hasn't undergone extensive testing yet. This means there may be potential issues that have not yet been discovered.

It's important to note that VTOL model operating in multi-copter (MC) mode may encounter challenges in windy conditions. Please exercise caution when testing in such conditions.

## Future Changes

Please be aware that both the setup procedure and firmware may change in response to user feedback and testing results. As we receive feedback and gather more data, adjustments and improvements will be made to enhance the VTOL experience within INAV.

## Your Feedback Matters

We highly value your feedback as it plays a crucial role in the development and refinement of INAV VTOL capabilities. Please share your experiences, suggestions, and any issues you encounter during testing. Your insights are invaluable in making INAV VTOL better for everyone.

# VTOL Configuration Steps
The VTOL functionality is achieved by transitioning between two mixer profiles with associated PID profiles.
1. **Profile 1:**
   - Configure it as a normal FixedWing/multi-copter.

2. **Profile 2:**
   - Configure it as a normal multi-copter/FixedWing.

3. **Mode Tab Settings:**
   - Set up switching in the mode tab.

4. *(Recommended)* **Transition Mixing (Multi-Rotor Profile):**
   - Configure transition mixing in the multi-rotor profile.

5. *(Recommended)* **VTOL-Specific Settings:**
   - Configure VTOL-specific settings.

6. *(Optional)* **Automated Switching (RTH):**
   - Optionally, set up automated switching in case of failsafe.

# Parameter Configuration (Partial List)

## Shared Parameters

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
## Profile-Specific Parameters in VTOL
- **Mixer Profile**
    - **Mixer Configuration:**
        - platform_type
        - motor_stop_on_low
        - motor_direction_inverted, and more·······
    - **Motor Mixing (mmix)**
    - **Servo Mixing (smix)**
- **PID Profile**
  - PIDs for Roll, Pitch, Yaw
  - PIDs for Navigation Modes
  - TPA (Throttle PID Attenuation) Settings
  - Rate Settings
- ·······

# Configuring as a Normal FixedWing/Multi-Copter in two profiles

To set up your vehicle as a normal FixedWing or multi-copter, follow these steps:

1. **Select the Mixer Profile and PID Profile:**
   - In the CLI, switch to the mixer_profile and pid_profile you wish to set first:
     ```
     mixer_profile 1
     set mixer_pid_profile_linking = ON  # Let the mixer_profile handle the pid_profile on this mixer_profile
     save
     ```

2. **Configure the FixedWing/Multi-Copter:**
   - Configure your vehicle as you normally would, or you can copy and paste default settings to expedite the process.
   - Consider conducting a test flight to ensure that everything operates as expected. And tune the settings, trim the servos.
   - You may encoter display probelms in the output mapping of the configurator, it differs from the actual mapping in FC. You can add a dummy motormix (throttle=0.01,props removed) or dummy servomix (rate=1) if needed, and delete them once all settings are finalized.

3. **Switch to Another Mixer Profile with PID Profile:**
   - In the CLI, switch to another mixer_profile along with the appropriate pid_profile:
     ```
     mixer_profile 2
     set mixer_pid_profile_linking = ON
     save
     ```

4. **Configure the Multi-Copter/FixedWing:**
   - Set up your multi-copter/FixedWing as usual, this time for mixer_profile 2 and pid_profile 2.
   - Utilize the 'MAX' input in the servo mixer to tilt the motors without altering the servo midpoint.
   - At this stage, focus on configuring profile-specific settings. You can streamline this process by copying and pasting the default PID settings.
   - Consider conducting a test flight to ensure that everything operates as expected. And tune the settings.
   - It is advisable to have a certain degree of control surface (elevon / elevator) mapping for stabilization even in MR mode. This helps improve control authority, especially when airspeed is high.

# Mode Tab Settings:
Here is a example:
![Alt text](Screenshots/mixer_profile.png)
|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| FW(profile1) with transition off |  MC(profile2) with transition on  | MC(profile2) with transition off |

We recommend using an RC mode switch to activate modes or switch profiles. Please note the following important considerations:


- Profile file switching becomes available after completing the runtime sensor calibration(15-30s after booting). And It is **not available** when a navigation mode or position hold is active.

- By default, `mixer_profile 1` is used, while `mixer_profile 2` is activated when the `MIXER PROFILE 2` mode box is selected. Once configured successfully, you will notice that the profiles and model preview automatically switch when you reflash the relevant INAV Configurator tabs. 

- Use the `MIXER TRANSITION` to gain airspeed in MC profile, set `MIXER TRANSITION` accordingly.

- Additionally, you can configure it as a 4-state switch by adding FW (profile 1) with  `MIXER TRANSITION`.

Conduct a bench test on the model (without props attached). The model can now switch between FixedWing and multi-copter modes while armed. Furthermore, it is capable of mid-air switching, resulting in an immediate stall upon entering fixed-wing mode

# Transition Mixing (Multi-Rotor Profile)
The 'transition input' is typically useful in MR (Multi-Rotor) mode to gain airspeed. Both the servo mixer and motor mixer can accept the `MIXER TRANSITION` as an input. When the transition mode is activated, the associated motor or servo will move accordingly. Please note that transition input is disabled when a navigation mode is activated. The use of Transition input is necessary to enable additional features such as VTOL RTH.
## Servo 'Transition Input'
Add new servo mixer rules, select 'Mixer Transition' in input. This will allow tilt the motor 45 deg in tilting rotor setup.
It 

In some tilting motor models, you may experience roll or yaw oscillations when `MIXER TRANSITION` is activated. To address this issue, you can add servo mixing rules with an opposite rate to compensate. Use a logic condition to ensure that these compensation servo mixing rules are active only when `MIXER TRANSITION` is activated.

You can achieve this by checking the value of 'Flight:MixerTransition Active,' which is set to 1 when `MIXER TRANSITION` is activated, either by the user or through automated transition.

## Motor 'Transition Input'

The motor 'transition input' is used for a dedicated forward motor in a fixed-wing mode setup. Please note that it is not fully supported by the configurator at this time.

The default `mmix` throttle value is 0.0. It will not appear in the `diff` command when the throttle value is 0.0 (unused), which causes the motor to stop.

- 0.0 < throttle <= 1.0: Normal mapping
- -1.0 < throttle <= 0.0: Motor stop (default value 0)
- -2.0 < throttle < -1.0: The motor will spin regardless of the radio's throttle position at a speed of `abs(throttle) - 1` when Mixer Transition is activated.

Here's an example: This configuration will spin motor number 5 (counting from 1) at 20% throttle, but only in transition mode. This setup is useful for gaining speed in a "4 rotor 1 pusher" configuration:

```
mmix 4 -1.200 0.000 0.000 0.000
```

After these settings, your model should be able to enter FixedWing mode without stalling.

# VTOL-Specific Settings
recommended settings
```
set nav_disarm_on_landing = OFF
set airmode_type = STICK_CENTER_ONCE 
set ahrs_inertia_comp_method = VELNED
set tpa_on_yaw = ON ##for yaw by tilting motor on the mc pid_profile
```

# Automated Switching (RTH):
This is one of the least tested features. This feature is primarily designed for Return to Home (RTH) in the event of a failsafe. When configured correctly, the model will use the Fixed-Wing (FW) mode to efficiently return home and then transition to Multi-Copter (MC) mode for easier landing.

To enable this feature, type follwoing command in cli

1. In your MC mode mixer profile (e.g., mixer_profile 2), set `mixer_automated_switch` to `ON`.
2. Set `mixer_switch_trans_timer` in the MC mode mixer profile to specify the time required for your model to gain sufficient airspeed before transitioning to FW mode. For example, you can set it to 30 decaseconds (30 ds).
3. In your FW mode mixer profile (e.g., mixer_profile 1), also set `mixer_automated_switch` to `ON`.
4. Save your settings.

When `mixer_automated_switch` is set to `ON` for a mixer profile, it means that the model will automatically schedule a transition when a request is made by the navigation controller, such as during an RTH operation when heading home (applies to MC mixer_profile) or when landing (applies to FW mixer_profile).

If you set `mixer_automated_switch` to `OFF` for all mixer profiles (the default setting), the model will not perform automated transitions.
