/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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
 * Setup for STMicroelectronics STM32F4-Discovery board.
 */

/*
 * Board identifier.
 */
#define BOARD_NAME                  "NAND and SRAM test board (codename Buod)"

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                12000000
#endif

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   300

/*
 * MCU type, supported types are defined in ./os/hal/platforms/hal_lld.h.
 */
#define STM32F407xx

/*
 * IO pins assignments.
 */
#define GPIOA_PIN0                  0
#define GPIOA_PIN1                  1
#define GPIOA_PIN2                  2
#define GPIOA_PIN3                  3
#define GPIOA_SPI1_NSS              4
#define GPIOA_PIN5                  5
#define GPIOA_PIN6                  6
#define GPIOA_PIN7                  7
#define GPIOA_PIN8                  8
#define GPIOA_USB_PRESENT           9
#define GPIOA_PIN10                 10
#define GPIOA_OTG_FS_DM             11
#define GPIOA_OTG_FS_DP             12
#define GPIOA_JTMS                  13
#define GPIOA_JTCK                  14
#define GPIOA_JTDI                  15



#define GPIOA_USB_PRESENT           9
#define GPIOA_PIN10                 10
#define GPIOA_OTG_FS_DM             11
#define GPIOA_OTG_FS_DP             12


#define GPIOB_PIN0                  0
#define GPIOB_NAND_WP               1
#define GPIOB_PIN2                  2
#define GPIOB_JTDO                  3
#define GPIOB_JTRST                 4
#define GPIOB_NVRAM_PWR             5
#define GPIOB_PIN6                  6
#define GPIOB_PIN7                  7
#define GPIOB_PIN8                  8
#define GPIOB_PIN9                  9
#define GPIOB_PIN10                 10
#define GPIOB_PIN11                 11
#define GPIOB_PIN12                 12
#define GPIOB_PIN13                 13
#define GPIOB_PIN14                 14
#define GPIOB_PIN15                 15

#define GPIOC_PIN0                  0
#define GPIOC_PIN1                  1
#define GPIOC_PIN2                  2
#define GPIOC_PIN3                  3
#define GPIOC_PIN4                  4
#define GPIOC_PIN5                  5
#define GPIOC_PIN6                  6
#define GPIOC_PIN7                  7
#define GPIOC_PIN8                  8
#define GPIOC_PIN9                  9
#define GPIOC_PIN10                 10
#define GPIOC_PIN11                 11
#define GPIOC_PIN12                 12
#define GPIOC_PIN13                 13
#define GPIOC_PIN14                 14
#define GPIOC_PIN15                 15

#define GPIOD_MEM_D2                0
#define GPIOD_MEM_D3                1
#define GPIOD_PIN2                  2
#define GPIOD_PIN3                  3
#define GPIOD_MEM_OE                4
#define GPIOD_MEM_WE                5
#define GPIOD_NAND_RB_NWAIT         6
#define GPIOD_NAND_CE1              7
#define GPIOD_MEM_D13               8
#define GPIOD_MEM_D14               9
#define GPIOD_MEM_D15               10
#define GPIOD_MEM_A16               11
#define GPIOD_MEM_A17               12
#define GPIOD_PIN13                 13
#define GPIOD_MEM_D0                14
#define GPIOD_MEM_D1                15

#define GPIOE_SRAM_LB               0
#define GPIOE_SRAM_UB               1
#define GPIOE_PIN2                  2
#define GPIOE_PIN3                  3
#define GPIOE_PIN4                  4
#define GPIOE_PIN5                  5
#define GPIOE_PIN6                  6
#define GPIOE_MEM_D4                7
#define GPIOE_MEM_D5                8
#define GPIOE_MEM_D6                9
#define GPIOE_MEM_D7                10
#define GPIOE_MEM_D8                11
#define GPIOE_MEM_D9                12
#define GPIOE_MEM_D10               13
#define GPIOE_MEM_D11               14
#define GPIOE_MEM_D12               15

