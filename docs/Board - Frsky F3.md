# Board - Frsky F3 XSRF3O AIO

## Hardware Features

Refer to the product web page:
[Frsky XSRF3O AIO F3 Flight Control](https://www.frsky-rc.com/product/xsrf3o/)

### Hardware Notes

TODO...


### Developer Notes

| Board I/O             | CPU Pin # | CPU Pin Name | AF?       | AFs
|-----------------------|-----------|--------------|-----------|---------------------------------------------------------------------------------------------|
| M1 / PWM1             | 46        | PB9          |           | I2C1_SDA, CAN_TX, TIM17_CH1, TIM4_CH4, TIM8_CH3, IR_OUT, COMP2_OUT, EVENTOUT |
| M2 / PWM2             | 45        | PB8          |           | I2C1_SCL, CAN_RX, TIM16_CH1, TIM4_CH3, TIM8_CH2, TIM1_BKIN, TSC_SYNC, COMP1_OUT, EVENTOUT |
| M3 / PWM3             | 19        | PB1          |           | TIM3_CH4, TIM1_CH3N, TIM8_CH3N, COMP4_OUT, TSC_G3_IO3, EVENTOUT -- ADC3_IN1, OPAMP3_VOUT- |
| M4 / PWM4             | 11        | PA1          |           | USART2_RTS_DE, TIM2_CH2, TSC_G1_IO2, TIM15_CH1N, RTC_REFIN, EVENTOUT -- ADC1_IN2, COMP1_INP, OPAMP1_VINP, OPAMP3_VINP |
| PWM5                  | 12        | PA2          | USART2_TX (AF7) | USART2_TX, TIM2_CH3, TIM15_CH1, TSC_G1_IO3, COMP2_OUT, EVENTOUT -- ADC1_IN3, COMP2_INM, OPAMP1_VOUT |
| PWM6                  | 13        | PA3          | USART2_TX (AF7) | USART2_RX, TIM2_CH4, TIM15_CH2, TSC_G1_IO4, EVENTOUT -- ADC1_IN4, OPAMP1_VINP, COMP2_INP, OPAMP1_VINM |
| PWM7                  | 10        | PA0          |           | USART2_CTS, TIM2_CH1_ETR,TIM8_BKIN, TIM8_ETR,TSC_G1_IO1, COMP1_OUT, EVENTOUT -- ADC1_IN1, COMP1_INM, RTC_ TAMP2, WKUP1, COMP7_INP |
| PWM8                  | 18        | PB0          |           | TIM3_CH3, TIM1_CH2N, TIM8_CH2N,TSC_G3_IO2, EVENTOUT -- ADC3_IN12, COMP4_INP, OPAMP3_VINP, OPAMP2_VINP |
| LED                   | 29        | PA8          |           | I2C2_SMBA,I2S2_MCK, USART1_CK, TIM1_CH1, TIM4_ETR, MCO, COMP3_OUT, EVENTOUT |
| Buzzer -              | ??        | PXX          |           | 
| UART1 TX / PIN #3     | 30        | PA9          |           | I2C2_SCL,I2S3_MCK, USART1_TX, TIM1_CH2, TIM2_CH3, TIM15_BKIN, TSC_G4_IO1, COMP5_OUT, EVENTOUT |
| UART1 RX / PIN #4     | 31        | PA10         |           | I2C2_SDA, USART1_RX, TIM1_CH3, TIM2_CH4, TIM8_BKIN, TIM17_BKIN, TSC_G4_IO2, COMP6_OUT, EVENTOUT |


TODO: Add pin direction to table above

SBUS output from the XSR goes to:  RC Channel 8 is mapped to RSSI
UART2_TX - PA14
UART2_RX - PA15 

Smart port from the XSR goes to:
UART3_TX - PB10
UART3_RX - PB11

led red, green, blue
OSD spi + gpio

SWD pinout

Analog input info:
	Voltage Sense:
		100K and 10K voltage divider.  Max voltage 36.3V (Confirm this is true...)

	Current Sense:
		Direct input to processor.  Max Voltage 3.3V


### Misc Info

Where to feed vin?
vin range?
Which LDO powers what?
	- Can a separate battery run the rx and beeper only?


What is powered when plugged into usb?
	: Beeper, UART1/GPS @ 3.3V, Smart Port @ 5V,  

VBattery voltage divider: 100K, 10K.  Max vin: 36.3V ? ADC: 12-6 bits of resolution


### Obsolete

There are few things to note on how things are connected on the board.

1. VBAT (J4)
This is a battery input to the board, and is also a input to voltage sensor.

2. J11 Power distribution
The RAM is user defined power rail, and all RAM through holes (J6, J7 and J11) are connected together. By connecting 5V or VBAT to RAM at J11, the RAM becomes 5V or VBAT power rail respectively. The VBAT on J11 can also be used to power the Board if necessary.

3. RSSI (J4)
The pin is labelled as RSSI, but it will not be used for RSSI input for a hardware configuration limitation. In this document, the "RSSI" is used to indicate the pin location, not the function.

4. UART1 in boot-loader/DFU mode
The UART1 is scanned during boot-loader/DFU mode, together with USB for possible interaction with a host PC. It is observed that devices that autonomously transmits some data, such as GPS, will prevent the MCU to talk to the USB. It is advised not to connect or disconnect such devices to/from UART1. UART2 is safe from this catch.

## iNav Specific Target Configuration

The first support for the OMNIBUS F3 appeared in BetaFlight.
The OMNIBUS target in iNav has different configuration from the BetaFlight support, to maximize the hardware resource utilization for navigation oriented use cases.

 [PIN CONFIGURATION PIC HERE]

### PWM Outputs

Six PWM outputs (PWM1~PWM6) are supported, but PWM5 and PWM6 is not available when UART3 is in use.
PWM7 and PWM8 are dedicated for I2C; in this document, they are used to indicate the pin location, not the function.

If servos are used on a multirotor mixer (i.e. Tricopter) PWM1 is remapped to servo and motor 1 is moved to PWM2 etc.

Note: Tested only for QUAD-X configuration.

### Hardware UART Ports

PPM/SBUS jumper for J8 is assumed to be configured for PPM (SBUS=R18 removed). With newer boards (the 1.1 Version) you don't have to swap an smd resistor to use SBUS anymore. It just works out of the box.

| UART  | Location | Note              |
|-------|----------|-------------------|
| UART1 |J13       |                   |
| UART2 |J12       |                   |
| UART3 |J22       | PWM5=TX3,PWM6=RX3 |

All UARTs are Serial RX capable.

### I2C

I2C is available on J22 PWM7 and PWM8

|signal | Location   | Alt. Location |
|-------|------------|---------------|
|SCL    | J22 (PWM7) | J3 (SCL)      |
|SDA    | J22 (PWM8) | J3 (SDA)      |

### RANGEFINDER

HC-SR04 rangefinder is supported when NOT using PPM.

|signal | Location   |
|-------|------------|
|TRIG   | J8 (PPM)   |
|ECHO   | J4 (RSSI)  |

5V rangefinder can be connected directly without inline resistors.

### OSD

Integrated OSD is supported.

### RSSI Sensor Input

The RSSI sensor adc is not supported due to the hardware configuration limitation.

## Usage in a Fixed Wing
Due to the way INAV handles PWM outputs the first 2 PWM outputs are reserved for the motor outputs. When using SBUS on UART3 as recommended this leaves only 2 additional outputs for the servos, as output 5 and 6 are blocked by UART3 serial for SBUS and 7 and 8 are used for I2C.

You can free PWM outputs 5 and 6 by simply connecting SBUS up to UART1. For FrSky there is no hardware inverter needed as the F3 chip UARTs can handle this without additional hardware. Just make sure that `sbus_inversion = ON` is set. However, you will not be able to use UART3, e.G. for telemetry.

This allows to control a standard airplane with rudder, ailerons and elevator. If you use flaps or a servo gimbal, you can bypass the FC by connecting it up to the receiver directly. 

The popular x4rsb for example outputs channels 1,2,3 as PWM in addition to SBUS. Since they are shared with the channels on SBUS you need to change the channel mapping to `123AETR4`and ignore the first 3 AUX channels within Cleanflight.
