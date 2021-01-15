/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    STM32F7xx/stm32_registry.h
 * @brief   STM32F7xx capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef STM32_REGISTRY_H
#define STM32_REGISTRY_H

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    STM32F7xx capabilities
 * @{
 */

/*===========================================================================*/
/* Common.                                                                   */
/*===========================================================================*/

/* RNG attributes.*/
#define STM32_HAS_RNG1                      TRUE

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_STORAGE_SIZE              128
#define STM32_RTC_TAMP_STAMP_HANDLER        Vector48
#define STM32_RTC_WKUP_HANDLER              Vector4C
#define STM32_RTC_ALARM_HANDLER             VectorE4
#define STM32_RTC_TAMP_STAMP_NUMBER         2
#define STM32_RTC_WKUP_NUMBER               3
#define STM32_RTC_ALARM_NUMBER              41
#define STM32_RTC_ALARM_EXTI                17
#define STM32_RTC_TAMP_STAMP_EXTI           21
#define STM32_RTC_WKUP_EXTI                 22
#define STM32_RTC_IRQ_ENABLE() do {                                         \
  nvicEnableVector(STM32_RTC_TAMP_STAMP_NUMBER, STM32_IRQ_EXTI21_PRIORITY); \
  nvicEnableVector(STM32_RTC_WKUP_NUMBER, STM32_IRQ_EXTI22_PRIORITY);       \
  nvicEnableVector(STM32_RTC_ALARM_NUMBER, STM32_IRQ_EXTI17_PRIORITY);      \
} while (false)

#if defined(STM32F732xx) || defined(STM32F733xx) || defined(STM32F756xx) || \
    defined(STM32F777xx) || defined(STM32F779xx) || defined(__DOXYGEN__)
#define STM32_HAS_HASH1                     TRUE
#define STM32_HAS_AES1                      TRUE
#define STM32_HASH1_DMA_MSK                 STM32_DMA_STREAM_ID_MSK(2, 7)
#define STM32_HASH1_DMA_CHN                 0x20000000

#else /* Devices without cryp nor hash.*/
#define STM32_HAS_HASH1                     FALSE
#define STM32_HAS_AES1                      FALSE
#endif

/*===========================================================================*/
/* STM32F722xx, STM32F723xx, STM32F732xx, STM32F733xx.                       */
/*===========================================================================*/
#if defined(STM32F722xx) || defined(STM32F723xx) || defined(STM32F732xx) || \
    defined(STM32F733xx) || defined(__DOXYGEN__)
/* ADC attributes.*/
#define STM32_ADC_HANDLER                   Vector88
#define STM32_ADC_NUMBER                    18

#define STM32_HAS_ADC1                      TRUE
#define STM32_ADC1_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 4))
#define STM32_ADC1_DMA_CHN                  0x00000000

#define STM32_HAS_ADC2                      TRUE
#define STM32_ADC2_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 3))
#define STM32_ADC2_DMA_CHN                  0x00001100

#define STM32_HAS_ADC3                      TRUE
#define STM32_ADC3_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 1))
#define STM32_ADC3_DMA_CHN                  0x00000022

#define STM32_HAS_ADC4                      FALSE

#define STM32_HAS_SDADC1                    FALSE
#define STM32_HAS_SDADC2                    FALSE
#define STM32_HAS_SDADC3                    FALSE

/* CAN attributes.*/
#define STM32_CAN_MAX_FILTERS               28

#define STM32_HAS_CAN1                      TRUE
#define STM32_CAN1_TX_HANDLER               Vector8C
#define STM32_CAN1_RX0_HANDLER              Vector90
#define STM32_CAN1_RX1_HANDLER              Vector94
#define STM32_CAN1_SCE_HANDLER              Vector98
#define STM32_CAN1_TX_NUMBER                19
#define STM32_CAN1_RX0_NUMBER               20
#define STM32_CAN1_RX1_NUMBER               21
#define STM32_CAN1_SCE_NUMBER               22

#define STM32_HAS_CAN2                      FALSE
#define STM32_HAS_CAN3                      FALSE

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  TRUE
#define STM32_DAC1_CH1_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_DAC1_CH1_DMA_CHN              0x00700000

#define STM32_HAS_DAC1_CH2                  TRUE
#define STM32_DAC1_CH2_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_DAC1_CH2_DMA_CHN              0x07000000

#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_CACHE_HANDLING            TRUE
#define STM32_DMA_SUPPORTS_DMAMUX           FALSE

#define STM32_HAS_DMA1                      TRUE
#define STM32_DMA1_CH0_HANDLER              Vector6C
#define STM32_DMA1_CH1_HANDLER              Vector70
#define STM32_DMA1_CH2_HANDLER              Vector74
#define STM32_DMA1_CH3_HANDLER              Vector78
#define STM32_DMA1_CH4_HANDLER              Vector7C
#define STM32_DMA1_CH5_HANDLER              Vector80
#define STM32_DMA1_CH6_HANDLER              Vector84
#define STM32_DMA1_CH7_HANDLER              VectorFC
#define STM32_DMA1_CH0_NUMBER               11
#define STM32_DMA1_CH1_NUMBER               12
#define STM32_DMA1_CH2_NUMBER               13
#define STM32_DMA1_CH3_NUMBER               14
#define STM32_DMA1_CH4_NUMBER               15
#define STM32_DMA1_CH5_NUMBER               16
#define STM32_DMA1_CH6_NUMBER               17
#define STM32_DMA1_CH7_NUMBER               47

#define STM32_HAS_DMA2                      TRUE
#define STM32_DMA2_CH0_HANDLER              Vector120
#define STM32_DMA2_CH1_HANDLER              Vector124
#define STM32_DMA2_CH2_HANDLER              Vector128
#define STM32_DMA2_CH3_HANDLER              Vector12C
#define STM32_DMA2_CH4_HANDLER              Vector130
#define STM32_DMA2_CH5_HANDLER              Vector150
#define STM32_DMA2_CH6_HANDLER              Vector154
#define STM32_DMA2_CH7_HANDLER              Vector158
#define STM32_DMA2_CH0_NUMBER               56
#define STM32_DMA2_CH1_NUMBER               57
#define STM32_DMA2_CH2_NUMBER               58
#define STM32_DMA2_CH3_NUMBER               59
#define STM32_DMA2_CH4_NUMBER               60
#define STM32_DMA2_CH5_NUMBER               68
#define STM32_DMA2_CH6_NUMBER               69
#define STM32_DMA2_CH7_NUMBER               70

/* ETH attributes.*/
#define STM32_HAS_ETH                       FALSE

/* EXTI attributes.*/
#define STM32_EXTI_NUM_LINES                24
#define STM32_EXTI_IMR1_MASK                0xFF000000

/* GPIO attributes.*/
#define STM32_HAS_GPIOA                     TRUE
#define STM32_HAS_GPIOB                     TRUE
#define STM32_HAS_GPIOC                     TRUE
#define STM32_HAS_GPIOD                     TRUE
#define STM32_HAS_GPIOE                     TRUE
#define STM32_HAS_GPIOH                     TRUE
#define STM32_HAS_GPIOF                     TRUE
#define STM32_HAS_GPIOG                     TRUE
#define STM32_HAS_GPIOI                     TRUE
#define STM32_HAS_GPIOJ                     FALSE
#define STM32_HAS_GPIOK                     FALSE
#define STM32_GPIO_EN_MASK                  (RCC_AHB1ENR_GPIOAEN |          \
                                             RCC_AHB1ENR_GPIOBEN |          \
                                             RCC_AHB1ENR_GPIOCEN |          \
                                             RCC_AHB1ENR_GPIODEN |          \
                                             RCC_AHB1ENR_GPIOEEN |          \
                                             RCC_AHB1ENR_GPIOFEN |          \
                                             RCC_AHB1ENR_GPIOGEN |          \
                                             RCC_AHB1ENR_GPIOHEN |          \
                                             RCC_AHB1ENR_GPIOIEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_EVENT_HANDLER            VectorBC