#define GPIOF_MEM_A0                0
#define GPIOF_MEM_A1                1
#define GPIOF_MEM_A2                2
#define GPIOF_MEM_A3                3
#define GPIOF_MEM_A4                4
#define GPIOF_MEM_A5                5
#define GPIOF_PIN6                  6
#define GPIOF_PIN7                  7
#define GPIOF_PIN8                  8
#define GPIOF_PIN9                  9
#define GPIOF_PIN10                 10
#define GPIOF_PIN11                 11
#define GPIOF_MEM_A6                12
#define GPIOF_MEM_A7                13
#define GPIOF_MEM_A8                14
#define GPIOF_MEM_A9                15

#define GPIOG_MEM_A10               0
#define GPIOG_MEM_A11               1
#define GPIOG_MEM_A12               2
#define GPIOG_MEM_A13               3
#define GPIOG_MEM_A14               4
#define GPIOG_MEM_A15               5
#define GPIOG_NAND_RB1              6
#define GPIOG_NAND_RB2              7
#define GPIOG_PIN8                  8
#define GPIOG_NAND_CE2              9
#define GPIOG_PIN10                 10
#define GPIOG_PIN11                 11
#define GPIOG_SRAM_CS1              12
#define GPIOG_PIN13                 13
#define GPIOG_PIN14                 14
#define GPIOG_PIN15                 15

#define GPIOH_OSC_IN                0
#define GPIOH_OSC_OUT               1
#define GPIOH_PIN2                  2
#define GPIOH_PIN3                  3
#define GPIOH_PIN4                  4
#define GPIOH_PIN5                  5
#define GPIOH_PIN6                  6
#define GPIOH_I2C3_SCL              7
#define GPIOH_I2C3_SDA              8
#define GPIOH_PIN9                  9
#define GPIOH_PIN10                 10
#define GPIOH_PIN11                 11
#define GPIOH_PIN12                 12
#define GPIOH_PIN13                 13
#define GPIOH_PIN14                 14
#define GPIOH_PIN15                 15

#define GPIOI_PIN0                  0
#define GPIOI_PIN1                  1
#define GPIOI_PIN2                  2
#define GPIOI_PIN3                  3
#define GPIOI_PIN4                  4
#define GPIOI_PIN5                  5
#define GPIOI_PIN6                  6
#define GPIOI_PIN7                  7
#define GPIOI_PIN8                  8
#define GPIOI_PIN9                  9
#define GPIOI_LED_R                 10
#define GPIOI_LED_G                 11
#define GPIOI_PIN12                 12
#define GPIOI_PIN13                 13
#define GPIOI_PIN14                 14
#define GPIOI_PIN15                 15

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * GPIOA setup:
 */
#define VAL_GPIOA_MODER             (PIN_MODE_INPUT(GPIOA_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOA_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOA_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOA_PIN3) |           \
                                     PIN_MODE_ALTERNATE(GPIOA_SPI1_NSS) |   \
                                     PIN_MODE_INPUT(GPIOA_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOA_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOA_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOA_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOA_USB_PRESENT) |    \
                                     PIN_MODE_INPUT(GPIOA_PIN10) |          \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DM) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DP) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_JTMS) |       \
                                     PIN_MODE_ALTERNATE(GPIOA_JTCK) |       \
                                     PIN_MODE_ALTERNATE(GPIOA_JTDI))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SPI1_NSS) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_PRESENT) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DM) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DP) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTMS) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTCK) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_JTDI))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_100M(GPIOA_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOA_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOA_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOA_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOA_SPI1_NSS) |      \
                                     PIN_OSPEED_100M(GPIOA_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOA_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOA_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOA_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOA_USB_PRESENT) |   \
                                     PIN_OSPEED_100M(GPIOA_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DM) |     \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DP) |     \
                                     PIN_OSPEED_100M(GPIOA_JTMS) |          \
                                     PIN_OSPEED_100M(GPIOA_JTCK) |          \
                                     PIN_OSPEED_100M(GPIOA_JTDI))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_PIN0) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN1) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_SPI1_NSS) |   \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_USB_PRESENT) |\
                                     PIN_PUPDR_FLOATING(GPIOA_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DM) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DP) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_JTMS) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_JTCK) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_JTDI))
