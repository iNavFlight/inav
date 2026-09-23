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

Check the ESC documentation for the list of protocols that are supported.

## Motor direction with DShot

A motor that turns the wrong way can be reversed from INAV instead of from the ESC configurator, as long as the ESC runs DShot and understands the spin direction commands (DShot commands 20 and 21 - the same ones turtle mode relies on; BLHeli_32, AM32 and Bluejay do).

`dshot_reversed_motors` is a bitmask: bit 0 is motor 1, bit 1 is motor 2, and so on. A set bit tells that ESC to spin opposite to the direction stored in the ESC. `set dshot_reversed_motors = 5` reverses motors 1 and 3.

The ESC does not store the command, so INAV sends it again on every arm, as soon as the setting changes and every two seconds while disarmed. Plugging in the battery after configuring over USB is covered by that; an ESC that resets in flight falls back to its stored direction, but it will not re-arm while the motor is being driven anyway.

Turtle mode inverts every motor relative to its configured direction and restores the configured directions on disarm.

The setting only works with DShot. With any other protocol it is ignored, and the direction has to be changed in the ESC or by swapping two motor wires.

## Servo outputs

By default, INAV uses 50Hz servo update rate. If you want to increase it, make sure that servos support
higher update rates. Only high end digital servos are capable of handling 200Hz and above!

## Servo output mapping

Not all outputs on a flight controller can be used for servo outputs. It is a hardware thing. Always check flight controller documentation. 

While motors are usually ordered sequentially, here is no standard output layout for servos! Some boards might not be supporting servos in _Multirotor_ configuration at all!

## Modifying output mapping

INAV 7 introduced extra functionality that let you force only some outputs to be either *MOTORS* or *SERVOS*, with some restrictions dictated by the hardware.

The main restrictions is that outputs are associated with timers, which can be shared between multiple outputs and  two outputs on the same timer need to have the same function.

The easiest way to modify outputs, is to use the Mixer tab in the Configurator, as it will clearly show you which timer is used by all outputs, but you can also use `timer_output_mode` on the cli.
