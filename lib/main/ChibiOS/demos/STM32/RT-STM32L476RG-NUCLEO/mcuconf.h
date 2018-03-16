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

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

/*
 * STM32L1xx drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the whole
 * driver is enabled in halconf.h.
 *
 * IRQ priorities:
 * 15...0       Lowest...Highest.
 *
 * DMA priorities:
 * 0...3        Lowest...Highest.
 */

#define STM32L4xx_MCUCONF

/*
 * HAL driver system settings.
 */
#define STM32_NO_INIT                       FALSE
#define STM32_VOS                           STM32_VOS_RANGE1
#define STM32_PVD_ENABLE                    FALSE
#define STM32_PLS                           STM32_PLS_LEV0
#define STM32_HSI16_ENABLED                 FALSE
#define STM32_LSI_ENABLED                   TRUE
#define STM32_HSE_ENABLED                   FALSE
#define STM32_LSE_ENABLED                   TRUE
#define STM32_MSIPLL_ENABLED                TRUE
#define STM32_ADC_CLOCK_ENABLED             TRUE
#define STM32_USB_CLOCK_ENABLED             TRUE
#define STM32_SAI1_CLOCK_ENABLED            TRUE
#define STM32_SAI2_CLOCK_ENABLED            TRUE
#define STM32_MSIRANGE                      STM32_MSIRANGE_4M
#define STM32_MSISRANGE                     STM32_MSISRANGE_4M
#define STM32_SW                            STM32_SW_PLL
#define STM32_PLLSRC                        STM32_PLLSRC_MSI
#define STM32_PLLM_VALUE                    1
#define STM32_PLLN_VALUE                    80
#define STM32_PLLP_VALUE                    7
#define STM32_PLLQ_VALUE                    6
#define STM32_PLLR_VALUE                    4
#define STM32_HPRE                          STM32_HPRE_DIV1
#define STM32_PPRE1                         STM32_PPRE1_DIV1
#define STM32_PPRE2                         STM32_PPRE2_DIV1
#define STM32_STOPWUCK                      STM32_STOPWUCK_MSI
#define STM32_MCOSEL                        STM32_MCOSEL_NOCLOCK
#define STM32_MCOPRE                        STM32_MCOPRE_DIV1
#define STM32_LSCOSEL                       STM32_LSCOSEL_NOCLOCK
#define STM32_PLLSAI1N_VALUE                72
#define STM32_PLLSAI1P_VALUE                7
#define STM32_PLLSAI1Q_VALUE                6
#define STM32_PLLSAI1R_VALUE                6
#define STM32_PLLSAI2N_VALUE                72
#define STM32_PLLSAI2P_VALUE                7
#define STM32_PLLSAI2R_VALUE                6
#define STM32_USART1SEL                     STM32_USART1SEL_SYSCLK
#define STM32_USART2SEL                     STM32_USART2SEL_SYSCLK
#define STM32_USART3SEL                     STM32_USART3SEL_SYSCLK
#define STM32_UART4SEL                      STM32_UART4SEL_SYSCLK
#define STM32_UART5SEL                      STM32_UART5SEL_SYSCLK
#define STM32_LPUART1SEL                    STM32_LPUART1SEL_SYSCLK
#define STM32_I2C1SEL                       STM32_I2C1SEL_SYSCLK
#define STM32_I2C2SEL                       STM32_I2C2SEL_SYSCLK
#define STM32_I2C3SEL                       STM32_I2C3SEL_SYSCLK
#define STM32_LPTIM1SEL                     STM32_LPTIM1SEL_PCLK1
#define STM32_LPTIM2SEL                     STM32_LPTIM2SEL_PCLK1
#define STM32_SAI1SEL                       STM32_SAI1SEL_OFF
#define STM32_SAI2SEL                       STM32_SAI2SEL_OFF
#define STM32_CLK48SEL                      STM32_CLK48SEL_PLLSAI1
#define STM32_ADCSEL                        STM32_ADCSEL_SYSCLK
#define STM32_SWPMI1SEL                     STM32_SWPMI1SEL_PCLK1
#define STM32_DFSDMSEL                      STM32_DFSDMSEL_PCLK1
#define STM32_RTCSEL                        STM32_RTCSEL_LSI

/*
 * ADC driver system settings.
 */
