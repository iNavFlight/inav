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

/**
 * @file    STM32L0xx/stm32_registry.h
 * @brief   STM32L0xx capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _STM32_REGISTRY_H_
#define _STM32_REGISTRY_H_

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    STM32L0xx capabilities
 * @{
 */
/*===========================================================================*/
/* STM32L051xx, STM32L061xx                                                  */
/*===========================================================================*/
#if defined(STM32L051xx) || defined(STM32L061xx) ||                         \
    defined(__DOXYGEN__)

/* ADC attributes.*/
#define STM32_HAS_ADC1                      TRUE
#define STM32_ADC_SUPPORTS_PRESCALER        TRUE
#define STM32_ADC_SUPPORTS_OVERSAMPLING     TRUE
#define STM32_ADC1_IRQ_SHARED_WITH_EXTI     TRUE
#define STM32_ADC1_HANDLER                  Vector70
#define STM32_ADC1_NUMBER                   12
#define STM32_ADC1_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_ADC1_DMA_CHN                  0x00000000

#define STM32_HAS_ADC2                      FALSE
#define STM32_HAS_ADC3                      FALSE
#define STM32_HAS_ADC4                      FALSE

/* CAN attributes.*/
#define STM32_HAS_CAN1                      FALSE
#define STM32_HAS_CAN2                      FALSE

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  FALSE
#define STM32_HAS_DAC1_CH2                  FALSE
#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_SUPPORTS_CSELR            TRUE
#define STM32_DMA1_NUM_CHANNELS             7
#define STM32_DMA2_NUM_CHANNELS             0
#define STM32_DMA1_CH1_HANDLER              Vector64
#define STM32_DMA1_CH23_HANDLER             Vector68
#define STM32_DMA1_CH4567_HANDLER           Vector6C
#define STM32_DMA1_CH1_NUMBER               9
#define STM32_DMA1_CH23_NUMBER              10
#define STM32_DMA1_CH4567_NUMBER            11

#define STM32_DMA1_CH2_NUMBER               STM32_DMA1_CH23_NUMBER
#define STM32_DMA1_CH3_NUMBER               STM32_DMA1_CH23_NUMBER
#define DMA1_CH2_CMASK                      0x00000006U
#define DMA1_CH3_CMASK                      0x00000006U

#define STM32_DMA1_CH4_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH5_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH6_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH7_NUMBER               STM32_DMA1_CH4567_NUMBER
#define DMA1_CH4_CMASK                      0x00000078U
#define DMA1_CH5_CMASK                      0x00000078U
#define DMA1_CH6_CMASK                      0x00000078U
#define DMA1_CH7_CMASK                      0x00000078U

/* ETH attributes.*/
#define STM32_HAS_ETH                       FALSE

/* EXTI attributes.*/
#define STM32_EXTI_NUM_LINES                23
#define STM32_EXTI_IMR_MASK                 0xFF840000U

#define STM32_EXTI_LINE01_HANDLER           Vector54
#define STM32_EXTI_LINE23_HANDLER           Vector58
#define STM32_EXTI_LINE4_15_HANDLER         Vector5C
#define STM32_EXTI_LINE16_HANDLER           Vector44
#define STM32_EXTI_LINE171920_HANDLER       Vector48
#define STM32_EXTI_LINE2122_HANDLER         Vector70

#define STM32_EXTI_LINE01_NUMBER            5
#define STM32_EXTI_LINE23_NUMBER            6
#define STM32_EXTI_LINE4_15_NUMBER          7
#define STM32_EXTI_LINE16_NUMBER            1
#define STM32_EXTI_LINE171920_NUMBER        2
#define STM32_EXTI_LINE2122_NUMBER          12