#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOA_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOA_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOA_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOA_SPI1_NSS) |         \
                                     PIN_ODR_HIGH(GPIOA_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOA_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOA_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOA_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOA_USB_PRESENT) |      \
                                     PIN_ODR_HIGH(GPIOA_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DM) |        \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DP) |        \
                                     PIN_ODR_HIGH(GPIOA_JTMS) |             \
                                     PIN_ODR_HIGH(GPIOA_JTCK) |             \
                                     PIN_ODR_HIGH(GPIOA_JTDI))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOA_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOA_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOA_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOA_SPI1_NSS, 5) |       \
                                     PIN_AFIO_AF(GPIOA_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOA_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOA_PIN7, 0))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOA_USB_PRESENT, 0) |    \
                                     PIN_AFIO_AF(GPIOA_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_DM, 10) |     \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_DP, 10) |     \
                                     PIN_AFIO_AF(GPIOA_JTMS, 0) |           \
                                     PIN_AFIO_AF(GPIOA_JTCK, 0) |           \
                                     PIN_AFIO_AF(GPIOA_JTDI, 0))

/*
 * GPIOB setup:
 */
#define VAL_GPIOB_MODER             (PIN_MODE_INPUT(GPIOB_PIN0) |           \
                                     PIN_MODE_OUTPUT(GPIOB_NAND_WP) |       \
                                     PIN_MODE_INPUT(GPIOB_PIN2) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_JTDO) |       \
                                     PIN_MODE_ALTERNATE(GPIOB_JTRST) |      \
                                     PIN_MODE_OUTPUT(GPIOB_NVRAM_PWR) |     \
                                     PIN_MODE_INPUT(GPIOB_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN15))

#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_NAND_WP) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_JTDO) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_JTRST) |      \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_NVRAM_PWR) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN15))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_100M(GPIOB_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOB_NAND_WP) |       \
                                     PIN_OSPEED_100M(GPIOB_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOB_JTDO) |          \
                                     PIN_OSPEED_100M(GPIOB_JTRST) |         \
                                     PIN_OSPEED_2M(GPIOB_NVRAM_PWR) |       \
                                     PIN_OSPEED_100M(GPIOB_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOB_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOB_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOB_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOB_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOB_PIN15))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_PIN0) |       \
                                     PIN_PUPDR_PULLDOWN(GPIOB_NAND_WP) |    \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_JTDO) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_JTRST) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_NVRAM_PWR) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN15))
#define VAL_GPIOB_ODR               (PIN_ODR_HIGH(GPIOB_PIN0) |             \
                                     PIN_ODR_LOW(GPIOB_NAND_WP) |           \
                                     PIN_ODR_HIGH(GPIOB_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOB_JTDO) |             \
                                     PIN_ODR_HIGH(GPIOB_JTRST) |            \
                                     PIN_ODR_LOW(GPIOB_NVRAM_PWR) |         \
                                     PIN_ODR_HIGH(GPIOB_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOB_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN15))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOB_NAND_WP, 0) |        \
                                     PIN_AFIO_AF(GPIOB_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOB_JTDO, 0) |           \
                                     PIN_AFIO_AF(GPIOB_JTRST, 0) |          \
                                     PIN_AFIO_AF(GPIOB_NVRAM_PWR, 0) |      \
                                     PIN_AFIO_AF(GPIOB_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN7, 0))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOB_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOB_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOB_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOB_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOB_PIN15, 0))

