/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * Setup for Texas Instruments TM4C123G Launchpad Board.
 */

/*
 * Board identifier.
 */
#define BOARD_TI_TM4C123G_LAUNCHPAD
#define BOARD_NAME              "Texas Instruments TM4C123G Launchpad"

/*
 * MCU type
 */
//#define TM4C1230C3PM
//#define TM4C1230D5PM
//#define TM4C1230E6PM
//#define TM4C1230H6PM
//#define TM4C1231C3PM
//#define TM4C1231D5PM
//#define TM4C1231D5PZ
//#define TM4C1231E6PM
//#define TM4C1231E6PZ
//#define TM4C1231H6PGE
//#define TM4C1231H6PM
//#define TM4C1231H6PZ
//#define TM4C1232C3PM
//#define TM4C1232D5PM
//#define TM4C1232E6PM
//#define TM4C1232H6PM
//#define TM4C1233C3PM
//#define TM4C1233D5PM
//#define TM4C1233D5PZ
//#define TM4C1233E6PM
//#define TM4C1233E6PZ
//#define TM4C1233H6PGE
//#define TM4C1233H6PM
//#define TM4C1233H6PZ
//#define TM4C1236D5PM
//#define TM4C1236E6PM
//#define TM4C1236H6PM
//#define TM4C1237D5PM
//#define TM4C1237D5PZ
//#define TM4C1237E6PM
//#define TM4C1237E6PZ
//#define TM4C1237H6PGE
//#define TM4C1237H6PM
//#define TM4C1237H6PZ
//#define TM4C123AE6PM
//#define TM4C123AH6PM
//#define TM4C123BE6PM
//#define TM4C123BE6PZ
//#define TM4C123BH6PGE
//#define TM4C123BH6PM
//#define TM4C123BH6PZ
//#define TM4C123BH6ZRB
//#define TM4C123FE6PM
//#define TM4C123FH6PM
//#define TM4C123GE6PM
//#define TM4C123GE6PZ
//#define TM4C123GH6PGE
#define TM4C123GH6PM
//#define TM4C123GH6PZ
//#define TM4C123GH6ZRB
//#define TM4C123GH5ZXR

/*
 * Board oscillators-related settings.
 */
#define TIVA_XTAL_VALUE         16000000

/*
 * IO pins assignments.
 */
#define GPIOA_UART0_RX          0
#define GPIOA_UART0_TX          1
#define GPIOA_SSI0_CLK          2
#define GPIOA_PIN3              3
#define GPIOA_SSI0_RX           4
#define GPIOA_SSI0_TX           5
#define GPIOA_PIN6              6
#define GPIOA_PIN7              7

#define GPIOB_PIN0              0
#define GPIOB_PIN1              1
#define GPIOB_I2C0_SCL          2
#define GPIOB_I2C0_SDA          3
#define GPIOB_PIN4              4
#define GPIOB_PIN5              5
#define GPIOB_PIN6              6
#define GPIOB_PIN7              7

#define GPIOC_TCK_SWCLK         0
#define GPIOC_TMS_SWDIO         1
#define GPIOC_TDI               2
#define GPIOC_TDO_SWO           3
#define GPIOC_PIN4              4
#define GPIOC_PIN5              5
#define GPIOC_PIN6              6
#define GPIOC_PIN7              7

#define GPIOD_PIN0              0
#define GPIOD_PIN1              1
#define GPIOD_PIN2              2
#define GPIOD_PIN3              3
#define GPIOD_PIN4              4
#define GPIOD_PIN5              5
#define GPIOD_PIN6              6
#define GPIOD_PIN7              7

#define GPIOE_PIN0              0
#define GPIOE_PIN1              1
#define GPIOE_PIN2              2
#define GPIOE_PIN3              3
#define GPIOE_PIN4              4
#define GPIOE_PIN5              5
#define GPIOE_PIN6              6
#define GPIOE_PIN7              7

#define GPIOF_SW2               0
#define GPIOF_LED_RED           1
#define GPIOF_LED_BLUE          2
#define GPIOF_LED_GREEN         3
#define GPIOF_SW1               4
#define GPIOF_PIN5              5
#define GPIOF_PIN6              6
#define GPIOF_PIN7              7

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 */
#define PIN_DATA_LOW(n)         (0U << (n))
#define PIN_DATA_HIGH(n)        (1U << (n))

#define PIN_DIR_IN(n)           (0U << (n))
#define PIN_DIR_OUT(n)          (1U << (n))

#define PIN_AFSEL_GPIO(n)       (0U << (n))
#define PIN_AFSEL_ALTERNATE(n)  (1U << (n))

#define PIN_ODR_DISABLE(n)      (0U << (n))
#define PIN_ODR_ENABLE(n)       (1U << (n))

#define PIN_PxR_DISABLE(n)      (0U << (n))
#define PIN_PxR_ENABLE(n)       (1U << (n))

#define PIN_DEN_DISABLE(n)      (0U << (n))
#define PIN_DEN_ENABLE(n)       (1U << (n))

#define PIN_AMSEL_DISABLE(n)    (0U << (n))
#define PIN_AMSEL_ENABLE(n)     (1U << (n))

#define PIN_DRxR_DISABLE(n)     (0U << (n))
#define PIN_DRxR_ENABLE(n)      (1U << (n))

#define PIN_SLR_DISABLE(n)      (0U << (n))
#define PIN_SLR_ENABLE(n)       (1U << (n))

#define PIN_PCTL_MODE(n, mode)  (mode << ((n) * 4))

/*
 * GPIOA Setup:
 *
 * PA0 - UART0 RX               ()
 * PA1 - UART0 TX               ()
 * PA2 - PIN2                   ()
 * PA3 - PIN3                   ()
 * PA4 - PIN4                   ()
 * PA5 - PIN5                   ()
 * PA6 - PIN6                   ()
 * PA7 - PIN7                   ()
 */
