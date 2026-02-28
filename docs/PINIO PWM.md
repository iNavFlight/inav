# PINIO PWM

INAV provides two mechanisms for generating output signals on GPIO pins:

1. **PINIO channels (0-3)** — Any PWM-capable timer output defined as `PINIOx_PIN` in the target. Supports full 0-100% duty cycle PWM at 24 kHz.
2. **LED strip idle level (channel 4)** — The WS2812 LED strip pin can be switched between idle-LOW and idle-HIGH between LED update bursts. Binary on/off only.

## PINIO PWM channels

PINIO channels are configured per-target in `target.h` using `PINIO1_PIN` through `PINIO4_PIN`. When a PINIO pin has a timer, it is automatically configured as a 24 kHz PWM output.

PWM duty cycle can be controlled via:
- **CLI:** `piniopwm [channel] <duty>` (duty = 0-100)
- **Programming framework:** Operation 52, Operand A = channel (0-3), Operand B = duty (0-100)

Setting duty to 0 stops PWM generation (pin goes LOW, or HIGH if `PINIO_FLAGS_INVERTED` is set in target.h).

Feature can be used to drive external devices such as a VTX power switch. Setting the PWM duty cycle to 100% or 0% effectively provides a digital on/off output. It is also used to simulate [OSD joystick](OSD%20Joystick.md) to control cameras.

PWM frequency is fixed to 24kHz with duty ratio between 0 and 100%:

![alt text](/docs/assets/images/led_pin_pwm.png  "PINIO PWM signal")

## LED strip idle level (channel 4)

When the LED strip feature is enabled, the WS2812 pin sends data bursts (~1 ms) every 10-20 ms. Between bursts, the pin idles at a configurable level.

The LED strip idle level is accessible as channel `4` (the next channel after PINIO hardware channels 0-3):

- **CLI:** `piniopwm 4 <value>` — value > 0 sets idle HIGH, 0 sets idle LOW
- **Programming framework:** Operation 52, Operand A = 4, Operand B = value (>0 = HIGH, 0 = LOW)

This can be used to drive a MOSFET or similar device connected to the LED pin, toggled by the programming framework based on flight mode, RC channel, GPS state, etc.

*Note: there will be a ~2 second LOW pulse on the LED pin during boot.*

### LED strip idle level timing

Normally LED pin is held low between WS2812 updates:

![alt text](/docs/assets/images/ws2811_packets.png  "ws2811 packets")
![alt text](/docs/assets/images/ws2811_data.png  "ws2811 data")

When idle is set HIGH, the pin is held high between updates. Total ws2812 pulse duration is ~1ms with ~9ms pauses. Connected devices should be tolerant of these brief transients.

# Generating PWM/output signals with programming framework

See operation 52 "PINIO PWM" in [Programming Framework](Programming%20Framework.md)

# Generating PWM/output signals from CLI

`piniopwm [channel] <duty>` — channel = 0-4, duty = 0-100

- One argument: sets duty on channel 0 (backward compatible)
- Two arguments: first is channel, second is duty
- No arguments: stops PWM on channel 0

# Example of driving LED

It is possible to drive single color LED with brightness control. Current consumption should not be greater then 1-2ma, thus LED can be used for indication only.

![alt text](/docs/assets/images/ledpinpwmled.png  "PINIO PWM LED")

# Example of driving powerful white LED

To drive power LED with brightness control, a MOSFET should be used:

![alt text](/docs/assets/images/ledpinpwmpowerled.png  "PINIO PWM power LED")

# Programming tab example for using a PINIO channel to switch a VTX or camera on and off
![screenshot of programming tab using PINIO](/docs/assets/images/led-as-pinio.png)
