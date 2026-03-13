# PINIO PWM

INAV provides two mechanisms for generating output signals on GPIO pins:

1. **PINIO channels (1-4)** — PWM-capable timer outputs, either defined in the target (`PINIOx_PIN`) or assigned by the user in the configurator's Output tab. Supports full 0-100% duty cycle PWM at 24 kHz.
2. **LED strip idle level (channel 0)** — The WS2812 LED strip pin can be switched between idle-LOW and idle-HIGH between LED update bursts. Binary on/off only.

## PINIO PWM channels

PINIO channels can come from two sources:

1. **Target-defined:** `PINIO1_PIN` through `PINIO4_PIN` in `target.h`. These are always available on supported boards.
2. **User-assigned:** Any timer output pad can be set to PINIO mode in the configurator's Output tab (timer_output_mode = PINIO). No target.h changes needed.

When a PINIO pin has a hardware timer, it is automatically configured as a 24 kHz PWM output. Pins without a timer fall back to GPIO on/off.

PWM duty cycle can be controlled via:
- **CLI:** `piniopwm <channel> <duty>` (channel = 1-4, duty = 0-100)
- **Programming framework:** Operation 52, Operand A = duty (0-100), Operand B = channel (1-4)
- **Mode boxes:** USER1-USER4 in the Modes tab toggle the channel on/off

Setting duty to 0 stops PWM generation (pin goes LOW, or HIGH if `PINIO_FLAGS_INVERTED` is set in target.h).

Feature can be used to drive external devices such as a VTX power switch. Setting the PWM duty cycle to 100% or 0% effectively provides a digital on/off output. It is also used to simulate [OSD joystick](OSD%20Joystick.md) to control cameras.

PWM frequency is fixed to 24kHz with duty ratio between 0 and 100%:

![alt text](/docs/assets/images/led_pin_pwm.png  "PINIO PWM signal")

## Mode box and programming framework interaction

PINIO channels can be controlled by both mode boxes (Modes tab) and the programming framework (Programming tab). When both are used:

- The **programming framework** sets the duty cycle (0-100%).
- The **mode box** gates the output on or off. When the mode is active, the programmed duty is output. When inactive, the pin is driven to 0%.

If no RC channel range is assigned to the USERx mode in the Modes tab, the programming framework has exclusive uninterrupted control — the mode box does not interfere.

The default duty (before the programming framework sets a value) is 100%, so toggling a mode box without any programming gives full on/off behavior.

## LED strip idle level (channel 0)

When the LED strip feature is enabled, the WS2812 pin sends data bursts (~1 ms) every 10-20 ms. Between bursts, the pin idles at a configurable level.

The LED strip idle level is accessible as channel `0`:

- **CLI:** `piniopwm 0 <value>` — value > 0 sets idle HIGH, 0 sets idle LOW
- **Programming framework:** Operation 52, Operand A = value (>0 = HIGH, 0 = LOW), Operand B = 0

This can be used to drive a MOSFET or similar device connected to the LED pin, toggled by the programming framework based on flight mode, RC channel, GPS state, etc.

*Note: there will be a ~2 second LOW pulse on the LED pin during boot.*

### LED strip idle level timing

Normally LED pin is held low between WS2812 updates:

![alt text](/docs/assets/images/ws2811_packets.png  "ws2811 packets")
![alt text](/docs/assets/images/ws2811_data.png  "ws2811 data")

When idle is set HIGH, the pin is held high between updates. Total ws2812 pulse duration is ~1ms with ~9ms pauses. Connected devices should be tolerant of these brief transients.

# Channel numbering

| Channel | Target | Description |
|---------|--------|-------------|
| 0 | LED strip | Binary idle level (HIGH/LOW) |
| 1-4 | PINIO 1-4 | Full 0-100% PWM at 24 kHz |

This numbering is consistent across the CLI (`piniopwm`), programming framework (operation 52), and the `pinio_box1`-`pinio_box4` settings.

# Generating PWM/output signals from CLI

`piniopwm [channel] <duty>` — channel = 0-4, duty = 0-100

- One argument: sets LED idle level (channel 0, backward compatible)
- Two arguments: first is channel (0 = LED idle, 1-4 = PINIO), second is duty
- No arguments: stops PWM on PINIO 1

# Example of driving LED

It is possible to drive single color LED with brightness control. Current consumption should not be greater then 1-2ma, thus LED can be used for indication only.

![alt text](/docs/assets/images/ledpinpwmled.png  "PINIO PWM LED")

# Example of driving powerful white LED

To drive power LED with brightness control, a MOSFET should be used:

![alt text](/docs/assets/images/ledpinpwmpowerled.png  "PINIO PWM power LED")

# Programming tab example for using a PINIO channel to switch a VTX or camera on and off
![screenshot of programming tab using PINIO](/docs/assets/images/led-as-pinio.png)
