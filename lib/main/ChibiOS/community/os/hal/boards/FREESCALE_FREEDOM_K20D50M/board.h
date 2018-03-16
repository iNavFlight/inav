/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

/*
 * Setup for Freescale Freedom K20D50M board.
 */

/*
 * Board identifier.
 */
#define BOARD_FREESCALE_FREEDOM_K20D50M
#define BOARD_NAME                  "Freescale Freedom K20D50M"

/* External 8 MHz crystal. */
#define KINETIS_XTAL_FREQUENCY      8000000UL

/*
 * MCU type
 */
#define K20x5

/*
 * Onboard features.
 */
#define GPIO_LED_RED    IOPORT3
#define PIN_LED_RED     3
#define GPIO_LED_GREEN  IOPORT4
#define PIN_LED_GREEN   4
#define GPIO_LED_BLUE   IOPORT1
#define PIN_LED_BLUE    2

/* Inertial sensor: MMA8451Q */
/* Default I2C address 0x1D */
#define I2C_GYRO I2C0

#define LINE_LED_RED    PAL_LINE(GPIO_LED_RED, PIN_LED_RED)
#define LINE_LED_GREEN  PAL_LINE(GPIO_LED_GREEN, PIN_LED_GREEN)
#define LINE_LED_BLUE   PAL_LINE(GPIO_LED_BLUE, PIN_LED_BLUE)
#define LINE_GYRO_SCL   PAL_LINE(GPIOB, 0U)
#define LINE_GYRO_SDA   PAL_LINE(GPIOB, 1U)
#define LINE_GYRO_INT1  PAL_LINE(GPIOC, 11U)
#define LINE_GYRO_INT2  PAL_LINE(GPIOC, 6U)

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
