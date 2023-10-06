# MixerProfile

A MixerProfile is a set of motor mixer, servo-mixer and platform type configuration settings to enable VTOL transitions.

Currently two profiles are supported on targets other than F411 and F722 (due to resource constraints on these FC). i.e VTOL transition is not available on  F411 and F722 targets.

By default, switching between profiles requires reboot to take affect. However, when all conditions are met, and a suitable [configuration](#configuration) has been applied, `mixer_profile` also  allows in-flight profile [switching](#rc-mode-settings) to allow things like VTOL operation. This is the recommended operating mode.

Please note that this is an emerging / experimental capability that will require some effort by the pilot to implement.

## Setup for VTOL
- A VTOL specific FC target or `timer_output_mode` overrides was required in the early stage of the development, But since unified mapping introduced in INAV 7.0 It is not needed anymore.
- ~~For mixer profile switching it is necessary to keep motor and servo PWM mapping consistent between Fixed-Wing (FW) and Multi-rotor (MR) profiles~~
- ~~Traditionally, FW and MR have had different enumerations to define the PWM mappings. For VTOL operation it is necessary to set the `timer_output_mode` overrides to allow a consistent enumeration and thus mapping between MR and FW modes.~~
- ~~In operation, it is necessary to set the `mixer_profile` and the `pid_profile` separately and to set a [RC mode](#rc-mode-settings) to switch between them.~~
## Configuration
### Timer overrides
Set the timer overrides for the outputs that you are intended to use.
For SITL builds, is not necessary to set timer overrides.
Please note that there are some display issues on the configurator that will show wrong mapping on the mixer_profile which has less motor/servo compared with the another
### Profile Switch

Setup the FW mode and MR mode separately in two different mixer profiles:

In this example, FW mode is `mixer_profile` 1 and MR mode is `mixer_profile` 2.

Currently, the INAV Configurator does not fully support `mixer_profile`, so some of the settings have to be done in CLI.

Add `set mixer_pid_profile_linking = ON` in order to enable `pid_profile` auto handling. It will change the `pid profile` index according to the `mixer_profile` index on FC boot and allow `mixer_profile` hot switching (this is recommended usage).

The following 2 `mixer_profile` sections are added in the CLI:

```
#switch to mixer_profile by cli
mixer_profile 1

set platform_type = AIRPLANE
set model_preview_type = 26
# motor stop feature have been moved to here
set motorstop_on_low = ON
# enable pid_profile auto handling (recommended).
set mixer_pid_profile_linking = ON
save
```
Then finish the aeroplane setting on mixer_profile 1

```
mixer_profile 2

set platform_type = TRICOPTER
set model_preview_type = 1
# also enable pid_profile auto handling
set mixer_pid_profile_linking = ON
save
```
Then finish the multi-rotor setting on `mixer_profile` 2.

Note that default profile is profile `1`.

You can use `MAX` servo input to set a fixed input for the tilting servo. Speed setting for `MAX` input is available in the CLI.

It is recommended to have some amount of control surface (elevon / elevator) mapped for stabilization even in MR mode to get improved authority when airspeed is high.

**Double check all settings in CLI with the `diff all` command**; make sure you have set the correct settings. Also check what will change with `mixer_profile`. For example servo output min / max range will not change. But `smix` and `mmix` will change.

### Mixer Transition input

Typically, 'transition input' will be useful in MR mode to gain airspeed.
Both the servo mixer and motor mixer can accept transition mode as an input.
The associated motor or servo will then move accordingly when transition mode is activated.
Transition input is disabled when navigation mode is activate

The use of Transition Mode is recommended to enable further features and future developments like fail-safe support. Mapping motor to servo output, or servo with logic conditions is **not** recommended

#### Servo

38 is the input source for transition input; use this to tilt motor to gain airspeed.

Example: Increase servo 1 output by +45 with speed of 150 when transition mode is activated for tilted motor setup:

```
# rule no; servo index; input source; rate; speed; activate logic function number
smix 6 1 38 45 150 -1
```
Please note there will be a time window that tilting motors is providing up lift but rear motor isn't. Result in a sudden pitch raise on the entering of the mode. More forward tilting servo position on transition input(you can use 'speed' in servo rules to slowly move to this position), A faster tilting servo speed on `MAX` servo input will reduce the time window. OR lower the throttle on the entering of the FW mode to mitigate the effect.

#### Motor

The default `mmix` throttle value is 0.0, It will not show in `diff` command when throttle value is 0.0 (unused); this causes the motor to stop.

- 0.0<throttle<=1.0 : normal mapping
- -1.0<throttle<=0.0 : motor stop, default value 0
- -2.0<throttle<-1.0 : spin regardless of the radio's throttle position at speed `abs(throttle)-1` when Mixer Transition is activated.

Example: This will spin motor number 5 (counting from 1) at 20%, in transition mode only, to gain speed for a "4 rotor 1 pusher" setup:

```
# motor number; throttle; roll; pitch; yaw
mmix 4 -1.200  0.000  0.000  0.000
```

### RC mode settings

It is recommend that the pilot uses a RC mode switch to activate modes or switch profiles.
Profile files Switching is not available until the runtime sensor calibration is done. Switching is NOT available when navigation mode is activate or position controller is activate, including altitude hold.

`mixer_profile` 1 will be used as default, `mixer_profile` 2 will be used when the `MIXER PROFILE 2` mode box is activated. Once successfully set, you can see the profiles / model preview etc. will switch accordingly when you view the relevant INAV Configurator tabs. Checking these tabs in the INAV Configurator will help make the setup easier.

Set `MIXER TRANSITION` accordingly when you want to use `MIXER TRANSITION` input for motors and servos. Here is sample of using the `MIXER TRANSITION` mode:

![Alt text](Screenshots/mixer_profile.png)

|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| FW(profile1) with transition off |  MC(profile2) with transition on  | MC(profile2) with transition off |

It is also possible to set it as 4 state switch by adding FW(profile1) with transition on.

### Automated Transition
This feature is mainly for RTH in a failsafe event. When set properly, model will use the FW mode to fly home efficiently, And land in the MC mode for easier landing.
Set `mixer_automated_switch` to `ON` in mixer_profile for MC mode. Set `mixer_switch_trans_timer` in mixer_profile for MC mode for the time required to gain airspeed for your model before entering to FW mode, for example, 50 ds. Finally set `mixer_automated_switch` to `ON` in mixer_profile for FW mode to let the model land in MC mode.
```
mixer_profile 2
set mixer_automated_switch = ON
set mixer_switch_trans_timer = 50
mixer_profile 1
set mixer_automated_switch = ON
save
```

`ON` for a mixer_profile\`s `mixer_automated_switch` means to schedule a Automated Transition when RTH head home(applies for MC mixer_profile) or RTH Land(applies for FW mixer_profile) is requested by navigation controller.

When `mixer_automated_switch`:`OFF` is set for all mixer_profiles(defaults). Model will not perform automated transition at all.


### TailSitter support
TailSitter is supported by add a 90deg offset to the board alignment. Set the board aliment normally in the mixer_profile for FW mode(`set platform_type = AIRPLANE`), The motor trust axis should be same direction as the airplane nose. Then, in the mixer_profile for takeoff and landing `set platform_type = TAILSITTER`. The `TAILSITTER` platform type is same as `MULTIROTOR` platform type, expect for a 90 deg board alignment offset. In `TAILSITTER` mixer_profile, when motor trust/airplane nose is pointing to the sky, 'airplane bottom'/'multi rotor front' should facing forward in model preview. Set the motor/servo mixer according to multirotor orientation, Model should roll around geography's longitudinal axis, the roll axis of `TAILSITTER` will be yaw axis of `AIRPLANE`. In addition, When `MIXER TRANSITION` input is activated, a 45deg offset will be add to the target angle for angle mode.

## Happy flying

Remember that this is currently an emerging capability:

* Test every thing on bench first.
* Remove the props and try `MIXER PROFILE 2`, `MIXER TRANSITION` RC modes while arming.
* Then try MR or FW mode separately see if there are any problems.
* Try it somewhere you can recover your model in case of fail-safe. Fail-safe behavior is unknown at the current stage of development.
* Use the INAV Discord for help and setup questions; use the Github Issues for reporting bugs and unexpected behaviors. For reporting on Github, a CLI `diff all`, a DVR and a Blackbox log of the incident will assist investigation.
