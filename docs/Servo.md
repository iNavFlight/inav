# Servo configuration

Servos can be configured from the graphical user interface's `Servos` tab.

* MID: middle/neutral point of the servo
* MIN: the minimum value that can be sent to the servo is MIN * Rate
* MAX: the maximum value that can be sent to the servo is MAX * Rate
* Rate: servo command = servo rate * mixer output
* Reverse: if enabled the servo output is reversed

CLI commands to configure servos:

The `servo` command is used to list or modify servo's configuration. To list the current servo's configuration run the `servo` command without parameters.

To change the configuration of a servo use the `servo` command with the following syntax: `servo <n> <min> <max> <mid> <rate>`. `<n>` is representing the index of the servo output defined by a servo mixer (See (mixer documentation)[https://github.com/iNavFlight/inav/blob/master/docs/Mixer.md]). The other parameters must be positive integers apart from the rate wich valid range is [-125, 125].

## Servo filtering

A low-pass filter can be enabled for the servos.  It may be useful for avoiding structural modes in the airframe, for example.

### Configuration

Currently, it can only be configured via the CLI:

Use `set servo_lpf_hz=20` to enable filtering. This will set servo low pass filter to 20Hz.

### Tuning

One method for tuning the filter cutoff is as follows:

1. Ensure your vehicle can move at least somewhat freely in the troublesome axis.  For example, if you are having yaw oscillations on a tricopter, ensure that the copter is supported in a way that allows it to rotate left and right to at least some degree.  Suspension near the CG is ideal.  Alternatively, you can just fly the vehicle and trigger the problematic condition you are trying to eliminate, although tuning will be more tedious.

2. Tap the vehicle at its end in the axis under evaluation.  Directly commanding the servo in question to move may also be used. In the tricopter example, tap the end of the tail boom from the side, or command a yaw using your transmitter.

3. If your vehicle oscillates for several seconds or even continues oscillating indefinitely, then the filter cutoff frequency should be reduced. Reduce the value of `servo_lowpass_freq` by half its current value and repeat the previous step.

4. If the oscillations are dampened within roughly a second or are no longer present, then you are done.  Be sure to run `save`.
