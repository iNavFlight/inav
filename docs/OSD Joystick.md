# OSD joystick

LED pin can be used to emulate 5key OSD joystick for OSD camera pin, while still driving ws2812 LEDs (shared functionality).

See [LED pin PWM](LED%20pin%20PWM.md) for more details.

Note that for cameras which support RuncamDevice protocol, there is alternative functionality using serial communication: [Runcam device](Runcam%20device.md)

Also special adapters exist to convert RuncamDevice protocol to OSD Joystick: [Runcam control adapter](https://www.runcam.com/download/runcam_control_adapter_manual.pdf)

# OSD Joystick schematics

![alt text](/docs/assets/images/osd_joystick_keys.png  "osd jystick keys")

Camera internal resistance seems to be 47kOhm or 9kOhm depending on camera model.

Each key effectively turns on voltage divider. Voltage is sensed by the camera and is compared to the list of keys voltages with some threshold.

Key voltage has to be held for at least 200ms.

To simulate 5key joystick, it is sufficient to generate correct voltage on camera OSD pin.

# Enabling OSD Joystick emulation

```set led_pin_pwm_mode=shared_high```

```set osd_joystick_enabled=on```

Also enable "Multi-color RGB LED Strip support" in Configuration tab.

# Connection diagram

We use LED pin PWM functionality with RC filter to generate voltage:

![alt text](/docs/assets/images/ledpinpwmfilter.png  "led pin pwm filter")

# Example PCB layout (SMD components)

RC Filter can be soldered on a small piece of PCB:

![alt text](/docs/assets/images/osd_joystick.jpg  "osd joystick")

# Configuring keys voltages

If default voltages does not work with your camera model, then you have to measure voltages and find out corresponding PWM duty ratios.

1. Connect 5keys joystick to camera.
2. Measure voltages on OSD pin while each key is pressed.
3. Connect camera to FC throught RC filter as shown on schematix above.
4. Enable OSD Joystick emulation (see "Enabling OSD Joystick emulation" above)
4. Use cli command ```led_pin_pwm <value>```, value = 0...100 to find out PWM values for each voltage.
5. Specify PWM values in configuration and save:

```set osd_joystick_down=0```

```set osd_joystick_up=48```

```set osd_joystick_left=63```

```set osd_joystick_right=28```

```set osd_joystick_enter=75```

```save```

# Entering OSD Joystick emulation mode

Emulation can be enabled in unarmed state only. 

OSD Joystick emulation mode is enabled using the following stick combination:

```Throttle:CENTER Yaw:RIGHT```


Than camera OSD can be navigated using right stick. See [Controls](Controls.md) for all stick combinations.

*Note that the same stick combination is used to enable 5keys joystick emulation with RuncamDevice protocol.*

Mode is exited using stick combination:

```Throttle:CENTER Yaw:LEFT```

# RC Box

There are 3 RC Boxes which can be used in armed and unarmed state:
- Camera 1 - Enter
- Camera 2 - Up
- Camera 3 - Down

Other keys can be emulated using Programming framework ( see [LED pin PWM](LED%20pin%20PWM.md) for more details ).

# Behavior on boot

There is ~2 seconds LOW pulse during boot sequence, which corresponds to DOWN key. Fortunately, cameras seem to ignore any key events  few seconds after statup.