#define STM32_I2C1_ERROR_HANDLER            VectorC0
#define STM32_I2C1_EVENT_NUMBER             31
#define STM32_I2C1_ERROR_NUMBER             32
#define STM32_I2C1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 5))
#define STM32_I2C1_RX_DMA_CHN               0x00100001
#define STM32_I2C1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 7) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C1_TX_DMA_CHN               0x11000000

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_EVENT_HANDLER            VectorC4
#define STM32_I2C2_ERROR_HANDLER            VectorC8
#define STM32_I2C2_EVENT_NUMBER             33
#define STM32_I2C2_ERROR_NUMBER             34
#define STM32_I2C2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_I2C2_RX_DMA_CHN               0x00007700
#define STM32_I2C2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 7)
#define STM32_I2C2_TX_DMA_CHN               0x70000000

#define STM32_HAS_I2C3                      TRUE
#define STM32_I2C3_EVENT_HANDLER            Vector160
#define STM32_I2C3_ERROR_HANDLER            Vector164
#define STM32_I2C3_EVENT_NUMBER             72
#define STM32_I2C3_ERROR_NUMBER             73
#define STM32_I2C3_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_I2C3_RX_DMA_CHN               0x00000300
#define STM32_I2C3_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_I2C3_TX_DMA_CHN               0x00030000

#define STM32_HAS_I2C4                      FALSE

/* QUADSPI attributes.*/
#define STM32_HAS_QUADSPI1                  TRUE
#define STM32_QUADSPI1_HANDLER              Vector1B0
#define STM32_QUADSPI1_NUMBER               92
#define STM32_QUADSPI1_DMA_MSK              STM32_DMA_STREAM_ID_MSK(2, 7)
#define STM32_QUADSPI1_DMA_CHN              0x30000000

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDMMC attributes.*/
#define STM32_HAS_SDMMC1                    TRUE
#define STM32_SDMMC1_HANDLER                Vector104
#define STM32_SDMMC1_NUMBER                 49
#define STM32_SDC_SDMMC1_DMA_MSK            (STM32_DMA_STREAM_ID_MSK(2, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SDC_SDMMC1_DMA_CHN            0x04004000

#define STM32_HAS_SDMMC2                    TRUE
#define STM32_SDMMC2_HANDLER                Vector1DC
#define STM32_SDMMC2_NUMBER                 103
#define STM32_SDC_SDMMC2_DMA_MSK            (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SDC_SDMMC2_DMA_CHN            0x00B0000B

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_SUPPORTS_I2S             TRUE
#define STM32_SPI1_I2S_FULLDUPLEX           TRUE
#define STM32_SPI1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2))
#define STM32_SPI1_RX_DMA_CHN               0x00000303
#define STM32_SPI1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI1_TX_DMA_CHN               0x00303000

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_SUPPORTS_I2S             TRUE
#define STM32_SPI2_I2S_FULLDUPLEX           TRUE
#define STM32_SPI2_RX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 3)
#define STM32_SPI2_RX_DMA_CHN               0x00000000
#define STM32_SPI2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_SPI2_TX_DMA_CHN               0x00000000

#define STM32_HAS_SPI3                      TRUE
#define STM32_SPI3_SUPPORTS_I2S             TRUE
#define STM32_SPI3_I2S_FULLDUPLEX           TRUE
#define STM32_SPI3_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_SPI3_RX_DMA_CHN               0x00000000
#define STM32_SPI3_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_SPI3_TX_DMA_CHN               0x00000000

#define STM32_HAS_SPI4                      TRUE
#define STM32_SPI4_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 3))
#define STM32_SPI4_RX_DMA_CHN               0x00005004
#define STM32_SPI4_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 4))
#define STM32_SPI4_TX_DMA_CHN               0x00050040

#define STM32_HAS_SPI5                      TRUE
#define STM32_SPI5_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 3) |            \
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI5_RX_DMA_CHN               0x00702000
#define STM32_SPI5_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 4) |            \
                                             STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SPI5_TX_DMA_CHN               0x07020000

#define STM32_HAS_SPI6                      FALSE

/* TIM attributes.*/
#define STM32_TIM_MAX_CHANNELS              6

#define STM32_HAS_TIM1                      TRUE
#define STM32_TIM1_IS_32BITS                FALSE
#define STM32_TIM1_CHANNELS                 6
#define STM32_TIM1_UP_HANDLER               VectorA4
#define STM32_TIM1_CC_HANDLER               VectorAC
#define STM32_TIM1_UP_NUMBER                25
#define STM32_TIM1_CC_NUMBER                27

#define STM32_HAS_TIM2                      TRUE
#define STM32_TIM2_IS_32BITS                TRUE
#define STM32_TIM2_CHANNELS                 4
#define STM32_TIM2_HANDLER                  VectorB0
#define STM32_TIM2_NUMBER                   28

#define STM32_HAS_TIM3                      TRUE
#define STM32_TIM3_IS_32BITS                FALSE
#define STM32_TIM3_CHANNELS                 4
#define STM32_TIM3_HANDLER                  VectorB4
#define STM32_TIM3_NUMBER                   29

#define STM32_HAS_TIM4                      TRUE
#define STM32_TIM4_IS_32BITS                FALSE
#define STM32_TIM4_CHANNELS                 4
#define STM32_TIM4_HANDLER                  VectorB8
#define STM32_TIM4_NUMBER                   30

#define STM32_HAS_TIM5                      TRUE
#define STM32_TIM5_IS_32BITS                TRUE
#define STM32_TIM5_CHANNELS                 4
#define STM32_TIM5_HANDLER                  Vector108
#define STM32_TIM5_NUMBER                   50

#define STM32_HAS_TIM6                      TRUE
#define STM32_TIM6_IS_32BITS                FALSE
#define STM32_TIM6_CHANNELS                 0
#define STM32_TIM6_HANDLER                  Vector118
#define STM32_TIM6_NUMBER                   54

#define STM32_HAS_TIM7                      TRUE
#define STM32_TIM7_IS_32BITS                FALSE
#define STM32_TIM7_CHANNELS                 0
#define STM32_TIM7_HANDLER                  Vector11C
#define STM32_TIM7_NUMBER                   55

#define STM32_HAS_TIM8                      TRUE
#define STM32_TIM8_IS_32BITS                FALSE
#define STM32_TIM8_CHANNELS                 6
#define STM32_TIM8_UP_HANDLER               VectorF0
#define STM32_TIM8_CC_HANDLER               VectorF8
#define STM32_TIM8_UP_NUMBER                44
#define STM32_TIM8_CC_NUMBER                46

#define STM32_HAS_TIM9                      TRUE
#define STM32_TIM9_IS_32BITS                FALSE
#define STM32_TIM9_CHANNELS                 2
#define STM32_TIM9_HANDLER                  VectorA0
#define STM32_TIM9_NUMBER                   24

#define STM32_HAS_TIM10                     TRUE
#define STM32_TIM10_IS_32BITS               FALSE
#define STM32_TIM10_CHANNELS                1
#define STM32_TIM10_HANDLER                 VectorA4 /* Note: same as STM32_TIM1_UP */
#define STM32_TIM10_NUMBER                  25 /* Note: same as STM32_TIM1_UP */

#define STM32_HAS_TIM11                     TRUE
#define STM32_TIM11_IS_32BITS               FALSE
#define STM32_TIM11_CHANNELS                1
#define STM32_TIM11_HANDLER                 VectorA8
#define STM32_TIM11_NUMBER                  26

#define STM32_HAS_TIM12                     TRUE
#define STM32_TIM12_IS_32BITS               FALSE
#define STM32_TIM12_CHANNELS                2
#define STM32_TIM12_HANDLER                 VectorEC
#define STM32_TIM12_NUMBER                  43