/* GPIO attributes.*/
#define STM32_HAS_GPIOA                     TRUE
#define STM32_HAS_GPIOB                     TRUE
#define STM32_HAS_GPIOC                     TRUE
#define STM32_HAS_GPIOD                     TRUE
#define STM32_HAS_GPIOE                     FALSE
#define STM32_HAS_GPIOF                     FALSE
#define STM32_HAS_GPIOG                     FALSE
#define STM32_HAS_GPIOH                     TRUE
#define STM32_HAS_GPIOI                     FALSE
#define STM32_HAS_GPIOJ                     FALSE
#define STM32_HAS_GPIOK                     FALSE
#define STM32_GPIO_EN_MASK                  (RCC_IOPENR_GPIOAEN |           \
                                             RCC_IOPENR_GPIOBEN |           \
                                             RCC_IOPENR_GPIOCEN |           \
                                             RCC_IOPENR_GPIODEN |           \
                                             RCC_IOPENR_GPIOHEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_GLOBAL_HANDLER           Vector9C
#define STM32_I2C1_GLOBAL_NUMBER            23
#define STM32_I2C1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_I2C1_RX_DMA_CHN               0x06000600
#define STM32_I2C1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C1_TX_DMA_CHN               0x00600060

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_GLOBAL_HANDLER           VectorA0
#define STM32_I2C2_GLOBAL_NUMBER            24
#define STM32_I2C2_RX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_I2C2_RX_DMA_CHN               0x00070000
#define STM32_I2C2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_I2C2_TX_DMA_CHN               0x00007000

#define STM32_HAS_I2C3                      FALSE
#define STM32_HAS_I2C4                      FALSE

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDIO attributes.*/
#define STM32_HAS_SDIO                      FALSE

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_SUPPORTS_I2S             FALSE
#define STM32_SPI1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_SPI1_RX_DMA_CHN               0x00000010
#define STM32_SPI1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_SPI1_TX_DMA_CHN               0x00000100

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_SUPPORTS_I2S             TRUE
#define STM32_SPI2_I2S_FULLDUPLEX           FALSE
#define STM32_SPI2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_SPI2_RX_DMA_CHN               0x00202000
#define STM32_SPI2_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_SPI2_TX_DMA_CHN               0x02020000

#define STM32_HAS_SPI3                      FALSE
#define STM32_HAS_SPI4                      FALSE
#define STM32_HAS_SPI5                      FALSE
#define STM32_HAS_SPI6                      FALSE

/* TIM attributes.*/
#define STM32_HAS_TIM2                      TRUE
#define STM32_TIM2_IS_32BITS                FALSE
#define STM32_TIM2_CHANNELS                 4
#define STM32_TIM2_HANDLER                  Vector7C
#define STM32_TIM2_NUMBER                   15

#define STM32_HAS_TIM6                      TRUE
#define STM32_TIM6_IS_32BITS                FALSE
#define STM32_TIM6_CHANNELS                 0
#define STM32_TIM6_HANDLER                  Vector84
#define STM32_TIM6_NUMBER                   17

#define STM32_HAS_TIM21                     TRUE
#define STM32_TIM21_IS_32BITS               FALSE
#define STM32_TIM21_CHANNELS                2
#define STM32_TIM21_HANDLER                 Vector90
#define STM32_TIM21_NUMBER                  20

#define STM32_HAS_TIM22                     TRUE
#define STM32_TIM22_IS_32BITS               FALSE
#define STM32_TIM22_CHANNELS                2
#define STM32_TIM22_HANDLER                 Vector98
#define STM32_TIM22_NUMBER                  22

#define STM32_HAS_TIM1                      FALSE
#define STM32_HAS_TIM3                      FALSE
#define STM32_HAS_TIM4                      FALSE
#define STM32_HAS_TIM5                      FALSE
#define STM32_HAS_TIM7                      FALSE
#define STM32_HAS_TIM8                      FALSE
#define STM32_HAS_TIM9                      FALSE
#define STM32_HAS_TIM10                     FALSE
#define STM32_HAS_TIM11                     FALSE
#define STM32_HAS_TIM12                     FALSE
#define STM32_HAS_TIM13                     FALSE
#define STM32_HAS_TIM14                     FALSE
#define STM32_HAS_TIM15                     FALSE
#define STM32_HAS_TIM16                     FALSE
#define STM32_HAS_TIM17                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorAC
#define STM32_USART1_NUMBER                 27
#define STM32_USART1_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 5))
#define STM32_USART1_RX_DMA_CHN             0x00030300
#define STM32_USART1_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_USART1_TX_DMA_CHN             0x00003030

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorB0
#define STM32_USART2_NUMBER                 28
#define STM32_USART2_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_USART2_RX_DMA_CHN             0x00440000
#define STM32_USART2_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_USART2_TX_DMA_CHN             0x04004000