/*
 * GPIOC setup:
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_100M(GPIOC_PIN0) |\
                                     PIN_OSPEED_100M(GPIOC_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOC_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_FLOATING(GPIOC_PIN0) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN1) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOC_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOC_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOC_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOC_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOC_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOC_PIN15, 0))

/*
 * GPIOD setup:
 */
#define VAL_GPIOD_MODER             (PIN_MODE_ALTERNATE(GPIOD_MEM_D2) |     \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_D3) |     \
                                     PIN_MODE_INPUT(GPIOD_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN3) |           \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_OE) |     \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_WE) |     \
                                     PIN_MODE_INPUT(GPIOD_NAND_RB_NWAIT) |  \
                                     PIN_MODE_ALTERNATE(GPIOD_NAND_CE1) |   \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_D13) |    \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_D14) |    \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_D15) |    \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_A16) |    \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_A17) |    \
                                     PIN_MODE_INPUT(GPIOD_PIN13) |          \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_D0) |     \
                                     PIN_MODE_ALTERNATE(GPIOD_MEM_D1))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_MEM_D2) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_D3) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_OE) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_WE) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_NAND_RB_NWAIT) |\
                                     PIN_OTYPE_PUSHPULL(GPIOD_NAND_CE1) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_D13) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_D14) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_D15) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_A16) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_A17) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_D0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_MEM_D1))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_100M(GPIOD_MEM_D2) |        \
                                     PIN_OSPEED_100M(GPIOD_MEM_D3) |        \
                                     PIN_OSPEED_100M(GPIOD_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOD_MEM_OE) |        \
                                     PIN_OSPEED_100M(GPIOD_MEM_WE) |        \
                                     PIN_OSPEED_100M(GPIOD_NAND_RB_NWAIT) | \
                                     PIN_OSPEED_100M(GPIOD_NAND_CE1) |      \
                                     PIN_OSPEED_100M(GPIOD_MEM_D13) |       \
                                     PIN_OSPEED_100M(GPIOD_MEM_D14) |       \
                                     PIN_OSPEED_100M(GPIOD_MEM_D15) |       \
                                     PIN_OSPEED_100M(GPIOD_MEM_A16) |       \
                                     PIN_OSPEED_100M(GPIOD_MEM_A17) |       \
                                     PIN_OSPEED_100M(GPIOD_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOD_MEM_D0) |        \
                                     PIN_OSPEED_100M(GPIOD_MEM_D1))

#define VAL_GPIOD_PUPDR             (PIN_PUPDR_FLOATING(GPIOD_MEM_D2) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_D3) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_OE) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_WE) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_NAND_RB_NWAIT) |\
                                     PIN_PUPDR_PULLUP(GPIOD_NAND_CE1) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_D13) |    \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_D14) |    \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_D15) |    \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_A16) |    \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_A17) |    \
                                     PIN_PUPDR_FLOATING(GPIOD_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_D0) |     \
                                     PIN_PUPDR_FLOATING(GPIOD_MEM_D1))
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_MEM_D2) |           \
                                     PIN_ODR_HIGH(GPIOD_MEM_D3) |           \
                                     PIN_ODR_HIGH(GPIOD_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOD_MEM_OE) |           \
                                     PIN_ODR_HIGH(GPIOD_MEM_WE) |           \
                                     PIN_ODR_HIGH(GPIOD_NAND_RB_NWAIT) |    \
                                     PIN_ODR_HIGH(GPIOD_NAND_CE1) |         \
                                     PIN_ODR_HIGH(GPIOD_MEM_D13) |          \
                                     PIN_ODR_HIGH(GPIOD_MEM_D14) |          \
                                     PIN_ODR_HIGH(GPIOD_MEM_D15) |          \
                                     PIN_ODR_HIGH(GPIOD_MEM_A16) |          \
                                     PIN_ODR_HIGH(GPIOD_MEM_A17) |          \
                                     PIN_ODR_HIGH(GPIOD_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOD_MEM_D0) |           \
                                     PIN_ODR_HIGH(GPIOD_MEM_D1))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_MEM_D2, 12) |        \
                                     PIN_AFIO_AF(GPIOD_MEM_D3, 12) |        \
                                     PIN_AFIO_AF(GPIOD_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOD_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOD_MEM_OE, 12) |        \
                                     PIN_AFIO_AF(GPIOD_MEM_WE, 12) |        \
                                     PIN_AFIO_AF(GPIOD_NAND_RB_NWAIT, 0) |  \
                                     PIN_AFIO_AF(GPIOD_NAND_CE1, 12))
