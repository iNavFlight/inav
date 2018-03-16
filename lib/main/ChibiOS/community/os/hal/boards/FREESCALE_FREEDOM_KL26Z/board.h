/*
    ChibiOS - Copyright (C) 2016 flabbergast <s3+flabbergast@sdfeu.org>

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
 * Setup for Freescale Freedom KL26Z board.
 */

/*
 * Board identifier.
 */
#define BOARD_FREESCALE_FREEDOM_KL26Z
#define BOARD_NAME                  "Freescale Freedom KL26Z"

/* External 8 MHz crystal. */
#define KINETIS_XTAL_FREQUENCY      8000000UL

/*
 * MCU type
 */
#define KL26

/*
 * Onboard features.
 */
#define GPIO_LED_RED        IOPORT5
#define PIN_LED_RED         29
#define GPIO_LED_GREEN      IOPORT5
#define PIN_LED_GREEN       31
#define GPIO_LED_BLUE       IOPORT4
#define PIN_LED_BLUE        5
#define GPIO_BUTTON         IOPORT4
#define PIN_BUTTON          0
#define GPIO_LIGHT_SENSOR   IOPORT5
#define PIN_LIGHT_SENSOR    22

/* Inertial sensor: FXOS8700CQ */
/* Default I2C address 0x1D */
/* Note: the pins PTE24/25 are assigned to I2C0 by default;
 *       if I2C0 is wanted on other pins, these need to be
 *       assigned another function explicitly!
 */
#define I2C_GYRO I2C0

#define LINE_LED_RED      PAL_LINE(GPIO_LED_RED, PIN_LED_RED)
#define LINE_LED_GREEN    PAL_LINE(GPIO_LED_GREEN, PIN_LED_GREEN)
#define LINE_LED_BLUE     PAL_LINE(GPIO_LED_BLUE, PIN_LED_BLUE)
#define LINE_BUTTON       PAL_LINE(GPIO_BUTTON, PIN_BUTTON)
#define LINE_LIGHT_SENSOR PAL_LINE(GPIO_LIGHT_SENSOR, PIN_LIGHT_SENSOR)
#define LINE_GYRO_SCL     PAL_LINE(GPIOE, 24U)
#define LINE_GYRO_SDA     PAL_LINE(GPIOE, 25U)
#define LINE_GYRO_INT1    PAL_LINE(GPIOD, 0U)
#define LINE_GYRO_INT2    PAL_LINE(GPIOD, 1U)

/*
 * Not configured:
 *  - TSI Slider on PTB16/TSI0_CH9 and PTB17/TSI_CH10
 *  - I2C inertial sensor on I2C0, routed to PTE25 and PTE25
 *    Note: these pins are assigned to I2C0 by default;
 *          if I2C0 is wanted on other pins, these need to be
 *          assigned another function explicitly!
 */

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
