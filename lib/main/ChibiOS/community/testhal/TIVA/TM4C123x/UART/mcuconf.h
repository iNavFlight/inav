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
 * TM4C123x drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the whole
 * driver is enabled in halconf.h.
 *
 * IRQ priorities:
 * 7...0       Lowest...Highest.
 */

#define TM4C123x_MCUCONF

/*
 * HAL driver system settings.
 */
#define TIVA_OSCSRC                         SYSCTL_RCC2_OSCSRC2_MO
#define TIVA_MOSC_ENABLE                    TRUE
#define TIVA_DIV400_VALUE                   1
#define TIVA_SYSDIV_VALUE                   2
#define TIVA_USESYSDIV_ENABLE               FALSE
#define TIVA_SYSDIV2LSB_ENABLE              FALSE
#define TIVA_BYPASS_VALUE                   0
#define TIVA_PWM_FIELDS                     (SYSCTL_RCC_USEPWMDIV |            \
                                             SYSCTL_RCC_PWMDIV_8)

/*
 * PAL driver system settings.
 */
#define TIVA_PAL_GPIOA_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOB_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOC_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOD_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOE_IRQ_PRIORITY         3
#define TIVA_PAL_GPIOF_IRQ_PRIORITY         3

/*
 * GPT driver system settings.
 */
#define TIVA_GPT_USE_GPT0                   FALSE
#define TIVA_GPT_USE_GPT1                   FALSE
#define TIVA_GPT_USE_GPT2                   FALSE
#define TIVA_GPT_USE_GPT3                   FALSE
#define TIVA_GPT_USE_GPT4                   FALSE
#define TIVA_GPT_USE_GPT5                   FALSE
#define TIVA_GPT_USE_WGPT0                  FALSE
#define TIVA_GPT_USE_WGPT1                  FALSE
#define TIVA_GPT_USE_WGPT2                  FALSE
#define TIVA_GPT_USE_WGPT3                  FALSE
#define TIVA_GPT_USE_WGPT4                  FALSE
#define TIVA_GPT_USE_WGPT5                  FALSE

#define TIVA_GPT_GPT0A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT1A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT2A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT3A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT4A_IRQ_PRIORITY         7
#define TIVA_GPT_GPT5A_IRQ_PRIORITY         7
#define TIVA_GPT_WGPT0A_IRQ_PRIORITY        7
#define TIVA_GPT_WGPT1A_IRQ_PRIORITY        7
#define TIVA_GPT_WGPT2A_IRQ_PRIORITY        7
#define TIVA_GPT_WGPT3A_IRQ_PRIORITY        7
#define TIVA_GPT_WGPT4A_IRQ_PRIORITY        7
#define TIVA_GPT_WGPT5A_IRQ_PRIORITY        7

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
#define TIVA_I2C_I2C0_IRQ_PRIORITY          4
#define TIVA_I2C_I2C1_IRQ_PRIORITY          4
#define TIVA_I2C_I2C2_IRQ_PRIORITY          4
#define TIVA_I2C_I2C3_IRQ_PRIORITY          4
#define TIVA_I2C_I2C4_IRQ_PRIORITY          4
#define TIVA_I2C_I2C5_IRQ_PRIORITY          4
#define TIVA_I2C_I2C6_IRQ_PRIORITY          4
#define TIVA_I2C_I2C7_IRQ_PRIORITY          4

/*
 * PWM driver system settings.
 */
#define TIVA_PWM_USE_PWM0                   FALSE
#define TIVA_PWM_USE_PWM1                   FALSE
#define TIVA_PWM_PWM0_FAULT_IRQ_PRIORITY    4
#define TIVA_PWM_PWM0_0_IRQ_PRIORITY        4
#define TIVA_PWM_PWM0_1_IRQ_PRIORITY        4
#define TIVA_PWM_PWM0_2_IRQ_PRIORITY        4
#define TIVA_PWM_PWM0_3_IRQ_PRIORITY        4
#define TIVA_PWM_PWM1_FAULT_IRQ_PRIORITY    4
#define TIVA_PWM_PWM1_0_IRQ_PRIORITY        4
#define TIVA_PWM_PWM1_1_IRQ_PRIORITY        4
#define TIVA_PWM_PWM1_2_IRQ_PRIORITY        4
#define TIVA_PWM_PWM1_3_IRQ_PRIORITY        4

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
 * UART driver system settings.
 */