#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_MEM_D13, 12) |       \
                                     PIN_AFIO_AF(GPIOD_MEM_D14, 12) |       \
                                     PIN_AFIO_AF(GPIOD_MEM_D15, 12) |       \
                                     PIN_AFIO_AF(GPIOD_MEM_A16, 12) |       \
                                     PIN_AFIO_AF(GPIOD_MEM_A17, 12) |       \
                                     PIN_AFIO_AF(GPIOD_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOD_MEM_D0, 12) |        \
                                     PIN_AFIO_AF(GPIOD_MEM_D1, 12))

/*
 * GPIOE setup:
 */
#define VAL_GPIOE_MODER             (PIN_MODE_ALTERNATE(GPIOE_SRAM_LB) |    \
                                     PIN_MODE_ALTERNATE(GPIOE_SRAM_UB) |    \
                                     PIN_MODE_INPUT(GPIOE_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN6) |           \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D4) |     \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D5) |     \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D6) |     \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D7) |     \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D8) |     \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D9) |     \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D10) |    \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D11) |    \
                                     PIN_MODE_ALTERNATE(GPIOE_MEM_D12))
#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_SRAM_LB) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOE_SRAM_UB) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D4) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D5) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D6) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D7) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D8) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D9) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D10) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D11) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOE_MEM_D12))
#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_100M(GPIOE_SRAM_LB) |       \
                                     PIN_OSPEED_100M(GPIOE_SRAM_UB) |       \
                                     PIN_OSPEED_100M(GPIOE_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOE_MEM_D4) |        \
                                     PIN_OSPEED_100M(GPIOE_MEM_D5) |        \
                                     PIN_OSPEED_100M(GPIOE_MEM_D6) |        \
                                     PIN_OSPEED_100M(GPIOE_MEM_D7) |        \
                                     PIN_OSPEED_100M(GPIOE_MEM_D8) |        \
                                     PIN_OSPEED_100M(GPIOE_MEM_D9) |        \
                                     PIN_OSPEED_100M(GPIOE_MEM_D10) |       \
                                     PIN_OSPEED_100M(GPIOE_MEM_D11) |       \
                                     PIN_OSPEED_100M(GPIOE_MEM_D12))
#define VAL_GPIOE_PUPDR             (PIN_PUPDR_FLOATING(GPIOE_SRAM_LB) |    \
                                     PIN_PUPDR_FLOATING(GPIOE_SRAM_UB) |    \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOE_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D4) |     \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D5) |     \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D6) |     \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D7) |     \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D8) |     \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D9) |     \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D10) |    \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D11) |    \
                                     PIN_PUPDR_FLOATING(GPIOE_MEM_D12))
#define VAL_GPIOE_ODR               (PIN_ODR_HIGH(GPIOE_SRAM_LB) |          \
                                     PIN_ODR_HIGH(GPIOE_SRAM_UB) |          \
                                     PIN_ODR_HIGH(GPIOE_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOE_MEM_D4) |           \
                                     PIN_ODR_HIGH(GPIOE_MEM_D5) |           \
                                     PIN_ODR_HIGH(GPIOE_MEM_D6) |           \
                                     PIN_ODR_HIGH(GPIOE_MEM_D7) |           \
                                     PIN_ODR_HIGH(GPIOE_MEM_D8) |           \
                                     PIN_ODR_HIGH(GPIOE_MEM_D9) |           \
                                     PIN_ODR_HIGH(GPIOE_MEM_D10) |          \
                                     PIN_ODR_HIGH(GPIOE_MEM_D11) |          \
                                     PIN_ODR_HIGH(GPIOE_MEM_D12))