#define STM32_HAS_LPUART1                   TRUE
#define STM32_LPUART1_HANDLER               VectorB4
#define STM32_LPUART1_NUMBER                29

#define STM32_HAS_USART3                    FALSE
#define STM32_HAS_UART4                     FALSE
#define STM32_HAS_UART5                     FALSE
#define STM32_HAS_USART6                    FALSE
#define STM32_HAS_UART7                     FALSE
#define STM32_HAS_UART8                     FALSE

/* USB attributes.*/
#define STM32_HAS_USB                       FALSE
#define STM32_HAS_OTG1                      FALSE
#define STM32_HAS_OTG2                      FALSE

/* IWDG attributes.*/
#define STM32_HAS_IWDG                      TRUE
#define STM32_IWDG_IS_WINDOWED              TRUE

/* LTDC attributes.*/
#define STM32_HAS_LTDC                      FALSE

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     FALSE

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      FALSE

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              TRUE

/*===========================================================================*/
/* STM32L052xx, STM32L062xx.                                                 */
/*===========================================================================*/
#elif defined(STM32L052xx) || defined(STM32L062xx) ||                       \
    defined(__DOXYGEN__)

/* ADC attributes.*/
#define STM32_HAS_ADC1                      TRUE
#define STM32_ADC_SUPPORTS_PRESCALER        TRUE
#define STM32_ADC_SUPPORTS_OVERSAMPLING     TRUE
#define STM32_ADC1_IRQ_SHARED_WITH_EXTI     TRUE
#define STM32_ADC1_HANDLER                  Vector70
#define STM32_ADC1_NUMBER                   12
#define STM32_ADC1_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_ADC1_DMA_CHN                  0x00000000

#define STM32_HAS_ADC2                      FALSE
#define STM32_HAS_ADC3                      FALSE
#define STM32_HAS_ADC4                      FALSE

/* CAN attributes.*/
#define STM32_HAS_CAN1                      FALSE
#define STM32_HAS_CAN2                      FALSE

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  TRUE
#define STM32_DAC1_CH1_DMA_MSK              (STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_DAC1_CH1_DMA_CHN              0x00000090

#define STM32_HAS_DAC1_CH2                  FALSE
#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_SUPPORTS_CSELR            TRUE
#define STM32_DMA1_NUM_CHANNELS             7
#define STM32_DMA2_NUM_CHANNELS             0
#define STM32_DMA1_CH1_HANDLER              Vector64
#define STM32_DMA1_CH23_HANDLER             Vector68
#define STM32_DMA1_CH4567_HANDLER           Vector6C
#define STM32_DMA1_CH1_NUMBER               9
#define STM32_DMA1_CH23_NUMBER              10
#define STM32_DMA1_CH4567_NUMBER            11

#define STM32_DMA1_CH2_NUMBER               STM32_DMA1_CH23_NUMBER
#define STM32_DMA1_CH3_NUMBER               STM32_DMA1_CH23_NUMBER
#define DMA1_CH2_CMASK                      0x00000006U
#define DMA1_CH3_CMASK                      0x00000006U

#define STM32_DMA1_CH4_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH5_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH6_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH7_NUMBER               STM32_DMA1_CH4567_NUMBER
#define DMA1_CH4_CMASK                      0x00000078U
#define DMA1_CH5_CMASK                      0x00000078U
#define DMA1_CH6_CMASK                      0x00000078U
#define DMA1_CH7_CMASK                      0x00000078U

/* ETH attributes.*/
#define STM32_HAS_ETH                       FALSE

/* EXTI attributes.*/
#define STM32_EXTI_NUM_LINES                23
#define STM32_EXTI_IMR_MASK                 0xFF840000U

