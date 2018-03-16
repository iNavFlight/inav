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

#define KL2x_MCUCONF

/*
 * HAL driver system settings.
 */

/* Select the MCU clocking mode below by enabling the appropriate block. */

/* FEI mode */
#if 0
#define KINETIS_MCG_MODE            KINETIS_MCG_MODE_FEI
#define KINETIS_SYSCLK_FREQUENCY    21000000UL
#endif /* 0 */

/* FEE mode - 24 MHz with external 32.768 kHz crystal */
#if 0
#define KINETIS_MCG_MODE            KINETIS_MCG_MODE_FEE
#define KINETIS_MCG_FLL_DMX32       1           /* Fine-tune for 32.768 kHz */
#define KINETIS_MCG_FLL_DRS         0           /* 732x FLL factor */
#define KINETIS_MCG_FLL_OUTDIV1     1           /* Divide 48 MHz FLL by 1 => 24 MHz */
#define KINETIS_MCG_FLL_OUTDIV4     2           /* Divide OUTDIV1 output by 2 => 12 MHz */
#define KINETIS_SYSCLK_FREQUENCY    23986176UL  /* 32.768 kHz*732 (~24 MHz) */
#define KINETIS_UART0_CLOCK_FREQ    (32768 * 732) /* FLL output */
#define KINETIS_UART0_CLOCK_SRC     1           /* Select FLL clock */
#define KINETIS_BUSCLK_FREQUENCY    (KINETIS_SYSCLK_FREQUENCY / KINETIS_MCG_FLL_OUTDIV4)
#endif /* 0 */

/* FEE mode - 48 MHz */
#if 0
#define KINETIS_MCG_MODE            KINETIS_MCG_MODE_FEE
#define KINETIS_MCG_FLL_DMX32       1           /* Fine-tune for 32.768 kHz */
#define KINETIS_MCG_FLL_DRS         1           /* 1464x FLL factor */
#define KINETIS_MCG_FLL_OUTDIV1     1           /* Divide 48 MHz FLL by 1 => 48 MHz */
#define KINETIS_MCG_FLL_OUTDIV4     2           /* Divide OUTDIV1 output by 2 => 24 MHz */
#define KINETIS_SYSCLK_FREQUENCY    47972352UL  /* 32.768 kHz * 1464 (~48 MHz) */
#endif /* 0 */

/*
 * SERIAL driver system settings.
 */
#define KINETIS_SERIAL_USE_UART0              TRUE

/*
 * EXTI driver system settings.
 */
#define KINETIS_EXTI_NUM_CHANNELS               1
#define KINETIS_EXT_PORTA_IRQ_PRIORITY          3
#define KINETIS_EXT_PORTB_IRQ_PRIORITY          3
#define KINETIS_EXT_PORTC_IRQ_PRIORITY          3
#define KINETIS_EXT_PORTD_IRQ_PRIORITY          3
#define KINETIS_EXT_PORTE_IRQ_PRIORITY          3

/*
 * Processor specific widths of each port.
 * Smaller numbers can be used if only lower pins in a port are being used to
 * generate interrupts. Can be set to 0 if a port is unused.
 */


#if 0
/* MK20 48pin */
#define KINETIS_EXT_PORTA_WIDTH                 20
#define KINETIS_EXT_PORTB_WIDTH                 18
#define KINETIS_EXT_PORTC_WIDTH                 8
#define KINETIS_EXT_PORTD_WIDTH                 8
#define KINETIS_EXT_PORTE_WIDTH                 0
#endif

#if 0
/* MK20 64pin  */
#define KINETIS_EXT_PORTA_WIDTH                 20
#define KINETIS_EXT_PORTB_WIDTH                 20
#define KINETIS_EXT_PORTC_WIDTH                 12
#define KINETIS_EXT_PORTD_WIDTH                 8
#define KINETIS_EXT_PORTE_WIDTH                 2
#endif

#if 0
/* MK20 144pin  */
#define KINETIS_EXT_PORTA_WIDTH                 30
#define KINETIS_EXT_PORTB_WIDTH                 24
#define KINETIS_EXT_PORTC_WIDTH                 20
#define KINETIS_EXT_PORTD_WIDTH                 16
#define KINETIS_EXT_PORTE_WIDTH                 13
#endif


#if 0
/* KL25 32pin */
#define KINETIS_EXT_PORTA_WIDTH                 21
#define KINETIS_EXT_PORTD_WIDTH                 8
#endif

#if 0
/* KL25 48pin */
#define KINETIS_EXT_PORTA_WIDTH                 21
#define KINETIS_EXT_PORTD_WIDTH                 8
#endif

#if 0
/* KL25 64pin  */
#define KINETIS_EXT_PORTA_WIDTH                 21
#define KINETIS_EXT_PORTD_WIDTH                 8
#endif

/* KL25 80pin  */
#define KINETIS_EXT_PORTA_WIDTH                 21
#define KINETIS_EXT_PORTD_WIDTH                 8

#ifndef KINETIS_EXT_PORTA_WIDTH
#define KINETIS_EXT_PORTA_WIDTH                 0
#endif

#ifndef KINETIS_EXT_PORTB_WIDTH
#define KINETIS_EXT_PORTB_WIDTH                 0
#endif

#ifndef KINETIS_EXT_PORTC_WIDTH
#define KINETIS_EXT_PORTC_WIDTH                 0
#endif

#ifndef KINETIS_EXT_PORTD_WIDTH
#define KINETIS_EXT_PORTD_WIDTH                 0
#endif

#ifndef KINETIS_EXT_PORTE_WIDTH
#define KINETIS_EXT_PORTE_WIDTH                 0
#endif

#endif /* _MCUCONF_H_ */