#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIOE_SRAM_LB, 12) |       \
                                     PIN_AFIO_AF(GPIOE_SRAM_UB, 12) |       \
                                     PIN_AFIO_AF(GPIOE_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOE_MEM_D4, 12))
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_MEM_D5, 12) |        \
                                     PIN_AFIO_AF(GPIOE_MEM_D6, 12) |        \
                                     PIN_AFIO_AF(GPIOE_MEM_D7, 12) |        \
                                     PIN_AFIO_AF(GPIOE_MEM_D8, 12) |        \
                                     PIN_AFIO_AF(GPIOE_MEM_D9, 12) |        \
                                     PIN_AFIO_AF(GPIOE_MEM_D10, 12) |       \
                                     PIN_AFIO_AF(GPIOE_MEM_D11, 12) |       \
                                     PIN_AFIO_AF(GPIOE_MEM_D12, 12))

/*
 * GPIOF setup:
 */
#define VAL_GPIOF_MODER             (PIN_MODE_ALTERNATE(GPIOF_MEM_A0) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A1) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A2) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A3) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A4) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A5) |     \
                                     PIN_MODE_INPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN11) |          \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A6) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A7) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A8) |     \
                                     PIN_MODE_ALTERNATE(GPIOF_MEM_A9))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_MEM_A0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A2) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A3) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A4) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A5) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A6) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A7) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A8) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_MEM_A9))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_100M(GPIOF_MEM_A0) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A1) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A2) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A3) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A4) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A5) |        \
                                     PIN_OSPEED_100M(GPIOF_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOF_MEM_A6) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A7) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A8) |        \
                                     PIN_OSPEED_100M(GPIOF_MEM_A9))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_FLOATING(GPIOF_MEM_A0) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A1) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A2) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A3) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A4) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A5) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A6) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A7) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A8) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_MEM_A9))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_MEM_A0) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A1) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A2) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A3) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A4) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A5) |           \
                                     PIN_ODR_HIGH(GPIOF_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOF_MEM_A6) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A7) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A8) |           \
                                     PIN_ODR_HIGH(GPIOF_MEM_A9))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_MEM_A0, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A1, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A2, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A3, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A4, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A5, 12) |        \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOF_MEM_A6, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A7, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A8, 12) |        \
                                     PIN_AFIO_AF(GPIOF_MEM_A9, 12))

/*
 * GPIOG setup:
 */
#define VAL_GPIOG_MODER             (PIN_MODE_ALTERNATE(GPIOG_MEM_A10) |    \
                                     PIN_MODE_ALTERNATE(GPIOG_MEM_A11) |    \
                                     PIN_MODE_ALTERNATE(GPIOG_MEM_A12) |    \
                                     PIN_MODE_ALTERNATE(GPIOG_MEM_A13) |    \
                                     PIN_MODE_ALTERNATE(GPIOG_MEM_A14) |    \
                                     PIN_MODE_ALTERNATE(GPIOG_MEM_A15) |    \
                                     PIN_MODE_ALTERNATE(GPIOG_NAND_RB1) |   \
                                     PIN_MODE_ALTERNATE(GPIOG_NAND_RB2) |   \
                                     PIN_MODE_INPUT(GPIOG_PIN8) |           \
                                     PIN_MODE_ALTERNATE(GPIOG_NAND_CE2) |   \
                                     PIN_MODE_INPUT(GPIOG_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN11) |          \
                                     PIN_MODE_ALTERNATE(GPIOG_SRAM_CS1) |   \
                                     PIN_MODE_INPUT(GPIOG_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN15))