#define STM32_HAS_TIM13                     TRUE
#define STM32_TIM13_IS_32BITS               FALSE
#define STM32_TIM13_CHANNELS                1
#define STM32_TIM13_HANDLER                 VectorF0 /* Note: same as STM32_TIM8_UP */
#define STM32_TIM13_NUMBER                  44 /* Note: same as STM32_TIM8_UP */

#define STM32_HAS_TIM14                     TRUE
#define STM32_TIM14_IS_32BITS               FALSE
#define STM32_TIM14_CHANNELS                1
#define STM32_TIM14_HANDLER                 VectorF4
#define STM32_TIM14_NUMBER                  45

#define STM32_HAS_TIM15                     FALSE
#define STM32_HAS_TIM16                     FALSE
#define STM32_HAS_TIM17                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE
#define STM32_HAS_TIM21                     FALSE
#define STM32_HAS_TIM22                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorD4
#define STM32_USART1_NUMBER                 37
#define STM32_USART1_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_USART1_RX_DMA_CHN             0x00400400
#define STM32_USART1_TX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(2, 7)
#define STM32_USART1_TX_DMA_CHN             0x40000000

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorD8
#define STM32_USART2_NUMBER                 38
#define STM32_USART2_RX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_USART2_RX_DMA_CHN             0x00400000
#define STM32_USART2_TX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_USART2_TX_DMA_CHN             0x04000000

#define STM32_HAS_USART3                    TRUE
#define STM32_USART3_HANDLER                VectorDC
#define STM32_USART3_NUMBER                 39
#define STM32_USART3_RX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 1)
#define STM32_USART3_RX_DMA_CHN             0x00000040
#define STM32_USART3_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_USART3_TX_DMA_CHN             0x00074000

#define STM32_HAS_UART4                     TRUE
#define STM32_UART4_HANDLER                 Vector110
#define STM32_UART4_NUMBER                  52
#define STM32_UART4_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 2)
#define STM32_UART4_RX_DMA_CHN              0x00000400
#define STM32_UART4_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_UART4_TX_DMA_CHN              0x00040000

#define STM32_HAS_UART5                     TRUE
#define STM32_UART5_HANDLER                 Vector114
#define STM32_UART5_NUMBER                  53
#define STM32_UART5_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 0)
#define STM32_UART5_RX_DMA_CHN              0x00000004
#define STM32_UART5_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 7)
#define STM32_UART5_TX_DMA_CHN              0x40000000

#define STM32_HAS_USART6                    TRUE
#define STM32_USART6_HANDLER                Vector15C
#define STM32_USART6_NUMBER                 71
#define STM32_USART6_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2))
#define STM32_USART6_RX_DMA_CHN             0x00000550
#define STM32_USART6_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 6) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 7))
#define STM32_USART6_TX_DMA_CHN             0x55000000

#define STM32_HAS_UART7                     TRUE
#define STM32_UART7_HANDLER                 Vector188
#define STM32_UART7_NUMBER                  82
#define STM32_UART7_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 3)
#define STM32_UART7_RX_DMA_CHN              0x00005000
#define STM32_UART7_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 1)
#define STM32_UART7_TX_DMA_CHN              0x00000050

#define STM32_HAS_UART8                     TRUE
#define STM32_UART8_HANDLER                 Vector18C
#define STM32_UART8_NUMBER                  83
#define STM32_UART8_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_UART8_RX_DMA_CHN              0x05000000
#define STM32_UART8_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 0)
#define STM32_UART8_TX_DMA_CHN              0x00000005

#define STM32_HAS_LPUART1                   FALSE

/* USB attributes.*/
#define STM32_OTG_STEPPING                  2
#define STM32_HAS_OTG1                      TRUE
#define STM32_OTG1_ENDPOINTS                5
#define STM32_OTG1_HANDLER                  Vector14C
#define STM32_OTG1_NUMBER                   67

#define STM32_HAS_OTG2                      TRUE
#define STM32_OTG2_ENDPOINTS                8
#define STM32_OTG2_HANDLER                  Vector174
#define STM32_OTG2_EP1OUT_HANDLER           Vector168
#define STM32_OTG2_EP1IN_HANDLER            Vector16C
#define STM32_OTG2_NUMBER                   77
#define STM32_OTG2_EP1OUT_NUMBER            74
#define STM32_OTG2_EP1IN_NUMBER             75

#define STM32_HAS_USB                       FALSE

/* IWDG attributes.*/
#define STM32_HAS_IWDG                      TRUE
#define STM32_IWDG_IS_WINDOWED              TRUE

/* LTDC attributes.*/
#define STM32_HAS_LTDC                      TRUE

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     TRUE

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      TRUE
#define STM32_FSMC_IS_FMC                   TRUE
#define STM32_FSMC_HANDLER                  Vector100
#define STM32_FSMC_NUMBER                   48

/* LTDC attributes.*/

/* DMA2D attributes.*/

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              TRUE

#endif /* defined(STM32F722xx) || defined(STM32F723xx) ||
          defined(STM32F732xx) || defined(STM32F733xx) */

/*===========================================================================*/
/* STM32F745xx, STM32F746xx, STM32F756xx.                                    */
/*===========================================================================*/
#if defined(STM32F745xx) || defined(STM32F746xx) || defined(STM32F756xx) || \
    defined(__DOXYGEN__)
/* ADC attributes.*/
#define STM32_ADC_HANDLER                   Vector88
#define STM32_ADC_NUMBER                    18

#define STM32_HAS_ADC1                      TRUE
#define STM32_ADC1_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 4))
#define STM32_ADC1_DMA_CHN                  0x00000000

#define STM32_HAS_ADC2                      TRUE
#define STM32_ADC2_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 3))
#define STM32_ADC2_DMA_CHN                  0x00001100

#define STM32_HAS_ADC3                      TRUE
#define STM32_ADC3_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 1))
#define STM32_ADC3_DMA_CHN                  0x00000022

#define STM32_HAS_ADC4                      FALSE

#define STM32_HAS_SDADC1                    FALSE
#define STM32_HAS_SDADC2                    FALSE
#define STM32_HAS_SDADC3                    FALSE

/* CAN attributes.*/
#define STM32_CAN_MAX_FILTERS               28

#define STM32_HAS_CAN1                      TRUE
#define STM32_CAN1_TX_HANDLER               Vector8C
#define STM32_CAN1_RX0_HANDLER              Vector90
#define STM32_CAN1_RX1_HANDLER              Vector94
#define STM32_CAN1_SCE_HANDLER              Vector98
#define STM32_CAN1_TX_NUMBER                19
#define STM32_CAN1_RX0_NUMBER               20
#define STM32_CAN1_RX1_NUMBER               21
#define STM32_CAN1_SCE_NUMBER               22

#define STM32_HAS_CAN2                      TRUE
#define STM32_CAN2_TX_HANDLER               Vector13C
#define STM32_CAN2_RX0_HANDLER              Vector140
#define STM32_CAN2_RX1_HANDLER              Vector144
#define STM32_CAN2_SCE_HANDLER              Vector148
#define STM32_CAN2_TX_NUMBER                63
#define STM32_CAN2_RX0_NUMBER               64
#define STM32_CAN2_RX1_NUMBER               65
#define STM32_CAN2_SCE_NUMBER               66

#define STM32_CAN3_MAX_FILTERS              14

#define STM32_HAS_CAN3                      TRUE
#define STM32_CAN3_TX_HANDLER               Vector1E0
#define STM32_CAN3_RX0_HANDLER              Vector1E4
#define STM32_CAN3_RX1_HANDLER              Vector1E8
#define STM32_CAN3_SCE_HANDLER              Vector1EC
#define STM32_CAN3_TX_NUMBER                104
#define STM32_CAN3_RX0_NUMBER               105
#define STM32_CAN3_RX1_NUMBER               106
#define STM32_CAN3_SCE_NUMBER               107

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  TRUE
#define STM32_DAC1_CH1_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_DAC1_CH1_DMA_CHN              0x00700000

