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

### Bidirectional DSHOT

With `dshot_bidir_enabled = ON` the ESC answers every DSHOT frame with its eRPM on the motor
wire (and, with `dshot_edt_enabled`, temperature, voltage and current), which feeds the RPM
filter and the ESC telemetry without a separate ESC telemetry UART. Set `motor_poles` to the
pole count of the motors so the eRPM is converted to RPM. The ESC firmware must support
bidirectional DSHOT (BLHeli_32, Bluejay, AM32).

On STM32 the reply is captured by the motor's own timer channel, so every motor output needs
its timer channel's DMA stream. On AT32 the motor pins are "bitbanged" instead, as in
Betaflight: a spare channel of TMR8 or TMR1 paces DMA transfers that drive and read the
whole GPIO port, one per port with motors on it. That timer must be unused by anything else
(LED strip, servos, beeper). If neither TMR8 nor TMR1 is free, bidirectional DSHOT is switched
off for that boot: the motors run plain DSHOT on their timers, there is no eRPM telemetry or
RPM filter, and the boot log reports "Bitbang DSHOT unavailable". Without
`dshot_bidir_enabled` DSHOT on AT32 stays on the timers as before.

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

## ESCs that are not driven from an output pin

A Spektrum Smart ESC is connected to a UART rather than to a motor output, because
its protocol is serial rather than a timer waveform. None of the output mapping
above applies to it. See [Spektrum Smart ESC](Spektrum%20Smart%20ESC.md).