#define VAL_GPIOA_DATA          (PIN_DATA_LOW(GPIOA_UART0_RX) |              \
                                 PIN_DATA_LOW(GPIOA_UART0_TX) |              \
                                 PIN_DATA_LOW(GPIOA_SSI0_CLK) |                  \
                                 PIN_DATA_LOW(GPIOA_PIN3) |                  \
                                 PIN_DATA_LOW(GPIOA_SSI0_RX) |                  \
                                 PIN_DATA_LOW(GPIOA_SSI0_TX) |                  \
                                 PIN_DATA_LOW(GPIOA_PIN6) |                  \
                                 PIN_DATA_LOW(GPIOA_PIN7))

#define VAL_GPIOA_DIR           (PIN_DIR_IN(GPIOA_UART0_RX) |                \
                                 PIN_DIR_IN(GPIOA_UART0_TX) |                \
                                 PIN_DIR_IN(GPIOA_SSI0_CLK) |                    \
                                 PIN_DIR_IN(GPIOA_PIN3) |                    \
                                 PIN_DIR_IN(GPIOA_SSI0_RX) |                    \
                                 PIN_DIR_IN(GPIOA_SSI0_TX) |                    \
                                 PIN_DIR_IN(GPIOA_PIN6) |                    \
                                 PIN_DIR_IN(GPIOA_PIN7))

#define VAL_GPIOA_AFSEL         (PIN_AFSEL_GPIO(GPIOA_UART0_RX) |            \
                                 PIN_AFSEL_GPIO(GPIOA_UART0_TX) |            \
                                 PIN_AFSEL_GPIO(GPIOA_SSI0_CLK) |                \
                                 PIN_AFSEL_GPIO(GPIOA_PIN3) |                \
                                 PIN_AFSEL_GPIO(GPIOA_SSI0_RX) |                \
                                 PIN_AFSEL_GPIO(GPIOA_SSI0_TX) |                \
                                 PIN_AFSEL_GPIO(GPIOA_PIN6) |                \
                                 PIN_AFSEL_GPIO(GPIOA_PIN7))

#define VAL_GPIOA_ODR           (PIN_ODR_DISABLE(GPIOA_UART0_RX) |           \
                                 PIN_ODR_DISABLE(GPIOA_UART0_TX) |           \
                                 PIN_ODR_DISABLE(GPIOA_SSI0_CLK) |               \
                                 PIN_ODR_DISABLE(GPIOA_PIN3) |               \
                                 PIN_ODR_DISABLE(GPIOA_SSI0_RX) |               \
                                 PIN_ODR_DISABLE(GPIOA_SSI0_TX) |               \
                                 PIN_ODR_DISABLE(GPIOA_PIN6) |               \
                                 PIN_ODR_DISABLE(GPIOA_PIN7))

#define VAL_GPIOA_PUR           (PIN_PxR_DISABLE(GPIOA_UART0_RX) |           \
                                 PIN_PxR_DISABLE(GPIOA_UART0_TX) |           \
                                 PIN_PxR_DISABLE(GPIOA_SSI0_CLK) |               \
                                 PIN_PxR_DISABLE(GPIOA_PIN3) |               \
                                 PIN_PxR_DISABLE(GPIOA_SSI0_RX) |               \
                                 PIN_PxR_DISABLE(GPIOA_SSI0_TX) |               \
                                 PIN_PxR_DISABLE(GPIOA_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOA_PIN7))

#define VAL_GPIOA_PDR           (PIN_PxR_DISABLE(GPIOA_UART0_RX) |           \
                                 PIN_PxR_DISABLE(GPIOA_UART0_TX) |           \
                                 PIN_PxR_DISABLE(GPIOA_SSI0_CLK) |               \
                                 PIN_PxR_DISABLE(GPIOA_PIN3) |               \
                                 PIN_PxR_DISABLE(GPIOA_SSI0_RX) |               \
                                 PIN_PxR_DISABLE(GPIOA_SSI0_TX) |               \
                                 PIN_PxR_DISABLE(GPIOA_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOA_PIN7))

#define VAL_GPIOA_DEN           (PIN_DEN_ENABLE(GPIOA_UART0_RX) |            \
                                 PIN_DEN_ENABLE(GPIOA_UART0_TX) |            \
                                 PIN_DEN_ENABLE(GPIOA_SSI0_CLK) |                \
                                 PIN_DEN_ENABLE(GPIOA_PIN3) |                \
                                 PIN_DEN_ENABLE(GPIOA_SSI0_RX) |                \
                                 PIN_DEN_ENABLE(GPIOA_SSI0_TX) |                \
                                 PIN_DEN_ENABLE(GPIOA_PIN6) |                \
                                 PIN_DEN_ENABLE(GPIOA_PIN7))

#define VAL_GPIOA_AMSEL         (PIN_AMSEL_DISABLE(GPIOA_UART0_RX) |         \
                                 PIN_AMSEL_DISABLE(GPIOA_UART0_TX) |         \
                                 PIN_AMSEL_DISABLE(GPIOA_SSI0_CLK) |             \
                                 PIN_AMSEL_DISABLE(GPIOA_PIN3))

#define VAL_GPIOA_DR2R          (PIN_DRxR_ENABLE(GPIOA_UART0_RX) |           \
                                 PIN_DRxR_ENABLE(GPIOA_UART0_TX) |           \
                                 PIN_DRxR_ENABLE(GPIOA_SSI0_CLK) |               \
                                 PIN_DRxR_ENABLE(GPIOA_PIN3) |               \
                                 PIN_DRxR_ENABLE(GPIOA_SSI0_RX) |               \
                                 PIN_DRxR_ENABLE(GPIOA_SSI0_TX) |               \
                                 PIN_DRxR_ENABLE(GPIOA_PIN6) |               \
                                 PIN_DRxR_ENABLE(GPIOA_PIN7))

