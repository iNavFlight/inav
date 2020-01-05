# Mixer and platform type

Mixing rules determine how servos and motors react to user and FC inputs. INAV supports various preset mixer configurations as well as custom mixing rules.

## Configuration

The mixer can be configured through the `Mixer` tab of the graphical user interface or using the CLI commands `mmix` and `smix`. `mmix` to define motor mixing rules and `smix` to define servo mixing rules.

To use a mixer preset first select the platform type then the mixer preset matching your aircraft and either press the `Load and apply` or `Load mixer` buttons. The `Load and apply` button will load the mixer, save it and ask to reboot the flight controller. The `Load mixer` button only loads the preset mixing rules, you can then edit them to suit your needs and when you are done you need to press the `Save and Reboot` button to save the rules.

Watch [this video](https://www.youtube.com/watch?v=0cLFu-5syi0) for a detailed description of the GUI and the documentation bellow for more details.

## Platform type

The platform type determines what features will be available to match the type of aircraft: available flight modes, flight modes behaviour, availability of flaps and displayed types of mixer presets. It can be set through the GUI's `Mixer tab` or through the CLI's `platform_type` setting.

Currently, following platform types are supported:

* MULTIROTOR
* AIRPLANE
* TRICOPTER

## Writing custom mixing rules

## Motor Mixing

A motor mixing rule is needed for each motor. Each rule defines weights that determine how the motor it applies to will change its speed relative to the requested throttle and flight dynamics: roll rate, pitch rate and yaw rate. The heigher a weight the more the input will have an impact on the speed of the motor. Refer to the following table for the meaning of each weight.

| Weight | Definition |
| ---------------------- | ---------- |
| THROTTLE    | Speed of the motor relative to throttle. Range [0.0, 1.0]. A motor with a weight of 0.5 will receive a command that will half of a motor with a 1.0 weight |
| ROLL    | Indicates how much roll authority this motor imparts to the roll rate of the aircraft. Range [-1.0, 1.0]. For fixed wing models this is usually set to 0. A positive value means that the motor needs to accelerate for a positive roll rate request (rolling right). A negative value means that the motor needs to decelerate. |
| PITCH    | Indicates how much pitch authority this motor imparts to the pitch rate of the aircraft. Range [-1.0, 1.0]. For fixed wing models this is usually set to 0. A positive value means that the motor needs to accelerate for a positive pitch rate request (pitching down). A negative value means that the motor needs to decelerate. |
| YAW    | Indicates how much yaw authority this motor imparts to the yaw rate of the aircraft. Range [-1.0, 1.0]. For fixed wing models with more than one motor this weight can be used to setup differential thrust. For fixed wing models with only one motor this is usually set to 0. A positive value means that the motor needs to accelerate for a positive yaw rate request (clockwise yaw seen from the top of the model). A negative value means that the motor needs to decelerate |

CLI commands to configure motor mixing rules:

The `mmix reset` command removes all the existing motor mixing rules.

The `mmix` command is used to list, create or modify rules. To list the currently defined rules run the `mmix` command without parameters.

To create or modify rules use the `mmix` command with the following syntax: `mmix <n> <throttle> <roll> <pitch> <yaw>`. `<n>` is representing the index of the motor output pin (integer). The other parameters are decimal weights for each of the inputs. To disable a mixing rule set the `throttle` weight to 0.

## Servo Mixing

At least one servo mixing rule is needed for each servo. Each rule defines how a servo will move relative to a specific input like a RC channel, or a requested flight dynamics rate or position from the flight controller.

Each servo mixing rule has the following parameters:
* Servo index: defines which servo the rule will apply to. The absolute value of the index is not important, what matters is only the relative difference between the used indexes. The rule with the smaller servo index will apply to the first servo, the next higher servo index to the second servo, etc. More than one rule can use the same servo index. The output of the rules with the same servo index are added together to give the final output for the specified servo.
* Input: the input for the mixing rule, see a summary of the input types table bellow.
* Weight: percentage of the input to forward to the servo. Range [-1000, 1000]. Mixing rule output = input * weight. If the output of a set of mixing rules is lower/higher than the defined servo min/max the output is clipped (the servo will never travel farther than the set min/max).
* Speed: maximum rate of change of the mixing rule output. Used to limit the servo speed. 1 corresponds to maximum 10µs/s output rate of change. Set to 0 for no speed limit. For example: 10 = full sweep (1000 to 2000) in 10s, 100 = full sweep in 1s.

| CLI input ID | Mixer input | Description |
|----|--------------------------|------------------------------------------------------------------------------|
| 0  | Stabilised ROLL          | Roll command from the flight controller. Depends on the selected flight mode(s) |
| 1  | Stabilised PITCH         | Pitch command from the flight controller. Depends on the selected flight mode(s) |
| 2  | Stabilised YAW           | Yaw command from the flight controller. Depends on the selected flight mode(s) |
| 3  | Stabilised THROTTLE      | Throttle command from the flight controller. Depends on the selected flight mode(s) |
| 4  | RC ROLL                  | Raw roll RC channel |
| 5  | RC PITCH                 | Raw pitch RC channel |
| 6  | RC YAW                   | Raw yaw RC channel |
| 7  | RC THROTTLE              | Raw throttle RC channel |
| 8  | RC channel 5             | Raw RC channel 5 |
| 9  | RC channel 6             | Raw RC channel 6 |
| 10 | RC channel 7             | Raw RC channel 7 |
| 11 | RC channel 8             | Raw RC channel 8 |
| 12 | GIMBAL PITCH             | Scaled pitch attitude of the aircraft [-90°, 90°] => [-500, 500] |
| 13 | GIMBAL ROLL              | Scaled roll attitude of the aircraft [-180°, 180°] => [-500, 500] |
| 14 | FEATURE FLAPS            | This input value is equal to the `flaperon_throw_offset` setting when the `FLAPERON` flight mode is enabled, 0 otherwise |
| 15 | RC channel 9             | Raw RC channel 9 |
| 16 | RC channel 10            | Raw RC channel 10 |
| 17 | RC channel 11            | Raw RC channel 11 |
| 18 | RC channel 12            | Raw RC channel 12 |
| 19 | RC channel 13            | Raw RC channel 13 |
| 20 | RC channel 14            | Raw RC channel 14 |
| 21 | RC channel 15            | Raw RC channel 15 |
| 22 | RC channel 16            | Raw RC channel 16 |
| 23 | Stabilized ROLL+         | Clipped between 0 and 1000 |       
| 24 | Stabilized ROLL-         | Clipped between -1000 and 0 |
| 25 | Stabilized PITCH+        | Clipped between 0 and 1000 |
| 26 | Stabilized PITCH-        | Clipped between -1000 and 0 |
| 27 | Stabilized YAW+          | Clipped between 0 and 1000 |
| 28 | Stabilized YAW-          | Clipped between -1000 and 0 |
| 29 | One                      | Constant value of 500 |

The `smix reset` command removes all the existing motor mixing rules.

The `smix` command is used to list, create or modify rules. To list the currently defined rules run the `smix` command without parameters.

To create or modify rules use the `smix` command with the following syntax: `smix <n> <servo_index> <input_id> <weight> <speed> <logic_condition_id>`. `<n>` is representing the index of the servo mixing rule to create or modify (integer). To disable a mixing rule set the weight to 0.

`logic_condition_id` default value is `-1` for rules that should be always executed. 

### Logic Conditions

[Logic Conditions](Logic%20Conditions.md) allows to activate/deactivate `smix` rules based on user input and flight parameters. If Logic Condition evaluates as `false`, smix rule connected with with LC will not be active and used inside the Mixer. 

This mechanism allows to move servos when desired conditions are met. For example, if an airplane is equipped with a pitot tube and flaps, flaps can be automatically deployed when airspeed goes below a threshold.

Other usages can be:

* automatic parachute deployment
* VTOL and especially tail-sitters that require change in mixings during flight mode transition
* crowbar airbrakes
* any kind of servo mixings that should be changed during flight