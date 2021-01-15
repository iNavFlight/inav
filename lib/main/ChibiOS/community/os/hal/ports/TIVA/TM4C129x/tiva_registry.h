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
 * @file    TM4C129x/tiva_registry.h
 * @brief   TM4C123x capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _TIVA_REGISTRY_H_
#define _TIVA_REGISTRY_H_

#if !defined(PART_TM4C1290NCPDT) && !defined(PART_TM4C1290NCZAD) && !defined(PART_TM4C1292NCPDT)\
  && !defined(PART_TM4C1292NCZAD) && !defined(PART_TM4C1294KCPDT) && !defined(PART_TM4C1294NCPDT)\
  && !defined(PART_TM4C1294NCZAD) && !defined(PART_TM4C1297NCZAD) && !defined(PART_TM4C1299KCZAD)\
  && !defined(PART_TM4C1299NCZAD) && !defined(PART_TM4C129CNCPDT) && !defined(PART_TM4C129CNCZAD)\
  && !defined(PART_TM4C129DNCPDT) && !defined(PART_TM4C129DNCZAD) && !defined(PART_TM4C129EKCPDT)\
  && !defined(PART_TM4C129ENCPDT) && !defined(PART_TM4C129ENCZAD) && !defined(PART_TM4C129LNCZAD)\
  && !defined(PART_TM4C129XKCZAD) && !defined(PART_TM4C129XNCZAD)
#error "No valid device defined."
#endif

#if !defined(TARGET_IS_TM4C129_RA0)
#error "No valid device revision defined."
#endif

/**
 * @brief   Sub-family identifier.
 */
#if !defined(TM4C129x) || defined(__DOXYGEN__)
#define TM4C129x
#endif

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    TM4C129x capabilities
 * @{
 */
 
/* GPIO attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1292NCPDT) || defined(PART_TM4C1294KCPDT)\
  || defined(PART_TM4C1294NCPDT) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129DNCPDT)\
  || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)
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
#if defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294NCZAD)\
  || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD) || defined(PART_TM4C1299NCZAD)\
  || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129ENCZAD)\
  || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
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
#define TIVA_HAS_GPIOR                      TRUE
#define TIVA_HAS_GPIOS                      TRUE
#define TIVA_HAS_GPIOT                      TRUE
#define TIVA_GPIO_PINS                      144
#endif

/* EPI attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_EPI0                       TRUE
#endif

/* CRC attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_CRC0                       TRUE
#endif

/* AES Accelerator attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD)
#define TIVA_HAS_AES                        FALSE
#endif
#if defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT) \
  || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)\
  || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD)\
  || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_AES                        TRUE
#endif

/* DES Accelerator attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD)
#define TIVA_HAS_DES                        FALSE
#endif
#if defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT)\
  || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)\
  || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD)\
  || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_DES                        TRUE
#endif

/* SHA/MD5 Accelerator attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD)
#define TIVA_HAS_SHA_MD5                    FALSE
#endif
#if defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT)\
  || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)\
  || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD)\
  || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_SHA_MD5                    TRUE
#endif

/* GPT attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_GPT0                       TRUE
#define TIVA_HAS_GPT1                       TRUE
#define TIVA_HAS_GPT2                       TRUE
#define TIVA_HAS_GPT3                       TRUE
#define TIVA_HAS_GPT4                       TRUE
#define TIVA_HAS_GPT5                       TRUE
#define TIVA_HAS_GPT6                       TRUE
#define TIVA_HAS_GPT7                       TRUE
#define TIVA_HAS_WGPT0                      FALSE
#define TIVA_HAS_WGPT1                      FALSE
#define TIVA_HAS_WGPT2                      FALSE
#define TIVA_HAS_WGPT3                      FALSE
#define TIVA_HAS_WGPT4                      FALSE
#define TIVA_HAS_WGPT5                      FALSE
#endif

/* WDT attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_WDT0                       TRUE
#define TIVA_HAS_WDT1                       TRUE
#endif

/* ADC attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_ADC0                       TRUE
#define TIVA_HAS_ADC1                       TRUE
#endif

/* UART attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_UART0                      TRUE
#define TIVA_HAS_UART1                      TRUE
#define TIVA_HAS_UART2                      TRUE
#define TIVA_HAS_UART3                      TRUE
#define TIVA_HAS_UART4                      TRUE
#define TIVA_HAS_UART5                      TRUE
#define TIVA_HAS_UART6                      TRUE
#define TIVA_HAS_UART7                      TRUE
#endif

/* QSSI attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_QSSI0                      TRUE
#define TIVA_HAS_QSSI1                      TRUE
#define TIVA_HAS_QSSI2                      TRUE
#define TIVA_HAS_QSSI3                      TRUE
#endif

/* I2C attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_I2C0                       TRUE
#define TIVA_HAS_I2C1                       TRUE
#define TIVA_HAS_I2C2                       TRUE
#define TIVA_HAS_I2C3                       TRUE
#define TIVA_HAS_I2C4                       TRUE
#define TIVA_HAS_I2C5                       TRUE
#define TIVA_HAS_I2C6                       TRUE
#define TIVA_HAS_I2C7                       TRUE
#define TIVA_HAS_I2C8                       TRUE
#define TIVA_HAS_I2C9                       TRUE
#endif

/* 1-Wire Master attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)
#define TIVA_HAS_1WIRE                      FALSE
#endif
#if defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_1WIRE                      TRUE
#endif

/* CAN attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_CAN0                       TRUE
#define TIVA_HAS_CAN1                       TRUE
#endif

/* Ethernet MAC attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1297NCZAD)\
  || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)
#define TIVA_HAS_ETHERNET_MAC               FALSE
#endif
#if defined(PART_TM4C1292NCPDT) || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT)\
  || defined(PART_TM4C1294NCPDT) || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD)\
  || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD)\
  || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_ETHERNET_MAC               TRUE
#endif

/* Ethernet PHY attributes.*/
#if defined(PART_TM4C1290NCPDT)|| defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT) \
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C129CNCPDT)\
  || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD)
#define TIVA_HAS_ETHERNET_PHY               FALSE
#endif
#if  defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT) || defined(PART_TM4C1294NCZAD)\
  || defined(PART_TM4C1299KCZAD) || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_ETHERNET_PHY               TRUE
#endif

/* USB attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_USB0                       TRUE
#endif

/* LCD attributes.*/
#if  defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD) || defined(PART_TM4C129DNCZAD)\
  || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_LCD                        TRUE
#endif
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT)\
  || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD)
#define TIVA_HAS_LCD                        FALSE
#endif

/* AC attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_AC0                        TRUE
#define TIVA_HAS_AC1                        TRUE
#define TIVA_HAS_AC2                        TRUE
#endif

/* PWM attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_PWM0                       TRUE
#define TIVA_HAS_PWM1                       FALSE
#endif

/* QEI attributes.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_HAS_QEI0                       TRUE
#define TIVA_HAS_QEI1                       FALSE
#endif

/**
 * @}
 */

#endif /* _TIVA_REGISTRY_H_ */

/**
 * @}
 */
