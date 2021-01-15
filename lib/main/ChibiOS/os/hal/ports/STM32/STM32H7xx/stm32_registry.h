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
 * @file    STM32H7xx/stm32_registry.h
 * @brief   STM32H7xx capabilities registry.
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
/* STM32H743xx, STM32H753xx.                                                 */
/*===========================================================================*/
#if defined(STM32H743xx) || defined(STM32H753xx) || defined(STM32H750xx) ||   \
    defined(__DOXYGEN__)

/* ADC attributes.*/
#define STM32_ADC12_HANDLER                 Vector88
#define STM32_ADC12_NUMBER                  18
#define STM32_ADC3_HANDLER                  Vector23C
#define STM32_ADC3_NUMBER                   127

#define STM32_HAS_ADC1                      TRUE

#define STM32_HAS_ADC2                      TRUE

#define STM32_HAS_ADC3                      TRUE

#define STM32_HAS_ADC4                      FALSE

#define STM32_HAS_SDADC1                    FALSE
#define STM32_HAS_SDADC2                    FALSE
#define STM32_HAS_SDADC3                    FALSE

/* CAN attributes.*/
#define STM32_HAS_CAN1                      FALSE
#define STM32_HAS_CAN2                      FALSE
#define STM32_HAS_CAN3                      FALSE

/* DAC attributes.*/
#define STM32_HAS_DAC1_CH1                  TRUE
#define STM32_HAS_DAC1_CH2                  TRUE
#define STM32_HAS_DAC2_CH1                  FALSE
#define STM32_HAS_DAC2_CH2                  FALSE

/* BDMA attributes.*/
#define STM32_HAS_BDMA1                     TRUE
#define STM32_BDMA1_CH0_HANDLER             Vector244
#define STM32_BDMA1_CH1_HANDLER             Vector248
#define STM32_BDMA1_CH2_HANDLER             Vector24C
#define STM32_BDMA1_CH3_HANDLER             Vector250
#define STM32_BDMA1_CH4_HANDLER             Vector254
#define STM32_BDMA1_CH5_HANDLER             Vector258
#define STM32_BDMA1_CH6_HANDLER             Vector25C
#define STM32_BDMA1_CH7_HANDLER             Vector260
#define STM32_BDMA1_CH0_NUMBER              129
#define STM32_BDMA1_CH1_NUMBER              130
#define STM32_BDMA1_CH2_NUMBER              131
#define STM32_BDMA1_CH3_NUMBER              132
#define STM32_BDMA1_CH4_NUMBER              133
#define STM32_BDMA1_CH5_NUMBER              134
#define STM32_BDMA1_CH6_NUMBER              135
#define STM32_BDMA1_CH7_NUMBER              136

/* DMA attributes.*/
#define STM32_ADVANCED_DMA                  TRUE
#define STM32_DMA_SUPPORTS_DMAMUX           TRUE

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
#define STM32_EXTI_ENHANCED
#define STM32_EXTI_NUM_LINES                34
#define STM32_EXTI_IMR1_MASK                0x1F800000U
#define STM32_EXTI_IMR2_MASK                0xFFFFFFFCU

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
#define STM32_GPIO_EN_MASK                  (RCC_AHB4ENR_GPIOAEN |          \
                                             RCC_AHB4ENR_GPIOBEN |          \
                                             RCC_AHB4ENR_GPIOCEN |          \
                                             RCC_AHB4ENR_GPIODEN |          \
                                             RCC_AHB4ENR_GPIOEEN |          \
                                             RCC_AHB4ENR_GPIOFEN |          \
                                             RCC_AHB4ENR_GPIOGEN |          \
                                             RCC_AHB4ENR_GPIOHEN |          \
                                             RCC_AHB4ENR_GPIOIEN |          \
                                             RCC_AHB4ENR_GPIOJEN |          \
                                             RCC_AHB4ENR_GPIOKEN)

/* I2C attributes.*/
#define STM32_HAS_I2C1                      TRUE
#define STM32_I2C1_EVENT_HANDLER            VectorBC
#define STM32_I2C1_ERROR_HANDLER            VectorC0
#define STM32_I2C1_EVENT_NUMBER             31
#define STM32_I2C1_ERROR_NUMBER             32

