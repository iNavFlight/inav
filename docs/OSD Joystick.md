# OSD joystick

LED pin can be used to emulate 5key OSD joystick for OSD camera pin, while still driving ws2812 LEDs (shared functionality).

Note that for cameras which support RuncamDevice protocol, there is alternative functionality using serial communication: Runcam device.md

# OSD Joystick schematics

* TODO * 

Camera internal resistance seems to be 47kOhm or 9kOhm depending on camera model.

Each key effectively turns on voltage divider. Voltage is sensed by the camera and is compared to the list of keys voltages with some threshold.

Key voltage has to be held for at least 200ms.

To simulate 5key joystick, it is sufficient to generate correct voltage on camera OSD pin.

# Enabling OSD Joystick emulation

set led_pin_pwm_mode = shared_high
set osd_joystick_enabled = on

# Connection diagram

We use LED pin PWM functionality with RC filter to generate voltage:

*schematics TODO - shows FC LED pin, ws2812 strip, RC filter, camera OSD pin*

470Ohm, 10uF


# Example PCB layout (SMD components)

RC Filter can be soldered on small piece of PCB:


# Configutring keys voltages

If default voltages does not work with your camera model, then you have to measure voltages and find out corresponding PWM duty ratios.

1. Connect 5keys joystick to camera.
2. Measure voltages on OSD pin while each key is pressed.
3. Connect camera to FC throught RC filter as shown on schematix above.
4. Enable OSD Joystick emulation (see "Enabling OSD Joystick emulation" above)
4. Use cli command led_pin_pwm <value>, value = 0...100 to find out PWM values for each voltage.
5. Specify PWM values in configuration and save:

set osd_joystick_down=0
set osd_joystick_up=48
set osd_joystick_left=63
set osd_joystick_right=28
set osd_joystick_enter=75

# Entering OSD Joystick emulation mode

OSD Joystick emulation mode is enabled using the following stick combination:
RIGHT CENTER

Mode is exited using stick combination:
LEFT CENTER

*Note that the same stick combination is used to enable 5keys joystick emulation with RuncamDevice protocol.*

Emulation can be enabled in unarmed state only. 

When you press key combination, correct voltage should be generated on OSD pin.