#define STM32_HAS_DAC1_CH2                  TRUE
#define STM32_DAC1_CH2_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_DAC1_CH2_DMA_CHN              0x07000000

#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_CACHE_HANDLING            TRUE
#define STM32_DMA_SUPPORTS_DMAMUX           FALSE

#define STM32_HAS_DMA1                      TRUE
#define STM32_DMA1_CH0_HANDLER              Vector6C
#define STM32_DMA1_CH1_HANDLER              Vector70
#define STM32_DMA1_CH2_HANDLER              Vector74
#define STM32_DMA1_CH3_HANDLER              Vector78
#define STM32_DMA1_CH4_HANDLER              Vector7C
#define STM32_DMA1_CH5_HANDLER              Vector80
#define STM32_DMA1_CH6_HANDLER              Vector84
#define STM32_DMA1_CH7_HANDLER              VectorFC
#define STM32_DMA1_CH0_NUMBER               11
#define STM32_DMA1_CH1_NUMBER               12
#define STM32_DMA1_CH2_NUMBER               13
#define STM32_DMA1_CH3_NUMBER               14
#define STM32_DMA1_CH4_NUMBER               15
#define STM32_DMA1_CH5_NUMBER               16
#define STM32_DMA1_CH6_NUMBER               17
#define STM32_DMA1_CH7_NUMBER               47

#define STM32_HAS_DMA2                      TRUE
#define STM32_DMA2_CH0_HANDLER              Vector120
#define STM32_DMA2_CH1_HANDLER              Vector124
#define STM32_DMA2_CH2_HANDLER              Vector128
#define STM32_DMA2_CH3_HANDLER              Vector12C
#define STM32_DMA2_CH4_HANDLER              Vector130
#define STM32_DMA2_CH5_HANDLER              Vector150
#define STM32_DMA2_CH6_HANDLER              Vector154
#define STM32_DMA2_CH7_HANDLER              Vector158
#define STM32_DMA2_CH0_NUMBER               56
#define STM32_DMA2_CH1_NUMBER               57
#define STM32_DMA2_CH2_NUMBER               58
#define STM32_DMA2_CH3_NUMBER               59
#define STM32_DMA2_CH4_NUMBER               60
#define STM32_DMA2_CH5_NUMBER               68
#define STM32_DMA2_CH6_NUMBER               69
#define STM32_DMA2_CH7_NUMBER               70

/* ETH attributes.*/
#define STM32_HAS_ETH                       TRUE
#define STM32_ETH_HANDLER                   Vector134
#define STM32_ETH_NUMBER                    61

/* EXTI attributes.*/
#define STM32_EXTI_NUM_LINES                24
#define STM32_EXTI_IMR1_MASK                0xFF000000

/* GPIO attributes.*/
#define STM32_HAS_GPIOA                     TRUE
#define STM32_HAS_GPIOB                     TRUE
#define STM32_HAS_GPIOC                     TRUE
#define STM32_HAS_GPIOD                     TRUE
#define STM32_HAS_GPIOE                     TRUE
#define STM32_HAS_GPIOH                     TRUE
#define STM32_HAS_GPIOF                     TRUE
#define STM32_HAS_GPIOG                     TRUE
#define STM32_HAS_GPIOI                     TRUE
#define STM32_HAS_GPIOJ                     TRUE
#define STM32_HAS_GPIOK                     TRUE
#define STM32_GPIO_EN_MASK                  (RCC_AHB1ENR_GPIOAEN |          \
                                             RCC_AHB1ENR_GPIOBEN |          \
                                             RCC_AHB1ENR_GPIOCEN |          \
                                             RCC_AHB1ENR_GPIODEN |          \
                                             RCC_AHB1ENR_GPIOEEN |          \
                                             RCC_AHB1ENR_GPIOFEN |          \
                                             RCC_AHB1ENR_GPIOGEN |          \
                                             RCC_AHB1ENR_GPIOHEN |          \
                                             RCC_AHB1ENR_GPIOIEN |          \
                                             RCC_AHB1ENR_GPIOJEN |          \
                                             RCC_AHB1ENR_GPIOKEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_EVENT_HANDLER            VectorBC
#define STM32_I2C1_ERROR_HANDLER            VectorC0
#define STM32_I2C1_EVENT_NUMBER             31
#define STM32_I2C1_ERROR_NUMBER             32
#define STM32_I2C1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 5))
#define STM32_I2C1_RX_DMA_CHN               0x00100001
#define STM32_I2C1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 7) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C1_TX_DMA_CHN               0x11000000

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_EVENT_HANDLER            VectorC4
#define STM32_I2C2_ERROR_HANDLER            VectorC8
#define STM32_I2C2_EVENT_NUMBER             33
#define STM32_I2C2_ERROR_NUMBER             34
#define STM32_I2C2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_I2C2_RX_DMA_CHN               0x00007700
#define STM32_I2C2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 7)
#define STM32_I2C2_TX_DMA_CHN               0x70000000

#define STM32_HAS_I2C3                      TRUE
#define STM32_I2C3_EVENT_HANDLER            Vector160
#define STM32_I2C3_ERROR_HANDLER            Vector164
#define STM32_I2C3_EVENT_NUMBER             72
#define STM32_I2C3_ERROR_NUMBER             73
#define STM32_I2C3_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_I2C3_RX_DMA_CHN               0x00000300
#define STM32_I2C3_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_I2C3_TX_DMA_CHN               0x00030000

#define STM32_HAS_I2C4                      TRUE
#define STM32_I2C4_EVENT_HANDLER            Vector1BC
#define STM32_I2C4_ERROR_HANDLER            Vector1C0
#define STM32_I2C4_EVENT_NUMBER             95
#define STM32_I2C4_ERROR_NUMBER             96
#define STM32_I2C4_RX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 2)
#define STM32_I2C4_RX_DMA_CHN               0x00000200
#define STM32_I2C4_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_I2C4_TX_DMA_CHN               0x00200000

/* QUADSPI attributes.*/
#define STM32_HAS_QUADSPI1                  TRUE
#define STM32_QUADSPI1_HANDLER              Vector1B0
#define STM32_QUADSPI1_NUMBER               92
#define STM32_QUADSPI1_DMA_MSK              STM32_DMA_STREAM_ID_MSK(2, 7)
#define STM32_QUADSPI1_DMA_CHN              0x30000000

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDMMC attributes.*/
#define STM32_HAS_SDMMC1                    TRUE
#define STM32_SDMMC1_HANDLER                Vector104
#define STM32_SDMMC1_NUMBER                 49
#define STM32_SDC_SDMMC1_DMA_MSK            (STM32_DMA_STREAM_ID_MSK(2, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SDC_SDMMC1_DMA_CHN            0x04004000

#define STM32_HAS_SDMMC2                    FALSE

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_SUPPORTS_I2S             TRUE
#define STM32_SPI1_I2S_FULLDUPLEX           TRUE
#define STM32_SPI1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2))
#define STM32_SPI1_RX_DMA_CHN               0x00000303
#define STM32_SPI1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI1_TX_DMA_CHN               0x00303000

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_SUPPORTS_I2S             TRUE
#define STM32_SPI2_I2S_FULLDUPLEX           TRUE
#define STM32_SPI2_RX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 3)
#define STM32_SPI2_RX_DMA_CHN               0x00000000
#define STM32_SPI2_TX_DMA_MSK               STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_SPI2_TX_DMA_CHN               0x00000000

#define STM32_HAS_SPI3                      TRUE
#define STM32_SPI3_SUPPORTS_I2S             TRUE
#define STM32_SPI3_I2S_FULLDUPLEX           TRUE
#define STM32_SPI3_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_SPI3_RX_DMA_CHN               0x00000000
#define STM32_SPI3_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_SPI3_TX_DMA_CHN               0x00000000

