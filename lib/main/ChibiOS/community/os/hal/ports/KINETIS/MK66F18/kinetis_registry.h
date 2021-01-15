/*
    ChibiOS - Copyright (C) 2014 Derek Mulcahy
                        (C) 2016 flabbergast <s3+flabbergast@sdfeu.org>

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
 * @file    MK66F18/kinetis_registry.h
 * @brief   MK66F18 capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef KINETIS_REGISTRY_H_
#define KINETIS_REGISTRY_H_

#if !defined(MK66F18) || defined(__DOXYGEN__)
#define MK66F18 
#endif

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @brief   Maximum system and core clock (f_SYS) frequency.
 */
#define KINETIS_SYSCLK_MAX      180000000L

/**
 * @brief   Maximum bus clock (f_BUS) frequency.
 */
#define KINETIS_BUSCLK_MAX      60000000L

/**
 * @brief   Maximum flash clock (f_FLASH) frequency.
 */
#define KINETIS_FLASHCLK_MAX    28000000L

/* ADC attributes.*/
#define KINETIS_HAS_ADC0            TRUE
#define KINETIS_ADC0_IRQ_VECTOR     VectorDC
#define KINETIS_HAS_ADC1            TRUE
#define KINETIS_ADC1_IRQ_VECTOR     Vector164

/* DAC attributes.*/
#define KINETIS_HAS_DAC0            TRUE
#define KINETIS_DAC0_IRQ_VECTOR     Vector120
#define KINETIS_HAS_DAC1            TRUE
#define KINETIS_DAC1_IRQ_VECTOR     Vector160

/* DMA attributes.*/
#define KINETIS_DMA0_IRQ_VECTOR     Vector40
#define KINETIS_DMA1_IRQ_VECTOR     Vector44
#define KINETIS_DMA2_IRQ_VECTOR     Vector48
#define KINETIS_DMA3_IRQ_VECTOR     Vector4C
#define KINETIS_DMA4_IRQ_VECTOR     Vector50
#define KINETIS_DMA5_IRQ_VECTOR     Vector54
#define KINETIS_DMA6_IRQ_VECTOR     Vector58
#define KINETIS_DMA7_IRQ_VECTOR     Vector5C
#define KINETIS_DMA8_IRQ_VECTOR     Vector60
#define KINETIS_DMA9_IRQ_VECTOR     Vector64
#define KINETIS_DMA10_IRQ_VECTOR     Vector68
#define KINETIS_DMA11_IRQ_VECTOR     Vector6C
#define KINETIS_DMA12_IRQ_VECTOR     Vector70
#define KINETIS_DMA13_IRQ_VECTOR     Vector74
#define KINETIS_DMA14_IRQ_VECTOR     Vector78
#define KINETIS_DMA15_IRQ_VECTOR     Vector7C
#define KINETIS_HAS_DMA_ERROR_IRQ   TRUE
#define KINETIS_DMA_ERROR_IRQ_VECTOR Vector80

/* EXT attributes.*/
#define KINETIS_PORTA_IRQ_VECTOR    Vector12C
#define KINETIS_PORTB_IRQ_VECTOR    Vector130
#define KINETIS_PORTC_IRQ_VECTOR    Vector134
#define KINETIS_PORTD_IRQ_VECTOR    Vector138
#define KINETIS_PORTE_IRQ_VECTOR    Vector13C
#define KINETIS_EXT_HAS_COMMON_CD_IRQ   FALSE
#define KINETIS_EXT_HAS_COMMON_BCDE_IRQ FALSE
#define KINETIS_GPIO_HAS_OPENDRAIN  TRUE

/* I2C attributes.*/
#define KINETIS_HAS_I2C0            TRUE
#define KINETIS_I2C0_IRQ_VECTOR     VectorA0
#define KINETIS_HAS_I2C1            TRUE
#define KINETIS_I2C1_IRQ_VECTOR     VectorA4