#define VAL_GPIOA_DR4R          (PIN_DRxR_DISABLE(GPIOA_UART0_RX) |          \
                                 PIN_DRxR_DISABLE(GPIOA_UART0_TX) |          \
                                 PIN_DRxR_DISABLE(GPIOA_SSI0_CLK) |              \
                                 PIN_DRxR_DISABLE(GPIOA_PIN3) |              \
                                 PIN_DRxR_DISABLE(GPIOA_SSI0_RX) |              \
                                 PIN_DRxR_DISABLE(GPIOA_SSI0_TX) |              \
                                 PIN_DRxR_DISABLE(GPIOA_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOA_PIN7))

#define VAL_GPIOA_DR8R          (PIN_DRxR_DISABLE(GPIOA_UART0_RX) |          \
                                 PIN_DRxR_DISABLE(GPIOA_UART0_TX) |          \
                                 PIN_DRxR_DISABLE(GPIOA_SSI0_CLK) |              \
                                 PIN_DRxR_DISABLE(GPIOA_PIN3) |              \
                                 PIN_DRxR_DISABLE(GPIOA_SSI0_RX) |              \
                                 PIN_DRxR_DISABLE(GPIOA_SSI0_TX) |              \
                                 PIN_DRxR_DISABLE(GPIOA_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOA_PIN7))


#define VAL_GPIOA_SLR           (PIN_SLR_DISABLE(GPIOA_UART0_RX) |           \
                                 PIN_SLR_DISABLE(GPIOA_UART0_TX) |           \
                                 PIN_SLR_DISABLE(GPIOA_SSI0_CLK) |               \
                                 PIN_SLR_DISABLE(GPIOA_PIN3) |               \
                                 PIN_SLR_DISABLE(GPIOA_SSI0_RX) |               \
                                 PIN_SLR_DISABLE(GPIOA_SSI0_TX) |               \
                                 PIN_SLR_DISABLE(GPIOA_PIN6) |               \
                                 PIN_SLR_DISABLE(GPIOA_PIN7))

#define VAL_GPIOA_PCTL          (PIN_PCTL_MODE(GPIOA_UART0_RX, 0) |          \
                                 PIN_PCTL_MODE(GPIOA_UART0_TX, 0) |          \
                                 PIN_PCTL_MODE(GPIOA_SSI0_CLK, 0) |              \
                                 PIN_PCTL_MODE(GPIOA_PIN3, 0) |              \
                                 PIN_PCTL_MODE(GPIOA_SSI0_RX, 0) |              \
                                 PIN_PCTL_MODE(GPIOA_SSI0_TX, 0) |              \
                                 PIN_PCTL_MODE(GPIOA_PIN6, 0) |              \
                                 PIN_PCTL_MODE(GPIOA_PIN7, 0))

/*
 * GPIOB Setup:
 *
 * PB0 - PIN0                   ()
 * PB1 - PIN1                   ()
 * PB2 - I2C0_SCL               ()
 * PB3 - I2C0_SDA               ()
 * PB4 - PIN4                   ()
 * PB5 - PIN5                   ()
 * PB6 - PIN6                   ()
 * PB7 - PIN7                   ()
 */
#define VAL_GPIOB_DATA          (PIN_DATA_LOW(GPIOB_PIN0) |                  \
                                 PIN_DATA_LOW(GPIOB_PIN1) |                  \
                                 PIN_DATA_LOW(GPIOB_I2C0_SCL) |              \
                                 PIN_DATA_LOW(GPIOB_I2C0_SDA) |              \
                                 PIN_DATA_LOW(GPIOB_PIN4) |                  \
                                 PIN_DATA_LOW(GPIOB_PIN5) |                  \
                                 PIN_DATA_LOW(GPIOB_PIN6) |                  \
                                 PIN_DATA_LOW(GPIOB_PIN7))

#define VAL_GPIOB_DIR           (PIN_DIR_IN(GPIOB_PIN0) |                    \
                                 PIN_DIR_IN(GPIOB_PIN1) |                    \
                                 PIN_DIR_IN(GPIOB_I2C0_SCL) |                \
                                 PIN_DIR_IN(GPIOB_I2C0_SDA) |                \
                                 PIN_DIR_IN(GPIOB_PIN4) |                    \
                                 PIN_DIR_IN(GPIOB_PIN5) |                    \
                                 PIN_DIR_IN(GPIOB_PIN6) |                    \
                                 PIN_DIR_IN(GPIOB_PIN7))

#define VAL_GPIOB_AFSEL         (PIN_AFSEL_GPIO(GPIOB_PIN0) |                \
                                 PIN_AFSEL_GPIO(GPIOB_PIN1) |                \
                                 PIN_AFSEL_GPIO(GPIOB_I2C0_SCL) |            \
                                 PIN_AFSEL_GPIO(GPIOB_I2C0_SDA) |            \
                                 PIN_AFSEL_GPIO(GPIOB_PIN4) |                \
                                 PIN_AFSEL_GPIO(GPIOB_PIN5) |                \
                                 PIN_AFSEL_GPIO(GPIOB_PIN6) |                \
                                 PIN_AFSEL_GPIO(GPIOB_PIN7))

#define VAL_GPIOB_DR2R          (PIN_DRxR_ENABLE(GPIOB_PIN0) |               \
                                 PIN_DRxR_ENABLE(GPIOB_PIN1) |               \
                                 PIN_DRxR_ENABLE(GPIOB_I2C0_SCL) |           \
                                 PIN_DRxR_ENABLE(GPIOB_I2C0_SDA) |           \
                                 PIN_DRxR_ENABLE(GPIOB_PIN4) |               \
                                 PIN_DRxR_ENABLE(GPIOB_PIN5) |               \
                                 PIN_DRxR_ENABLE(GPIOB_PIN6) |               \
                                 PIN_DRxR_ENABLE(GPIOB_PIN7))

