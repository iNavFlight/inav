# Runcam device 

Cameras which support [Runcam device protocol](https://support.runcam.com/hc/en-us/articles/360014537794-RunCam-Device-Protocol), can be configured using sticks.

Note that for cameras which has OSD pin, there is alternative functionality: [OSD Joystick](OSD%20Joystick.md).

Camera's RX/TX should be connected to FC's UART, which has "Runcam device" option selected.

# Entering Joystick emulation mode

Emulation can be enabled in unarmed state only. 

Joystick emulation mode is enabled using the following stick combination:

```RIGHT CENTER```


Than camera OSD can be navigated using right stick. See [Controls](Controls.md) for all stick combinations.

*Note that the same stick combination is used to enable [OSD Joystick](OSD%20Joystick.md).*

Mode is exited using stick combination:

```LEFT CENTER```

# RC Box

There are 3 RC Boxes which can be used in armed and unarmed state:
- Camera 1 - Simulate Wifi button
- Camera 2 - Simulate POWER button
- Camera 3 - Simulate Change Mode button.