#define VAL_GPIOG_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOG_MEM_A10) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_MEM_A11) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_MEM_A12) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_MEM_A13) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_MEM_A14) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_MEM_A15) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_NAND_RB1) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOG_NAND_RB2) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_NAND_CE2) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_SRAM_CS1) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN15))
#define VAL_GPIOG_OSPEEDR           (PIN_OSPEED_100M(GPIOG_MEM_A10) |       \
                                     PIN_OSPEED_100M(GPIOG_MEM_A11) |       \
                                     PIN_OSPEED_100M(GPIOG_MEM_A12) |       \
                                     PIN_OSPEED_100M(GPIOG_MEM_A13) |       \
                                     PIN_OSPEED_100M(GPIOG_MEM_A14) |       \
                                     PIN_OSPEED_100M(GPIOG_MEM_A15) |       \
                                     PIN_OSPEED_100M(GPIOG_NAND_RB1) |      \
                                     PIN_OSPEED_100M(GPIOG_NAND_RB2) |      \
                                     PIN_OSPEED_100M(GPIOG_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOG_NAND_CE2) |      \
                                     PIN_OSPEED_100M(GPIOG_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOG_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOG_SRAM_CS1) |      \
                                     PIN_OSPEED_100M(GPIOG_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOG_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOG_PIN15))

#define VAL_GPIOG_PUPDR             (PIN_PUPDR_FLOATING(GPIOG_MEM_A10) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_MEM_A11) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_MEM_A12) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_MEM_A13) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_MEM_A14) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_MEM_A15) |    \
                                     PIN_PUPDR_PULLUP(GPIOG_NAND_RB1) |     \
                                     PIN_PUPDR_FLOATING(GPIOG_NAND_RB2) |   \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN8) |       \
                                     PIN_PUPDR_PULLUP(GPIOG_NAND_CE2) |     \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_SRAM_CS1) |   \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN15))
#define VAL_GPIOG_ODR               (PIN_ODR_HIGH(GPIOG_MEM_A10) |          \
                                     PIN_ODR_HIGH(GPIOG_MEM_A11) |          \
                                     PIN_ODR_HIGH(GPIOG_MEM_A12) |          \
                                     PIN_ODR_HIGH(GPIOG_MEM_A13) |          \
                                     PIN_ODR_HIGH(GPIOG_MEM_A14) |          \
                                     PIN_ODR_HIGH(GPIOG_MEM_A15) |          \
                                     PIN_ODR_HIGH(GPIOG_NAND_RB1) |         \
                                     PIN_ODR_HIGH(GPIOG_NAND_RB2) |         \
                                     PIN_ODR_HIGH(GPIOG_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOG_NAND_CE2) |         \
                                     PIN_ODR_HIGH(GPIOG_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOG_SRAM_CS1) |         \
                                     PIN_ODR_HIGH(GPIOG_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN15))
#define VAL_GPIOG_AFRL              (PIN_AFIO_AF(GPIOG_MEM_A10, 12) |       \
                                     PIN_AFIO_AF(GPIOG_MEM_A11, 12) |       \
                                     PIN_AFIO_AF(GPIOG_MEM_A12, 12) |       \
                                     PIN_AFIO_AF(GPIOG_MEM_A13, 12) |       \
                                     PIN_AFIO_AF(GPIOG_MEM_A14, 12) |       \
                                     PIN_AFIO_AF(GPIOG_MEM_A15, 12) |       \
                                     PIN_AFIO_AF(GPIOG_NAND_RB1, 12) |      \
                                     PIN_AFIO_AF(GPIOG_NAND_RB2, 12))
#define VAL_GPIOG_AFRH              (PIN_AFIO_AF(GPIOG_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOG_NAND_CE2, 12) |      \
                                     PIN_AFIO_AF(GPIOG_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOG_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOG_SRAM_CS1, 12) |      \
                                     PIN_AFIO_AF(GPIOG_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOG_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOG_PIN15, 0))

/*
 * GPIOH setup:
 */
