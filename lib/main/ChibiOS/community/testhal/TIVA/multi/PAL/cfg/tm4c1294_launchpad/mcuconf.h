/*
    Copyright (C) 2014..2017 Marco Veeneman

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

/*
 * TM4C129x drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the whole
 * driver is enabled in halconf.h.
 *
 * IRQ priorities:
 * 7...0       Lowest...Highest.
 */

#define TM4C129x_MCUCONF

/*
 * HAL driver system settings.
 */
#define TIVA_MOSC_SINGLE_ENDED              FALSE
#define TIVA_RSCLKCFG_OSCSRC                SYSCTL_RSCLKCFG_OSCSRC_MOSC

/*
 * PAL driver system settings.
 */
#define TIVA_PAL_GPIOA_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOB_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOC_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOD_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOE_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOF_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOG_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOH_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOJ_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOK_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOL_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOM_IRQ_PRIORITY         3
#define TIVA_PAL_GPION_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOP0_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP1_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP2_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP3_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP4_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP5_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP6_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOP7_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ0_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ1_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ2_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ3_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ4_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ5_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ6_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOQ7_IRQ_PRIORITY        3
#define TIVA_PAL_GPIOR_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOS_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOT_IRQ_PRIORITY         3

/*
 * GPT driver system settings.
 */
#define TIVA_GPT_USE_GPT0                   FALSE
#define TIVA_GPT_USE_GPT1                   FALSE
#define TIVA_GPT_USE_GPT2                   FALSE
#define TIVA_GPT_USE_GPT3                   FALSE
#define TIVA_GPT_USE_GPT4                   FALSE
#define TIVA_GPT_USE_GPT5                   FALSE
#define TIVA_GPT_USE_GPT6                   FALSE
#define TIVA_GPT_USE_GPT7                   FALSE
#define TIVA_GPT_GPT0A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT1A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT2A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT3A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT4A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT5A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT6A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT7A_IRQ_PRIORITY         7

/*
 * I2C driver system settings.
 */
#define TIVA_I2C_USE_I2C0                   FALSE
#define TIVA_I2C_USE_I2C1                   FALSE
#define TIVA_I2C_USE_I2C2                   FALSE
#define TIVA_I2C_USE_I2C3                   FALSE
#define TIVA_I2C_USE_I2C4                   FALSE
#define TIVA_I2C_USE_I2C5                   FALSE
#define TIVA_I2C_USE_I2C6                   FALSE
#define TIVA_I2C_USE_I2C7                   FALSE
#define TIVA_I2C_USE_I2C8                   FALSE
#define TIVA_I2C_USE_I2C9                   FALSE
#define TIVA_I2C_I2C0_IRQ_PRIORITY          4
#define TIVA_I2C_I2C1_IRQ_PRIORITY          4
#define TIVA_I2C_I2C2_IRQ_PRIORITY          4
#define TIVA_I2C_I2C3_IRQ_PRIORITY          4
#define TIVA_I2C_I2C4_IRQ_PRIORITY          4
#define TIVA_I2C_I2C5_IRQ_PRIORITY          4
#define TIVA_I2C_I2C6_IRQ_PRIORITY          4
#define TIVA_I2C_I2C7_IRQ_PRIORITY          4
#define TIVA_I2C_I2C8_IRQ_PRIORITY          4
#define TIVA_I2C_I2C9_IRQ_PRIORITY          4

/*
 * PWM driver system settings.
 */
#define TIVA_PWM_USE_PWM0                   FALSE
#define TIVA_PWM_PWM0_FAULT_IRQ_PRIORITY    4
#define TIVA_PWM_PWM0_0_IRQ_PRIORITY        4
#define TIVA_PWM_PWM0_1_IRQ_PRIORITY        4
#define TIVA_PWM_PWM0_2_IRQ_PRIORITY        4
#define TIVA_PWM_PWM0_3_IRQ_PRIORITY        4

/*
 * SERIAL driver system settings.
 */
#define TIVA_SERIAL_USE_UART0               FALSE
#define TIVA_SERIAL_USE_UART1               FALSE
#define TIVA_SERIAL_USE_UART2               FALSE
#define TIVA_SERIAL_USE_UART3               FALSE
#define TIVA_SERIAL_USE_UART4               FALSE
#define TIVA_SERIAL_USE_UART5               FALSE
#define TIVA_SERIAL_USE_UART6               FALSE
#define TIVA_SERIAL_USE_UART7               FALSE
#define TIVA_SERIAL_UART0_PRIORITY          5
#define TIVA_SERIAL_UART1_PRIORITY          5
#define TIVA_SERIAL_UART2_PRIORITY          5
#define TIVA_SERIAL_UART3_PRIORITY          5
#define TIVA_SERIAL_UART4_PRIORITY          5
#define TIVA_SERIAL_UART5_PRIORITY          5
#define TIVA_SERIAL_UART6_PRIORITY          5
#define TIVA_SERIAL_UART7_PRIORITY          5

/*
 * ST driver system settings.
 */
#define TIVA_ST_IRQ_PRIORITY                2
#define TIVA_ST_USE_WIDE_TIMER              FALSE
#define TIVA_ST_TIMER_NUMBER                5
#define TIVA_ST_TIMER_LETTER                A