#define STM32_HAS_SPI4                      TRUE
#define STM32_SPI4_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 3))
#define STM32_SPI4_RX_DMA_CHN               0x00005004
#define STM32_SPI4_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 4))
#define STM32_SPI4_TX_DMA_CHN               0x00050040

#define STM32_HAS_SPI5                      TRUE
#define STM32_SPI5_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 3) |            \
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI5_RX_DMA_CHN               0x00702000
#define STM32_SPI5_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 4) |            \
                                             STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SPI5_TX_DMA_CHN               0x07020000

#define STM32_HAS_SPI6                      TRUE
#define STM32_SPI6_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SPI6_RX_DMA_CHN               0x01000000
#define STM32_SPI6_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI6_TX_DMA_CHN               0x00100000

/* TIM attributes.*/
#define STM32_TIM_MAX_CHANNELS              6

#define STM32_HAS_TIM1                      TRUE
#define STM32_TIM1_IS_32BITS                FALSE
#define STM32_TIM1_CHANNELS                 6
#define STM32_TIM1_UP_HANDLER               VectorA4
#define STM32_TIM1_CC_HANDLER               VectorAC
#define STM32_TIM1_UP_NUMBER                25
#define STM32_TIM1_CC_NUMBER                27

#define STM32_HAS_TIM2                      TRUE
#define STM32_TIM2_IS_32BITS                TRUE
#define STM32_TIM2_CHANNELS                 4
#define STM32_TIM2_HANDLER                  VectorB0
#define STM32_TIM2_NUMBER                   28

#define STM32_HAS_TIM3                      TRUE
#define STM32_TIM3_IS_32BITS                FALSE
#define STM32_TIM3_CHANNELS                 4
#define STM32_TIM3_HANDLER                  VectorB4
#define STM32_TIM3_NUMBER                   29

#define STM32_HAS_TIM4                      TRUE
#define STM32_TIM4_IS_32BITS                FALSE
#define STM32_TIM4_CHANNELS                 4
#define STM32_TIM4_HANDLER                  VectorB8
#define STM32_TIM4_NUMBER                   30

#define STM32_HAS_TIM5                      TRUE
#define STM32_TIM5_IS_32BITS                TRUE
#define STM32_TIM5_CHANNELS                 4
#define STM32_TIM5_HANDLER                  Vector108
#define STM32_TIM5_NUMBER                   50

#define STM32_HAS_TIM6                      TRUE
#define STM32_TIM6_IS_32BITS                FALSE
#define STM32_TIM6_CHANNELS                 0
#define STM32_TIM6_HANDLER                  Vector118
#define STM32_TIM6_NUMBER                   54

#define STM32_HAS_TIM7                      TRUE
#define STM32_TIM7_IS_32BITS                FALSE
#define STM32_TIM7_CHANNELS                 0
#define STM32_TIM7_HANDLER                  Vector11C
#define STM32_TIM7_NUMBER                   55

#define STM32_HAS_TIM8                      TRUE
#define STM32_TIM8_IS_32BITS                FALSE
#define STM32_TIM8_CHANNELS                 6
#define STM32_TIM8_UP_HANDLER               VectorF0
#define STM32_TIM8_CC_HANDLER               VectorF8
#define STM32_TIM8_UP_NUMBER                44
#define STM32_TIM8_CC_NUMBER                46

#define STM32_HAS_TIM9                      TRUE
#define STM32_TIM9_IS_32BITS                FALSE
#define STM32_TIM9_CHANNELS                 2
#define STM32_TIM9_HANDLER                  VectorA0
#define STM32_TIM9_NUMBER                   24

#define STM32_HAS_TIM10                     TRUE
#define STM32_TIM10_IS_32BITS               FALSE
#define STM32_TIM10_CHANNELS                1
#define STM32_TIM10_HANDLER                 VectorA4 /* Note: same as STM32_TIM1_UP */
#define STM32_TIM10_NUMBER                  25 /* Note: same as STM32_TIM1_UP */

#define STM32_HAS_TIM11                     TRUE
#define STM32_TIM11_IS_32BITS               FALSE
#define STM32_TIM11_CHANNELS                1
#define STM32_TIM11_HANDLER                 VectorA8
#define STM32_TIM11_NUMBER                  26

#define STM32_HAS_TIM12                     TRUE
#define STM32_TIM12_IS_32BITS               FALSE
#define STM32_TIM12_CHANNELS                2
#define STM32_TIM12_HANDLER                 VectorEC
#define STM32_TIM12_NUMBER                  43

#define STM32_HAS_TIM13                     TRUE
#define STM32_TIM13_IS_32BITS               FALSE
#define STM32_TIM13_CHANNELS                1
#define STM32_TIM13_HANDLER                 VectorF0 /* Note: same as STM32_TIM8_UP */
#define STM32_TIM13_NUMBER                  44 /* Note: same as STM32_TIM8_UP */

#define STM32_HAS_TIM14                     TRUE
#define STM32_TIM14_IS_32BITS               FALSE
#define STM32_TIM14_CHANNELS                1
#define STM32_TIM14_HANDLER                 VectorF4
#define STM32_TIM14_NUMBER                  45

#define STM32_HAS_TIM15                     FALSE
#define STM32_HAS_TIM16                     FALSE
#define STM32_HAS_TIM17                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE
#define STM32_HAS_TIM21                     FALSE
#define STM32_HAS_TIM22                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorD4
#define STM32_USART1_NUMBER                 37
#define STM32_USART1_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_USART1_RX_DMA_CHN             0x00400400
#define STM32_USART1_TX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(2, 7)
#define STM32_USART1_TX_DMA_CHN             0x40000000

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorD8
#define STM32_USART2_NUMBER                 38
#define STM32_USART2_RX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_USART2_RX_DMA_CHN             0x00400000
#define STM32_USART2_TX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_USART2_TX_DMA_CHN             0x04000000

#define STM32_HAS_USART3                    TRUE
#define STM32_USART3_HANDLER                VectorDC
#define STM32_USART3_NUMBER                 39
#define STM32_USART3_RX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 1)
#define STM32_USART3_RX_DMA_CHN             0x00000040
#define STM32_USART3_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_USART3_TX_DMA_CHN             0x00074000

#define STM32_HAS_UART4                     TRUE
#define STM32_UART4_HANDLER                 Vector110
#define STM32_UART4_NUMBER                  52
#define STM32_UART4_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 2)
#define STM32_UART4_RX_DMA_CHN              0x00000400
#define STM32_UART4_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_UART4_TX_DMA_CHN              0x00040000

#define STM32_HAS_UART5                     TRUE
#define STM32_UART5_HANDLER                 Vector114
#define STM32_UART5_NUMBER                  53
#define STM32_UART5_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 0)
#define STM32_UART5_RX_DMA_CHN              0x00000004
#define STM32_UART5_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 7)
#define STM32_UART5_TX_DMA_CHN              0x40000000

#define STM32_HAS_USART6                    TRUE
#define STM32_USART6_HANDLER                Vector15C
#define STM32_USART6_NUMBER                 71
#define STM32_USART6_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2))
#define STM32_USART6_RX_DMA_CHN             0x00000550
#define STM32_USART6_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 6) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 7))
#define STM32_USART6_TX_DMA_CHN             0x55000000

#define STM32_HAS_UART7                     TRUE
#define STM32_UART7_HANDLER                 Vector188
#define STM32_UART7_NUMBER                  82
#define STM32_UART7_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 3)
#define STM32_UART7_RX_DMA_CHN              0x00005000
#define STM32_UART7_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 1)
#define STM32_UART7_TX_DMA_CHN              0x00000050

#define STM32_HAS_UART8                     TRUE
#define STM32_UART8_HANDLER                 Vector18C
#define STM32_UART8_NUMBER                  83
#define STM32_UART8_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_UART8_RX_DMA_CHN              0x05000000
#define STM32_UART8_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 0)
#define STM32_UART8_TX_DMA_CHN              0x00000005

