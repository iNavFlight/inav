# LED pin PWM

Normally LED pin is used to drive WS2812 led strip. LED pin is held low, and every 10ms or 20ms a set of pulses is sent to change color of the 32 LEDs:

![alt text](/docs/assets/images/ws2811_packets.png  "ws2811 packets")
![alt text](/docs/assets/images/ws2811_data.png  "ws2811 data")

As alternative function, it is possible to generate PWM signal with specified duty ratio on the LED pin.

Feature can be used to drive external devices. It is also used to simulate OSD joystick to control cameras.

PWM frequency is fixed to 24kHz with duty ratio between 0 and 100%:

![alt text](/docs/assets/images/led_pin_pwm.png  "led pin pwm")

There are four modes of operation:
- low
- high
- shared_low
- shared_high

Mode is configured using ```led_pin_pwm_mode``` setting.

## Low
LED Pin is initialized to output low level by default and can be used to generate PWM signal.

ws2812 strip can not be controlled.

## High
LED Pin is initialized to output high level by default and can be used to generate PWM signal.

ws2812 strip can not be controlled.

## Shared low (default)
LED Pin is used to drive WS2812 strip. Pauses between pulses are low:

![alt text](/docs/assets/images/ws2811_packets.png  "ws2811 packets")

It is possible to generate PWM signal with duty ratio >0...100%. 

While PWM signal is generated, ws2811 strip is not updated. 

When PWM generation is disabled, LED pin is used to drive ws2812 strip. 

Total ws2812 pulses duration is ~1ms with ~9ms pauses. Thus connected device should ignore PWM singal with duty ratio < ~10%.

 

## Shared high
 LED Pin is used to drive WS2812 strip. Pauses between pulses are high. ws2812 pulses are prefixed with 300us low 'reset' pulse:

![alt text](/docs/assets/images/ws2811_packets_high.png  "ws2811 packets_high")
![alt text](/docs/assets/images/ws2811_data_high.png  "ws2811 data_high")

 It is possible to generate PWM signal with duty ratio 0...<100%. 
 
 While PWM signal is generated, ws2811 strip is not updated. 
 
 When PWM generation is disabled, LED pin is used to drive ws2812 strip. Total ws2812 pulses duration is ~1ms with ~9ms pauses. Thus connected device should ignore PWM signal with duty ratio > ~90%.
 
 After sending ws2812 protocol pulses for 32 LEDS, we held line high for 9ms, then send 50us low 'reset' pulse. 
 
 Datasheet for ws2812 protocol does not describe behavior for long high pulse, but in practice it works the same as 'reset' pulse. 
 
 To be safe, we also send correct low 'reset' pulse before starting next LEDs update sequence.
 
 This mode is used to simulate OSD joystick. It is Ok that effectively voltage level is held >90% while driving LEDs, because OSD joystick keypress voltages are below 90%.
 
 See OSD Joystick.md for more information.

# Generating PWM signal in programming framework

*TODO*

0...100 - enable PWM generation with specified duty cicle

-1 - disable PWM generation ( disable to allow ws2812 LEDs updates in shared modes )

# Generating PWM signal from CLI

```ledpinpwm <value>``` - value = 0...100 -  enable PWM generation with specified duty cycle

```ledpinpwm``` - disable PWM generation ( disable to allow ws2812 LEDs updates in shared modes )


# Example of driving single color LED

*TODO*

# Example of driving powerfull white LED

*TODO*

