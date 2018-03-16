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
 * @file    KL2x/kinetis_registry.h
 * @brief   KL2x capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef KINETIS_REGISTRY_H_
#define KINETIS_REGISTRY_H_

#if !defined(KL2x) || defined(__DOXYGEN__)
#define KL2x
#endif

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    KL2x capabilities
 * @{
 */

/*===========================================================================*/
/* Common features                                                           */
/*===========================================================================*/

/**
 * @brief   Maximum system and core clock (f_SYS) frequency.
 */
#define KINETIS_SYSCLK_MAX      48000000

/**
 * @brief   Maximum bus clock (f_BUS) frequency.
 */
#define KINETIS_BUSCLK_MAX      24000000

/* ADC attributes.*/
#define KINETIS_HAS_ADC0            TRUE
#define KINETIS_ADC0_IRQ_VECTOR     Vector7C
#define KINETIS_HAS_ADC1            FALSE

/* DMA attributes.*/
#define KINETIS_DMA0_IRQ_VECTOR     Vector40
#define KINETIS_DMA1_IRQ_VECTOR     Vector44
#define KINETIS_DMA2_IRQ_VECTOR     Vector48
#define KINETIS_DMA3_IRQ_VECTOR     Vector4C
#define KINETIS_HAS_DMA_ERROR_IRQ   FALSE

/* GPT attributes.*/
#define KINETIS_PIT_IRQ_VECTOR      Vector98
#define KINETIS_HAS_PIT_COMMON_IRQ  TRUE
#define KINETIS_HAS_PIT0            TRUE
#define KINETIS_HAS_PIT1            TRUE
#define KINETIS_HAS_PIT2            FALSE
#define KINETIS_HAS_PIT3            FALSE

/* I2C attributes.*/
#define KINETIS_HAS_I2C0            TRUE
#define KINETIS_I2C0_IRQ_VECTOR     Vector60
#define KINETIS_HAS_I2C1            TRUE
#define KINETIS_I2C1_IRQ_VECTOR     Vector64

/* Serial attributes */
#define KINETIS_HAS_SERIAL0         TRUE
#define KINETIS_SERIAL0_IRQ_VECTOR  Vector70
#define KINETIS_HAS_SERIAL1         TRUE
#define KINETIS_SERIAL1_IRQ_VECTOR  Vector74
#define KINETIS_HAS_SERIAL2         TRUE
#define KINETIS_SERIAL2_IRQ_VECTOR  Vector78
#define KINETIS_HAS_SERIAL_ERROR_IRQ FALSE

/* SPI attributes.*/
#define KINETIS_HAS_SPI0            TRUE
#define KINETIS_SPI0_IRQ_VECTOR     Vector68
#define KINETIS_HAS_SPI1            TRUE
#define KINETIS_SPI1_IRQ_VECTOR     Vector6C

/* TPM attributes.*/
#define KINETIS_HAS_TPM0            TRUE
#define KINETIS_TPM0_CHANNELS       6
#define KINETIS_TPM0_IRQ_VECTOR     Vector84
#define KINETIS_HAS_TPM1            TRUE
#define KINETIS_TPM1_CHANNELS       2
#define KINETIS_TPM1_IRQ_VECTOR     Vector88
#define KINETIS_HAS_TPM2            TRUE
#define KINETIS_TPM2_CHANNELS       2
#define KINETIS_TPM2_IRQ_VECTOR     Vector8C

/* USB attributes.*/
#define KINETIS_HAS_USB             TRUE
#define KINETIS_USB_IRQ_VECTOR      VectorA0

/* FTFA attributes.*/
#define KINETIS_FTFA_IRQ_VECTOR     Vector54

/* LPTMR attributes */
#define KINETIS_LPTMR0_IRQ_VECTOR   VectorB0

/*===========================================================================*/
/* KL25                                                                      */
/*===========================================================================*/
#if defined(KL25) || defined(__DOXYGEN__)

/* DAC attributes.*/
#define KINETIS_HAS_DAC0            TRUE
#define KINTEIS_DAC0_IRQ_VECTOR     VectorA4

/* EXT attributes.*/
#define KINETIS_PORTA_IRQ_VECTOR    VectorB8
#define KINETIS_PORTD_IRQ_VECTOR    VectorBC
#define KINETIS_EXT_HAS_COMMON_CD_IRQ   FALSE
#define KINETIS_EXT_HAS_COMMON_BCDE_IRQ FALSE
#define KINETIS_GPIO_HAS_OPENDRAIN  FALSE

/* I2S attributes.*/
#define KINETIS_HAS_I2S0            FALSE

/* MCG attributes.*/
#define KINETIS_HAS_MCG_LITE        FALSE

/* Serial attributes */
#define KINETIS_SERIAL0_IS_UARTLP   TRUE
#define KINETIS_SERIAL0_IS_LPUART   FALSE
#define KINETIS_SERIAL1_IS_LPUART   FALSE

/* USB attributes.*/
#define KINETIS_USB0_IS_USBOTG      TRUE
#define KINETIS_HAS_USB_CLOCK_RECOVERY FALSE

/*===========================================================================*/
/* KL26                                                                      */
/*===========================================================================*/
#elif defined(KL26) /* defined(KL25) */

/* DAC attributes.*/
#define KINETIS_HAS_DAC0            TRUE
#define KINTEIS_DAC0_IRQ_VECTOR     VectorA4

/* EXT attributes.*/
#define KINETIS_PORTA_IRQ_VECTOR    VectorB8
/* Common IRQ vector for PORTC and PORTD */
#define KINETIS_PORTD_IRQ_VECTOR    VectorBC
#define KINETIS_EXT_HAS_COMMON_CD_IRQ   TRUE
#define KINETIS_EXT_HAS_COMMON_BCDE_IRQ FALSE
#define KINETIS_GPIO_HAS_OPENDRAIN  FALSE

/* I2S attributes.*/
#define KINETIS_HAS_I2S0            TRUE
#define KINETIS_I2S0_IRQ_VECTOR     Vector9C

/* MCG attributes.*/
#define KINETIS_HAS_MCG_LITE        FALSE

/* Serial attributes */
#define KINETIS_SERIAL0_IS_UARTLP   TRUE
#define KINETIS_SERIAL0_IS_LPUART   FALSE
#define KINETIS_SERIAL1_IS_LPUART   FALSE

/* USB attributes.*/
#define KINETIS_USB0_IS_USBOTG      TRUE
#define KINETIS_HAS_USB_CLOCK_RECOVERY FALSE

/*===========================================================================*/
/* KL27                                                                      */
/*===========================================================================*/
#elif defined(KL27Zxxx) || defined(KL27Zxx) /* defined(KL26) */

#if !defined(KL27)
#define KL27
#endif

/* MCG attributes.*/
#define KINETIS_HAS_MCG_LITE        TRUE

/* Note: on this device, SERIAL2 IRQ is alternatively FlexIO IRQ. */
/* Serial attributes */
#define KINETIS_SERIAL0_IS_UARTLP   FALSE
#define KINETIS_SERIAL0_IS_LPUART   TRUE
#define KINETIS_SERIAL1_IS_LPUART   TRUE

/* USB attributes.*/
#define KINETIS_USB0_IS_USBOTG      FALSE
#define KINETIS_HAS_USB_CLOCK_RECOVERY TRUE

/*===========================================================================*/
/* KL27Zxxx (MKL27Z128* and MKL27Z256*) specific                             */
/*===========================================================================*/
#if defined(KL27Zxxx)

/* DAC attributes.*/
#define KINETIS_HAS_DAC0            TRUE
#define KINTEIS_DAC0_IRQ_VECTOR     VectorA4

/* EXT attributes.*/
#define KINETIS_PORTA_IRQ_VECTOR    VectorB8
/* Common IRQ vector for PORTC and PORTD */
#define KINETIS_PORTD_IRQ_VECTOR    VectorBC
#define KINETIS_EXT_HAS_COMMON_CD_IRQ   TRUE
#define KINETIS_EXT_HAS_COMMON_BCDE_IRQ FALSE
#define KINETIS_GPIO_HAS_OPENDRAIN  FALSE

/* I2S attributes.*/
#define KINETIS_HAS_I2S0            TRUE
#define KINETIS_I2S0_IRQ_VECTOR     Vector9C

/*===========================================================================*/
/* KL27Zxx (MKL27Z32* and MKL27Z264*) specific                               */
/*===========================================================================*/
#elif defined(KL27Zxx) /* defined(KL27Zxxx) */

/* Has CRC module */
/* Does not have USB voltage regulator */
/* Does have KEEP_ALIVE USB feature */

/* DAC attributes.*/
#define KINETIS_HAS_DAC0            FALSE

/* EXT attributes.*/
#define KINETIS_PORTA_IRQ_VECTOR    VectorB8
/* Common IRQ vector for PORTB to PORTE */
#define KINETIS_PORTD_IRQ_VECTOR    VectorBC
#define KINETIS_EXT_HAS_COMMON_CD_IRQ   FALSE
#define KINETIS_EXT_HAS_COMMON_BCDE_IRQ TRUE
#define KINETIS_GPIO_HAS_OPENDRAIN  FALSE

/* I2S attributes.*/
#define KINETIS_HAS_I2S0            FALSE

#endif /* defined(KL27Zxx) */

#else /* ! (KL25 || KL26 || KL27)  */
#error MCU type not described in kinetis_registry
#endif /* KL2Y */

/** @} */

#endif /* KINETIS_REGISTRY_H_ */

/** @} */