#define VAL_GPIOB_DR4R          (PIN_DRxR_DISABLE(GPIOB_PIN0) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN1) |              \
                                 PIN_DRxR_DISABLE(GPIOB_I2C0_SCL) |          \
                                 PIN_DRxR_DISABLE(GPIOB_I2C0_SDA) |          \
                                 PIN_DRxR_DISABLE(GPIOB_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN7))

#define VAL_GPIOB_DR8R          (PIN_DRxR_DISABLE(GPIOB_PIN0) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN1) |              \
                                 PIN_DRxR_DISABLE(GPIOB_I2C0_SCL) |          \
                                 PIN_DRxR_DISABLE(GPIOB_I2C0_SDA) |          \
                                 PIN_DRxR_DISABLE(GPIOB_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOB_PIN7))

#define VAL_GPIOB_ODR           (PIN_ODR_DISABLE(GPIOB_PIN0) |               \
                                 PIN_ODR_DISABLE(GPIOB_PIN1) |               \
                                 PIN_ODR_DISABLE(GPIOB_I2C0_SCL) |           \
                                 PIN_ODR_DISABLE(GPIOB_I2C0_SDA) |           \
                                 PIN_ODR_DISABLE(GPIOB_PIN4) |               \
                                 PIN_ODR_DISABLE(GPIOB_PIN5) |               \
                                 PIN_ODR_DISABLE(GPIOB_PIN6) |               \
                                 PIN_ODR_DISABLE(GPIOB_PIN7))

#define VAL_GPIOB_PUR           (PIN_PxR_DISABLE(GPIOB_PIN0) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN1) |               \
                                 PIN_PxR_DISABLE(GPIOB_I2C0_SCL) |           \
                                 PIN_PxR_DISABLE(GPIOB_I2C0_SDA) |           \
                                 PIN_PxR_DISABLE(GPIOB_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN7))

#define VAL_GPIOB_PDR           (PIN_PxR_DISABLE(GPIOB_PIN0) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN1) |               \
                                 PIN_PxR_DISABLE(GPIOB_I2C0_SCL) |           \
                                 PIN_PxR_DISABLE(GPIOB_I2C0_SDA) |           \
                                 PIN_PxR_DISABLE(GPIOB_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOB_PIN7))

#define VAL_GPIOB_SLR           (PIN_SLR_DISABLE(GPIOB_PIN0) |               \
                                 PIN_SLR_DISABLE(GPIOB_PIN1) |               \
                                 PIN_SLR_DISABLE(GPIOB_I2C0_SCL) |           \
                                 PIN_SLR_DISABLE(GPIOB_I2C0_SDA) |           \
                                 PIN_SLR_DISABLE(GPIOB_PIN4) |               \
                                 PIN_SLR_DISABLE(GPIOB_PIN5) |               \
                                 PIN_SLR_DISABLE(GPIOB_PIN6) |               \
                                 PIN_SLR_DISABLE(GPIOB_PIN7))

#define VAL_GPIOB_DEN           (PIN_DEN_ENABLE(GPIOB_PIN0) |                \
                                 PIN_DEN_ENABLE(GPIOB_PIN1) |                \
                                 PIN_DEN_ENABLE(GPIOB_I2C0_SCL) |            \
                                 PIN_DEN_ENABLE(GPIOB_I2C0_SDA) |            \
                                 PIN_DEN_ENABLE(GPIOB_PIN4) |                \
                                 PIN_DEN_ENABLE(GPIOB_PIN5) |                \
                                 PIN_DEN_ENABLE(GPIOB_PIN6) |                \
                                 PIN_DEN_ENABLE(GPIOB_PIN7))

#define VAL_GPIOB_AMSEL         (PIN_AMSEL_DISABLE(GPIOB_PIN0) |             \
                                 PIN_AMSEL_DISABLE(GPIOB_PIN1) |             \
                                 PIN_AMSEL_DISABLE(GPIOB_I2C0_SCL) |         \
                                 PIN_AMSEL_DISABLE(GPIOB_I2C0_SDA))

#define VAL_GPIOB_PCTL          (PIN_PCTL_MODE(GPIOB_PIN0, 0) |              \
                                 PIN_PCTL_MODE(GPIOB_PIN1, 0) |              \
                                 PIN_PCTL_MODE(GPIOB_I2C0_SCL, 0) |          \
                                 PIN_PCTL_MODE(GPIOB_I2C0_SDA, 0) |          \
                                 PIN_PCTL_MODE(GPIOB_PIN4, 0) |              \
                                 PIN_PCTL_MODE(GPIOB_PIN5, 0) |              \
                                 PIN_PCTL_MODE(GPIOB_PIN6, 0) |              \
                                 PIN_PCTL_MODE(GPIOB_PIN7, 0))

/*
 * GPIOC Setup:
 *
 * PC0 - TCK_SWCLK              (alternate 1)
 * PC1 - TMS_SWDIO              (alternate 1)
 * PC2 - TDI                    (alternate 1)
 * PC3 - TDO_SWO                (alternate 1)
 * PC4 - PIN4                   ()
 * PC5 - PIN5                   ()
 * PC6 - PIN6                   ()
 * PC7 - PIN7                   ()
 */

#define VAL_GPIOC_DATA          (PIN_DATA_LOW(GPIOC_TCK_SWCLK) |             \
                                 PIN_DATA_LOW(GPIOC_TMS_SWDIO) |             \
                                 PIN_DATA_LOW(GPIOC_TDI) |                   \
                                 PIN_DATA_LOW(GPIOC_TDO_SWO) |               \
                                 PIN_DATA_LOW(GPIOC_PIN4) |                  \
                                 PIN_DATA_LOW(GPIOC_PIN5) |                  \
                                 PIN_DATA_LOW(GPIOC_PIN6) |                  \
                                 PIN_DATA_LOW(GPIOC_PIN7))

