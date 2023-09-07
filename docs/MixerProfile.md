# MixerProfile

A MixerProfile is a set of motor mixer, servo-mixer and platform type configuration settings to enable VTOL transitions.

Currently two profiles are supported on targets other than F411 and F722 (due to resource constraints on these FC). i.e VTOL transition is not available on  F411 and F722 targets.

By default, switching between profiles requires reboot to take affect. However, when all conditions are met, and a suitable [configuration](#configuration) has been applied, `mixer_profile` also  allows in-flight profile [switching](#rc-mode-settings) to allow things like VTOL operation. This is the recommended operating mode.

Please note that this is an emerging / experimental capability that will require some effort by the pilot to implement.

## Setup for VTOL

- For mixer profile switching it is necessary to keep motor and servo PWM mapping consistent between Fixed-Wing (FW) and Multi-rotor (MR) profiles
- Traditionally, FW and MR have had different enumerations to define the PWM mappings. For VTOL operation it is necessary to change the source code  `timerHardware` table to allow a consistent enumeration and thus mapping between MR and FW modes.
- For this reason, a **VTOL specific FC target is required**. This means that the pilot must build a custom target. In future there may be official VTOL FC targets.
- In operation, it is necessary to set the `mixer_profile` and the `pid_profile` separately and to set a [RC mode](#rc-mode-settings) to switch between them.

## FC target

In order to keep motor and servo PWM mapping consistent and enable hot switching, the steps below are required.

The following sections use a MATEKF405TE\_SD board (folder name `MATEKF405TE`) configured for VTOL as a example.

The target name for VTOL build is `MATEKF405TE_SD_VTOL`, the standard target folder name is `MATEKF405TE`.

### CMakeLists.text modifications

#### Adding the VTOL target

Add the VTOL target definition to `CMakeLists.txt`, i.e. the third line below.

```c
target_stm32f405xg(MATEKF405TE)
target_stm32f405xg(MATEKF405TE_SD)
target_stm32f405xg(MATEKF405TE_SD_VTOL) //new target added
```
### target.c modifications

Two new enumerations are available to define the motors and servos used for VTOL.

It is **important** to map all the PWM outputs to `TIM_USE_VTOL_MOTOR` or `TIM_USE_VTOL_SERVO` to ensure consistency between the MR mapping and FW mapping.

For example, add the new section, encapsulated below by the `#ifdef MATEKF405TE_SD_VTOL` ... `else` section :

```c
timerHardware_t timerHardware[] = {
#ifdef MATEKF405TE_SD_VTOL
    // VTOL target specific mapping start from there
    DEF_TIM(TIM8,  CH4,  PC9,  TIM_USE_VTOL_MOTOR,   0, 0), // S1 for motor
    DEF_TIM(TIM8,  CH3,  PC8,  TIM_USE_VTOL_MOTOR,   0, 0), // S2 for motor
    DEF_TIM(TIM1,  CH3N, PB15, TIM_USE_VTOL_MOTOR,   0, 0), // S3 for motor
    DEF_TIM(TIM1,  CH1,  PA8,  TIM_USE_VTOL_MOTOR,   0, 1), // S4 for motor

    DEF_TIM(TIM2,  CH4,  PB11, TIM_USE_VTOL_SERVO,   0, 0), // S5 for servo
    DEF_TIM(TIM2,  CH3,  PB10, TIM_USE_VTOL_SERVO,   0, 0), // S6 for servo
    DEF_TIM(TIM2,  CH2,  PB3,  TIM_USE_VTOL_SERVO,   0, 0), // S7 for servo
    DEF_TIM(TIM2,  CH1,  PA15, TIM_USE_VTOL_SERVO,   0, 0), // S8 for servo

    DEF_TIM(TIM12, CH1,  PB14, TIM_USE_VTOL_SERVO,   0, 0), // S9  for servo
    DEF_TIM(TIM13, CH1,  PA6,  TIM_USE_VTOL_SERVO,   0, 0), // S10 for servo
    DEF_TIM(TIM4,  CH1,  PB6,  TIM_USE_VTOL_MOTOR,   0, 0), // S11 for motor
	// VTOL target specific mapping ends here
#else 
    // Non VOTL target start from here
	// .........omitted for brevity
#endif
    DEF_TIM(TIM3,  CH4,  PB1,  TIM_USE_LED,    0, 0), // 2812LED  D(1,2,5)
    DEF_TIM(TIM11, CH1,  PB9,  TIM_USE_BEEPER, 0, 0), // BEEPER PWM

    DEF_TIM(TIM9,  CH2,  PA3,  TIM_USE_PPM,    0, 0), //RX2
    DEF_TIM(TIM5,  CH3,  PA2,  TIM_USE_ANY,    0, 0), //TX2  softserial1_Tx
};
```

Note that using the VTOL enumerations does not affect the normal INAV requirement on the use of discrete timers for motors and servos.

### target.h modification

In `target.h`, define `ENABLE_MIXER_PROFILE_MCFW_HOTSWAP` to enable `mixer_profile` hot switching once you have set the `timer.c` PWM mapping:

```c
#ifdef MATEKF405TE_SD_VTOL
#define ENABLE_MIXER_PROFILE_MCFW_HOTSWAP //Enable hot swap
#define MATEKF405TE_SD //Define the original target name keep its original configuration such as USBD_PRODUCT_STRING
#endif
```

Once the target is built, it can be flashed to the FC.

## Configuration

### Profile Switch

Setup the FW mode and MR mode separately in two different mixer profiles:

In this example, FW mode is `mixer_profile` 1 and MR mode is `mixer_profile` 2.

Currently, the INAV Configurator does not support `mixer_profile`, so some of the settings have to be done in CLI.

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

Set `MIXER TRANSITION` accordingly when you want to use `MIXER TRANSITION` input for motors and servos, Here is sample of using the `MIXER TRANSITION` mode:

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


## Happy flying

Remember that this is currently an emerging capability:

* Test every thing on bench first.
* Then try MR or FW mode separately see if there are any problems.
* Try it somewhere you can recover your model in case of fail-safe. Fail-safe behavior is unknown at the current stage of development.
* Use the INAV Discord for help and setup questions; use the Github Issues for reporting bugs and unexpected behaviors. For reporting on Github, a CLI `diff all`, a DVR and a Blackbox log of the incident will assist investigation.