#define STM32_ADC_DUAL_MODE                 FALSE
#define STM32_ADC_COMPACT_SAMPLES           FALSE
#define STM32_ADC_USE_ADC1                  FALSE
#define STM32_ADC_USE_ADC2                  FALSE
#define STM32_ADC_USE_ADC3                  FALSE
#define STM32_ADC_ADC1_DMA_STREAM           STM32_DMA_STREAM_ID(1, 1)
#define STM32_ADC_ADC2_DMA_STREAM           STM32_DMA_STREAM_ID(1, 2)
#define STM32_ADC_ADC3_DMA_STREAM           STM32_DMA_STREAM_ID(1, 3)
#define STM32_ADC_ADC1_DMA_PRIORITY         2
#define STM32_ADC_ADC2_DMA_PRIORITY         2
#define STM32_ADC_ADC3_DMA_PRIORITY         2
#define STM32_ADC_ADC12_IRQ_PRIORITY        5
#define STM32_ADC_ADC3_IRQ_PRIORITY         5
#define STM32_ADC_ADC1_DMA_IRQ_PRIORITY     5
#define STM32_ADC_ADC2_DMA_IRQ_PRIORITY     5
#define STM32_ADC_ADC3_DMA_IRQ_PRIORITY     5
#define STM32_ADC_ADC123_CLOCK_MODE         ADC_CCR_CKMODE_AHB_DIV1

/*
 * CAN driver system settings.
 */
#define STM32_CAN_USE_CAN1                  FALSE
#define STM32_CAN_CAN1_IRQ_PRIORITY         11

/*
 * DAC driver system settings.
 */
#define STM32_DAC_DUAL_MODE                 FALSE
#define STM32_DAC_USE_DAC1_CH1              FALSE
#define STM32_DAC_USE_DAC1_CH2              FALSE
#define STM32_DAC_DAC1_CH1_IRQ_PRIORITY     10
#define STM32_DAC_DAC1_CH2_IRQ_PRIORITY     10
#define STM32_DAC_DAC1_CH1_DMA_PRIORITY     2
#define STM32_DAC_DAC1_CH2_DMA_PRIORITY     2
#define STM32_DAC_DAC1_CH1_DMA_STREAM       STM32_DMA_STREAM_ID(2, 4)
#define STM32_DAC_DAC1_CH2_DMA_STREAM       STM32_DMA_STREAM_ID(1, 4)

/*
 * EXT driver system settings.
 */
#define STM32_EXT_EXTI0_IRQ_PRIORITY        6
#define STM32_EXT_EXTI1_IRQ_PRIORITY        6
#define STM32_EXT_EXTI2_IRQ_PRIORITY        6
#define STM32_EXT_EXTI3_IRQ_PRIORITY        6
#define STM32_EXT_EXTI4_IRQ_PRIORITY        6
#define STM32_EXT_EXTI5_9_IRQ_PRIORITY      6
#define STM32_EXT_EXTI10_15_IRQ_PRIORITY    6
#define STM32_EXT_EXTI1635_38_IRQ_PRIORITY  6
#define STM32_EXT_EXTI18_IRQ_PRIORITY       6
#define STM32_EXT_EXTI19_IRQ_PRIORITY       6
#define STM32_EXT_EXTI20_IRQ_PRIORITY       6
#define STM32_EXT_EXTI2122_IRQ_PRIORITY     15


/*
 * GPT driver system settings.
 */
#define STM32_GPT_USE_TIM1                  FALSE
#define STM32_GPT_USE_TIM2                  FALSE
#define STM32_GPT_USE_TIM3                  FALSE
#define STM32_GPT_USE_TIM4                  FALSE
#define STM32_GPT_USE_TIM5                  FALSE
#define STM32_GPT_USE_TIM6                  FALSE
#define STM32_GPT_USE_TIM7                  FALSE
#define STM32_GPT_USE_TIM8                  FALSE
#define STM32_GPT_USE_TIM15                 FALSE
#define STM32_GPT_USE_TIM16                 FALSE
#define STM32_GPT_USE_TIM17                 FALSE
#define STM32_GPT_TIM1_IRQ_PRIORITY         7
#define STM32_GPT_TIM2_IRQ_PRIORITY         7
#define STM32_GPT_TIM3_IRQ_PRIORITY         7
#define STM32_GPT_TIM4_IRQ_PRIORITY         7
#define STM32_GPT_TIM5_IRQ_PRIORITY         7
#define STM32_GPT_TIM6_IRQ_PRIORITY         7
#define STM32_GPT_TIM7_IRQ_PRIORITY         7
#define STM32_GPT_TIM8_IRQ_PRIORITY         7
#define STM32_GPT_TIM15_IRQ_PRIORITY        7
#define STM32_GPT_TIM16_IRQ_PRIORITY        7
#define STM32_GPT_TIM17_IRQ_PRIORITY        7