#define TIVA_UART_USE_UART0                 TRUE
#define TIVA_UART_USE_UART1                 FALSE
#define TIVA_UART_USE_UART2                 FALSE
#define TIVA_UART_USE_UART3                 FALSE
#define TIVA_UART_USE_UART4                 FALSE
#define TIVA_UART_USE_UART5                 FALSE
#define TIVA_UART_USE_UART6                 FALSE
#define TIVA_UART_USE_UART7                 FALSE
#define TIVA_UART_UART0_PRIORITY            5
#define TIVA_UART_UART1_PRIORITY            5
#define TIVA_UART_UART2_PRIORITY            5
#define TIVA_UART_UART3_PRIORITY            5
#define TIVA_UART_UART4_PRIORITY            5
#define TIVA_UART_UART5_PRIORITY            5
#define TIVA_UART_UART6_PRIORITY            5
#define TIVA_UART_UART7_PRIORITY            5
#define TIVA_UART_UART0_RX_UDMA_CHANNEL     8
#define TIVA_UART_UART1_RX_UDMA_CHANNEL     22
#define TIVA_UART_UART2_RX_UDMA_CHANNEL     0
#define TIVA_UART_UART3_RX_UDMA_CHANNEL     16
#define TIVA_UART_UART4_RX_UDMA_CHANNEL     18
#define TIVA_UART_UART5_RX_UDMA_CHANNEL     6
#define TIVA_UART_UART6_RX_UDMA_CHANNEL     10
#define TIVA_UART_UART7_RX_UDMA_CHANNEL     20
#define TIVA_UART_UART0_TX_UDMA_CHANNEL     9
#define TIVA_UART_UART1_TX_UDMA_CHANNEL     23
#define TIVA_UART_UART2_TX_UDMA_CHANNEL     1
#define TIVA_UART_UART3_TX_UDMA_CHANNEL     17
#define TIVA_UART_UART4_TX_UDMA_CHANNEL     19
#define TIVA_UART_UART5_TX_UDMA_CHANNEL     7
#define TIVA_UART_UART6_TX_UDMA_CHANNEL     11
#define TIVA_UART_UART7_TX_UDMA_CHANNEL     21
#define TIVA_UART_UART0_RX_UDMA_MAPPING     0
#define TIVA_UART_UART1_RX_UDMA_MAPPING     0
#define TIVA_UART_UART2_RX_UDMA_MAPPING     1
#define TIVA_UART_UART3_RX_UDMA_MAPPING     2
#define TIVA_UART_UART4_RX_UDMA_MAPPING     2
#define TIVA_UART_UART5_RX_UDMA_MAPPING     2
#define TIVA_UART_UART6_RX_UDMA_MAPPING     2
#define TIVA_UART_UART7_RX_UDMA_MAPPING     2
#define TIVA_UART_UART0_TX_UDMA_MAPPING     0
#define TIVA_UART_UART1_TX_UDMA_MAPPING     0
#define TIVA_UART_UART2_TX_UDMA_MAPPING     1
#define TIVA_UART_UART3_TX_UDMA_MAPPING     2
#define TIVA_UART_UART4_TX_UDMA_MAPPING     2
#define TIVA_UART_UART5_TX_UDMA_MAPPING     2
#define TIVA_UART_UART6_TX_UDMA_MAPPING     2
#define TIVA_UART_UART7_TX_UDMA_MAPPING     2

/*
 * ST driver system settings.
 */
#define TIVA_ST_IRQ_PRIORITY                2
#define TIVA_ST_USE_WIDE_TIMER              TRUE
#define TIVA_ST_TIMER_NUMBER                5
#define TIVA_ST_TIMER_LETTER                A
