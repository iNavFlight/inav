# LED pin PWM

Normally LED pin is used to drive WS2812 led strip. LED pin is held low, and every 10ms or 20ms a set of pulses is sent to change color of the 32 LEDs:

![alt text](/docs/assets/images/ws2811_packets.png  "ws2811 packets")
![alt text](/docs/assets/images/ws2811_data.png  "ws2811 data")

As alternative function, it is possible to generate PWM signal with specified duty ratio on the LED pin.

Feature can be used to drive external devices. It is also used to simulate [OSD joystick](OSD%20Joystick.md) to control cameras.

PWM frequency is fixed to 24kHz with duty ratio between 0 and 100%:

![alt text](/docs/assets/images/led_pin_pwm.png  "led pin pwm")

There are four modes of operation:
- low
- high
- shared_low
- shared_high

Mode is configured using ```led_pin_pwm_mode``` setting: ```LOW```, ```HIGH```, ```SHARED_LOW```, ```SHARED_HIGH```

*Note that in any mode, there will be ~2 seconds LOW pulse on boot.*

## LOW
LED Pin is initialized to output low level by default and can be used to generate PWM signal.

ws2812 strip can not be controlled.

## HIGH
LED Pin is initialized to output high level by default and can be used to generate PWM signal.

ws2812 strip can not be controlled.

## SHARED_LOW (default)
LED Pin is used to drive WS2812 strip. Pauses between pulses are low:

![alt text](/docs/assets/images/ws2811_packets.png  "ws2811 packets")

It is possible to generate PWM signal with duty ratio >0...100%. 

While PWM signal is generated, ws2811 strip is not updated. 

When PWM generation is disabled, LED pin is used to drive ws2812 strip. 

Total ws2812 pulses duration is ~1ms with ~9ms pauses. Thus connected device should ignore PWM signal with duty ratio < ~10%.

## SHARED_HIGH
LED Pin is used to drive WS2812 strip. Pauses between pulses are high. ws2812 pulses are prefixed with 50us low 'reset' pulse:

![alt text](/docs/assets/images/ws2811_packets_high.png  "ws2811 packets_high")
![alt text](/docs/assets/images/ws2811_data_high.png  "ws2811 data_high")

 It is possible to generate PWM signal with duty ratio 0...<100%. 
 
 While PWM signal is generated, ws2811 strip is not updated. 
 
 When PWM generation is disabled, LED pin is used to drive ws2812 strip. Total ws2812 pulses duration is ~1ms with ~9ms pauses. Thus connected device should ignore PWM signal with duty ratio > ~90%.
 
 After sending ws2812 protocol pulses for 32 LEDS, we held line high for 9ms, then send 50us low 'reset' pulse. Datasheet for ws2812 protocol does not describe behavior for long high pulse, but in practice it works the same as 'reset' pulse. To be safe, we also send correct low 'reset' pulse before starting next LEDs update sequence.
 
 This mode is used to simulate OSD joystick. It is Ok that effectively voltage level is held >90% while driving LEDs, because OSD joystick keypress voltages are below 90%.
 
 See [OSD Joystick](OSD%20Joystick.md) for more information.

# Generating PWM signal with programming framework

See "LED Pin PWM" operation in [Programming Framework](Programming%20Framework.md)


# Generating PWM signal from CLI

```ledpinpwm <value>``` - value = 0...100 -  enable PWM generation with specified duty cycle

```ledpinpwm``` - disable PWM generation ( disable to allow ws2812 LEDs updates in shared modes )


# Example of driving LED

It is possible to drive single color LED with brightness control. Current consumption should not be greater then 1-2ma, thus LED can be used for indication only.

![alt text](/docs/assets/images/ledpinpwmled.png  "led pin pwm led")

# Example of driving powerfull white LED

To drive power LED with brightness control, Mosfet should be used:

![alt text](/docs/assets/images/ledpinpwmpowerled.png  "led pin pwm power_led")