/*
 * I2C driver system settings.
 */
#define STM32_I2C_USE_I2C1                  FALSE
#define STM32_I2C_USE_I2C2                  FALSE
#define STM32_I2C_USE_I2C3                  FALSE
#define STM32_I2C_BUSY_TIMEOUT              50
#define STM32_I2C_I2C1_RX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 7)
#define STM32_I2C_I2C1_TX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 6)
#define STM32_I2C_I2C2_RX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 5)
#define STM32_I2C_I2C2_TX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 4)
#define STM32_I2C_I2C3_RX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 3)
#define STM32_I2C_I2C3_TX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 2)
#define STM32_I2C_I2C1_IRQ_PRIORITY         5
#define STM32_I2C_I2C2_IRQ_PRIORITY         5
#define STM32_I2C_I2C3_IRQ_PRIORITY         5
#define STM32_I2C_I2C1_DMA_PRIORITY         3
#define STM32_I2C_I2C2_DMA_PRIORITY         3
#define STM32_I2C_I2C3_DMA_PRIORITY         3
#define STM32_I2C_DMA_ERROR_HOOK(i2cp)      osalSysHalt("DMA failure")

/*
 * ICU driver system settings.
 */
#define STM32_ICU_USE_TIM1                  FALSE
#define STM32_ICU_USE_TIM2                  FALSE
#define STM32_ICU_USE_TIM3                  FALSE
#define STM32_ICU_USE_TIM4                  FALSE
#define STM32_ICU_USE_TIM5                  FALSE
#define STM32_ICU_USE_TIM8                  FALSE
#define STM32_ICU_TIM1_IRQ_PRIORITY         7
#define STM32_ICU_TIM2_IRQ_PRIORITY         7
#define STM32_ICU_TIM3_IRQ_PRIORITY         7
#define STM32_ICU_TIM4_IRQ_PRIORITY         7
#define STM32_ICU_TIM5_IRQ_PRIORITY         7
#define STM32_ICU_TIM8_IRQ_PRIORITY         7

/*
 * PWM driver system settings.
 */
#define STM32_PWM_USE_ADVANCED              FALSE
#define STM32_PWM_USE_TIM1                  FALSE
#define STM32_PWM_USE_TIM2                  FALSE
#define STM32_PWM_USE_TIM3                  FALSE
#define STM32_PWM_USE_TIM4                  FALSE
#define STM32_PWM_USE_TIM5                  FALSE
#define STM32_PWM_USE_TIM8                  FALSE
#define STM32_PWM_TIM1_IRQ_PRIORITY         7
#define STM32_PWM_TIM2_IRQ_PRIORITY         7
#define STM32_PWM_TIM3_IRQ_PRIORITY         7
#define STM32_PWM_TIM4_IRQ_PRIORITY         7
#define STM32_PWM_TIM5_IRQ_PRIORITY         7
#define STM32_PWM_TIM8_IRQ_PRIORITY         7

/*
 * SDC driver system settings.
 */
#define STM32_SDC_USE_SDMMC1                FALSE
#define STM32_SDC_SDMMC_UNALIGNED_SUPPORT   TRUE
#define STM32_SDC_SDMMC_WRITE_TIMEOUT       250
#define STM32_SDC_SDMMC_READ_TIMEOUT        25
#define STM32_SDC_SDMMC_CLOCK_DELAY         10
#define STM32_SDC_SDMMC1_DMA_PRIORITY       3
#define STM32_SDC_SDMMC1_IRQ_PRIORITY       9
#define STM32_SDC_SDMMC1_DMA_STREAM         STM32_DMA_STREAM_ID(2, 4)

/*
 * SERIAL driver system settings.
 */
#define STM32_SERIAL_USE_USART1             FALSE
#define STM32_SERIAL_USE_USART2             TRUE
#define STM32_SERIAL_USE_USART3             FALSE
#define STM32_SERIAL_USE_LPUART1            FALSE
#define STM32_SERIAL_USART1_PRIORITY        12
#define STM32_SERIAL_USART2_PRIORITY        12
#define STM32_SERIAL_USART3_PRIORITY        12
#define STM32_SERIAL_LPUART1_PRIORITY       12

/*
 * SPI driver system settings.
 */