#define STM32_EXTI_LINE01_HANDLER           Vector54
#define STM32_EXTI_LINE23_HANDLER           Vector58
#define STM32_EXTI_LINE4_15_HANDLER         Vector5C
#define STM32_EXTI_LINE16_HANDLER           Vector44
#define STM32_EXTI_LINE171920_HANDLER       Vector48
#define STM32_EXTI_LINE2122_HANDLER         Vector70

#define STM32_EXTI_LINE01_NUMBER            5
#define STM32_EXTI_LINE23_NUMBER            6
#define STM32_EXTI_LINE4_15_NUMBER          7
#define STM32_EXTI_LINE16_NUMBER            1
#define STM32_EXTI_LINE171920_NUMBER        2
#define STM32_EXTI_LINE2122_NUMBER          12

/* GPIO attributes.*/
#define STM32_HAS_GPIOA                     TRUE
#define STM32_HAS_GPIOB                     TRUE
#define STM32_HAS_GPIOC                     TRUE
#define STM32_HAS_GPIOD                     TRUE
#define STM32_HAS_GPIOE                     FALSE
#define STM32_HAS_GPIOF                     FALSE
#define STM32_HAS_GPIOG                     FALSE
#define STM32_HAS_GPIOH                     TRUE
#define STM32_HAS_GPIOI                     FALSE
#define STM32_HAS_GPIOJ                     FALSE
#define STM32_HAS_GPIOK                     FALSE
#define STM32_GPIO_EN_MASK                  (RCC_IOPENR_GPIOAEN |           \
                                             RCC_IOPENR_GPIOBEN |           \
                                             RCC_IOPENR_GPIOCEN |           \
                                             RCC_IOPENR_GPIODEN |           \
                                             RCC_IOPENR_GPIOHEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_GLOBAL_HANDLER           Vector9C
#define STM32_I2C1_GLOBAL_NUMBER            23
#define STM32_I2C1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_I2C1_RX_DMA_CHN               0x06000600
#define STM32_I2C1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C1_TX_DMA_CHN               0x00600060

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_GLOBAL_HANDLER           VectorA0
#define STM32_I2C2_GLOBAL_NUMBER            24
#define STM32_I2C2_RX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_I2C2_RX_DMA_CHN               0x00070000
#define STM32_I2C2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_I2C2_TX_DMA_CHN               0x00007000

#define STM32_HAS_I2C3                      FALSE

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDIO attributes.*/
#define STM32_HAS_SDIO                      FALSE

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_SPI1_RX_DMA_CHN               0x00000010
#define STM32_SPI1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_SPI1_TX_DMA_CHN               0x00000100

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_SPI2_RX_DMA_CHN               0x00202000
#define STM32_SPI2_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_SPI2_TX_DMA_CHN               0x02020000

#define STM32_HAS_SPI3                      FALSE
#define STM32_HAS_SPI4                      FALSE
#define STM32_HAS_SPI5                      FALSE
#define STM32_HAS_SPI6                      FALSE

/* TIM attributes.*/
#define STM32_TIM_MAX_CHANNELS              4

#define STM32_HAS_TIM2                      TRUE
#define STM32_TIM2_IS_32BITS                FALSE
#define STM32_TIM2_CHANNELS                 4
#define STM32_TIM2_HANDLER                  Vector7C
#define STM32_TIM2_NUMBER                   15

#define STM32_HAS_TIM6                      TRUE
#define STM32_TIM6_IS_32BITS                FALSE
#define STM32_TIM6_CHANNELS                 0
#define STM32_TIM6_HANDLER                  Vector84
#define STM32_TIM6_NUMBER                   17

#define STM32_HAS_TIM21                     TRUE
#define STM32_TIM21_IS_32BITS               FALSE
#define STM32_TIM21_CHANNELS                2
#define STM32_TIM21_HANDLER                 Vector90
#define STM32_TIM21_NUMBER                  20

#define STM32_HAS_TIM22                     TRUE
#define STM32_TIM22_IS_32BITS               FALSE
#define STM32_TIM22_CHANNELS                2
#define STM32_TIM22_HANDLER                 Vector98
#define STM32_TIM22_NUMBER                  22