#define VAL_GPIOC_DIR           (PIN_DIR_IN(GPIOC_TCK_SWCLK) |               \
                                 PIN_DIR_IN(GPIOC_TMS_SWDIO) |               \
                                 PIN_DIR_IN(GPIOC_TDI) |                     \
                                 PIN_DIR_OUT(GPIOC_TDO_SWO) |                \
                                 PIN_DIR_IN(GPIOC_PIN4) |                    \
                                 PIN_DIR_IN(GPIOC_PIN5) |                    \
                                 PIN_DIR_IN(GPIOC_PIN6) |                    \
                                 PIN_DIR_IN(GPIOC_PIN7))

#define VAL_GPIOC_AFSEL         (PIN_AFSEL_ALTERNATE(GPIOC_TCK_SWCLK) |      \
                                 PIN_AFSEL_ALTERNATE(GPIOC_TMS_SWDIO) |      \
                                 PIN_AFSEL_ALTERNATE(GPIOC_TDI) |            \
                                 PIN_AFSEL_ALTERNATE(GPIOC_TDO_SWO) |        \
                                 PIN_AFSEL_GPIO(GPIOC_PIN4) |                \
                                 PIN_AFSEL_GPIO(GPIOC_PIN5) |                \
                                 PIN_AFSEL_GPIO(GPIOC_PIN6) |                \
                                 PIN_AFSEL_GPIO(GPIOC_PIN7))

#define VAL_GPIOC_DR2R          (PIN_DRxR_ENABLE(GPIOC_TCK_SWCLK) |          \
                                 PIN_DRxR_ENABLE(GPIOC_TMS_SWDIO) |          \
                                 PIN_DRxR_ENABLE(GPIOC_TDI) |                \
                                 PIN_DRxR_ENABLE(GPIOC_TDO_SWO) |            \
                                 PIN_DRxR_ENABLE(GPIOC_PIN4) |               \
                                 PIN_DRxR_ENABLE(GPIOC_PIN5) |               \
                                 PIN_DRxR_ENABLE(GPIOC_PIN6) |               \
                                 PIN_DRxR_ENABLE(GPIOC_PIN7))

#define VAL_GPIOC_DR4R          (PIN_DRxR_DISABLE(GPIOC_TCK_SWCLK) |         \
                                 PIN_DRxR_DISABLE(GPIOC_TMS_SWDIO) |         \
                                 PIN_DRxR_DISABLE(GPIOC_TDI) |               \
                                 PIN_DRxR_DISABLE(GPIOC_TDO_SWO) |           \
                                 PIN_DRxR_DISABLE(GPIOC_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOC_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOC_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOC_PIN7))

#define VAL_GPIOC_DR8R          (PIN_DRxR_DISABLE(GPIOC_TCK_SWCLK) |         \
                                 PIN_DRxR_DISABLE(GPIOC_TMS_SWDIO) |         \
                                 PIN_DRxR_DISABLE(GPIOC_TDI) |               \
                                 PIN_DRxR_DISABLE(GPIOC_TDO_SWO) |           \
                                 PIN_DRxR_DISABLE(GPIOC_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOC_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOC_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOC_PIN7))

#define VAL_GPIOC_ODR           (PIN_ODR_DISABLE(GPIOC_TCK_SWCLK) |          \
                                 PIN_ODR_DISABLE(GPIOC_TMS_SWDIO) |          \
                                 PIN_ODR_DISABLE(GPIOC_TDI) |                \
                                 PIN_ODR_DISABLE(GPIOC_TDO_SWO) |            \
                                 PIN_ODR_DISABLE(GPIOC_PIN4) |               \
                                 PIN_ODR_DISABLE(GPIOC_PIN5) |               \
                                 PIN_ODR_DISABLE(GPIOC_PIN6) |               \
                                 PIN_ODR_DISABLE(GPIOC_PIN7))

#define VAL_GPIOC_PUR           (PIN_PxR_DISABLE(GPIOC_TCK_SWCLK) |          \
                                 PIN_PxR_DISABLE(GPIOC_TMS_SWDIO) |          \
                                 PIN_PxR_DISABLE(GPIOC_TDI) |                \
                                 PIN_PxR_DISABLE(GPIOC_TDO_SWO) |            \
                                 PIN_PxR_DISABLE(GPIOC_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOC_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOC_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOC_PIN7))

#define VAL_GPIOC_PDR           (PIN_PxR_DISABLE(GPIOC_TCK_SWCLK) |          \
                                 PIN_PxR_DISABLE(GPIOC_TMS_SWDIO) |          \
                                 PIN_PxR_DISABLE(GPIOC_TDI) |                \
                                 PIN_PxR_DISABLE(GPIOC_TDO_SWO) |            \
                                 PIN_PxR_DISABLE(GPIOC_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOC_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOC_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOC_PIN7))

#define VAL_GPIOC_SLR           (PIN_SLR_DISABLE(GPIOC_TCK_SWCLK) |          \
                                 PIN_SLR_DISABLE(GPIOC_TMS_SWDIO) |          \
                                 PIN_SLR_DISABLE(GPIOC_TDI) |                \
                                 PIN_SLR_DISABLE(GPIOC_TDO_SWO) |            \
                                 PIN_SLR_DISABLE(GPIOC_PIN4) |               \
                                 PIN_SLR_DISABLE(GPIOC_PIN5) |               \
                                 PIN_SLR_DISABLE(GPIOC_PIN6) |               \
                                 PIN_SLR_DISABLE(GPIOC_PIN7))

#define VAL_GPIOC_DEN           (PIN_DEN_ENABLE(GPIOC_TCK_SWCLK) |           \
                                 PIN_DEN_ENABLE(GPIOC_TMS_SWDIO) |           \
                                 PIN_DEN_ENABLE(GPIOC_TDI) |                 \
                                 PIN_DEN_ENABLE(GPIOC_TDO_SWO) |             \
                                 PIN_DEN_ENABLE(GPIOC_PIN4) |                \
                                 PIN_DEN_ENABLE(GPIOC_PIN5) |                \
                                 PIN_DEN_ENABLE(GPIOC_PIN6) |                \
                                 PIN_DEN_ENABLE(GPIOC_PIN7))