#define STM32_HAS_LPUART1                   FALSE

/* USB attributes.*/
#define STM32_OTG_STEPPING                  2
#define STM32_HAS_OTG1                      TRUE
#define STM32_OTG1_ENDPOINTS                5
#define STM32_OTG1_HANDLER                  Vector14C
#define STM32_OTG1_NUMBER                   67

#define STM32_HAS_OTG2                      TRUE
#define STM32_OTG2_ENDPOINTS                8
#define STM32_OTG2_HANDLER                  Vector174
#define STM32_OTG2_EP1OUT_HANDLER           Vector168
#define STM32_OTG2_EP1IN_HANDLER            Vector16C
#define STM32_OTG2_NUMBER                   77
#define STM32_OTG2_EP1OUT_NUMBER            74
#define STM32_OTG2_EP1IN_NUMBER             75

#define STM32_HAS_USB                       FALSE

/* IWDG attributes.*/
#define STM32_HAS_IWDG                      TRUE
#define STM32_IWDG_IS_WINDOWED              TRUE

/* LTDC attributes.*/
#define STM32_HAS_LTDC                      TRUE

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     TRUE

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      TRUE
#define STM32_FSMC_IS_FMC                   TRUE
#define STM32_FSMC_HANDLER                  Vector100
#define STM32_FSMC_NUMBER                   48

/* LTDC attributes.*/
#define STM32_LTDC_EV_HANDLER               Vector1A0
#define STM32_LTDC_ER_HANDLER               Vector1A4
#define STM32_LTDC_EV_NUMBER                88
#define STM32_LTDC_ER_NUMBER                89

/* DMA2D attributes.*/
#define STM32_DMA2D_HANDLER                 Vector1A8
#define STM32_DMA2D_NUMBER                  90

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              FALSE

#endif /* defined(STM32F745xx) || defined(STM32F746xx) || defined(STM32F756xx) */

/*===========================================================================*/
/* STM32F767xx, STM32F769xx, STM32F777xx, STM32F779xx.                       */
/*===========================================================================*/
#if defined(STM32F767xx) || defined(STM32F769xx) ||                         \
    defined(STM32F777xx) || defined(STM32F779xx) ||                         \
    defined(__DOXYGEN__)
/* ADC attributes.*/
#define STM32_ADC_HANDLER                   Vector88
#define STM32_ADC_NUMBER                    18

#define STM32_HAS_ADC1                      TRUE
#define STM32_ADC1_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 4))
#define STM32_ADC1_DMA_CHN                  0x00000000

#define STM32_HAS_ADC2                      TRUE
#define STM32_ADC2_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 3))
#define STM32_ADC2_DMA_CHN                  0x00001100

#define STM32_HAS_ADC3                      TRUE
#define STM32_ADC3_DMA_MSK                  (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 1))
#define STM32_ADC3_DMA_CHN                  0x00000022

#define STM32_HAS_ADC4                      FALSE

#define STM32_HAS_SDADC1                    FALSE
#define STM32_HAS_SDADC2                    FALSE
#define STM32_HAS_SDADC3                    FALSE

/* CAN attributes.*/
#define STM32_CAN_MAX_FILTERS               28

#define STM32_HAS_CAN1                      TRUE
#define STM32_CAN1_TX_HANDLER               Vector8C
#define STM32_CAN1_RX0_HANDLER              Vector90
#define STM32_CAN1_RX1_HANDLER              Vector94
#define STM32_CAN1_SCE_HANDLER              Vector98
#define STM32_CAN1_TX_NUMBER                19
#define STM32_CAN1_RX0_NUMBER               20
#define STM32_CAN1_RX1_NUMBER               21
#define STM32_CAN1_SCE_NUMBER               22

#define STM32_HAS_CAN2                      TRUE
#define STM32_CAN2_TX_HANDLER               Vector13C
#define STM32_CAN2_RX0_HANDLER              Vector140
#define STM32_CAN2_RX1_HANDLER              Vector144
#define STM32_CAN2_SCE_HANDLER              Vector148
#define STM32_CAN2_TX_NUMBER                63
#define STM32_CAN2_RX0_NUMBER               64
#define STM32_CAN2_RX1_NUMBER               65
#define STM32_CAN2_SCE_NUMBER               66

#define STM32_CAN3_MAX_FILTERS              14

#define STM32_HAS_CAN3                      TRUE
#define STM32_CAN3_TX_HANDLER               Vector1E0
#define STM32_CAN3_RX0_HANDLER              Vector1E4
#define STM32_CAN3_RX1_HANDLER              Vector1E8
#define STM32_CAN3_SCE_HANDLER              Vector1EC
#define STM32_CAN3_TX_NUMBER                104
#define STM32_CAN3_RX0_NUMBER               105
#define STM32_CAN3_RX1_NUMBER               106
#define STM32_CAN3_SCE_NUMBER               107

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  TRUE
#define STM32_DAC1_CH1_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_DAC1_CH1_DMA_CHN              0x00700000

#define STM32_HAS_DAC1_CH2                  TRUE
#define STM32_DAC1_CH2_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_DAC1_CH2_DMA_CHN              0x07000000

#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_CACHE_HANDLING            TRUE
#define STM32_DMA_SUPPORTS_DMAMUX           FALSE

#define STM32_HAS_DMA1                      TRUE
#define STM32_DMA1_CH0_HANDLER              Vector6C
#define STM32_DMA1_CH1_HANDLER              Vector70
#define STM32_DMA1_CH2_HANDLER              Vector74
#define STM32_DMA1_CH3_HANDLER              Vector78
#define STM32_DMA1_CH4_HANDLER              Vector7C
#define STM32_DMA1_CH5_HANDLER              Vector80
#define STM32_DMA1_CH6_HANDLER              Vector84
#define STM32_DMA1_CH7_HANDLER              VectorFC
#define STM32_DMA1_CH0_NUMBER               11
#define STM32_DMA1_CH1_NUMBER               12
#define STM32_DMA1_CH2_NUMBER               13
#define STM32_DMA1_CH3_NUMBER               14
#define STM32_DMA1_CH4_NUMBER               15
#define STM32_DMA1_CH5_NUMBER               16
#define STM32_DMA1_CH6_NUMBER               17
#define STM32_DMA1_CH7_NUMBER               47

#define STM32_HAS_DMA2                      TRUE
#define STM32_DMA2_CH0_HANDLER              Vector120
#define STM32_DMA2_CH1_HANDLER              Vector124
#define STM32_DMA2_CH2_HANDLER              Vector128
#define STM32_DMA2_CH3_HANDLER              Vector12C
#define STM32_DMA2_CH4_HANDLER              Vector130
#define STM32_DMA2_CH5_HANDLER              Vector150
#define STM32_DMA2_CH6_HANDLER              Vector154
#define STM32_DMA2_CH7_HANDLER              Vector158
#define STM32_DMA2_CH0_NUMBER               56
#define STM32_DMA2_CH1_NUMBER               57
#define STM32_DMA2_CH2_NUMBER               58
#define STM32_DMA2_CH3_NUMBER               59
#define STM32_DMA2_CH4_NUMBER               60
#define STM32_DMA2_CH5_NUMBER               68
#define STM32_DMA2_CH6_NUMBER               69
#define STM32_DMA2_CH7_NUMBER               70

/* ETH attributes.*/
#define STM32_HAS_ETH                       TRUE
#define STM32_ETH_HANDLER                   Vector134
#define STM32_ETH_NUMBER                    61

/* EXTI attributes.*/
#define STM32_EXTI_NUM_LINES                24
#define STM32_EXTI_IMR1_MASK                0xFF000000