/* Serial attributes.*/
#define KINETIS_HAS_SERIAL0         TRUE
#define KINETIS_SERIAL0_IRQ_VECTOR  VectorBC
#define KINETIS_HAS_SERIAL1         TRUE
#define KINETIS_SERIAL1_IRQ_VECTOR  VectorC4
#define KINETIS_HAS_SERIAL2         TRUE
#define KINETIS_SERIAL2_IRQ_VECTOR  VectorCC
#define KINETIS_HAS_SERIAL3         TRUE
#define KINETIS_SERIAL3_IRQ_VECTOR  VectorD4
#define KINETIS_HAS_SERIAL_ERROR_IRQ TRUE
#define KINETIS_SERIAL0_ERROR_IRQ_VECTOR VectorC0
#define KINETIS_SERIAL1_ERROR_IRQ_VECTOR VectorC8
#define KINETIS_SERIAL2_ERROR_IRQ_VECTOR VectorD0
#define KINETIS_SERIAL3_ERROR_IRQ_VECTOR VectorD8
#define KINETIS_SERIAL0_IS_LPUART   FALSE
#define KINETIS_SERIAL0_IS_UARTLP   FALSE
#define KINETIS_SERIAL1_IS_LPUART   FALSE
#define KINETIS_SERIAL1_IS_UARTLP   FALSE
#define KINETIS_SERIAL2_IS_LPUART   FALSE
#define KINETIS_SERIAL2_IS_UARTLP   FALSE
#define KINETIS_SERIAL3_IS_LPUART   FALSE
#define KINETIS_SERIAL3_IS_UARTLP   FALSE

/* SPI attributes.*/
#define KINETIS_HAS_SPI0            TRUE
#define KINETIS_SPI0_IRQ_VECTOR     VectorA8
#define KINETIS_HAS_SPI1            TRUE
#define KINETIS_SPI1_IRQ_VECTOR     VectorAC

/* FlexTimer attributes.*/
#define KINETIS_FTM0_CHANNELS 8
#define KINETIS_FTM1_CHANNELS 2
#define KINETIS_FTM2_CHANNELS 2
#define KINETIS_FTM3_CHANNELS 8

#define KINETIS_HAS_FTM0            TRUE
#define KINETIS_FTM0_IRQ_VECTOR     VectorE8
#define KINETIS_HAS_FTM1            TRUE
#define KINETIS_FTM1_IRQ_VECTOR     VectorEC
#define KINETIS_HAS_FTM2            TRUE
#define KINETIS_FTM2_IRQ_VECTOR     VectorF0
#define KINETIS_HAS_FTM3            TRUE
#define KINETIS_FTM3_IRQ_VECTOR     Vector15C

/* GPT attributes.*/
#define KINETIS_HAS_PIT0            TRUE
#define KINETIS_PIT0_IRQ_VECTOR     Vector100
#define KINETIS_HAS_PIT1            TRUE
#define KINETIS_PIT1_IRQ_VECTOR     Vector104
#define KINETIS_HAS_PIT2            TRUE
#define KINETIS_PIT2_IRQ_VECTOR     Vector108
#define KINETIS_HAS_PIT3            TRUE
#define KINETIS_PIT3_IRQ_VECTOR     Vector10C
#define KINETIS_HAS_PIT_COMMON_IRQ  FALSE

/* USB attributes.*/
#define KINETIS_HAS_USB             TRUE
#define KINETIS_USB_IRQ_VECTOR      Vector114
#define KINETIS_USB0_IS_USBOTG      TRUE
#define KINETIS_HAS_USB_CLOCK_RECOVERY TRUE

/* LPTMR attributes.*/
#define KINETIS_LPTMR0_IRQ_VECTOR   Vector128

/* SDHC (SDC, MMC, SDIO) attributes */
#define KINETIS_HAS_SDHC            TRUE
#define KINETIS_SDHC_IRQ_VECTOR     Vector184

/** @} */

#endif /* KINETIS_REGISTRY_H_ */

/** @} */
