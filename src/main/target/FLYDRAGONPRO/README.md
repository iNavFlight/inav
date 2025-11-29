FlydragonRC Flydragon Pro
=========================

Flight controllers originally designed for helicopters using Rotorflight.
Two variants exist, one with ICM42688P IMU and one with MPU6000. Both share the same target in INAV.

Built-in periperals
-------------------

Both models contain
- STM32F722RET6 MCU
- SPL06-001 barometer
- W25N01GVZEIG 1GBit Blackbox
- ExpressLRS receiver using ESP8285 and SX1280 connected to UART1. ELRS target is "FD R24D 2.4GHz RX"
- large button connected to BOOT0
- large button is backlit by WS2812B RGB LED
- "GPS" expansion port with 5V, UART5, I2C and GND
- "DSM" expansion port with 3.3V, GND and UART3 RX
- "EXT" expansion port with BAT+, GND, BZZP and 5V.

Buzzer should be connected between 5V and BZZP.  
The BAT+ pin measures voltage up to 62V. When measuring the flight battery it is recommended to connect BAT+ but not GND to prevent unintended current paths.

None of the external connections route to the receiver, they are all connected to the STM32F7 Flight Controller.
The receiver can be disabled using USER1, which controls a pinio on pin PA15.

Pin configuration
-----------------

The ESC, RPM, RX2 and TX2 pins are Servo/Motor outputs by default. However, when UART4 or UART2 are assigned a function in the ports tab, the pins will become a UART instead. See the table below.

**The RPM pin features a filtering circuit that limits UART4 RX to 115200 baud. This means CRSF won't work on UART4, while slower protocols like SBUS will.**

| Marking on the case | Both UART2 and UART4 unused | UART2 in use            | UART4 in use            | Both UART2 and UART4 in use |
|---------------------|-----------------------------|-------------------------|-------------------------|-----------------------------|
| TAIL                | Output S1                   | Output S1               | Output S1               | Output S1                   |
| CH3                 | Output S2                   | Output S2               | Output S2               | Output S2                   |
| CH2                 | Output S3                   | Output S3               | Output S3               | Output S3                   |
| CH1                 | Output S4                   | Output S4               | Output S4               | Output S4                   |
| ESC                 | Output S5                   | Output S5               | UART4 TX                | UART4 TX                    |
| RPM                 | Output S6                   | Output S6               | UART4 RX                | UART4 RX                    |
| RX2                 | Output S7                   | UART2 RX                | Output S5               | UART2 RX                    |
| TX2                 | Output S8                   | UART2 TX                | Output S6               | UART2 TX                    |
| AUX                 | Output S9                   | Output S7               | Output S7               | Output S5                   |
| GPS RX5             | UART5 RX                    | UART5 RX                | UART5 RX                | UART5 RX                    |
| GPS TX5             | UART5 TX                    | UART5 TX                | UART5 TX                | UART5 TX                    |
| GPS SCL             | I2C SCL                     | I2C SCL                 | I2C SCL                 | I2C SCL                     |
| GPS SDA             | I2C SDA                     | I2C SDA                 | I2C SDA                 | I2C SDA                     |
| DSM RX3             | UART3 RX                    | UART3 RX                | UART3 RX                | UART3 RX                    |
| EXT BAT+            | battery voltage max 62V     | battery voltage max 62V | battery voltage max 62V | battery voltage max 62V     |
| EXT BZZP            | Buzzer positive pin         | Buzzer positive pin     | Buzzer positive pin     | Buzzer positive pin         |
| built-in ELRS       | UART1                       | UART1                   | UART1                   | UART1                       |


Hardware layout
---------------


| Marking on the case | STM32 pin |                                     Servo |                 UART |                                        I2C |
|---------------------|-----------|------------------------------------------:|---------------------:|-------------------------------------------:|
| TAIL                | PC9       |                        TIM3CH4<br>TIM8CH4 |                  n/a |                                   I2C3 SDA |
| CH3                 | PC8       |                        TIM3CH3<br>TIM8CH3 |                  n/a |                                        n/a |
| CH2                 | PC7       |                        TIM3CH2<br>TIM8CH2 |                  n/a |                                        n/a |
| CH1                 | PC6       |                        TIM3CH1<br>TIM8CH1 |                  n/a |                                        n/a |
| ESC                 | PA0       |                        TIM2CH1<br>TIM5CH1 |             UART4 TX |                                        n/a |
| RPM                 | PA1       |                        TIM2CH2<br>TIM5CH2 |             UART4 RX |                                        n/a |
| RX2                 | PA3       |             TIM2CH4<br>TIM5CH4<br>TIM9CH2 |             UART2 RX |                                        n/a |
| TX2                 | PA2       |             TIM2CH3<br>TIM5CH3<br>TIM9CH1 |             UART2 TX |                                        n/a |
| AUX                 | PB9       |                       TIM4CH4<br>TIM11CH1 |                  n/a |                                   I2C1 SDA |
| GPS RX5             | PD2       |                                       n/a |             UART5 RX |                                        n/a |
| GPS TX5             | PC12      |                                       n/a |             UART5 TX |                                        n/a |
| GPS SCL             | PB6       |                                   TIM4CH1 |             UART1 TX |                                   I2C1 SCL |
| GPS SDA             | PB7       |                                   TIM4CH2 |             UART1 RX |                                   I2C1 SDA |
| DSM RX3             | PC11      |                                       n/a | UART3 RX<br>UART4 RX |                                        n/a |
| EXT BAT+            | PC0       |                                       n/a |                  n/a |                                        n/a |
| EXT BZZP            | PA8       | TIM1CH1<br>pin is behind a FET for buzzer |                  n/a | I2C3 SCL<br>pin is behind a FET for buzzer |
| RGB LED             | PB8       |                                   TIM4CH1 |             UART1 TX |                                   I2C1 SCL |
| BUTTON              | BOOT0     |                                       n/a |                  n/a |                                        n/a |
| built-in ELRS       | PA9/PA10  |                                       n/a |                UART1 |                                        n/a |

PC1 is ADC for servo plug bank.
PC2 is ADC for the built-in BEC.