#define VAL_GPIOC_AMSEL         (PIN_AMSEL_DISABLE(GPIOC_TCK_SWCLK) |        \
                                 PIN_AMSEL_DISABLE(GPIOC_TMS_SWDIO) |        \
                                 PIN_AMSEL_DISABLE(GPIOC_TDI) |              \
                                 PIN_AMSEL_DISABLE(GPIOC_TDO_SWO))

#define VAL_GPIOC_PCTL          (PIN_PCTL_MODE(GPIOC_TCK_SWCLK, 1) |         \
                                 PIN_PCTL_MODE(GPIOC_TMS_SWDIO, 1) |         \
                                 PIN_PCTL_MODE(GPIOC_TDI, 1) |               \
                                 PIN_PCTL_MODE(GPIOC_TDO_SWO, 1) |           \
                                 PIN_PCTL_MODE(GPIOC_PIN4, 0) |              \
                                 PIN_PCTL_MODE(GPIOC_PIN5, 0) |              \
                                 PIN_PCTL_MODE(GPIOC_PIN6, 0) |              \
                                 PIN_PCTL_MODE(GPIOC_PIN7, 0))

/*
 * GPIOD Setup:
 *
 * PD0 - PIN0                   ()
 * PD1 - PIN1                   ()
 * PD2 - PIN2                   ()
 * PD3 - PIN3                   ()
 * PD4 - PIN4                   ()
 * PD5 - PIN5                   ()
 * PD6 - PIN6                   ()
 * PD7 - PIN7                   ()
 */
#define VAL_GPIOD_DATA          (PIN_DATA_LOW(GPIOD_PIN0) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN1) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN2) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN3) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN4) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN5) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN6) |                  \
                                 PIN_DATA_LOW(GPIOD_PIN7))

#define VAL_GPIOD_DIR           (PIN_DIR_IN(GPIOD_PIN0) |                    \
                                 PIN_DIR_IN(GPIOD_PIN1) |                    \
                                 PIN_DIR_IN(GPIOD_PIN2) |                    \
                                 PIN_DIR_IN(GPIOD_PIN3) |                    \
                                 PIN_DIR_IN(GPIOD_PIN4) |                    \
                                 PIN_DIR_IN(GPIOD_PIN5) |                    \
                                 PIN_DIR_IN(GPIOD_PIN6) |                    \
                                 PIN_DIR_IN(GPIOD_PIN7))

#define VAL_GPIOD_AFSEL         (PIN_AFSEL_GPIO(GPIOD_PIN0) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN1) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN2) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN3) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN4) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN5) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN6) |                \
                                 PIN_AFSEL_GPIO(GPIOD_PIN7))

#define VAL_GPIOD_DR2R          (PIN_DRxR_ENABLE(GPIOD_PIN0) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN1) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN2) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN3) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN4) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN5) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN6) |               \
                                 PIN_DRxR_ENABLE(GPIOD_PIN7))

#define VAL_GPIOD_DR4R          (PIN_DRxR_DISABLE(GPIOD_PIN0) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN1) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN2) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN3) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN7))

#define VAL_GPIOD_DR8R          (PIN_DRxR_DISABLE(GPIOD_PIN0) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN1) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN2) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN3) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOD_PIN7))

#define VAL_GPIOD_ODR           (PIN_ODR_DISABLE(GPIOD_PIN0) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN1) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN2) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN3) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN4) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN5) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN6) |               \
                                 PIN_ODR_DISABLE(GPIOD_PIN7))

#define VAL_GPIOD_PUR           (PIN_PxR_DISABLE(GPIOD_PIN0) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN1) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN2) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN3) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN7))

#define VAL_GPIOD_PDR           (PIN_PxR_DISABLE(GPIOD_PIN0) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN1) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN2) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN3) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOD_PIN7))

#define VAL_GPIOD_SLR           (PIN_SLR_DISABLE(GPIOD_PIN0) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN1) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN2) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN3) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN4) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN5) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN6) |               \
                                 PIN_SLR_DISABLE(GPIOD_PIN7))

#define VAL_GPIOD_DEN           (PIN_DEN_ENABLE(GPIOD_PIN0) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN1) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN2) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN3) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN4) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN5) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN6) |                \
                                 PIN_DEN_ENABLE(GPIOD_PIN7))

#define VAL_GPIOD_AMSEL         (PIN_AMSEL_DISABLE(GPIOD_PIN0) |             \
                                 PIN_AMSEL_DISABLE(GPIOD_PIN1) |             \
                                 PIN_AMSEL_DISABLE(GPIOD_PIN2) |             \
                                 PIN_AMSEL_DISABLE(GPIOD_PIN3))

#define VAL_GPIOD_PCTL          (PIN_PCTL_MODE(GPIOD_PIN0, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN1, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN2, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN3, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN4, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN5, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN6, 0) |              \
                                 PIN_PCTL_MODE(GPIOD_PIN7, 0))

/*
 * GPIOE Setup:
 *
 * PE0 - PIN0                   ()
 * PE1 - PIN1                   ()
 * PE2 - PIN2                   ()
 * PE3 - PIN3                   ()
 * PE4 - PIN4                   ()
 * PE5 - PIN5                   ()
 * PE6 - PIN6                   ()
 * PE7 - PIN7                   ()
 */
#define VAL_GPIOE_DATA          (PIN_DATA_LOW(GPIOE_PIN0) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN1) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN2) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN3) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN4) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN5) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN6) |                  \
                                 PIN_DATA_LOW(GPIOE_PIN7))