#define STM32_HAS_TIM1                      FALSE
#define STM32_HAS_TIM3                      FALSE
#define STM32_HAS_TIM4                      FALSE
#define STM32_HAS_TIM5                      FALSE
#define STM32_HAS_TIM7                      FALSE
#define STM32_HAS_TIM8                      FALSE
#define STM32_HAS_TIM9                      FALSE
#define STM32_HAS_TIM10                     FALSE
#define STM32_HAS_TIM11                     FALSE
#define STM32_HAS_TIM12                     FALSE
#define STM32_HAS_TIM13                     FALSE
#define STM32_HAS_TIM14                     FALSE
#define STM32_HAS_TIM15                     FALSE
#define STM32_HAS_TIM16                     FALSE
#define STM32_HAS_TIM17                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorAC
#define STM32_USART1_NUMBER                 27
#define STM32_USART1_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 5))
#define STM32_USART1_RX_DMA_CHN             0x00030300
#define STM32_USART1_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_USART1_TX_DMA_CHN             0x00003030

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorB0
#define STM32_USART2_NUMBER                 28
#define STM32_USART2_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_USART2_RX_DMA_CHN             0x00440000
#define STM32_USART2_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_USART2_TX_DMA_CHN             0x04004000

#define STM32_HAS_LPUART1                   TRUE
#define STM32_LPUART1_HANDLER               VectorB4
#define STM32_LPUART1_NUMBER                29

#define STM32_HAS_USART3                    FALSE
#define STM32_HAS_UART4                     FALSE
#define STM32_HAS_UART5                     FALSE
#define STM32_HAS_USART6                    FALSE
#define STM32_HAS_UART7                     FALSE
#define STM32_HAS_UART8                     FALSE

/* USB attributes.*/
#define STM32_HAS_USB                       TRUE
#define STM32_USB_ACCESS_SCHEME_2x16        TRUE
#define STM32_USB_PMA_SIZE                  1024
#define STM32_USB_HAS_BCDR                  TRUE
#define STM32_USB1_LP_HANDLER               VectorBC
#define STM32_USB1_LP_NUMBER                31
#define STM32_USB1_HP_HANDLER               VectorBC
#define STM32_USB1_HP_NUMBER                31

#define STM32_HAS_OTG1                      FALSE
#define STM32_HAS_OTG2                      FALSE

/* IWDG attributes.*/
#define STM32_HAS_IWDG                      TRUE
#define STM32_IWDG_IS_WINDOWED              TRUE

/* LTDC attributes.*/
#define STM32_HAS_LTDC                      FALSE

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     FALSE

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      FALSE

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              TRUE

/*===========================================================================*/
/* STM32L053xx, STM32L063xx.                                                 */
/*===========================================================================*/
#elif defined(STM32L053xx) || defined(STM32L063xx) ||                       \
    defined(__DOXYGEN__)

/* ADC attributes.*/
#define STM32_HAS_ADC1                      TRUE
#define STM32_ADC_SUPPORTS_PRESCALER        TRUE
#define STM32_ADC_SUPPORTS_OVERSAMPLING     TRUE
#define STM32_ADC1_IRQ_SHARED_WITH_EXTI     TRUE
#define STM32_ADC1_HANDLER                  Vector70
#define STM32_ADC1_NUMBER                   12
#define STM32_ADC1_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_ADC1_DMA_CHN                  0x00000000

#define STM32_HAS_ADC2                      FALSE
#define STM32_HAS_ADC3                      FALSE
#define STM32_HAS_ADC4                      FALSE

/* CAN attributes.*/
#define STM32_HAS_CAN1                      FALSE
#define STM32_HAS_CAN2                      FALSE

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  FALSE
#define STM32_DAC1_CH1_DMA_MSK              (STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_DAC1_CH1_DMA_CHN              0x00000090

#define STM32_HAS_DAC1_CH2                  FALSE
#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_SUPPORTS_CSELR            TRUE
#define STM32_DMA1_NUM_CHANNELS             7
#define STM32_DMA2_NUM_CHANNELS             0
#define STM32_DMA1_CH1_HANDLER              Vector64
#define STM32_DMA1_CH23_HANDLER             Vector68
#define STM32_DMA1_CH4567_HANDLER           Vector6C
#define STM32_DMA1_CH1_NUMBER               9
#define STM32_DMA1_CH23_NUMBER              10
#define STM32_DMA1_CH4567_NUMBER            11