/* GPIO attributes.*/
#define STM32_HAS_GPIOA                     TRUE
#define STM32_HAS_GPIOB                     TRUE
#define STM32_HAS_GPIOC                     TRUE
#define STM32_HAS_GPIOD                     TRUE
#define STM32_HAS_GPIOE                     TRUE
#define STM32_HAS_GPIOH                     TRUE
#define STM32_HAS_GPIOF                     TRUE
#define STM32_HAS_GPIOG                     TRUE
#define STM32_HAS_GPIOI                     TRUE
#define STM32_HAS_GPIOJ                     TRUE
#define STM32_HAS_GPIOK                     TRUE
#define STM32_GPIO_EN_MASK                  (RCC_AHB1ENR_GPIOAEN |          \
                                             RCC_AHB1ENR_GPIOBEN |          \
                                             RCC_AHB1ENR_GPIOCEN |          \
                                             RCC_AHB1ENR_GPIODEN |          \
                                             RCC_AHB1ENR_GPIOEEN |          \
                                             RCC_AHB1ENR_GPIOFEN |          \
                                             RCC_AHB1ENR_GPIOGEN |          \
                                             RCC_AHB1ENR_GPIOHEN |          \
                                             RCC_AHB1ENR_GPIOIEN |          \
                                             RCC_AHB1ENR_GPIOJEN |          \
                                             RCC_AHB1ENR_GPIOKEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_EVENT_HANDLER            VectorBC
#define STM32_I2C1_ERROR_HANDLER            VectorC0
#define STM32_I2C1_EVENT_NUMBER             31
#define STM32_I2C1_ERROR_NUMBER             32
#define STM32_I2C1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 5))
#define STM32_I2C1_RX_DMA_CHN               0x00100001
#define STM32_I2C1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 7) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C1_TX_DMA_CHN               0x11000000

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_EVENT_HANDLER            VectorC4
#define STM32_I2C2_ERROR_HANDLER            VectorC8
#define STM32_I2C2_EVENT_NUMBER             33
#define STM32_I2C2_ERROR_NUMBER             34
#define STM32_I2C2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_I2C2_RX_DMA_CHN               0x00007700
#define STM32_I2C2_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_I2C2_TX_DMA_CHN               0x70080000

#define STM32_HAS_I2C3                      TRUE
#define STM32_I2C3_EVENT_HANDLER            Vector160
#define STM32_I2C3_ERROR_HANDLER            Vector164
#define STM32_I2C3_EVENT_NUMBER             72
#define STM32_I2C3_ERROR_NUMBER             73
#define STM32_I2C3_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_I2C3_RX_DMA_CHN               0x00000310
#define STM32_I2C3_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_I2C3_TX_DMA_CHN               0x00030008

#define STM32_HAS_I2C4                      TRUE
#define STM32_I2C4_EVENT_HANDLER            Vector1BC
#define STM32_I2C4_ERROR_HANDLER            Vector1C0
#define STM32_I2C4_EVENT_NUMBER             95
#define STM32_I2C4_ERROR_NUMBER             96
#define STM32_I2C4_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 1))
#define STM32_I2C4_RX_DMA_CHN               0x00000280
#define STM32_I2C4_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_I2C4_TX_DMA_CHN               0x08200000

/* QUADSPI attributes.*/
#define STM32_HAS_QUADSPI1                  TRUE
#define STM32_QUADSPI1_HANDLER              Vector1B0
#define STM32_QUADSPI1_NUMBER               92
#define STM32_QUADSPI1_DMA_MSK              (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 7))
#define STM32_QUADSPI1_DMA_CHN              0x30000B00

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDMMC attributes.*/
#define STM32_HAS_SDMMC1                    TRUE
#define STM32_SDMMC1_HANDLER                Vector104
#define STM32_SDMMC1_NUMBER                 49
#define STM32_SDC_SDMMC1_DMA_MSK            (STM32_DMA_STREAM_ID_MSK(2, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SDC_SDMMC1_DMA_CHN            0x04004000

#define STM32_HAS_SDMMC2                    TRUE
#define STM32_SDMMC2_HANDLER                Vector1DC
#define STM32_SDMMC2_NUMBER                 103
#define STM32_SDC_SDMMC2_DMA_MSK            (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SDC_SDMMC2_DMA_CHN            0x00B0000B

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_SUPPORTS_I2S             TRUE
#define STM32_SPI1_I2S_FULLDUPLEX           TRUE
#define STM32_SPI1_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2))
#define STM32_SPI1_RX_DMA_CHN               0x00000303
#define STM32_SPI1_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI1_TX_DMA_CHN               0x00303000

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_SUPPORTS_I2S             TRUE
#define STM32_SPI2_I2S_FULLDUPLEX           TRUE
#define STM32_SPI2_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 3))
#define STM32_SPI2_RX_DMA_CHN               0x00000090
#define STM32_SPI2_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 4) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 6))
#define STM32_SPI2_TX_DMA_CHN               0x09000000

#define STM32_HAS_SPI3                      TRUE
#define STM32_SPI3_SUPPORTS_I2S             TRUE
#define STM32_SPI3_I2S_FULLDUPLEX           TRUE
#define STM32_SPI3_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 2))
#define STM32_SPI3_RX_DMA_CHN               0x00000000
#define STM32_SPI3_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(1, 5) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 7))
#define STM32_SPI3_TX_DMA_CHN               0x00000000

#define STM32_HAS_SPI4                      TRUE
#define STM32_SPI4_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 0) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 3))
#define STM32_SPI4_RX_DMA_CHN               0x00005004
#define STM32_SPI4_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 4))
#define STM32_SPI4_TX_DMA_CHN               0x00050940

#define STM32_HAS_SPI5                      TRUE
#define STM32_SPI5_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 3)|\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI5_RX_DMA_CHN               0x00902000
#define STM32_SPI5_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 4)|\
                                             STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SPI5_TX_DMA_CHN               0x07020000

#define STM32_HAS_SPI6                      TRUE
#define STM32_SPI6_RX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 6))
#define STM32_SPI6_RX_DMA_CHN               0x01000000
#define STM32_SPI6_TX_DMA_MSK               (STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_SPI6_TX_DMA_CHN               0x00100000

/* TIM attributes.*/
#define STM32_TIM_MAX_CHANNELS              6

#define STM32_HAS_TIM1                      TRUE
#define STM32_TIM1_IS_32BITS                FALSE
#define STM32_TIM1_CHANNELS                 6
#define STM32_TIM1_UP_HANDLER               VectorA4
#define STM32_TIM1_CC_HANDLER               VectorAC
#define STM32_TIM1_UP_NUMBER                25
#define STM32_TIM1_CC_NUMBER                27

#define STM32_HAS_TIM2                      TRUE
#define STM32_TIM2_IS_32BITS                TRUE
#define STM32_TIM2_CHANNELS                 4
#define STM32_TIM2_HANDLER                  VectorB0
#define STM32_TIM2_NUMBER                   28

#define STM32_HAS_TIM3                      TRUE
#define STM32_TIM3_IS_32BITS                FALSE
#define STM32_TIM3_CHANNELS                 4
#define STM32_TIM3_HANDLER                  VectorB4
#define STM32_TIM3_NUMBER                   29

#define STM32_HAS_TIM4                      TRUE
#define STM32_TIM4_IS_32BITS                FALSE
#define STM32_TIM4_CHANNELS                 4
#define STM32_TIM4_HANDLER                  VectorB8
#define STM32_TIM4_NUMBER                   30

#define STM32_HAS_TIM5                      TRUE
#define STM32_TIM5_IS_32BITS                TRUE
#define STM32_TIM5_CHANNELS                 4
#define STM32_TIM5_HANDLER                  Vector108
#define STM32_TIM5_NUMBER                   50

#define STM32_HAS_TIM6                      TRUE
#define STM32_TIM6_IS_32BITS                FALSE
#define STM32_TIM6_CHANNELS                 0
#define STM32_TIM6_HANDLER                  Vector118
#define STM32_TIM6_NUMBER                   54

