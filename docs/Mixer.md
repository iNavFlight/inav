# Mixer and platform type

INAV supports a number of mixing configurations as well as custom mixing.  Mixer configurations determine how the servos and motors work together to control the aircraft.

## Configuration

INAV Configurator provides graphical user interface for mixer configuration. All supported vehicle types are configurable with _mixer presets_ using Configurator. `mmix` and `smix` manual configuration in CLI should be used only for backup/restore purposes. 

User interface is described in [this video](https://www.youtube.com/watch?v=0cLFu-5syi0)

## Platform type

INAV can be used on a variety of vehicle types configured via Configurator or `platform_type` CLI property. Certain settings applies only when specific platform type is selected. For example, _flaps_ can be configured only if **AIRPLANE** platform type is used. The same goes for flight modes, output mappings, stabilization algorithms, etc. 

Currently, following platform types are supported:

* MULTIROTOR
* AIRPLANE
* TRICOPTER

## Motor Mixing

Custom motor mixing allows for completely customized motor configurations. Each motor must be defined with a custom mixing table for that motor. The mix must reflect how close each motor is with reference to the CG (Center of Gravity) of the flight controller. A motor closer to the CG of the flight controller will need to travel less distance than a motor further away.  

Steps to configure custom mixer in the CLI:

1. Use `mmix reset` to erase any existing custom mixing.
1. Issue a `mmix` statement for each motor.

The mmix statement has the following syntax: `mmix n THROTTLE ROLL PITCH YAW`

| Mixing table parameter | Definition |
| ---------------------- | ---------- |
| n    | Motor ordering number |
| THROTTLE    | All motors that are used in this configuration are set to 1.0. Unused set to 0.0. |
| ROLL    | Indicates how much roll authority this motor imparts to the roll of the flight controller. Accepts values nominally from 1.0 to -1.0. |
| PITCH    | Indicates the pitch authority this motor has over the flight controller. Also accepts values nominally from 1.0 to -1.0. |
| YAW    | Indicates the direction of the motor rotation in a relationship with the flight controller. 1.0 = CCW -1.0 = CW. |

Note: the `mmix` command may show a motor mix that is not active, custom motor mixes are only active for models that use custom mixers.

## Servo Mixing

Custom servo mixing rules can be applied to each servo.  Rules are applied in the CLI using `smix`. Rules link flight controller stabilization and receiver signals to physical PWM output pins on the FC board. Currently, pin id's 0 and 1 can only be used for motor outputs. Other pins may or may not work depending on the board you are using.

The smix statement has the following syntax: `smix n SERVO_ID SIGNAL_SOURCE RATE SPEED` 
For example, `smix 0 2 0 100 0` will create rule number 0 assigning Stabilised Roll to the third PWM pin on the FC board will full rate and no speed limit.

| id | Flight Controller Output signal sources |
|----|-----------------|
| 0  | Stabilised ROLL |
| 1  | Stabilised PITCH |
| 2  | Stabilised YAW |
| 3  | Stabilised THROTTLE |
| 4  | RC ROLL |
| 5  | RC PITCH |
| 6  | RC YAW |
| 7  | RC THROTTLE |
| 8  | RC AUX 1 |
| 9  | RC AUX 2 |
| 10 | RC AUX 3 |
| 11 | RC AUX 4 |
| 12 | GIMBAL PITCH |
| 13 | GIMBAL ROLL |
| 14 | FEATURE FLAPS |

| id |  Servo Slot Optional Setup |
|----|--------------|
| 0  | GIMBAL PITCH |
| 1  | GIMBAL ROLL |
| 2  | ELEVATOR / SINGLECOPTER_4 |
| 3  | FLAPPERON 1 (LEFT) / SINGLECOPTER_1 |
| 4  | FLAPPERON 2 (RIGHT) / BICOPTER_LEFT / DUALCOPTER_LEFT / SINGLECOPTER_2 |
| 5  | RUDDER / BICOPTER_RIGHT / DUALCOPTER_RIGHT / SINGLECOPTER_3 |
| 6  | THROTTLE (Based ONLY on the first motor output) |
| 7  | FLAPS |

### Servo rule rate

Servo rule rate should be understood as a weight of a rule. To obtain full servo throw without clipping sum of all `smix` rates for a servo should equal `100`. For example, is servo #2 should be driven by sources 0 and 1 (Stabilized Roll and Stabilized Pitch) with equal strength, correct rules would be:

```
smix 0 2 0 50 0
smix 1 2 1 50 0
```  

To obtain the stronger input of one source, increase the rate of this source while decreasing the others. For example, to drive servo #2 in 75% from source 0 and in 25% from source 1, correct rules would be:

```
smix 0 2 0 75 0
smix 1 2 1 25 0
```  

If a sum of weights would be bigger than `100`, clipping to servo min and max values might appear.

> Note: the `smix` command may show a servo mix that is not active, custom servo mixes are only active for models that use custom mixers.

### Servo speed

Custom servo mixer allows defining the speed of change for given servo rule. By default, all speeds are set to `0`, that means limiting is _NOT_ applied and rules source is directly written to a servo. That mean, if, for example, source (AUX) changes from 1000 to 2000 in one cycle, servo output will also change from 1000 to 2000 in one cycle. In this case, speed is limited only by the servo itself.

If value different than `0` is set as rule speed, the speed of change will be lowered accordingly. 

`1 speed = 10 us/s`

**Example speed values**
* 0 = no limiting
* 1 = 10us/s -> full servo sweep (from 1000 to 2000) is performed in 100s 
* 10 = 100us/s -> full sweep (from 1000 to 2000)  is performed in 10s
* 100 = 1000us/s -> full sweep in 1s
* 200 = 2000us/s -> full sweep in 0.5s 

Servo speed might be useful for functions like flaps, landing gear retraction and other where full speed provided for hardware is too much.

## Servo Reversing

Servos can be reversed using Configurator _Servo_ tab and _Reverse_ checkbox.

## Servo configuration

The cli `servo` command defines the settings for the servo outputs.
The cli mixer `smix` command controllers how the mixer maps internal FC data (RC input, PID stabilization output, channel forwarding, etc) to servo outputs.

## Servo filtering

A low-pass filter can be enabled for the servos.  It may be useful for avoiding structural modes in the airframe, for example.  

### Configuration

Currently, it can only be configured via the CLI:

Use `set servo_lpf_hz=20` to enable filtering. This will set servo low pass filter to 20Hz.

### Tuning

One method for tuning the filter cutoff is as follows:

1. Ensure your vehicle can move at least somewhat freely in the troublesome axis.  For example, if you are having yaw oscillations on a tricopter, ensure that the copter is supported in a way that allows it to rotate left and right to at least some degree.  Suspension near the CG is ideal.  Alternatively, you can just fly the vehicle and trigger the problematic condition you are trying to eliminate, although tuning will be more tedious.

2. Tap the vehicle at its end in the axis under evaluation.  Directly commanding the servo in question to move may also be used.  In the tricopter example, tap the end of the tail boom from the side, or command a yaw using your transmitter.

3. If your vehicle oscillates for several seconds or even continues oscillating indefinitely, then the filter cutoff frequency should be reduced. Reduce the value of `servo_lowpass_freq` by half its current value and repeat the previous step.

4. If the oscillations are dampened within roughly a second or are no longer present, then you are done.  Be sure to run `save`.
