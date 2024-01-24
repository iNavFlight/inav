# ESC and servo outputs

## ESC protocols

INAV support the following ESC protocols:

* "standard" PWM with 50-400Hz update rate
* OneShot125
* OneShot42
* Multishot
* Brushed motors
* DSHOT150, DSHOT300, DSHOT600

ESC protocol can be selected in Configurator. No special configuration is required.

Check ESC documentation of the list of protocols that it is supporting.

## Servo outputs

By default, INAV uses 50Hz servo update rate. If you want to increase it, make sure that servos support
higher update rates. Only high end digital servos are capable of handling 200Hz and above!

## Servo output mapping

Not all outputs on a flight controller can be used for servo outputs. It is a hardware thing. Always check flight controller documentation. 

While motors are usually ordered sequentially, here is no standard output layout for servos! Some boards might not be supporting servos in _Multirotor_ configuration at all!

## Modifying output mapping

### Modifying all outputs at the same time

Since INAV 5, it has been possible to force *ALL* outputs to be `MOTORS` or `SERVOS`.

Traditional ESCs usually can be controlled via a servo output, but would require calibration.

This can be done with the `output_mode` CLI setting. Allowed values:

* `AUTO` assigns outputs according to the default mapping
* `SERVOS` assigns all outputs to servos
* `MOTORS` assigns all outputs to motors

### Modifying only some outputs

INAV 7 introduced extra functionality that let you force only some outputs to be either *MOTORS* or *SERVOS*, with some restrictions dictated by the hardware.

The mains restrictions is that outputs need to be associated with timers, which are usually shared between multiple outputs. Two outputs on the same timer need to have the same function.

The easiest way to modify outputs, is to use the Mixer tab in the Configurator, as it will clearly show you which timer is used by all outputs, but you can also use `timer_output_mode` on the cli.
This can be used in conjunction to the previous method, in that cass all outputs will follow `output_mode` and `timer_output_mode` overrides are applied after that.