#define VAL_GPIOE_DIR           (PIN_DIR_IN(GPIOE_PIN0) |                    \
                                 PIN_DIR_IN(GPIOE_PIN1) |                    \
                                 PIN_DIR_IN(GPIOE_PIN2) |                    \
                                 PIN_DIR_IN(GPIOE_PIN3) |                    \
                                 PIN_DIR_IN(GPIOE_PIN4) |                    \
                                 PIN_DIR_IN(GPIOE_PIN5) |                    \
                                 PIN_DIR_IN(GPIOE_PIN6) |                    \
                                 PIN_DIR_IN(GPIOE_PIN7))

#define VAL_GPIOE_AFSEL         (PIN_AFSEL_GPIO(GPIOE_PIN0) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN1) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN2) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN3) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN4) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN5) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN6) |                \
                                 PIN_AFSEL_GPIO(GPIOE_PIN7))

#define VAL_GPIOE_DR2R          (PIN_DRxR_ENABLE(GPIOE_PIN0) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN1) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN2) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN3) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN4) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN5) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN6) |               \
                                 PIN_DRxR_ENABLE(GPIOE_PIN7))

#define VAL_GPIOE_DR4R          (PIN_DRxR_DISABLE(GPIOE_PIN0) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN1) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN2) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN3) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN7))

#define VAL_GPIOE_DR8R          (PIN_DRxR_DISABLE(GPIOE_PIN0) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN1) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN2) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN3) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN4) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOE_PIN7))

#define VAL_GPIOE_ODR           (PIN_ODR_DISABLE(GPIOE_PIN0) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN1) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN2) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN3) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN4) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN5) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN6) |               \
                                 PIN_ODR_DISABLE(GPIOE_PIN7))

#define VAL_GPIOE_PUR           (PIN_PxR_DISABLE(GPIOE_PIN0) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN1) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN2) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN3) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN7))

#define VAL_GPIOE_PDR           (PIN_PxR_DISABLE(GPIOE_PIN0) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN1) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN2) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN3) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN4) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOE_PIN7))

#define VAL_GPIOE_SLR           (PIN_SLR_DISABLE(GPIOE_PIN0) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN1) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN2) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN3) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN4) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN5) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN6) |               \
                                 PIN_SLR_DISABLE(GPIOE_PIN7))

#define VAL_GPIOE_DEN           (PIN_DEN_ENABLE(GPIOE_PIN0) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN1) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN2) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN3) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN4) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN5) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN6) |                \
                                 PIN_DEN_ENABLE(GPIOE_PIN7))

#define VAL_GPIOE_AMSEL         (PIN_AMSEL_DISABLE(GPIOE_PIN0) |             \
                                 PIN_AMSEL_DISABLE(GPIOE_PIN1) |             \
                                 PIN_AMSEL_DISABLE(GPIOE_PIN2) |             \
                                 PIN_AMSEL_DISABLE(GPIOE_PIN3))

#define VAL_GPIOE_PCTL          (PIN_PCTL_MODE(GPIOE_PIN0, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN1, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN2, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN3, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN4, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN5, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN6, 0) |              \
                                 PIN_PCTL_MODE(GPIOE_PIN7, 0))

/*
 * GPIOF Setup:
 *
 * PF0 - SW2                    ()
 * PF1 - LED_RED                ()
 * PF2 - LED_BLUE               ()
 * PF3 - LED_GREEN              ()
 * PF4 - SW1                    ()
 * PF5 - PIN5                   ()
 * PF6 - PIN6                   ()
 * PF7 - PIN7                   ()
 */

#define VAL_GPIOF_DATA          (PIN_DATA_LOW(GPIOF_SW2) |                   \
                                 PIN_DATA_LOW(GPIOF_LED_RED) |               \
                                 PIN_DATA_LOW(GPIOF_LED_BLUE) |              \
                                 PIN_DATA_LOW(GPIOF_LED_GREEN) |             \
                                 PIN_DATA_LOW(GPIOF_SW1) |                   \
                                 PIN_DATA_LOW(GPIOF_PIN5) |                  \
                                 PIN_DATA_LOW(GPIOF_PIN6) |                  \
                                 PIN_DATA_LOW(GPIOF_PIN7))

#define VAL_GPIOF_DIR           (PIN_DIR_IN(GPIOF_SW2) |                     \
                                 PIN_DIR_IN(GPIOF_LED_RED) |                 \
                                 PIN_DIR_IN(GPIOF_LED_BLUE) |                \
                                 PIN_DIR_IN(GPIOF_LED_GREEN) |               \
                                 PIN_DIR_IN(GPIOF_SW1) |                     \
                                 PIN_DIR_IN(GPIOF_PIN5) |                    \
                                 PIN_DIR_IN(GPIOF_PIN6) |                    \
                                 PIN_DIR_IN(GPIOF_PIN7))

#define VAL_GPIOF_AFSEL         (PIN_AFSEL_GPIO(GPIOF_SW2) |                 \
                                 PIN_AFSEL_GPIO(GPIOF_LED_RED) |             \
                                 PIN_AFSEL_GPIO(GPIOF_LED_BLUE) |            \
                                 PIN_AFSEL_GPIO(GPIOF_LED_GREEN) |           \
                                 PIN_AFSEL_GPIO(GPIOF_SW1) |                 \
                                 PIN_AFSEL_GPIO(GPIOF_PIN5) |                \
                                 PIN_AFSEL_GPIO(GPIOF_PIN6) |                \
                                 PIN_AFSEL_GPIO(GPIOF_PIN7))

#define VAL_GPIOF_DR2R          (PIN_DRxR_ENABLE(GPIOF_SW2) |                \
                                 PIN_DRxR_ENABLE(GPIOF_LED_RED) |            \
                                 PIN_DRxR_ENABLE(GPIOF_LED_BLUE) |           \
                                 PIN_DRxR_ENABLE(GPIOF_LED_GREEN) |          \
                                 PIN_DRxR_ENABLE(GPIOF_SW1) |                \
                                 PIN_DRxR_ENABLE(GPIOF_PIN5) |               \
                                 PIN_DRxR_ENABLE(GPIOF_PIN6) |               \
                                 PIN_DRxR_ENABLE(GPIOF_PIN7))