#define STM32_SPI_USE_SPI1                  FALSE
#define STM32_SPI_USE_SPI2                  FALSE
#define STM32_SPI_USE_SPI3                  FALSE
#define STM32_SPI_SPI1_RX_DMA_STREAM        STM32_DMA_STREAM_ID(2, 3)
#define STM32_SPI_SPI1_TX_DMA_STREAM        STM32_DMA_STREAM_ID(2, 4)
#define STM32_SPI_SPI2_RX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 4)
#define STM32_SPI_SPI2_TX_DMA_STREAM        STM32_DMA_STREAM_ID(1, 5)
#define STM32_SPI_SPI3_RX_DMA_STREAM        STM32_DMA_STREAM_ID(2, 1)
#define STM32_SPI_SPI3_TX_DMA_STREAM        STM32_DMA_STREAM_ID(2, 2)
#define STM32_SPI_SPI1_DMA_PRIORITY         1
#define STM32_SPI_SPI2_DMA_PRIORITY         1
#define STM32_SPI_SPI3_DMA_PRIORITY         1
#define STM32_SPI_SPI1_IRQ_PRIORITY         10
#define STM32_SPI_SPI2_IRQ_PRIORITY         10
#define STM32_SPI_SPI3_IRQ_PRIORITY         10
#define STM32_SPI_DMA_ERROR_HOOK(spip)      osalSysHalt("DMA failure")

/*
 * ST driver system settings.
 */
#define STM32_ST_IRQ_PRIORITY               8
#define STM32_ST_USE_TIMER                  2

/*
 * UART driver system settings.
 */
#define STM32_UART_USE_USART1               FALSE
#define STM32_UART_USE_USART2               TRUE
#define STM32_UART_USE_USART3               FALSE
#define STM32_UART_USE_UART4                FALSE
#define STM32_UART_USE_UART5                FALSE
#define STM32_UART_USE_LPUART1              FALSE
#define STM32_UART_USART1_RX_DMA_STREAM     STM32_DMA_STREAM_ID(2, 7)
#define STM32_UART_USART1_TX_DMA_STREAM     STM32_DMA_STREAM_ID(2, 6)
#define STM32_UART_USART2_RX_DMA_STREAM     STM32_DMA_STREAM_ID(1, 6)
#define STM32_UART_USART2_TX_DMA_STREAM     STM32_DMA_STREAM_ID(1, 7)
#define STM32_UART_USART3_RX_DMA_STREAM     STM32_DMA_STREAM_ID(1, 3)
#define STM32_UART_USART3_TX_DMA_STREAM     STM32_DMA_STREAM_ID(1, 2)
#define STM32_UART_UART4_RX_DMA_STREAM      STM32_DMA_STREAM_ID(2, 5)
#define STM32_UART_UART4_TX_DMA_STREAM      STM32_DMA_STREAM_ID(2, 3)
#define STM32_UART_UART5_RX_DMA_STREAM      STM32_DMA_STREAM_ID(2, 2)
#define STM32_UART_UART5_TX_DMA_STREAM      STM32_DMA_STREAM_ID(2, 1)
#define STM32_UART_LPUART1_RX_DMA_STREAM    STM32_DMA_STREAM_ID(2, 7)
#define STM32_UART_LPUART1_TX_DMA_STREAM    STM32_DMA_STREAM_ID(2, 6)
#define STM32_UART_USART1_IRQ_PRIORITY      12
#define STM32_UART_USART2_IRQ_PRIORITY      12
#define STM32_UART_USART3_IRQ_PRIORITY      12
#define STM32_UART_UART4_IRQ_PRIORITY       12
#define STM32_UART_UART5_IRQ_PRIORITY       12
#define STM32_UART_USART1_DMA_PRIORITY      0
#define STM32_UART_USART2_DMA_PRIORITY      0
#define STM32_UART_USART3_DMA_PRIORITY      0
#define STM32_UART_UART4_DMA_PRIORITY       0
#define STM32_UART_UART5_DMA_PRIORITY       0
#define STM32_UART_LPUART1_DMA_PRIORITY     0
#define STM32_UART_DMA_ERROR_HOOK(uartp)    osalSysHalt("DMA failure")

/*
 * USB driver system settings.
 */
#define STM32_USB_USE_OTG1                  FALSE
#define STM32_USB_OTG1_IRQ_PRIORITY         14
#define STM32_USB_OTG1_RX_FIFO_SIZE         512
#define STM32_USB_OTG_THREAD_PRIO           LOWPRIO
#define STM32_USB_OTG_THREAD_STACK_SIZE     128
#define STM32_USB_OTGFIFO_FILL_BASEPRI      0

/*
 * WDG driver system settings.
 */
#define STM32_WDG_USE_IWDG                  FALSE

#endif /* _MCUCONF_H_ */