#define STM32_DMA1_CH2_NUMBER               STM32_DMA1_CH23_NUMBER
#define STM32_DMA1_CH3_NUMBER               STM32_DMA1_CH23_NUMBER
#define DMA1_CH2_CMASK                      0x00000006U
#define DMA1_CH3_CMASK                      0x00000006U

#define STM32_DMA1_CH4_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH5_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH6_NUMBER               STM32_DMA1_CH4567_NUMBER
#define STM32_DMA1_CH7_NUMBER               STM32_DMA1_CH4567_NUMBER
#define DMA1_CH4_CMASK                      0x00000078U
#define DMA1_CH5_CMASK                      0x00000078U
#define DMA1_CH6_CMASK                      0x00000078U
#define DMA1_CH7_CMASK                      0x00000078U

/* ETH attributes.*/
#define STM32_HAS_ETH                       FALSE

/* EXTI attributes.*/
#define STM32_EXTI_NUM_LINES                23
#define STM32_EXTI_IMR_MASK                 0xFF840000U

#define STM32_EXTI_LINE01_HANDLER           Vector54
#define STM32_EXTI_LINE23_HANDLER           Vector58
#define STM32_EXTI_LINE4_15_HANDLER         Vector5C
#define STM32_EXTI_LINE16_HANDLER           Vector44
#define STM32_EXTI_LINE171920_HANDLER       Vector48
#define STM32_EXTI_LINE2122_HANDLER         Vector70

#define STM32_EXTI_LINE01_NUMBER            5
#define STM32_EXTI_LINE23_NUMBER            6
#define STM32_EXTI_LINE4_15_NUMBER          7
#define STM32_EXTI_LINE16_NUMBER            1
#define STM32_EXTI_LINE171920_NUMBER        2
#define STM32_EXTI_LINE2122_NUMBER          12

/* GPIO attributes.*/
#define STM32_HAS_GPIOA                     TRUE
#define STM32_HAS_GPIOB                     TRUE
#define STM32_HAS_GPIOC                     TRUE
#define STM32_HAS_GPIOD                     TRUE
#define STM32_HAS_GPIOE                     FALSE
#define STM32_HAS_GPIOF                     FALSE
#define STM32_HAS_GPIOG                     FALSE
#define STM32_HAS_GPIOH                     TRUE
#define STM32_HAS_GPIOI                     FALSE
#define STM32_HAS_GPIOJ                     FALSE
#define STM32_HAS_GPIOK                     FALSE
#define STM32_GPIO_EN_MASK                  (RCC_IOPENR_GPIOAEN |           \
                                             RCC_IOPENR_GPIOBEN |           \
                                             RCC_IOPENR_GPIOCEN |           \
                                             RCC_IOPENR_GPIODEN |           \
                                             RCC_IOPENR_GPIOHEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_GLOBAL_HANDLER           Vector9C
#define STM32_I2C1_GLOBAL_NUMBER            23
#define STM32_I2C1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_I2C1_RX_DMA_CHN               0x06000600
#define STM32_I2C1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C1_TX_DMA_CHN               0x00600060

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_GLOBAL_HANDLER           VectorA0
#define STM32_I2C2_GLOBAL_NUMBER            24
#define STM32_I2C2_RX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_I2C2_RX_DMA_CHN               0x00070000
#define STM32_I2C2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_I2C2_TX_DMA_CHN               0x00007000

#define STM32_HAS_I2C3                      FALSE

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDIO attributes.*/
#define STM32_HAS_SDIO                      FALSE

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_SPI1_RX_DMA_CHN               0x00000010
#define STM32_SPI1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_SPI1_TX_DMA_CHN               0x00000100

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_SPI2_RX_DMA_CHN               0x00202000
#define STM32_SPI2_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_SPI2_TX_DMA_CHN               0x02020000

