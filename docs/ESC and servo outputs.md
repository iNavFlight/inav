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

This is a convenience feature. Changing the motor direction in the ESC configuration, or swapping two motor wires, is the preferred and more robust way.

A motor that turns the wrong way can be reversed from INAV instead of from the ESC configurator, as long as the ESC runs DShot and understands the spin direction commands (DShot commands 20 and 21 - the same ones turtle mode relies on; BLHeli_32, AM32, Bluejay and BLHeli_S do).

`dshot_reversed_motors` is a bitmask: bit 0 is motor 1, bit 1 is motor 2, and so on. A set bit tells that ESC to spin opposite to the direction stored in the ESC. `set dshot_reversed_motors = 5` reverses motors 1 and 3.

The ESC does not store the command, and it only accepts it once it has started up and armed itself at zero throttle. INAV therefore sends the directions on every arm and, while any bit is set, every two seconds while disarmed and every 250 ms while armed with all motors stopped. Each time, motor output is held for about 20 ms. Plugging in the battery after configuring over USB is covered once the ESC has finished starting up and the next refresh has run, and on arm if the ESC has finished starting up; with motor stop the armed refresh also catches an ESC that arms later. An ESC that resets in flight only re-arms on DShot 0 (motor stop); until the next refresh reaches it, within about 300 ms, it spins in its stored direction. Without motor stop it does not re-arm in flight.

Some ESCs (Bluejay, BLHeli_S) keep a received direction until they lose power, also through a flight controller reboot. INAV sends the configured directions once after boot and on every arm, even when no bit is set (not with reversible motors).

The motor direction depends on the flight controller configuration: after `defaults` or any other settings reset, check the directions in the Motors tab (props off).

Turtle mode inverts every motor relative to its configured direction and restores the configured directions on disarm.

Leave `dshot_reversed_motors` at 0 with 3D / reversible motors and set the direction in the ESC: AM32 ignores the spin direction commands in 3D mode.

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