#define STM32_HAS_I2C2                      TRUE
#define STM32_I2C2_EVENT_HANDLER            VectorC4
#define STM32_I2C2_ERROR_HANDLER            VectorC8
#define STM32_I2C2_EVENT_NUMBER             33
#define STM32_I2C2_ERROR_NUMBER             34

#define STM32_HAS_I2C3                      TRUE
#define STM32_I2C3_EVENT_HANDLER            Vector160
#define STM32_I2C3_ERROR_HANDLER            Vector164
#define STM32_I2C3_EVENT_NUMBER             72
#define STM32_I2C3_ERROR_NUMBER             73

#define STM32_HAS_I2C4                      TRUE
#define STM32_I2C4_EVENT_HANDLER            Vector1BC
#define STM32_I2C4_ERROR_HANDLER            Vector1C0
#define STM32_I2C4_EVENT_NUMBER             95
#define STM32_I2C4_ERROR_NUMBER             96

/* QUADSPI attributes.*/
#define STM32_HAS_QUADSPI1                  FALSE

/* RTC attributes.*/
#define STM32_HAS_RTC                       TRUE
#define STM32_RTC_HAS_SUBSECONDS            TRUE
#define STM32_RTC_HAS_PERIODIC_WAKEUPS      TRUE
#define STM32_RTC_NUM_ALARMS                2
#define STM32_RTC_HAS_INTERRUPTS            FALSE

/* SDMMC attributes.*/
#define STM32_HAS_SDMMC1                    FALSE
#define STM32_HAS_SDMMC2                    FALSE

/* SPI attributes.*/
#define STM32_HAS_SPI1                      TRUE
#define STM32_SPI1_SUPPORTS_I2S             TRUE
#define STM32_SPI1_I2S_FULLDUPLEX           TRUE
#define STM32_SPI1_HANDLER                  VectorCC
#define STM32_SPI1_NUMBER                   35

#define STM32_HAS_SPI2                      TRUE
#define STM32_SPI2_SUPPORTS_I2S             TRUE
#define STM32_SPI2_I2S_FULLDUPLEX           TRUE
#define STM32_SPI2_HANDLER                  VectorD0
#define STM32_SPI2_NUMBER                   36

#define STM32_HAS_SPI3                      TRUE
#define STM32_SPI3_SUPPORTS_I2S             TRUE
#define STM32_SPI3_I2S_FULLDUPLEX           TRUE
#define STM32_SPI3_HANDLER                  Vector10C
#define STM32_SPI3_NUMBER                   51

#define STM32_HAS_SPI4                      TRUE
#define STM32_SPI4_SUPPORTS_I2S             FALSE
#define STM32_SPI4_HANDLER                  Vector190
#define STM32_SPI4_NUMBER                   84

#define STM32_HAS_SPI5                      TRUE
#define STM32_SPI5_SUPPORTS_I2S             FALSE
#define STM32_SPI5_HANDLER                  Vector194
#define STM32_SPI5_NUMBER                   85

#define STM32_HAS_SPI6                      TRUE
#define STM32_SPI6_SUPPORTS_I2S             FALSE
#define STM32_SPI6_HANDLER                  Vector198
#define STM32_SPI6_NUMBER                   86

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

#define STM32_HAS_TIM12                     TRUE
#define STM32_TIM12_IS_32BITS               FALSE
#define STM32_TIM12_CHANNELS                2
#define STM32_TIM12_HANDLER                 VectorEC
#define STM32_TIM12_NUMBER                  43

#define STM32_HAS_TIM13                     TRUE
#define STM32_TIM13_IS_32BITS               FALSE
#define STM32_TIM13_CHANNELS                1
#define STM32_TIM13_HANDLER                 VectorF0
#define STM32_TIM13_NUMBER                  44

#define STM32_HAS_TIM14                     TRUE
#define STM32_TIM14_IS_32BITS               FALSE
#define STM32_TIM14_CHANNELS                1
#define STM32_TIM14_HANDLER                 VectorF4
#define STM32_TIM14_NUMBER                  45

#define STM32_HAS_TIM15                     FALSE
#define STM32_TIM15_IS_32BITS               FALSE
#define STM32_TIM15_CHANNELS                2
#define STM32_TIM15_HANDLER                 Vector210
#define STM32_TIM15_NUMBER                  116