#define VAL_GPIOH_MODER             (PIN_MODE_INPUT(GPIOH_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOH_OSC_OUT) |        \
                                     PIN_MODE_INPUT(GPIOH_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN6) |           \
                                     PIN_MODE_ALTERNATE(GPIOH_I2C3_SCL) |   \
                                     PIN_MODE_ALTERNATE(GPIOH_I2C3_SDA) |   \
                                     PIN_MODE_INPUT(GPIOH_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN15))
#define VAL_GPIOH_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOH_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOH_OSC_OUT) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN6) |       \
                                     PIN_OTYPE_OPENDRAIN(GPIOH_I2C3_SCL) |  \
                                     PIN_OTYPE_OPENDRAIN(GPIOH_I2C3_SDA) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN15))
#define VAL_GPIOH_OSPEEDR           (PIN_OSPEED_100M(GPIOH_OSC_IN) |        \
                                     PIN_OSPEED_100M(GPIOH_OSC_OUT) |       \
                                     PIN_OSPEED_100M(GPIOH_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN6) |          \
                                     PIN_OSPEED_2M(GPIOH_I2C3_SCL) |        \
                                     PIN_OSPEED_2M(GPIOH_I2C3_SDA) |        \
                                     PIN_OSPEED_100M(GPIOH_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN15))
#define VAL_GPIOH_PUPDR             (PIN_PUPDR_FLOATING(GPIOH_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_OSC_OUT) |    \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_I2C3_SCL) |   \
                                     PIN_PUPDR_FLOATING(GPIOH_I2C3_SDA) |   \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN15))
#define VAL_GPIOH_ODR               (PIN_ODR_HIGH(GPIOH_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIOH_OSC_OUT) |          \
                                     PIN_ODR_HIGH(GPIOH_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOH_I2C3_SCL) |         \
                                     PIN_ODR_HIGH(GPIOH_I2C3_SDA) |         \
                                     PIN_ODR_HIGH(GPIOH_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN15))
#define VAL_GPIOH_AFRL              (PIN_AFIO_AF(GPIOH_OSC_IN, 0) |         \
                                     PIN_AFIO_AF(GPIOH_OSC_OUT, 0) |        \
                                     PIN_AFIO_AF(GPIOH_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOH_I2C3_SCL, 4))
#define VAL_GPIOH_AFRH              (PIN_AFIO_AF(GPIOH_I2C3_SDA, 4) |       \
                                     PIN_AFIO_AF(GPIOH_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN15, 0))

/*
 * GPIOI setup:
 */
#define VAL_GPIOI_MODER             (PIN_MODE_INPUT(GPIOI_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN9) |           \
                                     PIN_MODE_OUTPUT(GPIOI_LED_R) |         \
                                     PIN_MODE_OUTPUT(GPIOI_LED_G) |         \
                                     PIN_MODE_INPUT(GPIOI_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN15))
#define VAL_GPIOI_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOI_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_LED_R) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_LED_G) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN15))
#define VAL_GPIOI_OSPEEDR           (PIN_OSPEED_100M(GPIOI_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOI_LED_R) |         \
                                     PIN_OSPEED_100M(GPIOI_LED_G) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN15))
#define VAL_GPIOI_PUPDR             (PIN_PUPDR_FLOATING(GPIOI_PIN0) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN1) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_LED_R) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_LED_G) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN15))
#define VAL_GPIOI_ODR               (PIN_ODR_HIGH(GPIOI_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN9) |             \
                                     PIN_ODR_LOW(GPIOI_LED_R) |             \
                                     PIN_ODR_LOW(GPIOI_LED_G) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN15))
#define VAL_GPIOI_AFRL              (PIN_AFIO_AF(GPIOI_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN7, 0))
#define VAL_GPIOI_AFRH              (PIN_AFIO_AF(GPIOI_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOI_LED_R, 0) |          \
                                     PIN_AFIO_AF(GPIOI_LED_G, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN15, 0))

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
