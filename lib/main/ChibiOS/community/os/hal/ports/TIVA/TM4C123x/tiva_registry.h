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

/**
 * @file    TM4C123x/tiva_registry.h
 * @brief   TM4C123x capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _TIVA_REGISTRY_H_
#define _TIVA_REGISTRY_H_

/*===========================================================================*/
/* Defined device check.                                                     */
/*===========================================================================*/

#if !defined(PART_TM4C1230C3PM) && !defined(PART_TM4C1230D5PM) &&                       \
  !defined(PART_TM4C1230E6PM) && !defined(PART_TM4C1230H6PM) &&                         \
  !defined(PART_TM4C1231C3PM) && !defined(PART_TM4C1231D5PM) &&                         \
  !defined(PART_TM4C1231D5PZ) && !defined(PART_TM4C1231E6PM) &&                         \
  !defined(PART_TM4C1231E6PZ) && !defined(PART_TM4C1231H6PGE) &&                        \
  !defined(PART_TM4C1231H6PM) && !defined(PART_TM4C1231H6PZ) &&                         \
  !defined(PART_TM4C1232C3PM) && !defined(PART_TM4C1232D5PM) &&                         \
  !defined(PART_TM4C1232E6PM) && !defined(PART_TM4C1232H6PM) &&                         \
  !defined(PART_TM4C1233C3PM) && !defined(PART_TM4C1233D5PM) &&                         \
  !defined(PART_TM4C1233D5PZ) && !defined(PART_TM4C1233E6PM) &&                         \
  !defined(PART_TM4C1233E6PZ) && !defined(PART_TM4C1233H6PGE) &&                        \
  !defined(PART_TM4C1233H6PM) && !defined(PART_TM4C1233H6PZ) &&                         \
  !defined(PART_TM4C1236D5PM) && !defined(PART_TM4C1236E6PM) &&                         \
  !defined(PART_TM4C1236H6PM) && !defined(PART_TM4C1237D5PM) &&                         \
  !defined(PART_TM4C1237D5PZ) && !defined(PART_TM4C1237E6PM) &&                         \
  !defined(PART_TM4C1237E6PZ) && !defined(PART_TM4C1237H6PGE) &&                        \
  !defined(PART_TM4C1237H6PM) && !defined(PART_TM4C1237H6PZ) &&                         \
  !defined(PART_TM4C123AE6PM) && !defined(PART_TM4C123AH6PM) &&                         \
  !defined(PART_TM4C123BE6PM) && !defined(PART_TM4C123BE6PZ) &&                         \
  !defined(PART_TM4C123BH6PGE) && !defined(PART_TM4C123BH6PM) &&                        \
  !defined(PART_TM4C123BH6PZ) && !defined(PART_TM4C123BH6ZRB) &&                        \
  !defined(PART_TM4C123FE6PM) && !defined(PART_TM4C123FH6PM) &&                         \
  !defined(PART_TM4C123GE6PM) && !defined(PART_TM4C123GE6PZ) &&                         \
  !defined(PART_TM4C123GH6PGE) && !defined(PART_TM4C123GH6PM) &&                        \
  !defined(PART_TM4C123GH6PZ) && !defined(PART_TM4C123GH6ZRB) &&                        \
  !defined(PART_TM4C123GH5ZXR)
#error "No valid device defined."
#endif

#if !defined(TARGET_IS_TM4C123_RA1) && !defined(TARGET_IS_TM4C123_RA2) && \
    !defined(TARGET_IS_TM4C123_RA3) && !defined(TARGET_IS_TM4C123_RB0) && \
    !defined(TARGET_IS_TM4C123_RB1)
#error "No valid device revision defined."
#endif

/**
 * @brief   Sub-family identifier.
 */
