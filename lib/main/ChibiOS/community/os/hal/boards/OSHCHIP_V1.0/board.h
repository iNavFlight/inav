/*
    Copyright (C) 2016 flabbergast

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/* Board identifier. */
#define BOARD_OSHCHIP_V10
#define BOARD_NAME              "OSHChip_V1.0"

/* Board oscillators-related settings. */
#define NRF51_XTAL_VALUE        16000000

/* Non-header GPIO pins. */
#define LED_RED        8
#define LED_GREEN      5
#define LED_BLUE       3

/* Common peripheral GPIO pins. */
#define UART_TX        20
#define UART_RX        18

/* GPIO on DIP pins. */
#define OSHCHIP_PIN1   20
#define OSHCHIP_PIN2   18
#define OSHCHIP_PIN3   16
#define OSHCHIP_PIN4   15
#define OSHCHIP_PIN5   12
#define OSHCHIP_PIN6   11
#define OSHCHIP_PIN7   9
/* Pin 8 is GND */
#define OSHCHIP_PIN9   1
#define OSHCHIP_PIN10  2
#define OSHCHIP_PIN11  0
#define OSHCHIP_PIN12  27
#define OSHCHIP_PIN13  26
#define OSHCHIP_PIN14  24
#define OSHCHIP_PIN15  21
/* Pin 16 is VCC */

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