#define STM32_HAS_SPI3                      FALSE
#define STM32_HAS_SPI4                      FALSE
#define STM32_HAS_SPI5                      FALSE
#define STM32_HAS_SPI6                      FALSE

/* TIM attributes.*/
#define STM32_TIM_MAX_CHANNELS              4

#define STM32_HAS_TIM2                      TRUE
#define STM32_TIM2_IS_32BITS                FALSE
#define STM32_TIM2_CHANNELS                 4
#define STM32_TIM2_HANDLER                  Vector7C
#define STM32_TIM2_NUMBER                   15

#define STM32_HAS_TIM6                      TRUE
#define STM32_TIM6_IS_32BITS                FALSE
#define STM32_TIM6_CHANNELS                 0
#define STM32_TIM6_HANDLER                  Vector84
#define STM32_TIM6_NUMBER                   17

#define STM32_HAS_TIM21                     TRUE
#define STM32_TIM21_IS_32BITS               FALSE
#define STM32_TIM21_CHANNELS                2
#define STM32_TIM21_HANDLER                 Vector90
#define STM32_TIM21_NUMBER                  20

#define STM32_HAS_TIM22                     TRUE
#define STM32_TIM22_IS_32BITS               FALSE
#define STM32_TIM22_CHANNELS                2
#define STM32_TIM22_HANDLER                 Vector98
#define STM32_TIM22_NUMBER                  22

#define STM32_HAS_TIM1                      FALSE
#define STM32_HAS_TIM3                      FALSE
#define STM32_HAS_TIM4                      FALSE
#define STM32_HAS_TIM5                      FALSE
#define STM32_HAS_TIM7                      FALSE
#define STM32_HAS_TIM8                      FALSE
#define STM32_HAS_TIM9                      FALSE
#define STM32_HAS_TIM10                     FALSE
#define STM32_HAS_TIM11                     FALSE
#define STM32_HAS_TIM12                     FALSE
#define STM32_HAS_TIM13                     FALSE
#define STM32_HAS_TIM14                     FALSE
#define STM32_HAS_TIM15                     FALSE
#define STM32_HAS_TIM16                     FALSE
#define STM32_HAS_TIM17                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorAC
#define STM32_USART1_NUMBER                 27
#define STM32_USART1_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 5))
#define STM32_USART1_RX_DMA_CHN             0x00030300
#define STM32_USART1_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_USART1_TX_DMA_CHN             0x00003030

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorB0
#define STM32_USART2_NUMBER                 28
#define STM32_USART2_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_USART2_RX_DMA_CHN             0x00440000
#define STM32_USART2_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_USART2_TX_DMA_CHN             0x04004000

#define STM32_HAS_LPUART1                   TRUE
#define STM32_LPUART1_HANDLER               VectorB4
#define STM32_LPUART1_NUMBER                29

#define STM32_HAS_USART3                    FALSE
#define STM32_HAS_UART4                     FALSE
#define STM32_HAS_UART5                     FALSE
#define STM32_HAS_USART6                    FALSE
#define STM32_HAS_UART7                     FALSE
#define STM32_HAS_UART8                     FALSE

/* USB attributes.*/
#define STM32_HAS_USB                       TRUE
#define STM32_USB_ACCESS_SCHEME_2x16        TRUE
#define STM32_USB_PMA_SIZE                  1024
#define STM32_USB_HAS_BCDR                  TRUE
#define STM32_USB1_LP_HANDLER               VectorBC
#define STM32_USB1_LP_NUMBER                31
#define STM32_USB1_HP_HANDLER               VectorBC
#define STM32_USB1_HP_NUMBER                31

#define STM32_HAS_OTG1                      FALSE
#define STM32_HAS_OTG2                      FALSE

/* IWDG attributes.*/
#define STM32_HAS_IWDG                      TRUE
#define STM32_IWDG_IS_WINDOWED              TRUE

/* LTDC attributes.*/
#define STM32_HAS_LTDC                      FALSE

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     FALSE

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      FALSE

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              TRUE

#else
#error "STM32L0xx device not specified"
#endif

/** @} */

#endif /* _STM32_REGISTRY_H_ */

/** @} */