#define STM32_HAS_TIM16                     FALSE
#define STM32_TIM16_IS_32BITS               FALSE
#define STM32_TIM16_CHANNELS                1
#define STM32_TIM16_HANDLER                 Vector214
#define STM32_TIM16_NUMBER                  117

#define STM32_HAS_TIM17                     FALSE
#define STM32_TIM17_IS_32BITS               FALSE
#define STM32_TIM17_CHANNELS                1
#define STM32_TIM17_HANDLER                 Vector218
#define STM32_TIM17_NUMBER                  118

#define STM32_HAS_TIM8                      FALSE
#define STM32_HAS_TIM9                      FALSE
#define STM32_HAS_TIM10                     FALSE
#define STM32_HAS_TIM11                     FALSE
#define STM32_HAS_TIM18                     FALSE
#define STM32_HAS_TIM19                     FALSE
#define STM32_HAS_TIM20                     FALSE
#define STM32_HAS_TIM21                     FALSE
#define STM32_HAS_TIM22                     FALSE

/* USART attributes.*/
#define STM32_HAS_USART1                    TRUE
#define STM32_USART1_HANDLER                VectorD4
#define STM32_USART1_NUMBER                 37

#define STM32_HAS_USART2                    TRUE
#define STM32_USART2_HANDLER                VectorD8
#define STM32_USART2_NUMBER                 38

#define STM32_HAS_USART3                    TRUE
#define STM32_USART3_HANDLER                VectorDC
#define STM32_USART3_NUMBER                 39

#define STM32_HAS_UART4                     TRUE
#define STM32_UART4_HANDLER                 Vector110
#define STM32_UART4_NUMBER                  52

#define STM32_HAS_UART5                     TRUE
#define STM32_UART5_HANDLER                 Vector114
#define STM32_UART5_NUMBER                  53

#define STM32_HAS_USART6                    TRUE
#define STM32_USART6_HANDLER                Vector15C
#define STM32_USART6_NUMBER                 71

#define STM32_HAS_UART7                     TRUE
#define STM32_UART7_HANDLER                 Vector188
#define STM32_UART7_NUMBER                  82

#define STM32_HAS_UART8                     TRUE
#define STM32_UART8_HANDLER                 Vector18C
#define STM32_UART8_NUMBER                  83

#define STM32_HAS_LPUART1                   FALSE

/* USB attributes.*/
#define STM32_OTG_STEPPING                  2
#define STM32_HAS_OTG1                      TRUE
#define STM32_OTG1_ENDPOINTS                8
#define STM32_OTG1_HANDLER                  Vector1D4
#define STM32_OTG1_EP1OUT_HANDLER           Vector1C8
#define STM32_OTG1_EP1IN_HANDLER            Vector1CC
#define STM32_OTG1_NUMBER                   101
#define STM32_OTG1_EP1OUT_NUMBER            98
#define STM32_OTG1_EP1IN_NUMBER             99

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
#define STM32_LTDC_EV_HANDLER               Vector1A0
#define STM32_LTDC_ER_HANDLER               Vector1A4
#define STM32_LTDC_EV_NUMBER                88
#define STM32_LTDC_ER_NUMBER                89

/* DMA2D attributes.*/
#define STM32_HAS_DMA2D                     TRUE
#define STM32_DMA2D_HANDLER                 Vector1A8
#define STM32_DMA2D_NUMBER                  90

/* FSMC attributes.*/
#define STM32_HAS_FSMC                      TRUE
#define STM32_FSMC_IS_FMC                   TRUE
#define STM32_FSMC_HANDLER                  Vector100
#define STM32_FSMC_NUMBER                   48

/* CRC attributes.*/
#define STM32_HAS_CRC                       TRUE
#define STM32_CRC_PROGRAMMABLE              TRUE

/* DCMI attributes.*/
#define STM32_HAS_DCMI                      TRUE
#define STM32_DCMI_HANDLER                  Vector178
#define STM32_DCMI_NUMBER                   78

#endif /* defined(STM32H743xx) || defined(STM32H753xx) */
/** @} */

#endif /* STM32_REGISTRY_H */

/** @} */