#if !defined(TM4C123x) || defined(__DOXYGEN__)
#define TM4C123x
#endif

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    TM4C123x capabilities
 * @{
 */

/* GPIO attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM)  \
  || defined(PART_TM4C1232E6PM) || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1236D5PM)  \
  || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM) || defined(PART_TM4C123AE6PM)  \
  || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM)
#define TIVA_HAS_GPIOA                      TRUE
#define TIVA_HAS_GPIOB                      TRUE
#define TIVA_HAS_GPIOC                      TRUE
#define TIVA_HAS_GPIOD                      TRUE
#define TIVA_HAS_GPIOE                      TRUE
#define TIVA_HAS_GPIOF                      TRUE
#define TIVA_HAS_GPIOG                      TRUE
#define TIVA_HAS_GPIOH                      FALSE
#define TIVA_HAS_GPIOJ                      FALSE
#define TIVA_HAS_GPIOK                      FALSE
#define TIVA_HAS_GPIOL                      FALSE
#define TIVA_HAS_GPIOM                      FALSE
#define TIVA_HAS_GPION                      FALSE
#define TIVA_HAS_GPIOP                      FALSE
#define TIVA_HAS_GPIOQ                      FALSE
#define TIVA_HAS_GPIOR                      FALSE
#define TIVA_HAS_GPIOS                      FALSE
#define TIVA_HAS_GPIOT                      FALSE
#define TIVA_GPIO_PINS                      56
#endif
#if defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM) || defined(PART_TM4C1231E6PM)   \
  || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1237D5PM)  \
  || defined(PART_TM4C1237E6PM) || defined(PART_TM4C1237H6PM) || defined(PART_TM4C123BE6PM)  \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GH6PM)
#define TIVA_HAS_GPIOA                      TRUE
#define TIVA_HAS_GPIOB                      TRUE
#define TIVA_HAS_GPIOC                      TRUE
#define TIVA_HAS_GPIOD                      TRUE
#define TIVA_HAS_GPIOE                      TRUE
#define TIVA_HAS_GPIOF                      TRUE
#define TIVA_HAS_GPIOG                      FALSE
#define TIVA_HAS_GPIOH                      FALSE
#define TIVA_HAS_GPIOJ                      FALSE
#define TIVA_HAS_GPIOK                      FALSE
#define TIVA_HAS_GPIOL                      FALSE
#define TIVA_HAS_GPIOM                      FALSE
#define TIVA_HAS_GPION                      FALSE
#define TIVA_HAS_GPIOP                      FALSE
#define TIVA_HAS_GPIOQ                      FALSE
#define TIVA_HAS_GPIOR                      FALSE
#define TIVA_HAS_GPIOS                      FALSE
#define TIVA_HAS_GPIOT                      FALSE
#define TIVA_GPIO_PINS                      48
#endif
#if defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PZ) || defined(PART_TM4C1231H6PZ)   \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PZ) || defined(PART_TM4C1233H6PZ)  \
  || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PZ)  \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123GE6PZ)  \
  || defined(PART_TM4C123GH6PZ)
#define TIVA_HAS_GPIOA                      TRUE
#define TIVA_HAS_GPIOB                      TRUE
#define TIVA_HAS_GPIOC                      TRUE
#define TIVA_HAS_GPIOD                      TRUE
#define TIVA_HAS_GPIOE                      TRUE
#define TIVA_HAS_GPIOF                      TRUE
#define TIVA_HAS_GPIOG                      TRUE
#define TIVA_HAS_GPIOH                      TRUE
#define TIVA_HAS_GPIOJ                      TRUE
#define TIVA_HAS_GPIOK                      TRUE
#define TIVA_HAS_GPIOL                      TRUE
#define TIVA_HAS_GPIOM                      FALSE
#define TIVA_HAS_GPION                      FALSE
#define TIVA_HAS_GPIOP                      FALSE
#define TIVA_HAS_GPIOQ                      FALSE
#define TIVA_HAS_GPIOR                      FALSE
#define TIVA_HAS_GPIOS                      FALSE
#define TIVA_HAS_GPIOT                      FALSE
#define TIVA_GPIO_PINS                      88
#endif
#if defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1237H6PGE)\
  || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123GH6PGE)
#define TIVA_HAS_GPIOA                      TRUE
#define TIVA_HAS_GPIOB                      TRUE
#define TIVA_HAS_GPIOC                      TRUE
#define TIVA_HAS_GPIOD                      TRUE
#define TIVA_HAS_GPIOE                      TRUE
#define TIVA_HAS_GPIOF                      TRUE
#define TIVA_HAS_GPIOG                      TRUE
#define TIVA_HAS_GPIOH                      TRUE
#define TIVA_HAS_GPIOJ                      TRUE
#define TIVA_HAS_GPIOK                      TRUE
#define TIVA_HAS_GPIOL                      TRUE
#define TIVA_HAS_GPIOM                      TRUE
#define TIVA_HAS_GPION                      TRUE
#define TIVA_HAS_GPIOP                      TRUE
#define TIVA_HAS_GPIOQ                      FALSE
#define TIVA_HAS_GPIOR                      FALSE
#define TIVA_HAS_GPIOS                      FALSE
#define TIVA_HAS_GPIOT                      FALSE
#define TIVA_GPIO_PINS                      112
#endif
#if defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_GPIOA                      TRUE
#define TIVA_HAS_GPIOB                      TRUE
#define TIVA_HAS_GPIOC                      TRUE
#define TIVA_HAS_GPIOD                      TRUE
#define TIVA_HAS_GPIOE                      TRUE
#define TIVA_HAS_GPIOF                      TRUE
#define TIVA_HAS_GPIOG                      TRUE
#define TIVA_HAS_GPIOH                      TRUE
#define TIVA_HAS_GPIOJ                      TRUE
#define TIVA_HAS_GPIOK                      TRUE
#define TIVA_HAS_GPIOL                      TRUE
#define TIVA_HAS_GPIOM                      TRUE
#define TIVA_HAS_GPION                      TRUE
#define TIVA_HAS_GPIOP                      TRUE
#define TIVA_HAS_GPIOQ                      TRUE
#define TIVA_HAS_GPIOR                      FALSE
#define TIVA_HAS_GPIOS                      FALSE
#define TIVA_HAS_GPIOT                      FALSE
#define TIVA_GPIO_PINS                      120
#endif

/* GPTM attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_GPT0                       TRUE
#define TIVA_HAS_GPT1                       TRUE
#define TIVA_HAS_GPT2                       TRUE
#define TIVA_HAS_GPT3                       TRUE
#define TIVA_HAS_GPT4                       TRUE
#define TIVA_HAS_GPT5                       TRUE
#define TIVA_HAS_GPT6                       FALSE
#define TIVA_HAS_GPT7                       FALSE
#define TIVA_HAS_WGPT0                      TRUE
#define TIVA_HAS_WGPT1                      TRUE
#define TIVA_HAS_WGPT2                      TRUE
#define TIVA_HAS_WGPT3                      TRUE
#define TIVA_HAS_WGPT4                      TRUE
#define TIVA_HAS_WGPT5                      TRUE
#endif

/* WDT attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_WDT0                       TRUE
#define TIVA_HAS_WDT1                       TRUE
#endif

/* ADC attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_ADC0                       TRUE
#define TIVA_HAS_ADC1                       TRUE
#endif

/* UART attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_UART0                      TRUE
#define TIVA_HAS_UART1                      TRUE
#define TIVA_HAS_UART2                      TRUE
#define TIVA_HAS_UART3                      TRUE
#define TIVA_HAS_UART4                      TRUE
#define TIVA_HAS_UART5                      TRUE
#define TIVA_HAS_UART6                      TRUE
#define TIVA_HAS_UART7                      TRUE
#endif

/* SPI attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_SSI0                       TRUE
#define TIVA_HAS_SSI1                       TRUE
#define TIVA_HAS_SSI2                       TRUE
#define TIVA_HAS_SSI3                       TRUE
#endif

/* I2C attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PZ) || defined(PART_TM4C1232C3PM) \
  || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM) || defined(PART_TM4C1232H6PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PZ) || defined(PART_TM4C1233H6PGE) \
  || defined(PART_TM4C1233H6PZ) || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM)  \
  || defined(PART_TM4C1236H6PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PZ)  \
  || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) \
  || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123FE6PM) \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_I2C0                       TRUE
#define TIVA_HAS_I2C1                       TRUE
#define TIVA_HAS_I2C2                       TRUE
#define TIVA_HAS_I2C3                       TRUE
#define TIVA_HAS_I2C4                       TRUE
#define TIVA_HAS_I2C5                       TRUE
#define TIVA_HAS_I2C6                       FALSE
#define TIVA_HAS_I2C7                       FALSE
#define TIVA_HAS_I2C8                       FALSE
#define TIVA_HAS_I2C9                       FALSE
#endif
#if defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM) || defined(PART_TM4C1231E6PM)   \
  || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1237D5PM)  \
  || defined(PART_TM4C1237E6PM) || defined(PART_TM4C1237H6PM) || defined(PART_TM4C123BE6PM)  \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GH6PM)
#define TIVA_HAS_I2C0                       TRUE
#define TIVA_HAS_I2C1                       TRUE
#define TIVA_HAS_I2C2                       TRUE
#define TIVA_HAS_I2C3                       TRUE
#define TIVA_HAS_I2C4                       FALSE
#define TIVA_HAS_I2C5                       FALSE
#define TIVA_HAS_I2C6                       FALSE
#define TIVA_HAS_I2C7                       FALSE
#define TIVA_HAS_I2C8                       FALSE
#define TIVA_HAS_I2C9                       FALSE
#endif

/* CAN attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ)
#define TIVA_HAS_CAN0                       TRUE
#define TIVA_HAS_CAN1                       FALSE
#endif
#if defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PM)   \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PM) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123FE6PM) \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GE6PZ)  \
  || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) || defined(PART_TM4C123GH6PZ) \
  || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_CAN0                       TRUE
#define TIVA_HAS_CAN1                       TRUE
#endif

/* USB attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PM)  \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PM) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB)
#define TIVA_HAS_USB0                       FALSE
#endif
#if defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)   \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM)  \
  || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) \
  || defined(PART_TM4C123GH6PM) || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) \
  || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_USB0                       TRUE
#endif

/* AC attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1232C3PM)  \
  || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM) || defined(PART_TM4C1232H6PM)  \
  || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM) || defined(PART_TM4C1233E6PM)  \
  || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM)  \
  || defined(PART_TM4C1236H6PM) || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237H6PM) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123FE6PM)  \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GH6PM)
#define TIVA_HAS_AC0                        TRUE
#define TIVA_HAS_AC1                        TRUE
#define TIVA_HAS_AC2                        FALSE
#endif
#if defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PZ) || defined(PART_TM4C1231H6PGE)  \
  || defined(PART_TM4C1231H6PZ) || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PZ) || defined(PART_TM4C1237D5PZ) \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PZ) \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PZ) \
  || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE)\
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_AC0                        TRUE
#define TIVA_HAS_AC1                        TRUE
#define TIVA_HAS_AC2                        TRUE
#endif

/* PWM attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ)
#define TIVA_HAS_PWM0                       FALSE
#define TIVA_HAS_PWM1                       FALSE
#endif
#if defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PM)   \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PM) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123FE6PM) \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GE6PZ)  \
  || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) || defined(PART_TM4C123GH6PZ) \
  || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_PWM0                       TRUE
#define TIVA_HAS_PWM1                       TRUE
#endif

/* QEI attributes.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)
#define TIVA_HAS_QEI0                       FALSE
#define TIVA_HAS_QEI1                       FALSE
#endif
#if defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE)  \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_HAS_QEI0                       TRUE
#define TIVA_HAS_QEI1                       TRUE
#endif

/**
 * @}
 */

#endif /* _TIVA_REGISTRY_H_ */

/**
 * @}
 */
