Radiomaster NEXUS X and NEXUS XR
================================

Flight controllers originally designed for Helicopters using Rotorflight.
Based on STM32F722RET6. Both NEXUS X and XR share the same targets in iNav.

Built-in periperals
-------------------

Both models contain a ICM42688P IMU, a SPL06 barometer and a W25N02KVZEIR blackbox memory chip.

The NEXUS XR also contains a serial ELRS receiver based on the RP4TD-M using ESP32 and two SX1281.
It is connected to the main STM32F7 Flight Controller on UART5.
None of the external connections route to the receiver, they are all connected to the STM32F7 Flight Controller.
The receiver can be disabled using USER1, which controls a pinio on pin PC8.

Pin configuration
-----------------

All pin orders are from left to right, when looking at the connector on the flight controller.
Note that the order is incorrect on radiomaster's website, it has RX and TX swapped.

| Marking on the case | NEXUSX                                                | NEXUSX_9SERVOS                                        | NEXUSX_NOI2C                                          |
|---------------------|-------------------------------------------------------|-------------------------------------------------------|-------------------------------------------------------|
| S1                  | Output S1                                             | Output S1                                             | Output S1                                             |
| S2                  | Output S2                                             | Output S2                                             | Output S2                                             |
| S3                  | Output S3                                             | Output S3                                             | Output S3                                             |
| TAIL                | Output S4                                             | Output S4                                             | Output S4                                             |
| ESC                 | Output S5                                             | Output S5                                             | Output S5                                             |
| RPM                 | Output S6                                             | Output S6                                             | Output S6                                             |
| TLM                 | Output S7                                             | Output S7                                             | Output S7                                             |
| AUX                 | UART1 TX                                              | Output S8                                             | Output S8                                             |
| SBUS                | UART1 RX                                              | Output S9                                             | Output S9                                             |
| A                   | UART4<br>pin order:<br>TX, RX, 5V, GND                | UART4<br>pin order:<br>TX, RX, 5V, GND                | UART4<br>pin order:<br>TX, RX, 5V, GND                |
| B                   | UART6<br>pin order:<br>TX, RX, 5V, GND                | UART6<br>pin order:<br>TX, RX, 5V, GND                | UART6<br>pin order:<br>TX, RX, 5V, GND                |
| C                   | I2C<br>pin order:<br>SCL, SDA, 5V, GND                | I2C<br>pin order:<br>SCL, SDA, 5V, GND                | UART3<br>pin order:<br>TX, RX, 5V, GND                |
| EXT-V               | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND | battery voltage<br>max 60V<br>pin order:<br>Vbat, GND |
| built-in ELRS       | UART5                                                 | UART5                                                 | UART5                                                 |
Hardware layout
---------------


| Marking on the case | STM32 pin |                         Servo |           UART |      I2C |
|---------------------|-----------|------------------------------:|---------------:|---------:|
| S1                  | PB4       |                       TIM3CH1 |            n/a |      n/a |
| S2                  | PB5       |                       TIM3CH2 |            n/a |      n/a |
| S3                  | PB0       |                       TIM3CH3 |            n/a |      n/a |
| TAIL                | PA15      |                       TIM2CH1 |            n/a |      n/a |
| ESC                 | PA9       |                       TIM1CH2 |       UART1 TX |      n/a |
| RPM                 | PA2       | TIM2CH3<br>TIM5CH3<br>TIM9CH1 |       UART2 TX |      n/a |
| TLM                 | PA3       | TIM2CH4<br>TIM5CH4<br>TIM9CH2 |       UART2 RX |      n/a |
| AUX                 | PB6       |                       TIM4CH1 |       UART1 TX | I2C1 SCL |
| SBUS                | PB7       |                       TIM4CH2 |       UART1 RX | I2C1 SDA |
| A                   | PA1/PB0   |                  TIM1<br>TIM5 | UART2<br>UART4 |      n/a |
| B                   | PC7/PC6   |                  TIM3<br>TIM8 |          UART6 |      n/a |
| C                   | PB11/PB10 |                          TIM2 |          UART3 |     I2C2 |
| EXT-V               | PC0       |                           n/a | n/a            | n/a      |
| built-in ELRS       | PC12/PD2  |                           n/a |          UART5 |      n/a |

The pinout is extremely similar to the F7C reference design from Rotorflight.
https://github.com/rotorflight/rotorflight-ref-design/blob/master/Reference-Design-F7C.md