#define VAL_GPIOF_DR4R          (PIN_DRxR_DISABLE(GPIOF_SW2) |               \
                                 PIN_DRxR_DISABLE(GPIOF_LED_RED) |           \
                                 PIN_DRxR_DISABLE(GPIOF_LED_BLUE) |          \
                                 PIN_DRxR_DISABLE(GPIOF_LED_GREEN) |         \
                                 PIN_DRxR_DISABLE(GPIOF_SW1) |               \
                                 PIN_DRxR_DISABLE(GPIOF_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOF_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOF_PIN7))

#define VAL_GPIOF_DR8R          (PIN_DRxR_DISABLE(GPIOF_SW2) |               \
                                 PIN_DRxR_DISABLE(GPIOF_LED_RED) |           \
                                 PIN_DRxR_DISABLE(GPIOF_LED_BLUE) |          \
                                 PIN_DRxR_DISABLE(GPIOF_LED_GREEN) |         \
                                 PIN_DRxR_DISABLE(GPIOF_SW1) |               \
                                 PIN_DRxR_DISABLE(GPIOF_PIN5) |              \
                                 PIN_DRxR_DISABLE(GPIOF_PIN6) |              \
                                 PIN_DRxR_DISABLE(GPIOF_PIN7))

#define VAL_GPIOF_ODR           (PIN_ODR_DISABLE(GPIOF_SW2) |                \
                                 PIN_ODR_DISABLE(GPIOF_LED_RED) |            \
                                 PIN_ODR_DISABLE(GPIOF_LED_BLUE) |           \
                                 PIN_ODR_DISABLE(GPIOF_LED_GREEN) |          \
                                 PIN_ODR_DISABLE(GPIOF_SW1) |                \
                                 PIN_ODR_DISABLE(GPIOF_PIN5) |               \
                                 PIN_ODR_DISABLE(GPIOF_PIN6) |               \
                                 PIN_ODR_DISABLE(GPIOF_PIN7))

#define VAL_GPIOF_PUR           (PIN_PxR_DISABLE(GPIOF_SW2) |                \
                                 PIN_PxR_DISABLE(GPIOF_LED_RED) |            \
                                 PIN_PxR_DISABLE(GPIOF_LED_BLUE) |           \
                                 PIN_PxR_DISABLE(GPIOF_LED_GREEN) |          \
                                 PIN_PxR_DISABLE(GPIOF_SW1) |                \
                                 PIN_PxR_DISABLE(GPIOF_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOF_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOF_PIN7))

#define VAL_GPIOF_PDR           (PIN_PxR_DISABLE(GPIOF_SW2) |                \
                                 PIN_PxR_DISABLE(GPIOF_LED_RED) |            \
                                 PIN_PxR_DISABLE(GPIOF_LED_BLUE) |           \
                                 PIN_PxR_DISABLE(GPIOF_LED_GREEN) |          \
                                 PIN_PxR_DISABLE(GPIOF_SW1) |                \
                                 PIN_PxR_DISABLE(GPIOF_PIN5) |               \
                                 PIN_PxR_DISABLE(GPIOF_PIN6) |               \
                                 PIN_PxR_DISABLE(GPIOF_PIN7))

#define VAL_GPIOF_SLR           (PIN_SLR_DISABLE(GPIOF_SW2) |                \
                                 PIN_SLR_DISABLE(GPIOF_LED_RED) |            \
                                 PIN_SLR_DISABLE(GPIOF_LED_BLUE) |           \
                                 PIN_SLR_DISABLE(GPIOF_LED_GREEN) |          \
                                 PIN_SLR_DISABLE(GPIOF_SW1) |                \
                                 PIN_SLR_DISABLE(GPIOF_PIN5) |               \
                                 PIN_SLR_DISABLE(GPIOF_PIN6) |               \
                                 PIN_SLR_DISABLE(GPIOF_PIN7))

#define VAL_GPIOF_DEN           (PIN_DEN_ENABLE(GPIOF_SW2) |                 \
                                 PIN_DEN_ENABLE(GPIOF_LED_RED) |             \
                                 PIN_DEN_ENABLE(GPIOF_LED_BLUE) |            \
                                 PIN_DEN_ENABLE(GPIOF_LED_GREEN) |           \
                                 PIN_DEN_ENABLE(GPIOF_SW1) |                 \
                                 PIN_DEN_ENABLE(GPIOF_PIN5) |                \
                                 PIN_DEN_ENABLE(GPIOF_PIN6) |                \
                                 PIN_DEN_ENABLE(GPIOF_PIN7))

#define VAL_GPIOF_AMSEL         (PIN_AMSEL_DISABLE(GPIOF_SW2) |              \
                                 PIN_AMSEL_DISABLE(GPIOF_LED_RED) |          \
                                 PIN_AMSEL_DISABLE(GPIOF_LED_BLUE) |         \
                                 PIN_AMSEL_DISABLE(GPIOF_LED_GREEN))

#define VAL_GPIOF_PCTL          (PIN_PCTL_MODE(GPIOF_SW2, 0) |               \
                                 PIN_PCTL_MODE(GPIOF_LED_RED, 0) |           \
                                 PIN_PCTL_MODE(GPIOF_LED_BLUE, 0) |          \
                                 PIN_PCTL_MODE(GPIOF_LED_GREEN, 0) |         \
                                 PIN_PCTL_MODE(GPIOF_SW1, 0) |               \
                                 PIN_PCTL_MODE(GPIOF_PIN5, 0) |              \
                                 PIN_PCTL_MODE(GPIOF_PIN6, 0) |              \
                                 PIN_PCTL_MODE(GPIOF_PIN7, 0))

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
