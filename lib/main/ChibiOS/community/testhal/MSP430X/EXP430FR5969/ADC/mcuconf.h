/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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

#ifndef MCUCONF_H
#define MCUCONF_H

/*
 * MSP430X drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the driver
 * is enabled in halconf.h.
 * 
 */

#define MSP430X_MCUCONF

/* HAL driver system settings */
#define MSP430X_ACLK_SRC MSP430X_VLOCLK
#define MSP430X_LFXTCLK_FREQ 0
#define MSP430X_HFXTCLK_FREQ 0
#define MSP430X_DCOCLK_FREQ 8000000
#define MSP430X_MCLK_DIV 1
#define MSP430X_SMCLK_DIV 32

/*
 * SERIAL driver system settings.
 */
#define MSP430X_SERIAL_USE_USART0         TRUE
#define MSP430X_USART0_CLK_SRC            MSP430X_SMCLK_SRC
#define MSP430X_SERIAL_USE_USART1         FALSE
#define MSP430X_SERIAL_USE_USART2         FALSE
#define MSP430X_SERIAL_USE_USART3         FALSE

/*
 * ST driver system settings.
 */
#define MSP430X_ST_CLK_SRC MSP430X_SMCLK_SRC
#define MSP430X_ST_TIMER_TYPE B
#define MSP430X_ST_TIMER_INDEX 0

/*
 * SPI driver system settings.
 */
#define MSP430X_SPI_USE_SPIA1 FALSE
#define MSP430X_SPI_USE_SPIB0 FALSE
#define MSP430X_SPI_EXCLUSIVE_DMA TRUE

/*
 * ADC driver system settings
 */
#define MSP430X_ADC_EXCLUSIVE_DMA TRUE
#define MSP430X_ADC1_FREQ 5000000 / 256

#endif /* _MCUCONF_H_ */
