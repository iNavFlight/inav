# MixerProfile
A MixerProfile is a set of motor mixer,servomixer,platform type configuration settings.

Currently Two profiles are supported expect f411 and f722. The default profile is profile `1`.

Switching between profiles requires reboot to take affect by default. But When the conditions are met, It allows inflight profile switching to allow things like vtol build.

# Setup for vtol
- Need to keep motor/servo pwm mapping consistent among profiles
- FW and MC have different way to map the pwm mapping, we need to change `timerHardware`` for to make it have same mapping.
- A vtol specific fc target is required. There might more official vtol fc target in the future
- Set mixer_profile, pid_profile separately, and set RC mode to switch them
## FC target
To keep motor/servo pwm mapping consistent and enable hot switching. Here is MATEKF405TE_SD board (folder name `MATEKF405TE`) used for vtol as a example.

The target name for vtol build is MATEKF405TE_SD_VTOL, target folder name is MATEKF405TE.
when it is done, build your target and flash it to your fc,.

### CMakeLists.txt
```c
target_stm32f405xg(MATEKF405TE)
target_stm32f405xg(MATEKF405TE_SD)
target_stm32f405xg(MATEKF405TE_SD_VTOL) //new target added
```
### target.c
It is **important** to map all the serial output to `TIM_USE_VTOL_MOTOR` or `TIM_USE_VTOL_SERVO` to ensure same mapping among MC mapping and FW mapping.
```c
timerHardware_t timerHardware[] = {
#ifdef MATEKF405TE_SD_VTOL // Vtol target specific mapping start from there 
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
#else // Vtol target specific mapping end there 
    //Non votl target start from here
    .........omitted
#endif
    DEF_TIM(TIM3,  CH4,  PB1,  TIM_USE_LED,    0, 0), // 2812LED  D(1,2,5)
    DEF_TIM(TIM11, CH1,  PB9,  TIM_USE_BEEPER, 0, 0), // BEEPER PWM
    
    DEF_TIM(TIM9,  CH2,  PA3,  TIM_USE_PPM,    0, 0), //RX2
    DEF_TIM(TIM5,  CH3,  PA2,  TIM_USE_ANY,    0, 0), //TX2  softserial1_Tx
};
```
### target.h

Then define `ENABLE_MIXER_PROFILE_MCFW_HOTSWAP` to enable mixer_profile hot switching once you have set the pwm mapping
```c
#ifdef MATEKF405TE_SD_VTOL
#define ENABLE_MIXER_PROFILE_MCFW_HOTSWAP //Enable hot swap
#define MATEKF405TE_SD //Define the original target name keep its original configuration such as USBD_PRODUCT_STRING
#endif
```

## Configuration
### Profile switch
Setup the FW mode and MC mode separately in two different mixer profile,

I will use FW mode as mixer_profile 1, and MC as mixer_profile 2 as example.
At current state, inav-configurator do not support mixer_profile, some of the settings have to be done in cli

`set mixer_pid_profile_linking = ON` to enable pid_profile auto handling. It will change the pid_profile index according to mixer_profile index on fc boot and mixer_profile hot switching(recommended)
```
mixer_profile 1 #switch to mixer_profile by cli

set platform_type = AIRPLANE
set model_preview_type = 26
set motorstop_on_low = ON # motor stop feature have been moved to here
set mixer_pid_profile_linking = ON # enable pid_profile auto handling(recommended).
save
```
Then finish the airplane setting on mixer_profile 1

```
mixer_profile 2

set platform_type = TRICOPTER
set model_preview_type = 1
set mixer_pid_profile_linking = ON # also enable pid_profile auto handling
save
```
Then finish the multirotor setting on mixer_profile 2. 

You can use `MAX` servo input to set a fixed input for the tilting servo. speed setting for `MAX` input is available in cli.

It is recommended to have some amount of control surface (elevon/elevator) mapped for stabilization even in MC mode to get improved authority when airspeed is high.  

**Double check all settings in cli by `diff all` command**, make sure you have correct settings. And see what will change with mixer_profile. For example servo output min/max range will not change. But `smix` and `mmix` will change. 

### Mixer Transition input
Normally 'transition input' will be useful in MC mode to gain airspeed. 
Servo mixer and motor mixer can accept transition mode as input.  
It will move accordingly when transition mode is activated. 

The use of Transition mode is recommended to enable further features/developments like failsafe support. Map motor to servo output, or servo with logic condition is not recommended 
#### servo
38 is the input source for transition input, use this to tilt motor to gain airspeed.

example:Add servo 1 output by +45 in speed of 150 when transition mode is activate for tilted motor setup
```
# rule no; servo index; input source; rate; speed; activate logic function number
smix 6 1 38 45 150 -1
```

#### motor
the default mmix throttle value is 0.0, It will not show in diff command when throttle value is 0.0(unused), which motor will stop.
- 0.0<throttle<=1.0 : normal mapping
- -1.0<throttle<=0.0 : motor stop, default value 0
- -2.0<throttle<-1.0 : spin regardless of the radio\`s throttle position at speed `abs(throttle)-1` when Mixer Transition is activated

example:It will spin motor number 5(count from 1) at 20% in transition mode only to gain speed for 4 rotor 1 pusher setup
```
# motor number; throttle; roll; pitch; yaw
mmix 4 -1.200  0.000  0.000  0.000
```

### RC mode settings
It currently support take a rc input to activate modes or switch profiles
Mixer_profile 1 will be used as default, mixer_profile 2 will be used when `MIXER PROFILE 2` box is activated, Once successfully set, You can see profiles/model preview/etc will switch accordingly when you change inav-configurator tabs, it will make the setting easier.

Set `MIXER TRANSITION` accordingly when you want to use `MIXER TRANSITION` input for motors and servos, Here is sample of 
![Alt text](Screenshots/mixer_profile.png)
|  1000~1300 |  1300~1700  | 1700~2000 |
| :-- | :-- | :-- |
| FW(profile1) with transition off |  MC(profile2) with transition on  | MC(profile2) with transition off |

It is also possible to set it as 4 state switch by adding FW(profile1) with transition on
## Happy flying
Test every thing on bench first, Then try MC or FW one by one to see if there is any problems.
Try it some where you can recover your model in case of failsafe.
Failsafe behavior is unknown at current stage. 