#define STM32_HAS_TIM7                      TRUE
#define STM32_TIM7_IS_32BITS                FALSE
#define STM32_TIM7_CHANNELS                 0
#define STM32_TIM7_HANDLER                  Vector11C
#define STM32_TIM7_NUMBER                   55

#define STM32_HAS_TIM8                      TRUE
#define STM32_TIM8_IS_32BITS                FALSE
#define STM32_TIM8_CHANNELS                 6
#define STM32_TIM8_UP_HANDLER               VectorF0
#define STM32_TIM8_CC_HANDLER               VectorF8
#define STM32_TIM8_UP_NUMBER                44
#define STM32_TIM8_CC_NUMBER                46

#define STM32_HAS_TIM9                      TRUE
#define STM32_TIM9_IS_32BITS                FALSE
#define STM32_TIM9_CHANNELS                 2
#define STM32_TIM9_HANDLER                  VectorA0
#define STM32_TIM9_NUMBER                   24

#define STM32_HAS_TIM10                     TRUE
#define STM32_TIM10_IS_32BITS               FALSE
#define STM32_TIM10_CHANNELS                1
#define STM32_TIM10_HANDLER                 VectorA4 /* Note: same as STM32_TIM1_UP */
#define STM32_TIM10_NUMBER                  25 /* Note: same as STM32_TIM1_UP */

#define STM32_HAS_TIM11                     TRUE
#define STM32_TIM11_IS_32BITS               FALSE
#define STM32_TIM11_CHANNELS                1
#define STM32_TIM11_HANDLER                 VectorA8
#define STM32_TIM11_NUMBER                  26

#define STM32_HAS_TIM12                     TRUE
#define STM32_TIM12_IS_32BITS               FALSE
#define STM32_TIM12_CHANNELS                2
#define STM32_TIM12_HANDLER                 VectorEC
#define STM32_TIM12_NUMBER                  43

#define STM32_HAS_TIM13                     TRUE
#define STM32_TIM13_IS_32BITS               FALSE
#define STM32_TIM13_CHANNELS                1
#define STM32_TIM13_HANDLER                 VectorF0 /* Note: same as STM32_TIM8_UP */
#define STM32_TIM13_NUMBER                  44 /* Note: same as STM32_TIM8_UP */

#define STM32_HAS_TIM14                     TRUE
#define STM32_TIM14_IS_32BITS               FALSE
#define STM32_TIM14_CHANNELS                1
#define STM32_TIM14_HANDLER                 VectorF4
#define STM32_TIM14_NUMBER                  45

#define STM32_HAS_TIM15                     FALSE
#define STM32_HAS_TIM16                     FALSE
#define STM32_HAS_TIM17                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE
#define STM32_HAS_TIM21                     FALSE
#define STM32_HAS_TIM22                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorD4
#define STM32_USART1_NUMBER                 37
#define STM32_USART1_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 2) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 5))
#define STM32_USART1_RX_DMA_CHN             0x00400400
#define STM32_USART1_TX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(2, 7)
#define STM32_USART1_TX_DMA_CHN             0x40000000

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorD8
#define STM32_USART2_NUMBER                 38
#define STM32_USART2_RX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 5)
#define STM32_USART2_RX_DMA_CHN             0x00400000
#define STM32_USART2_TX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_USART2_TX_DMA_CHN             0x04000000

#define STM32_HAS_USART3                    TRUE
#define STM32_USART3_HANDLER                VectorDC
#define STM32_USART3_NUMBER                 39
#define STM32_USART3_RX_DMA_MSK             STM32_DMA_STREAM_ID_MSK(1, 1)
#define STM32_USART3_RX_DMA_CHN             0x00000040
#define STM32_USART3_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(1, 3) |\
                                             STM32_DMA_STREAM_ID_MSK(1, 4))
#define STM32_USART3_TX_DMA_CHN             0x00074000

#define STM32_HAS_UART4                     TRUE
#define STM32_UART4_HANDLER                 Vector110
#define STM32_UART4_NUMBER                  52
#define STM32_UART4_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 2)
#define STM32_UART4_RX_DMA_CHN              0x00000400
#define STM32_UART4_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 4)
#define STM32_UART4_TX_DMA_CHN              0x00040000

#define STM32_HAS_UART5                     TRUE
#define STM32_UART5_HANDLER                 Vector114
#define STM32_UART5_NUMBER                  53
#define STM32_UART5_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 0)
#define STM32_UART5_RX_DMA_CHN              0x00000004
#define STM32_UART5_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 7)
#define STM32_UART5_TX_DMA_CHN              0x40000000

#define STM32_HAS_USART6                    TRUE
#define STM32_USART6_HANDLER                Vector15C
#define STM32_USART6_NUMBER                 71
#define STM32_USART6_RX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 1) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 2))
#define STM32_USART6_RX_DMA_CHN             0x00000550
#define STM32_USART6_TX_DMA_MSK             (STM32_DMA_STREAM_ID_MSK(2, 6) |\
                                             STM32_DMA_STREAM_ID_MSK(2, 7))
#define STM32_USART6_TX_DMA_CHN             0x55000000

#define STM32_HAS_UART7                     TRUE
#define STM32_UART7_HANDLER                 Vector188
#define STM32_UART7_NUMBER                  82
#define STM32_UART7_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 3)
#define STM32_UART7_RX_DMA_CHN              0x00005000
#define STM32_UART7_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 1)
#define STM32_UART7_TX_DMA_CHN              0x00000050

#define STM32_HAS_UART8                     TRUE
#define STM32_UART8_HANDLER                 Vector18C
#define STM32_UART8_NUMBER                  83
#define STM32_UART8_RX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 6)
#define STM32_UART8_RX_DMA_CHN              0x05000000
#define STM32_UART8_TX_DMA_MSK              STM32_DMA_STREAM_ID_MSK(1, 0)
#define STM32_UART8_TX_DMA_CHN              0x00000005

#define STM32_HAS_LPUART1                   FALSE

/* USB attributes.*/
#define STM32_OTG_STEPPING                  2
#define STM32_HAS_OTG1                      TRUE
#define STM32_OTG1_ENDPOINTS                5
#define STM32_OTG1_HANDLER                  Vector14C
#define STM32_OTG1_NUMBER                   67

#define STM32_HAS_OTG2                      TRUE
#define STM32_OTG2_ENDPOINTS                8
#define STM32_OTG2_HANDLER                  Vector174
#define STM32_OTG2_EP1OUT_HANDLER           Vector168
#define STM32_OTG2_EP1IN_HANDLER            Vector16C
#define STM32_OTG2_NUMBER                   77
#define STM32_OTG2_EP1OUT_NUMBER            74
#define STM32_OTG2_EP1IN_NUMBER             75

#define STM32_HAS_USB                       FALSE

/* IWDG attributes.*/
#define STM32_HAS_IWDG                      TRUE
#define STM32_IWDG_IS_WINDOWED              TRUE

/* LTDC attributes.*/
#define STM32_HAS_LTDC                      TRUE

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     TRUE

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      TRUE
#define STM32_FSMC_IS_FMC                   TRUE
#define STM32_FSMC_HANDLER                  Vector100
#define STM32_FSMC_NUMBER                   48

/* LTDC attributes.*/
#define STM32_LTDC_EV_HANDLER               Vector1A0
#define STM32_LTDC_ER_HANDLER               Vector1A4
#define STM32_LTDC_EV_NUMBER                88
#define STM32_LTDC_ER_NUMBER                89

/* DMA2D attributes.*/
#define STM32_DMA2D_HANDLER                 Vector1A8
#define STM32_DMA2D_NUMBER                  90

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              FALSE

#endif /* defined(STM32F767xx) || defined(STM32F769xx) ||
          defined(STM32F777xx) || defined(STM32F779xx) */
/** @} */

#endif /* STM32_REGISTRY_H */

/** @} */
